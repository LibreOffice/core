/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  This code is property of Serenity Systems Intl
 *  All rights reserverd.
 *
 ************************************************************************/

#define INCL_WIN
#include <svpm.h>

#include "Os2Clipboard.hxx"

// same typedefs from win32 sdk
typedef unsigned short WORD;
typedef unsigned long DWORD;

#pragma pack(push, 1)

typedef struct {
  PM_BYTE rgbBlue;
  PM_BYTE rgbGreen;
  PM_BYTE rgbRed;
  PM_BYTE rgbReserved;
} RGBQUAD, *LPRGBQUAD;

typedef struct
{
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} W32_BITMAPFILEHEADER, *PW32_BITMAPFILEHEADER;

typedef struct
{
    DWORD   biSize;
    LONG    biWidth;
    LONG    biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
} W32_BITMAPINFOHEADER, *PW32_BITMAPINFOHEADER;

#pragma pack(pop)

// store screen bitcount
LONG    lBitCountScreen;

/*
 * Convert an OOo bitmap to an OS/2 bitmap handle
 *
 * An OOo bitmap is a BITMAPFILEHEADER structure followed by a Windows DIB
 *
 * OS/2 InfoHeader is a superset of Win32 InhoHeader, so we can just copy
 * the win32 memory over the os2 memory and fix the cbFix field.
 * colortable and bitmap data share the same format.
 *
*/
HBITMAP OOoBmpToOS2Handle( Any &aAnyB)
{
    // copy bitmap to clipboard
    Sequence<sal_Int8> ByteStream;
    aAnyB >>= ByteStream;

    // get w32 file header data
    PW32_BITMAPFILEHEADER pbfh = (PW32_BITMAPFILEHEADER)ByteStream.getArray();
    // get w32 info header
    PW32_BITMAPINFOHEADER pbih = (PW32_BITMAPINFOHEADER) (pbfh+1);

    // create os2 infoheader2 (same fields of w32)
    BITMAPINFOHEADER2 bih2;
    memset( &bih2, 0, sizeof( bih2));
    memcpy( &bih2, pbih, pbih->biSize);
    bih2.cbFix = sizeof(bih2);

    // Determine size of color table
    int iNumColors, numbits=bih2.cPlanes * bih2.cBitCount;
    if (numbits != 24)
        iNumColors = bih2.cclrUsed ? bih2.cclrUsed : 2<<numbits;
    else
        iNumColors = bih2.cclrUsed;
    int iColorTableSize = iNumColors*sizeof(RGB2);

    // allocate bitmap info2 (header2+colortable)
    PBITMAPINFO2 pbi2=(PBITMAPINFO2) malloc( sizeof(BITMAPINFOHEADER2)+iColorTableSize);
    // setup header fields
    memcpy( pbi2, &bih2, sizeof(BITMAPINFOHEADER2));
    // copy color palette (follows pbih)
    memcpy( &pbi2->argbColor[0], (pbih+1), iColorTableSize);

    // get bitmap data
    PBYTE pbPelData = (PBYTE)ByteStream.getArray() + pbfh->bfOffBits;
    HPS hps = WinGetPS(HWND_DESKTOP);
    HBITMAP hbm = GpiCreateBitmap( hps, &bih2, CBM_INIT, pbPelData, pbi2);
    debug_printf( "OOoBmpToOS2Handle hbm %x\n", hbm);
    WinReleasePS(hps);

    // return handle
    return hbm;
}

