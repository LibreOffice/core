/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: redlnitr.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:19:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _REDLNITR_HXX
#define _REDLNITR_HXX

#include "ndhints.hxx"

#ifndef IDOCUMENTREDLINEACCESS_HXX_INCLUDED
#include <IDocumentRedlineAccess.hxx>
#endif

#include "swfont.hxx"
#ifndef _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#endif

class SwTxtNode;
class SwDoc;
class SfxItemSet;
class SwAttrHandler;

class SwExtend
{
    SwFont *pFnt;
    const SvUShorts &rArr;  // XAMA: Array of xub_StrLen
    xub_StrLen nStart;
    xub_StrLen nPos;
    xub_StrLen nEnd;
    sal_Bool _Leave( SwFont& rFnt, xub_StrLen nNew );
    sal_Bool Inside() const { return ( nPos >= nStart && nPos < nEnd ); }
    void ActualizeFont( SwFont &rFnt, xub_StrLen nAttr );
public:
    SwExtend( const SvUShorts &rA, xub_StrLen nSt ) : pFnt(0), rArr( rA ),
        nStart( nSt ), nPos( STRING_LEN ), nEnd( nStart + rA.Count() ) {}
    ~SwExtend() { delete pFnt; }
    sal_Bool IsOn() const { return pFnt != 0; }
    void Reset() { if( pFnt ) { delete pFnt; pFnt = NULL; } nPos = STRING_LEN; }
    sal_Bool Leave( SwFont& rFnt, xub_StrLen nNew )
        { if( pFnt ) return _Leave( rFnt, nNew ); return sal_False; }
    short Enter( SwFont& rFnt, xub_StrLen nNew );
    xub_StrLen Next( xub_StrLen nNext );
    SwFont* GetFont()  { return pFnt; }
    void UpdateFont( SwFont &rFnt ) { ActualizeFont( rFnt, rArr[ nPos - nStart ] ); }
};

class SwRedlineItr
{
    SwpHtStart_SAR aHints;
    const SwDoc& rDoc;
    const SwTxtNode& rNd;
    SwAttrHandler& rAttrHandler;
    SfxItemSet *pSet;
    SwExtend *pExt;
    ULONG nNdIdx;
    xub_StrLen nFirst;
    xub_StrLen nAct;
    xub_StrLen nStart;
    xub_StrLen nEnd;
    sal_Bool bOn;
    sal_Bool bShow;

    void _Clear( SwFont* pFnt );
    sal_Bool _ChkSpecialUnderline() const;
    void FillHints( MSHORT nAuthor, RedlineType_t eType );
    short _Seek( SwFont& rFnt, xub_StrLen nNew, xub_StrLen nOld );
    xub_StrLen _GetNextRedln( xub_StrLen nNext );
    inline short EnterExtend( SwFont& rFnt, xub_StrLen nNew )
        { if( pExt ) return pExt->Enter( rFnt, nNew ); return 0; }
    inline xub_StrLen NextExtend( xub_StrLen nNext )
        { if( pExt ) return pExt->Next( nNext ); return nNext; }
public:
    SwRedlineItr( const SwTxtNode& rTxtNd, SwFont& rFnt, SwAttrHandler& rAH,
        xub_StrLen nRedlPos, sal_Bool bShw, const SvUShorts *pArr = 0,
        xub_StrLen nStart = STRING_LEN );
    ~SwRedlineItr();
    inline sal_Bool IsOn() const { return bOn || ( pExt && pExt->IsOn() ); }
    inline void Clear( SwFont* pFnt ) { if( bOn ) _Clear( pFnt ); }
    void ChangeTxtAttr( SwFont* pFnt, SwTxtAttr &rHt, sal_Bool bChg );
    inline short Seek( SwFont& rFnt, xub_StrLen nNew, xub_StrLen nOld )
        { if( bShow || pExt ) return _Seek( rFnt, nNew, nOld ); return 0; }
    inline void Reset() { if( nAct != nFirst ) nAct = STRING_LEN;
                          if( pExt ) pExt->Reset(); }
    inline xub_StrLen GetNextRedln( xub_StrLen nNext )
        { if( bShow || pExt ) return _GetNextRedln( nNext ); return nNext; }
    inline sal_Bool ChkSpecialUnderline() const
        { if ( IsOn() ) return _ChkSpecialUnderline(); return sal_False; }
    sal_Bool CheckLine( xub_StrLen nChkStart, xub_StrLen nChkEnd );
    inline sal_Bool LeaveExtend( SwFont& rFnt, xub_StrLen nNew )
        { return pExt->Leave(rFnt, nNew ); }
    inline sal_Bool ExtOn() { if( pExt ) return pExt->IsOn(); return sal_False; }
    inline void UpdateExtFont( SwFont &rFnt ) {
        ASSERT( ExtOn(), "UpdateExtFont without ExtOn" )
        pExt->UpdateFont( rFnt ); }
};


#endif

