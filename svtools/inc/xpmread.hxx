/*************************************************************************
 *
 *  $RCSfile: xpmread.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XPMREAD_HXX
#define _XPMREAD_HXX

#ifndef _BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#ifdef _XPMPRIVATE

#define XPMTEMPBUFSIZE      0x00008000
#define XPMSTRINGBUF        0x00008000

#define XPMIDENTIFIER       0x00000001          // mnIdentifier includes on of the six phases
#define XPMDEFINITION       0x00000002          // the XPM format consists of
#define XPMVALUES           0x00000003
#define XPMCOLORS           0x00000004
#define XPMPIXELS           0x00000005
#define XPMEXTENSIONS       0x00000006
#define XPMENDEXT           0x00000007


#define XPMREMARK           0x00000001          // defines used by mnStatus
#define XPMDOUBLE           0x00000002
#define XPMSTRING           0x00000004
#define XPMFINISHED         0x00000008

#define XPMCASESENSITIVE    0x00000001
#define XPMCASENONSENSITIVE 0x00000002

// ---------
// - Enums -
// ---------

enum ReadState
{
    XPMREAD_OK,
    XPMREAD_ERROR,
    XPMREAD_NEED_MORE
};

// -------------
// - XPMReader -
// -------------

class BitmapWriteAccess;
class Graphic;

class XPMReader : public GraphicReader
{
private:

    SvStream&           mrIStm;
    Bitmap              maBmp;
    BitmapWriteAccess*  mpAcc;
    Bitmap              maMaskBmp;
    BitmapWriteAccess*  mpMaskAcc;
    long                mnLastPos;

    ULONG               mnWidth;
    ULONG               mnHeight;
    ULONG               mnColors;
    ULONG               mnCpp;                              // characters per pix
    BOOL                mbTransparent;
    BOOL                mbStatus;
    ULONG               mnStatus;
    ULONG               mnIdentifier;
    BYTE                mcThisByte;
    BYTE                mcLastByte;
    ULONG               mnTempAvail;
    BYTE*               mpTempBuf;
    BYTE*               mpTempPtr;
    BYTE*               mpFastColorTable;
    BYTE*               mpColMap;
    ULONG               mnStringSize;
    BYTE*               mpStringBuf;
    ULONG               mnParaSize;
    BYTE*               mpPara;

    BOOL                ImplGetString( void );
    BOOL                ImplGetColor( ULONG );
    BOOL                ImplGetScanLine( ULONG );
    BOOL                ImplGetColSub( BYTE* );
    BOOL                ImplGetColKey( BYTE );
    void                ImplGetRGBHex( BYTE*, ULONG );
    BOOL                ImplGetPara( ULONG numb );
    BOOL                ImplCompare( BYTE*, BYTE*, ULONG, ULONG nmode = XPMCASENONSENSITIVE );
    ULONG               ImplGetULONG( ULONG nPara );

public:
                        XPMReader( SvStream& rStm, void* pCallData );
    virtual             ~XPMReader();

    ReadState           ReadXPM( Graphic& rGraphic );
};

#endif // _XPMPRIVATE

// -------------
// - ImportXPM -
// -------------

BOOL ImportXPM( SvStream& rStream, Graphic& rGraphic, void* pCallerData );

#endif // _XPMREAD_HXX
