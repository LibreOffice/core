/*************************************************************************
 *
 *  $RCSfile: stlsheet.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:15 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------
#include "document.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"

#include "scitems.hxx"
#include <svx/boxitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/paperinf.hxx>
#include <svx/pbinitem.hxx>
#include <svx/sizeitem.hxx>
#include <svx/ulspitem.hxx>
#include <sfx2/printer.hxx>
#include <svtools/itempool.hxx>
#include <svtools/itemset.hxx>
#include <svtools/smplhint.hxx>

#include "globstr.hrc"

//------------------------------------------------------------------------

TYPEINIT1(ScStyleSheet, SfxStyleSheet);

#define TWO_CM      1134
#define HFDIST_CM   142

//========================================================================

ScStyleSheet::ScStyleSheet( const String&       rName,
                            ScStyleSheetPool&   rPool,
                            SfxStyleFamily      eFamily,
                            USHORT              nMask )

    :   SfxStyleSheet   ( rName, rPool, eFamily, nMask )
{
}

//------------------------------------------------------------------------

ScStyleSheet::ScStyleSheet( const ScStyleSheet& rStyle ) :
    SfxStyleSheet   ( rStyle )
{
}

//------------------------------------------------------------------------

__EXPORT ScStyleSheet::~ScStyleSheet()
{
}

//------------------------------------------------------------------------

BOOL __EXPORT ScStyleSheet::HasFollowSupport() const
{
    return FALSE;
}

//------------------------------------------------------------------------

BOOL __EXPORT ScStyleSheet::HasParentSupport () const
{
    BOOL bHasParentSupport = FALSE;

    switch ( GetFamily() )
    {
        case SFX_STYLE_FAMILY_PARA: bHasParentSupport = TRUE;   break;
        case SFX_STYLE_FAMILY_PAGE: bHasParentSupport = FALSE;  break;
    }

    return bHasParentSupport;
}

//------------------------------------------------------------------------

BOOL __EXPORT ScStyleSheet::SetParent( const String& rParentName )
{
    BOOL bResult = FALSE;
    String aEffName = rParentName;
    SfxStyleSheetBase* pStyle = rPool.Find( aEffName, nFamily );
    if (!pStyle)
    {
        SfxStyleSheetIterator* pIter = rPool.CreateIterator( nFamily, SFXSTYLEBIT_ALL );
        pStyle = pIter->First();
        if (pStyle)
            aEffName = pStyle->GetName();
    }

    if ( pStyle && aEffName != GetName() )
    {
        bResult = SfxStyleSheet::SetParent( aEffName );
        if (bResult)
        {
            SfxItemSet& rParentSet = pStyle->GetItemSet();
            GetItemSet().SetParent( &rParentSet );
        }
    }

    return bResult;
}

//------------------------------------------------------------------------

SfxItemSet& __EXPORT ScStyleSheet::GetItemSet()
{
    if ( !pSet )
    {
        switch ( GetFamily() )
        {
            case SFX_STYLE_FAMILY_PAGE:
                {
                    // Seitenvorlagen sollen nicht ableitbar sein,
                    // deshalb werden an dieser Stelle geeignete
                    // Werte eingestellt. (==Standard-Seitenvorlage)

                    SfxItemPool& rPool = GetPool().GetPool();
                    pSet = new SfxItemSet( rPool,
                                           ATTR_BACKGROUND, ATTR_BACKGROUND,
                                           ATTR_BORDER, ATTR_SHADOW,
                                           ATTR_LRSPACE, ATTR_PAGE_NULLVALS,
                                           0 );

                    //  Wenn gerade geladen wird, wird auch der Set hinterher aus der Datei
                    //  gefuellt, es brauchen also keine Defaults gesetzt zu werden.
                    //  GetPrinter wuerde dann auch einen neuen Printer anlegen, weil der
                    //  gespeicherte Printer noch nicht geladen ist!

                    ScDocument* pDoc = ((ScStyleSheetPool&)GetPool()).GetDocument();
                    if ( pDoc && pDoc->IsLoadingDone() )
                    {
                        // Setzen von sinnvollen Default-Werten:
                        //!!! const-Document wegcasten (im Ctor mal bei Gelegenheit aendern)
                        SfxPrinter*     pPrinter = pDoc->GetPrinter();
                        USHORT          nBinCount = pPrinter->GetPaperBinCount();
                        SvxPageItem     aPageItem( ATTR_PAGE );
                        // #50536# PaperBin auf Default lassen,
                        // nicht auf aktuelle Drucker-Einstellung umsetzen
                        SvxSizeItem     aPaperSizeItem(
                                            ATTR_PAGE_SIZE,
                                            SvxPaperInfo::GetPaperSize(pPrinter) );

                        SvxSetItem      aHFSetItem(
                                            (const SvxSetItem&)
                                            rPool.GetDefaultItem(ATTR_PAGE_HEADERSET) );

                        SfxItemSet&     rHFSet = aHFSetItem.GetItemSet();
                        SvxSizeItem     aHFSizeItem( // 0,5 cm + Abstand
                                            ATTR_PAGE_SIZE,
                                            Size( 0, (long)( 500 / HMM_PER_TWIPS ) + HFDIST_CM ) );

                        SvxULSpaceItem  aHFDistItem ( HFDIST_CM,// nUp
                                                      HFDIST_CM,// nLow
                                                      ATTR_ULSPACE );

                        SvxLRSpaceItem  aLRSpaceItem( TWO_CM,   // nLeft
                                                      TWO_CM,   // nRight
                                                      TWO_CM,   // nTLeft
                                                      0,        // nFirstLineOffset
                                                      ATTR_LRSPACE );
                        SvxULSpaceItem  aULSpaceItem( TWO_CM,   // nUp
                                                      TWO_CM,   // nLow
                                                      ATTR_ULSPACE );
                        SvxBoxInfoItem  aBoxInfoItem( ATTR_BORDER_INNER );

                        aBoxInfoItem.SetTable( FALSE );
                        aBoxInfoItem.SetDist( TRUE );
                        aBoxInfoItem.SetValid( VALID_DISTANCE, TRUE );

                        aPageItem.SetLandscape( ORIENTATION_LANDSCAPE
                                                == pPrinter->GetOrientation() );

                        rHFSet.Put( aBoxInfoItem );
                        rHFSet.Put( aHFSizeItem );
                        rHFSet.Put( aHFDistItem );
                        rHFSet.Put( SvxLRSpaceItem( 0,0,0,0, ATTR_LRSPACE ) ); // Rand auf Null setzen

                        pSet->Put( aHFSetItem, ATTR_PAGE_HEADERSET );
                        pSet->Put( aHFSetItem, ATTR_PAGE_FOOTERSET );
                        pSet->Put( aBoxInfoItem ); // PoolDefault wg. Formatvorlagen
                                                   // nicht ueberschreiben!

                        rPool.SetPoolDefaultItem( aPageItem );
                        rPool.SetPoolDefaultItem( aPaperSizeItem );
                        rPool.SetPoolDefaultItem( aLRSpaceItem );
                        rPool.SetPoolDefaultItem( aULSpaceItem );
                        rPool.SetPoolDefaultItem( SfxUInt16Item( ATTR_PAGE_SCALE, 100 ) );
                        rPool.SetPoolDefaultItem( SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, 0 ) );
                    }
                }
                break;

            case SFX_STYLE_FAMILY_PARA:
            default:
                pSet = new SfxItemSet( GetPool().GetPool(),
                                       ATTR_PATTERN_START, ATTR_PATTERN_END,
                                       0 );
                break;
        }
        bMySet = TRUE;
    }

    return *pSet;
}

//------------------------------------------------------------------------

BOOL __EXPORT ScStyleSheet::IsUsed() const
{
    if ( GetFamily() == SFX_STYLE_FAMILY_PARA )
    {
        ScDocument* pDoc = ((ScStyleSheetPool&)rPool).GetDocument();
        return pDoc && pDoc->IsStyleSheetUsed( *this );
    }
    else
        return TRUE;
}

//------------------------------------------------------------------------

void __EXPORT ScStyleSheet::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    if ( rHint.ISA(SfxSimpleHint) )
        if ( ((SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
            GetItemSet().SetParent( NULL );
}

//------------------------------------------------------------------------

//  #66123# schmutzige Tricks, um die Standard-Vorlage immer als "Standard" zu speichern,
//  obwohl der fuer den Benutzer sichtbare Name uebersetzt ist:

const String& ScStyleSheet::GetName() const
{
    const String& rBase = SfxStyleSheet::GetName();
    const String* pForceStdName = ((ScStyleSheetPool&)rPool).GetForceStdName();
    if ( pForceStdName && rBase == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
        return *pForceStdName;
    else
        return rBase;
}

const String& ScStyleSheet::GetParent() const
{
    const String& rBase = SfxStyleSheet::GetParent();
    const String* pForceStdName = ((ScStyleSheetPool&)rPool).GetForceStdName();
    if ( pForceStdName && rBase == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
        return *pForceStdName;
    else
        return rBase;
}

const String& ScStyleSheet::GetFollow() const
{
    const String& rBase = SfxStyleSheet::GetFollow();
    const String* pForceStdName = ((ScStyleSheetPool&)rPool).GetForceStdName();
    if ( pForceStdName && rBase == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
        return *pForceStdName;
    else
        return rBase;
}

//  Verhindern, dass ein Style "Standard" angelegt wird, wenn das nicht der
//  Standard-Name ist, weil sonst beim Speichern zwei Styles denselben Namen haetten
//  (Beim Laden wird der Style direkt per Make mit dem Namen erzeugt, so dass diese
//  Abfrage dann nicht gilt)
//! Wenn irgendwann aus dem Laden SetName aufgerufen wird, muss fuer das Laden ein
//! Flag gesetzt und abgefragt werden.
//! Die ganze Abfrage muss raus, wenn fuer eine neue Datei-Version die Namens-Umsetzung wegfaellt.

BOOL ScStyleSheet::SetName( const String& rNew )
{
    String aFileStdName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_STANDARD));
    if ( rNew == aFileStdName && aFileStdName != ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
        return FALSE;
    else
        return SfxStyleSheet::SetName( rNew );
}