/*
 * Convert an OS/2 bitmap handle to OOo bitmap
 *
 * First we need to copy the bitmap to a PS, then we can get bitmap data.
 *
*/
int OS2HandleToOOoBmp( HBITMAP hbm, Sequence< sal_Int8 >* OOoDIBStream)
{
    HAB     hab = WinQueryAnchorBlock(HWND_DESKTOP);
    HDC     hdc;
    SIZEL   sizl;
    HPS     hps;
    PM_BYTE*    pbBuffer;
    ULONG   cbBuffer;

    struct {
        BITMAPINFOHEADER2 bmp2;
        RGB2 argb2Color[0x100];
    } bm;

    if (!lBitCountScreen) {
        HPS hps = WinGetPS(HWND_DESKTOP);
        HDC hdc = GpiQueryDevice(hps);
        DevQueryCaps(hdc, CAPS_COLOR_BITCOUNT, 1L, &lBitCountScreen);
        WinReleasePS(hps);
    }

    // STEP 1: get OS/2 bitmap data and header
    // get bitmap header
    memset(&(bm.bmp2), 0, sizeof(bm.bmp2));
    bm.bmp2.cbFix = 16;
    GpiQueryBitmapInfoHeader(hbm, &bm.bmp2);

    /* Data only actually stored in clipboard quality */
    if ( lBitCountScreen < bm.bmp2.cBitCount )
        bm.bmp2.cBitCount = lBitCountScreen;

    if ( bm.bmp2.cBitCount == 16 )
        bm.bmp2.cBitCount = 24;

    if ( bm.bmp2.cPlanes != 1 ) {
        return 0;
    }

    if ( (hdc = DevOpenDC(hab, OD_MEMORY, "*", 0L, (PDEVOPENDATA) NULL, (HDC) NULL)) == (HDC) NULL ) {
        return 0;
    }

    sizl.cx = bm.bmp2.cx;
    sizl.cy = bm.bmp2.cy;
    if ( (hps = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC)) == (HPS) NULL ) {
        DevCloseDC(hdc);
        return 0;
    }
    // copy bitmap to hps
    GpiSetBitmap(hps, hbm);

    // buffer lengths
    cbBuffer = (((bm.bmp2.cBitCount * bm.bmp2.cx) + 31) / 32) * 4 * bm.bmp2.cy * bm.bmp2.cPlanes;
    pbBuffer = (PM_BYTE*) malloc( cbBuffer);
    // now get bitmap data
    GpiQueryBitmapBits(hps, 0L, (LONG) bm.bmp2.cy, pbBuffer, (BITMAPINFO2*)&bm);
    // free OS/2 resources
    GpiSetBitmap(hps, (HBITMAP) NULL);
    GpiDestroyPS(hps);
    DevCloseDC(hdc);

    // STEP 2: now convert to Win32 DIB
    // Determine size of color table
    int iNumColors, numbits=bm.bmp2.cPlanes * bm.bmp2.cBitCount;
    if (numbits != 24)
        iNumColors = bm.bmp2.cclrUsed ? bm.bmp2.cclrUsed : 2<<numbits;
    else
        iNumColors = bm.bmp2.cclrUsed;
    int iColorTableSize = iNumColors*sizeof(RGBQUAD);

    // reallocate data stream object size
    OOoDIBStream->realloc( sizeof( W32_BITMAPFILEHEADER )
                + sizeof( W32_BITMAPINFOHEADER) + iColorTableSize + cbBuffer);

    // fill w32 file header data
    PW32_BITMAPFILEHEADER pbfh = (PW32_BITMAPFILEHEADER) OOoDIBStream->getArray();
    memset( pbfh, 0, sizeof( W32_BITMAPFILEHEADER));
    pbfh->bfType = 'MB';
    pbfh->bfSize = sizeof( W32_BITMAPFILEHEADER )
                + sizeof( W32_BITMAPINFOHEADER) + iColorTableSize + cbBuffer;
    pbfh->bfOffBits = sizeof( W32_BITMAPFILEHEADER) + sizeof( W32_BITMAPINFOHEADER) + iColorTableSize;

    // fill w32 info header
    PW32_BITMAPINFOHEADER pbih = (PW32_BITMAPINFOHEADER) (pbfh+1);
    // copy header fields (only win32 ones) and fix size
    memcpy( pbih, &bm.bmp2, sizeof(W32_BITMAPINFOHEADER));
    pbih->biSize = sizeof(W32_BITMAPINFOHEADER);

    // fill color palette (follows pbih)
    memcpy( (pbih+1), &bm.argb2Color[0], iColorTableSize);

    // fill bitmap data
    memcpy( (char*) pbfh + pbfh->bfOffBits, pbBuffer, cbBuffer);

    // done
    free( pbBuffer);
    return 1;
}

#ifdef TESTBMP

#include <io.h>
#include <fcntl.h>
#include <stdio.h>

int main( void)
{
    HAB hAB = WinQueryAnchorBlock( HWND_DESKTOP );

    // query clipboard data to get mimetype
    if( WinOpenClipbrd( hAB ) )
    {
        ULONG handle = WinQueryClipbrdData( hAB, CF_BITMAP);
        if (handle) {
            Sequence< sal_Int8 > winDIBStream;
            // convert to oustring and return it
            if (OS2HandleToOOoBmp( handle, &winDIBStream) == 1) {
                printf( "Conversion ok.\n");
                int fd = open( "test.bmp", O_BINARY | O_CREAT | O_TRUNC | O_RDWR);
                printf( "writing to fd %d\n", fd);
                write( fd, winDIBStream.getArray(), winDIBStream.getLength());
                close( fd);
            } else
                printf( "failed conversion.\n");

        }
        WinCloseClipbrd( hAB);
    }
    return 0;
}

#endif //TESTBMP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
