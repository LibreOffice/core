/*************************************************************************
 *
 *  $RCSfile: fntcache.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 09:54:39 $
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
#ifndef _FNTCACHE_HXX
#define _FNTCACHE_HXX


#ifndef _SV_FONT_HXX //autogen
#include <vcl/font.hxx>
#endif
#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif
#ifndef _GDIOBJ_HXX //autogen
#include <vcl/gdiobj.hxx>
#endif

#include "swtypes.hxx"
#include "swcache.hxx"

class Printer;
class OutputDevice;
class FontMetric;
class SwFntObj;
class SwDrawTextInfo;   // DrawText
class SwScriptInfo;
class ViewShell;
class SwSubFont;

/*************************************************************************
 *                      class SwFntCache
 *************************************************************************/

class SwFntCache : public SwCache
{
public:

    inline SwFntCache() : SwCache(50,50
#ifndef PRODUCT
    , ByteString( RTL_CONSTASCII_STRINGPARAM(
                        "Globaler Font-Cache pFntCache" ))
#endif
    ) {}

    inline SwFntObj *First( ) { return (SwFntObj *)SwCache::First(); }
    inline SwFntObj *Next( SwFntObj *pFntObj)
        { return (SwFntObj *)SwCache::Next( (SwCacheObj *)pFntObj ); }
    void Flush();
};

// Font-Cache, globale Variable, in txtinit.Cxx angelegt/zerstoert
extern SwFntCache *pFntCache;
extern SwFntObj *pLastFont;
extern BYTE *pMagicNo;
extern Color *pWaveCol;

/*************************************************************************
 *                      class SwFntObj
 *************************************************************************/

class SwFntObj : public SwCacheObj
{
    friend class SwFntAccess;
    friend void _InitCore();
    friend void _FinitCore();

    Font aFont;
    Font *pScrFont;
    Font *pPrtFont;
    OutputDevice* pPrinter;
    USHORT nGuessedLeading;
    USHORT nExtLeading;
    USHORT nScrAscent;
    USHORT nPrtAscent;
    USHORT nScrHeight;
    USHORT nPrtHeight;
    USHORT nPropWidth;
    USHORT nZoom;
    BOOL bSymbol : 1;
    BOOL bPaintBlank : 1;

    static long nPixWidth;
    static MapMode *pPixMap;
    static OutputDevice *pPixOut;

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwFntObj)

    SwFntObj( const SwSubFont &rFont, const void* pOwner,
              ViewShell *pSh );

    virtual ~SwFntObj();

    inline       Font *GetScrFont()     { return pScrFont; }
    inline       Font *GetFont()        { return &aFont; }
    inline const Font *GetFont() const  { return &aFont; }

    inline USHORT GetGuessedLeading() const  { return nGuessedLeading; }
    inline USHORT GetExtLeading() const  { return nExtLeading; }

    USHORT GetFontAscent( const ViewShell *pSh, const OutputDevice& rOut );
    USHORT GetFontHeight( const ViewShell *pSh, const OutputDevice& rOut );
    USHORT GetFontLeading( const ViewShell *pSh, const OutputDevice& rOut );

    void GuessLeading( const ViewShell& rSh, const FontMetric& rMet );

    void SetDevFont( const ViewShell *pSh, OutputDevice& rOut );
    inline OutputDevice* GetPrt() const { return pPrinter; }
    inline USHORT   GetZoom() const { return nZoom; }
    inline USHORT   GetPropWidth() const { return nPropWidth; }
    inline BOOL     IsSymbol() const { return bSymbol; }

    void   DrawText( SwDrawTextInfo &rInf );
    Size  GetTextSize( SwDrawTextInfo &rInf );
    xub_StrLen GetCrsrOfst( SwDrawTextInfo &rInf );

    void CreateScrFont( const ViewShell& rSh, const OutputDevice& rOut );
    void CreatePrtFont( const OutputDevice& rOut );
};

/*************************************************************************
 *                      class SwFntAccess
 *************************************************************************/


class SwFntAccess : public SwCacheAccess
{
    ViewShell *pShell;
protected:
    virtual SwCacheObj *NewObj( );

public:
    SwFntAccess( const void * &rMagic, USHORT &rIndex, const void *pOwner,
                 ViewShell *pShell,
                 BOOL bCheck = FALSE  );
    inline SwFntObj* Get() { return (SwFntObj*) SwCacheAccess::Get(); };
};


#endif
