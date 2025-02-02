// ==========================================================
// Bitmap conversion routines
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
// - Herv� Drolon (drolon@infonie.fr)
// - Jani Kajala (janik@remedy.fi)
//
// This file is part of FreeImage 3
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================

#include "FreeImage.h"
#include "Utilities.h"

// ----------------------------------------------------------

#define RGB555(b, g, r) ((((b) >> 3) << FI16_555_BLUE_SHIFT) | (((g) >> 3) << FI16_555_GREEN_SHIFT) | (((r) >> 3) << FI16_555_RED_SHIFT))

// ----------------------------------------------------------
//  internal conversions X to 16 bits (555)
// ----------------------------------------------------------

void DLL_CALLCONV
FreeImage_ConvertLine1To16_555(uint8_t *target, uint8_t *source, int width_in_pixels, FIRGBA8 *palette) {
	uint16_t *new_bits = (uint16_t *)target;

	for (int cols = 0; cols < width_in_pixels; cols++) {
		int index = (source[cols >> 3] & (0x80 >> (cols & 0x07))) != 0 ? 1 : 0;

		new_bits[cols] = RGB555(palette[index].blue, palette[index].green, palette[index].red);
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine4To16_555(uint8_t *target, uint8_t *source, int width_in_pixels, FIRGBA8 *palette) {
	uint16_t *new_bits = (uint16_t *)target;
	FIBOOL lonibble = FALSE;
	int x = 0;

	for (int cols = 0; cols < width_in_pixels; cols++) {
		FIRGBA8 *grab_palette;

		if (lonibble) {
			grab_palette = palette + LOWNIBBLE(source[x++]);
		} else {
			grab_palette = palette + (HINIBBLE(source[x]) >> 4);								
		}

		new_bits[cols] = RGB555(grab_palette->blue, grab_palette->green, grab_palette->red);

		lonibble = !lonibble;
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine8To16_555(uint8_t *target, uint8_t *source, int width_in_pixels, FIRGBA8 *palette) {
	uint16_t *new_bits = (uint16_t *)target;

	for (int cols = 0; cols < width_in_pixels; cols++) {
		FIRGBA8 *grab_palette = palette + source[cols];

		new_bits[cols] = RGB555(grab_palette->blue, grab_palette->green, grab_palette->red);
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine16_565_To16_555(uint8_t *target, uint8_t *source, int width_in_pixels) {
	uint16_t *src_bits = (uint16_t *)source;
	uint16_t *new_bits = (uint16_t *)target;

	for (int cols = 0; cols < width_in_pixels; cols++) {
		new_bits[cols] = RGB555((((src_bits[cols] & FI16_565_BLUE_MASK) >> FI16_565_BLUE_SHIFT) * 0xFF) / 0x1F,
			                    (((src_bits[cols] & FI16_565_GREEN_MASK) >> FI16_565_GREEN_SHIFT) * 0xFF) / 0x3F,
								(((src_bits[cols] & FI16_565_RED_MASK) >> FI16_565_RED_SHIFT) * 0xFF) / 0x1F);
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine24To16_555(uint8_t *target, uint8_t *source, int width_in_pixels) {
	uint16_t *new_bits = (uint16_t *)target;

	for (int cols = 0; cols < width_in_pixels; cols++) {
		new_bits[cols] = RGB555(source[FI_RGBA_BLUE], source[FI_RGBA_GREEN], source[FI_RGBA_RED]);

		source += 3;
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine32To16_555(uint8_t *target, uint8_t *source, int width_in_pixels) {
	uint16_t *new_bits = (uint16_t *)target;

	for (int cols = 0; cols < width_in_pixels; cols++) {
		new_bits[cols] = RGB555(source[FI_RGBA_BLUE], source[FI_RGBA_GREEN], source[FI_RGBA_RED]);

		source += 4;
	}
}

// ----------------------------------------------------------
//   smart convert X to 16 bits
// ----------------------------------------------------------

FIBITMAP * DLL_CALLCONV
FreeImage_ConvertTo16Bits555(FIBITMAP *dib) {
	if(!FreeImage_HasPixels(dib) || (FreeImage_GetImageType(dib) != FIT_BITMAP)) return NULL;

	const int width = FreeImage_GetWidth(dib);
	const int height = FreeImage_GetHeight(dib);
	const int bpp = FreeImage_GetBPP(dib);

	if(bpp == 16) {
		if ((FreeImage_GetRedMask(dib) == FI16_565_RED_MASK) && (FreeImage_GetGreenMask(dib) == FI16_565_GREEN_MASK) && (FreeImage_GetBlueMask(dib) == FI16_565_BLUE_MASK)) {
			// RGB 565
			FIBITMAP *new_dib = FreeImage_Allocate(width, height, 16, FI16_555_RED_MASK, FI16_555_GREEN_MASK, FI16_555_BLUE_MASK);
			if(new_dib == NULL) {
				return NULL;
			}
			for (int rows = 0; rows < height; rows++) {
				FreeImage_ConvertLine16_565_To16_555(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width);
			}

			// copy metadata from src to dst
			FreeImage_CloneMetadata(new_dib, dib);

			return new_dib;
		} else {
			// RGB 555
			return FreeImage_Clone(dib);
		}
	}
	else {
		// other bpp cases => convert to RGB 555
		FIBITMAP *new_dib = FreeImage_Allocate(width, height, 16, FI16_555_RED_MASK, FI16_555_GREEN_MASK, FI16_555_BLUE_MASK);
		if(new_dib == NULL) {
			return NULL;
		}

		// copy metadata from src to dst
		FreeImage_CloneMetadata(new_dib, dib);

		switch (bpp) {
			case 1 :
			{
				for (int rows = 0; rows < height; rows++) {
					FreeImage_ConvertLine1To16_555(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib));
				}

				return new_dib;
			}

			case 4 :
			{
				for (int rows = 0; rows < height; rows++) {
					FreeImage_ConvertLine4To16_555(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib));
				}

				return new_dib;
			}

			case 8 :
			{
				for (int rows = 0; rows < height; rows++) {
					FreeImage_ConvertLine8To16_555(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib));
				}

				return new_dib;
			}

			case 24 :
			{
				for (int rows = 0; rows < height; rows++) {
					FreeImage_ConvertLine24To16_555(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width);
				}

				return new_dib;
			}

			case 32 :
			{
				for (int rows = 0; rows < height; rows++) {
					FreeImage_ConvertLine32To16_555(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width);
				}

				return new_dib;
			}

			default :
				// unreachable code ...
				FreeImage_Unload(new_dib);
				break;

		}
	}

	return NULL;
}
