/*************************************************************************
 *
 *  $RCSfile: outliner.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: mt $ $Date: 2000-11-24 11:30:26 $
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

#include <outl_pch.hxx>

#pragma hdrstop

#include <math.h>

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif

#define _OUTLINER_CXX
#include <outliner.hxx>
#include <paralist.hxx>
#include <outlobj.hxx>
#include <outleeng.hxx>
#include <outlundo.hxx>
#include <eeitem.hxx>
#include <editstat.hxx>

#ifndef _EDITOBJ_HXX //autogen
#include <editobj.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif

#ifndef _SV_METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif

#ifndef _SVX_NUMITEM_HXX
#include <numitem.hxx>
#endif

#ifndef _SVX_ADJITEM_HXX
#include <adjitem.hxx>
#endif

#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif

#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif

#ifndef _SVX_SVXFONT_HXX
#include <svxfont.hxx>
#endif

#ifndef _SVX_BRSHITEM_HXX
#include <brshitem.hxx>
#endif

#define DEFAULT_SCALE   75

static const USHORT nDefStyles = 3; // Sonderbehandlung fuer die ersten 3 Ebenen
static const USHORT nDefBulletIndent = 800;
static const USHORT nDefBulletWidth = 700;
static const USHORT pDefBulletIndents[nDefStyles]=  { 1400, 800, 800 };
static const USHORT pDefBulletWidths[nDefStyles] =  { 1000, 850, 700 };

USHORT lcl_ImplGetDefBulletWidth( USHORT nDepth )
{
    return ( nDepth < nDefStyles ) ? pDefBulletWidths[nDepth] : nDefBulletWidth;
}

USHORT lcl_ImplGetDefBulletIndent( USHORT nDepth )
{
    USHORT nI = 0;
    for ( USHORT n = 0; n <= nDepth; n++ )
        nI += ( n < nDefStyles ) ? pDefBulletIndents[n] : nDefBulletIndent;
    return nI;
}

SvxLRSpaceItem lcl_ImplGetDefLRSpaceItem( USHORT nDepth, MapUnit eMapUnit )
{
    SvxLRSpaceItem aLRSpace( EE_PARA_OUTLLRSPACE );

    long nWidth = OutputDevice::LogicToLogic( (long) lcl_ImplGetDefBulletWidth( nDepth ), MAP_100TH_MM, eMapUnit );
    long nIndent = OutputDevice::LogicToLogic( (long) lcl_ImplGetDefBulletIndent( nDepth ), MAP_100TH_MM, eMapUnit );
    aLRSpace.SetTxtLeft( (USHORT) nIndent );
    aLRSpace.SetTxtFirstLineOfst( - (short)nWidth );

    return aLRSpace;
}

// ----------------------------------------------------------------------
// Outliner
// ----------------------------------------------------------------------
DBG_NAME(Outliner);

Paragraph* Outliner::Insert(const XubString& rText, ULONG nAbsPos, USHORT nDepth)
{
    DBG_CHKTHIS(Outliner,0);
    DBG_ASSERT(pParaList->GetParagraphCount(),"Insert:No Paras");

    Paragraph* pPara;

    if( nDepth < nMinDepth )
        nDepth = nMinDepth;
    else if( nDepth > nMaxDepth )
        nDepth = nMaxDepth;

    ULONG nParagraphCount = pParaList->GetParagraphCount();
    if( nAbsPos > nParagraphCount )
        nAbsPos = nParagraphCount;

    if( bFirstParaIsEmpty )
    {
        pPara = pParaList->GetParagraph( 0 );
        if( pPara->GetDepth() != nDepth )
        {
            nDepthChangedHdlPrevDepth = pPara->GetDepth();
            pPara->SetDepth( nDepth );
            pHdlParagraph = pPara;
            DepthChangedHdl();
        }
        pPara->nFlags |= PARAFLAG_HOLDDEPTH;
        SetText( rText, pPara );
    }
    else
    {
        BOOL bUpdate = pEditEngine->GetUpdateMode();
        pEditEngine->SetUpdateMode( FALSE );
        bBlockInsCallback = TRUE;
        pPara = new Paragraph( nDepth );
        pParaList->Insert( pPara, nAbsPos );
        pEditEngine->InsertParagraph( (USHORT)nAbsPos, String() );
        DBG_ASSERT(pPara==pParaList->GetParagraph(nAbsPos),"Insert:Failed");
        ImplInitDepth( (USHORT)nAbsPos, nDepth, FALSE );
        pHdlParagraph = pPara;
        ParagraphInsertedHdl();
        pPara->nFlags |= PARAFLAG_HOLDDEPTH;
        SetText( rText, pPara );
        bBlockInsCallback = FALSE;
        pEditEngine->SetUpdateMode( bUpdate );
    }
    bFirstParaIsEmpty = FALSE;
    DBG_ASSERT(pEditEngine->GetParagraphCount()==pParaList->GetParagraphCount(),"SetText failed")
    return pPara;
}


void Outliner::ParagraphInserted( USHORT nPara )
{
    DBG_CHKTHIS(Outliner,0);

    if ( bBlockInsCallback )
        return;

    if( bPasting || pEditEngine->IsInUndo() )
    {
        Paragraph* pPara = new Paragraph( 0xffff );
        pParaList->Insert( pPara, nPara );
        if( pEditEngine->IsInUndo() )
        {
            pPara->nFlags = PARAFLAG_SETBULLETTEXT;
            pPara->bVisible = TRUE;
            const SfxUInt16Item& rLevel = (const SfxUInt16Item&) pEditEngine->GetParaAttrib( nPara, EE_PARA_OUTLLEVEL );
            pPara->SetDepth( rLevel.GetValue() );
        }
    }
    else
    {
        USHORT nDepth = 0;
        if ( nPara )
            nDepth = pParaList->GetParagraph( nPara-1 )->GetDepth();

        Paragraph* pPara = new Paragraph( nDepth );
        pParaList->Insert( pPara, nPara );

        if( !pEditEngine->IsInUndo() )
        {
            ImplCalcBulletText( nPara, TRUE, FALSE );
            pHdlParagraph = pPara;
            ParagraphInsertedHdl();
        }
    }
}

void Outliner::ParagraphDeleted( USHORT nPara )
{
    DBG_CHKTHIS(Outliner,0);

    if ( bBlockInsCallback || ( nPara == EE_PARA_ALL ) )
        return;

    Paragraph* pPara = pParaList->GetParagraph( nPara );
    USHORT nDepth = pPara->GetDepth();

    if( !pEditEngine->IsInUndo() )
    {
        pHdlParagraph = pPara;
        ParagraphRemovingHdl();
    }

    pParaList->Remove( nPara );
    delete pPara;

    if( !pEditEngine->IsInUndo() )
    {
        pPara = pParaList->GetParagraph( nPara );
        if ( pPara && ( pPara->GetDepth() > nDepth ) )
        {
            ImplCalcBulletText( nPara, TRUE, FALSE );
            // naechsten auf gleicher Ebene suchen...
            while ( pPara && pPara->GetDepth() > nDepth )
                pPara = pParaList->GetParagraph( ++nPara );
        }

        if ( pPara && ( pPara->GetDepth() == nDepth ) )
            ImplCalcBulletText( nPara, TRUE, FALSE );
    }
}

void Outliner::Init( USHORT nMode )
{
    nOutlinerMode = nMode;

    Clear();

    ULONG nCtrl = pEditEngine->GetControlWord();
    nCtrl &= ~(EE_CNTRL_OUTLINER|EE_CNTRL_OUTLINER2);

    switch ( ImplGetOutlinerMode() )
    {
        case OUTLINERMODE_TEXTOBJECT:
        {
            SetMinDepth( 0 );
            SetMaxDepth( 9 );
        }
        break;
        case OUTLINERMODE_TITLEOBJECT:
        {
            SetMinDepth( 0 );
            SetMaxDepth( 9 );
        }
        break;
        case OUTLINERMODE_OUTLINEOBJECT:
        {
            SetMinDepth( 1 );
            SetMaxDepth( 9 );
            nCtrl |= EE_CNTRL_OUTLINER2;
        }
        break;
        case OUTLINERMODE_OUTLINEVIEW:
        {
            SetMinDepth( 0 );
            SetMaxDepth( 9 );
            nCtrl |= EE_CNTRL_OUTLINER;
        }
        break;
        default: DBG_ERROR( "Outliner::Init - Invalid Mode!" );
    }

    pEditEngine->SetControlWord( nCtrl );

    ImplInitDepth( 0, GetMinDepth(), FALSE );
}

void Outliner::SetMinDepth( USHORT nDepth, BOOL bCheckParagraphs )
{
    if( nMinDepth != nDepth )
    {
        nMinDepth = nDepth;

        if( bCheckParagraphs && nMinDepth )
        {
            // Prueft nur dass kein Absatz kleiner ist,
            // es werden keine Ebenen proportional verschoben!
            USHORT nParagraphs = (USHORT)pParaList->GetParagraphCount();
            for ( USHORT nPara = 0; nPara < nParagraphs; nPara++ )
            {
                Paragraph* pPara = pParaList->GetParagraph( nPara );
                if( pPara->GetDepth() < nMinDepth )
                {
                    SetDepth( pPara, nMinDepth );
                }
            }
        }
    }
}

void Outliner::SetMaxDepth( USHORT nDepth, BOOL bCheckParagraphs )
{
    if( nMaxDepth != nDepth )
    {
        nMaxDepth = nDepth;

        if( bCheckParagraphs )
        {
            USHORT nParagraphs = (USHORT)pParaList->GetParagraphCount();
            for ( USHORT nPara = 0; nPara < nParagraphs; nPara++ )
            {
                Paragraph* pPara = pParaList->GetParagraph( nPara );
                if( pPara->GetDepth() > nMaxDepth )
                {
                    SetDepth( pPara, nMaxDepth );
                }
            }
        }
    }
}

USHORT Outliner::GetDepth( USHORT nPara ) const
{
    Paragraph* pPara = pParaList->GetParagraph( nPara );
    DBG_ASSERT( pPara, "Outliner::GetDepth - Paragraph not found!" );
    return pPara ? pPara->GetDepth() : 0;
}

void Outliner::SetDepth( Paragraph* pPara, USHORT nNewDepth )
{
    DBG_CHKTHIS(Outliner,0);

    if ( nNewDepth < nMinDepth )
        nNewDepth = nMinDepth;
    else if ( nNewDepth > nMaxDepth )
        nNewDepth = nMaxDepth;

    if ( nNewDepth != pPara->GetDepth() )
    {
        nDepthChangedHdlPrevDepth = pPara->GetDepth();
        pHdlParagraph = pPara;

        ImplInitDepth( (USHORT) GetAbsPos( pPara ), nNewDepth, TRUE );

        DepthChangedHdl();
    }
}

OutlinerParaObject* Outliner::CreateParaObject( USHORT nStartPara, USHORT nCount ) const
{
    DBG_CHKTHIS(Outliner,0);

    if ( ( nStartPara + nCount ) > pParaList->GetParagraphCount() )
        nCount = pParaList->GetParagraphCount() - nStartPara;

    if( !nCount )
        return NULL;

    OutlinerParaObject* pPObj = new OutlinerParaObject( nCount );
    pPObj->pText = pEditEngine->CreateTextObject( nStartPara, nCount );
    pPObj->SetOutlinerMode( GetMode() );
    pPObj->bIsEditDoc = ( ImplGetOutlinerMode() == OUTLINERMODE_TEXTOBJECT ) ? TRUE : FALSE;

    USHORT nLastPara = nStartPara + nCount - 1;
    for ( USHORT nPara = nStartPara; nPara <= nLastPara; nPara++ )
        pPObj->pDepthArr[ nPara-nStartPara] = GetDepth( nPara );

    return pPObj;
}

void Outliner::SetText( const XubString& rText, Paragraph* pPara )
{
    DBG_CHKTHIS(Outliner,0);
    DBG_ASSERT(pPara,"SetText:No Para")

    BOOL bUpdate = pEditEngine->GetUpdateMode();
    pEditEngine->SetUpdateMode( FALSE );
    bBlockInsCallback = TRUE;

    USHORT nPara = (USHORT)pParaList->GetAbsPos( pPara );

    if( !rText.Len() )
    {
        pEditEngine->SetText( nPara, rText );
        ImplInitDepth( nPara, pPara->GetDepth(), FALSE );
    }
    else
    {
        XubString aText( rText );
        aText.ConvertLineEnd( LINEEND_LF );

        if( aText.GetChar( aText.Len()-1 ) == '\x0A' )
            aText.Erase( aText.Len()-1, 1 ); // letzten Umbruch loeschen

        USHORT nCount = aText.GetTokenCount( '\x0A' );
        USHORT nPos = 0;
        USHORT nInsPos = nPara+1;
        while( nCount > nPos )
        {
            XubString aStr = aText.GetToken( nPos, '\x0A' );

            USHORT nCurDepth;
            if( nPos )
            {
                pPara = new Paragraph( 0 );
                nCurDepth = 0;
            }
            else
                nCurDepth = pPara->GetDepth();

            // Im Outliner-Modus die Tabulatoren filtern und die
            // Einrueckung ueber ein LRSpaceItem einstellen
            // Im EditEngine-Modus ueber Maltes Tabulatoren einruecken
            if( ( ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEOBJECT ) ||
                ( ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEVIEW ) )
            {
                // Tabs raus
                USHORT nTabs = 0;
                while ( ( nTabs < aStr.Len() ) && ( aStr.GetChar( nTabs ) == '\t' ) )
                    nTabs++;
                if ( nTabs )
                    aStr.Erase( 0, nTabs );

                // Tiefe beibehalten ?  (siehe Outliner::Insert)
                if( !(pPara->nFlags & PARAFLAG_HOLDDEPTH) )
                {
                    nCurDepth = nTabs;
                    if( nCurDepth < nMinDepth )
                        nCurDepth = nMinDepth;
                    pPara->SetDepth( nCurDepth );
                    pPara->nFlags &= (~PARAFLAG_HOLDDEPTH);
                }
            }
            if( nPos ) // nicht mit dem ersten Absatz
            {
                pParaList->Insert( pPara, nInsPos );
                pEditEngine->InsertParagraph( nInsPos, aStr );
                pHdlParagraph = pPara;
                ParagraphInsertedHdl();
            }
            else
            {
                nInsPos--;
                pEditEngine->SetText( nInsPos, aStr );
            }
            ImplInitDepth( nInsPos, nCurDepth, FALSE );
            nInsPos++;
            nPos++;
        }
    }

    DBG_ASSERT(pParaList->GetParagraphCount()==pEditEngine->GetParagraphCount(),"SetText failed!")
    bBlockInsCallback = FALSE;
    bFirstParaIsEmpty = FALSE;
    pEditEngine->SetUpdateMode( bUpdate );
}

// pView == 0 -> Tabulatoren nicht beachten

BOOL Outliner::ImpConvertEdtToOut(Paragraph* pPara,ULONG nPara,EditView* pView)
{
    DBG_CHKTHIS(Outliner,0);
    DBG_ASSERT(pPara,"FilterTabs:No Para");

    BOOL bConverted = FALSE;
    USHORT nTabs = 0;
    ESelection aDelSel;

    const SfxItemSet& rAttrs = pEditEngine->GetParaAttribs( (USHORT)nPara );
    BOOL bAlreadyOutliner = rAttrs.GetItemState( EE_PARA_OUTLLRSPACE ) == SFX_ITEM_ON ? TRUE : FALSE;

    XubString aName;
    XubString aHeading_US( RTL_CONSTASCII_USTRINGPARAM( "Heading" ) );
    XubString aHeading_GERM( RTL_CONSTASCII_USTRINGPARAM( "berschrift" ) );
    XubString aNumber_GERM( RTL_CONSTASCII_USTRINGPARAM( "Numerierung" ) );

    XubString aStr( pEditEngine->GetText( (USHORT)nPara ) );
    xub_Unicode* pPtr = (xub_Unicode*)aStr.GetBuffer();

    USHORT nHeadingNumberStart = 0;
    USHORT nNumberingNumberStart = 0;
    SfxStyleSheet* pStyle= pEditEngine->GetStyleSheet( (USHORT)nPara );
    if( pStyle )
    {
        aName = pStyle->GetName();
        USHORT nSearch;
        if ( ( nSearch = aName.Search( aHeading_US ) ) != STRING_NOTFOUND )
            nHeadingNumberStart = nSearch + aHeading_US.Len();
        else if ( ( nSearch = aName.Search( aHeading_GERM ) ) != STRING_NOTFOUND )
            nHeadingNumberStart = nSearch + aHeading_GERM.Len();
        else if ( ( nSearch = aName.Search( aNumber_GERM ) ) != STRING_NOTFOUND )
            nNumberingNumberStart = nSearch + aNumber_GERM.Len();
    }

    if ( nHeadingNumberStart || nNumberingNumberStart )
    {
        // PowerPoint-Import ?
        if( nHeadingNumberStart && ( aStr.Len() >= 2 ) &&
                ( pPtr[0] != '\t' ) && ( pPtr[1] == '\t' ) )
        {
            // Bullet & Tab raus
            aDelSel = ESelection( (USHORT)nPara, 0, (USHORT)nPara, 2 );
        }

        USHORT nPos = nHeadingNumberStart ? nHeadingNumberStart : nNumberingNumberStart;
        String aLevel = aName.Copy( nPos );
        aLevel.EraseLeadingChars( ' ' );
        nTabs = aLevel.ToInt32();
        if( nTabs )
            nTabs--; // ebene 0 = "heading 1"
        bConverted = TRUE;
    }
    else
    {
        //  Fuehrende Tabulatoren filtern
        while( *pPtr == '\t' )
        {
            pPtr++;
            nTabs++;
        }
        // Tabulatoren aus dem Text entfernen
        if( nTabs )
            aDelSel = ESelection( (USHORT)nPara, 0, (USHORT)nPara, nTabs );

        //  LRSpaceItem aufaddieren
        const SvxLRSpaceItem& rIndent=(const SvxLRSpaceItem&)rAttrs.Get( bAlreadyOutliner ? EE_PARA_OUTLLRSPACE : EE_PARA_LRSPACE );
        USHORT nIndent = rIndent.GetTxtLeft();
        if( nIndent )
        {
            USHORT nOffs = nIndent / (USHORT)pEditEngine->GetDefTab();
            if( !nOffs )
                nOffs = 1;  // immer mind. 1 Tab
            nTabs += nOffs;
        }
    }

    if ( aDelSel.HasRange() )
    {
        if ( pView )
        {
            pView->SetSelection( aDelSel );
            pView->DeleteSelected();
        }
        else
            pEditEngine->QuickDelete( aDelSel );
    }

    // MT: OutlLevel wird beim RTF-Import von der EditEngine eingestellt,
    // weil String-Vergleich vom Vorlagen-Namen unbrauchbar
    const SfxUInt16Item& rLevel = (const SfxUInt16Item&) pEditEngine->GetParaAttrib( nPara, EE_PARA_OUTLLEVEL );
    USHORT nOutlLevel = rLevel.GetValue();

    if ( !nPara )
        nOutlLevel = nMinDepth;
    else if( nOutlLevel < nMinDepth )
        nOutlLevel = nMinDepth;
    else if( nOutlLevel >= nMaxDepth )
        nOutlLevel = nMaxDepth-1;

    ImplInitDepth( nPara, nOutlLevel, FALSE );

    return bConverted;
}

void Outliner::SetText( const OutlinerParaObject& rPObj )
{
    DBG_CHKTHIS(Outliner,0);

    BOOL bUpdate = pEditEngine->GetUpdateMode();
    pEditEngine->SetUpdateMode( FALSE );

    BOOL bUndo = pEditEngine->IsUndoEnabled();
    EnableUndo( FALSE );

    Init( rPObj.GetOutlinerMode() );

    bBlockInsCallback = TRUE;
    pEditEngine->SetText( *(rPObj.pText) );
    bBlockInsCallback = FALSE;
    bFirstParaIsEmpty = FALSE;

    pParaList->Clear( TRUE );
    for( USHORT nCurPara = 0; nCurPara < rPObj.nCount; nCurPara++ )
    {
        Paragraph* pPara = new Paragraph( rPObj.pDepthArr[ nCurPara ] );
        pParaList->Insert( pPara, LIST_APPEND );
        ImplCheckNumBulletItem( nCurPara );
    }

    ImplCheckParagraphs( 0, (USHORT) (pParaList->GetParagraphCount()-1) );

    EnableUndo( bUndo );
    pEditEngine->SetUpdateMode( bUpdate );

    DBG_ASSERT( pParaList->GetParagraphCount()==rPObj.Count(),"SetText failed")
    DBG_ASSERT( pEditEngine->GetParagraphCount()==rPObj.Count(),"SetText failed")
}

void Outliner::AddText( const OutlinerParaObject& rPObj )
{
    DBG_CHKTHIS(Outliner,0);
    Paragraph* pPara;

    ULONG nCount = rPObj.Count();

    BOOL bUpdate = pEditEngine->GetUpdateMode();
    pEditEngine->SetUpdateMode( FALSE );

    bBlockInsCallback = TRUE;
    ULONG nPara;
    if( bFirstParaIsEmpty )
    {
        pParaList->Clear( TRUE );
        pEditEngine->SetText( *(rPObj.pText) );
        nPara = 0;
    }
    else
    {
        nPara = pParaList->GetParagraphCount();
        pEditEngine->InsertParagraph( EE_PARA_APPEND, *(rPObj.pText) );
    }
    bBlockInsCallback = FALSE;
    bFirstParaIsEmpty = FALSE;

    for( USHORT n = 0; n < rPObj.nCount; n++ )
    {
        pPara = new Paragraph( rPObj.pDepthArr[ n ] );
        pParaList->Insert( pPara, LIST_APPEND );
        USHORT nP = nPara+n;
        DBG_ASSERT(pParaList->GetAbsPos(pPara)==nP,"AddText:Out of sync")
        ImplInitDepth( nP, pPara->GetDepth(), FALSE );
    }
    DBG_ASSERT( pEditEngine->GetParagraphCount()==pParaList->GetParagraphCount(), "SetText: OutOfSync" );

    ImplCheckParagraphs( (USHORT)nPara, (USHORT) (pParaList->GetParagraphCount()-1) );

    pEditEngine->SetUpdateMode( bUpdate );
}

void __EXPORT Outliner::FieldClicked( const SvxFieldItem& rField, USHORT nPara, USHORT nPos )
{
    DBG_CHKTHIS(Outliner,0);

    if ( aFieldClickedHdl.IsSet() )
    {
        EditFieldInfo aFldInfo( this, rField, nPara, nPos );
        aFldInfo.SetSimpleClick( TRUE );
        aFieldClickedHdl.Call( &aFldInfo );
    }
}


void __EXPORT Outliner::FieldSelected( const SvxFieldItem& rField, USHORT nPara, USHORT nPos )
{
    DBG_CHKTHIS(Outliner,0);
    if ( !aFieldClickedHdl.IsSet() )
        return;

    EditFieldInfo aFldInfo( this, rField, nPara, nPos );
    aFldInfo.SetSimpleClick( FALSE );
    aFieldClickedHdl.Call( &aFldInfo );
}


XubString __EXPORT Outliner::CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    DBG_CHKTHIS(Outliner,0);
    if ( !aCalcFieldValueHdl.IsSet() )
        return String( ' ' );

    EditFieldInfo aFldInfo( this, rField, nPara, nPos );
    // Die FldColor ist mit COL_LIGHTGRAY voreingestellt.
    if ( rpFldColor )
        aFldInfo.SetFldColor( *rpFldColor );

    aCalcFieldValueHdl.Call( &aFldInfo );
    if ( aFldInfo.GetTxtColor() )
    {
        delete rpTxtColor;
        rpTxtColor = new Color( *aFldInfo.GetTxtColor() );
    }

    delete rpFldColor;
    rpFldColor = aFldInfo.GetFldColor() ? new Color( *aFldInfo.GetFldColor() ) : 0;

    return aFldInfo.GetRepresentation();
}

void Outliner::SetStyleSheet( ULONG nPara, SfxStyleSheet* pStyle )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetStyleSheet( (USHORT)nPara, pStyle );

    Paragraph* pPara = pParaList->GetParagraph( nPara );
    pPara->nFlags |= PARAFLAG_SETBULLETTEXT;

    ImplCheckNumBulletItem( (USHORT) nPara );
}

void Outliner::ImpDeleteBulletArray( XubString** pTexts, ULONG nCount )
{
    for( ULONG nAbsPos = 0; nAbsPos < nCount; nAbsPos++ )
        delete pTexts[ nAbsPos ];
    delete pTexts;
}

void Outliner::SetVisible( Paragraph* pPara, BOOL bVisible )
{
    DBG_CHKTHIS(Outliner,0);
    DBG_ASSERT( pPara, "SetVisible: pPara = NULL" );

    pPara->bVisible = bVisible;
    ULONG nPara = pParaList->GetAbsPos( pPara );
    pEditEngine->ShowParagraph( (USHORT)nPara, bVisible );
}

void Outliner::ImplCheckNumBulletItem( USHORT nPara )
{
    Paragraph* pPara = pParaList->GetParagraph( nPara );

    // Wenn es ein SvxNumBulletItem gibt, ueberschreibt dieses die
    // Einstellungen von BulletItem und LRSpaceItem.
    const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&) pEditEngine->GetParaAttrib( nPara, EE_PARA_NUMBULLET );
    if ( rNumBullet.GetNumRule()->GetLevelCount() > pPara->GetDepth() )
    {
        SvxLRSpaceItem aNewLRSpace( EE_PARA_LRSPACE );
        CreateLRSpaceItem( rNumBullet, pPara->GetDepth(), aNewLRSpace );
        if ( pEditEngine->HasParaAttrib( nPara, EE_PARA_LRSPACE ) )
        {
            const SvxLRSpaceItem& rOldLRSpace = (const SvxLRSpaceItem&)pEditEngine->GetParaAttrib( nPara, EE_PARA_LRSPACE );
            aNewLRSpace.SetRight( rOldLRSpace.GetRight() );
        }

        SfxItemSet aAttrs( pEditEngine->GetParaAttribs( nPara ) );
        aAttrs.Put( aNewLRSpace);
        pPara->aBulSize.Width() = -1;
        pEditEngine->SetParaAttribs( nPara, aAttrs );
    }
}

BOOL Outliner::CreateLRSpaceItem( const SvxNumBulletItem& rNumBullet, USHORT nLevel, SvxLRSpaceItem& rLRSpace )
{
    const SvxNumberFormat* pFmt = rNumBullet.GetNumRule()->Get( nLevel );
    if ( pFmt )
    {
        rLRSpace.SetTxtFirstLineOfst( pFmt->GetFirstLineOffset() );
        rLRSpace.SetTxtLeft( pFmt->GetAbsLSpace() );
    }
    return pFmt ? TRUE : FALSE;
}

void Outliner::ImplSetLevelDependendStyleSheet( USHORT nPara )
{
    DBG_ASSERT( ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEOBJECT, "SetLevelDependendStyleSheet: Wrong Mode!" );

    SfxStyleSheet* pCurrentStyle = GetStyleSheet( nPara );
    if ( pCurrentStyle )
    {
        String aNewStyleSheetName( pCurrentStyle->GetName() );
        aNewStyleSheetName.Erase( aNewStyleSheetName.Len()-1, 1 );
        aNewStyleSheetName += String::CreateFromInt32( GetDepth( nPara ) );
        SfxStyleSheet* pNewStyle = (SfxStyleSheet*)GetStyleSheetPool()->Find( aNewStyleSheetName, pCurrentStyle->GetFamily() );
        DBG_ASSERT( pNewStyle, "AutoStyleSheetName - Style not found!" );
        if ( pNewStyle && ( pNewStyle != pCurrentStyle ) )
        {
             SfxItemSet aOldAttrs( pEditEngine->GetParaAttribs( nPara ) );
            SetStyleSheet( nPara, pNewStyle );
            if ( aOldAttrs.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
            {
                SfxItemSet aAttrs( pEditEngine->GetParaAttribs( nPara ) );
                aAttrs.Put( aOldAttrs.Get( EE_PARA_NUMBULLET ) );
                pEditEngine->SetParaAttribs( nPara, aAttrs );
            }
        }
    }
}

void Outliner::ImplInitDepth( USHORT nPara, USHORT nDepth, BOOL bCreateUndo, BOOL bUndoAction )
{
    DBG_CHKTHIS(Outliner,0);

    Paragraph* pPara = pParaList->GetParagraph( nPara );
    USHORT nOldDepth = pPara->GetDepth();
    pPara->SetDepth( nDepth );

    // Bei IsInUndo brauchen Attribute und Style nicht eingestellt werden,
    // dort werden die alten Werte durch die EditEngine restauriert.

    if( !IsInUndo() )
    {
        BOOL bUpdate = pEditEngine->GetUpdateMode();
        pEditEngine->SetUpdateMode( FALSE );

        BOOL bUndo = bCreateUndo && IsUndoEnabled();
        if ( bUndo && bUndoAction )
            UndoActionStart( OLUNDO_DEPTH );

        SfxItemSet aAttrs( pEditEngine->GetParaAttribs( nPara ) );
        aAttrs.Put( SfxUInt16Item( EE_PARA_OUTLLEVEL, nDepth ) );
        // MT: OUTLLRSPACE immer default, sollte mal irgendwie eingestellt werden...
        aAttrs.Put( lcl_ImplGetDefLRSpaceItem( nDepth, GetRefMapMode().GetMapUnit() ) );
        pEditEngine->SetParaAttribs( nPara, aAttrs );
        ImplCheckNumBulletItem( nPara );
        ImplCalcBulletText( nPara, FALSE, FALSE );

        if ( bUndo )
        {
            InsertUndo( new OutlinerUndoChangeDepth( this, nPara, nOldDepth, nDepth ) );
            if ( bUndoAction )
                UndoActionEnd( OLUNDO_DEPTH );
        }

        pEditEngine->SetUpdateMode( bUpdate );
    }
}

void Outliner::SetParaAttribs( ULONG nPara, const SfxItemSet& rSet )
{
    DBG_CHKTHIS(Outliner,0);

    Paragraph* pPara = pParaList->GetParagraph( nPara );
    if( pPara )
    {
        if ( !pEditEngine->IsInUndo() && pEditEngine->IsUndoEnabled() )
            UndoActionStart( OLUNDO_ATTR );

        BOOL bLRSpaceChanged =
            ( rSet.GetItemState( EE_PARA_LRSPACE ) == SFX_ITEM_ON  ) &&
            ( !(rSet.Get( EE_PARA_LRSPACE ) == pEditEngine->GetParaAttrib( (USHORT)nPara, EE_PARA_LRSPACE ) ) );

        pEditEngine->SetParaAttribs( (USHORT)nPara, rSet );

        if ( bLRSpaceChanged )
        {
            const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&)pEditEngine->GetParaAttrib( (USHORT)nPara, EE_PARA_NUMBULLET );
            Paragraph* pPara = pParaList->GetParagraph( nPara );
            if ( rNumBullet.GetNumRule()->GetLevelCount() > pPara->GetDepth() )
            {
                SvxNumBulletItem* pNewNumBullet = (SvxNumBulletItem*) rNumBullet.Clone();
                EditEngine::ImportBulletItem( *pNewNumBullet, pPara->GetDepth(), NULL, (SvxLRSpaceItem*)&rSet.Get( EE_PARA_LRSPACE ) );
                SfxItemSet aAttribs( rSet );
                aAttribs.Put( *pNewNumBullet );
                pEditEngine->SetParaAttribs( (USHORT)nPara, aAttribs );
                delete pNewNumBullet;
            }
        }

        ImplCheckNumBulletItem( (USHORT)nPara );
        ImplCheckParagraphs( (USHORT)nPara, (USHORT)nPara );

        if ( !pEditEngine->IsInUndo() && pEditEngine->IsUndoEnabled() )
            UndoActionEnd( OLUNDO_ATTR );
    }
}

BOOL Outliner::Expand( Paragraph* pPara )
{
    DBG_CHKTHIS(Outliner,0);

    if ( pParaList->HasHiddenChilds( pPara ) )
    {
        OLUndoExpand* pUndo;
        BOOL bUndo = IsUndoEnabled() && !IsInUndo();
        if( bUndo )
        {
            UndoActionStart( OLUNDO_EXPAND );
            pUndo = new OLUndoExpand( this, OLUNDO_EXPAND );
            pUndo->pParas = 0;
            pUndo->nCount = (USHORT)pParaList->GetAbsPos( pPara );
        }
        pHdlParagraph = pPara;
        bIsExpanding = TRUE;
        pParaList->Expand( pPara );
        ExpandHdl();
        InvalidateBullet( pPara, pParaList->GetAbsPos(pPara) );
        if( bUndo )
        {
            InsertUndo( pUndo );
            UndoActionEnd( OLUNDO_EXPAND );
        }
        return TRUE;
    }
    return FALSE;
}


BOOL Outliner::Collapse( Paragraph* pPara )
{
    DBG_CHKTHIS(Outliner,0);
    if ( pParaList->HasVisibleChilds( pPara ) ) // expandiert
    {
        OLUndoExpand* pUndo;
        BOOL bUndo = FALSE;
        if( !IsInUndo() && IsUndoEnabled() )
            bUndo = TRUE;
        if( bUndo )
        {
            UndoActionStart( OLUNDO_COLLAPSE );
            pUndo = new OLUndoExpand( this, OLUNDO_COLLAPSE );
            pUndo->pParas = 0;
            pUndo->nCount = (USHORT)pParaList->GetAbsPos( pPara );
        }
        pHdlParagraph = pPara;
        bIsExpanding = FALSE;
        pParaList->Collapse( pPara );
        ExpandHdl();
        InvalidateBullet( pPara, pParaList->GetAbsPos(pPara) );
        if( bUndo )
        {
            InsertUndo( pUndo );
            UndoActionEnd( OLUNDO_COLLAPSE );
        }
        return TRUE;
    }
    return FALSE;
}


Font Outliner::ImpCalcBulletFont( USHORT nPara ) const
{
    const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&) pEditEngine->GetParaAttrib( nPara, EE_PARA_NUMBULLET );
    Paragraph* pPara = pParaList->GetParagraph( nPara );
    const SvxNumberFormat* pFmt = rNumBullet.GetNumRule()->Get( pPara->GetDepth() );

    DBG_ASSERT( pFmt && ( pFmt->GetNumType() != SVX_NUM_BITMAP ), "ImpCalcBulletFont: Missing or BitmapBullet!" );

    USHORT nScale = pEditEngine->IsFlatMode() ? DEFAULT_SCALE : pFmt->GetBulletRelSize();
    ULONG nScaledLineHeight = pEditEngine->GetStandardFont( nPara ).GetSize().Height();
    nScaledLineHeight *= nScale*10;
    nScaledLineHeight /= 1000;

    Font aBulletFont;
    if ( pFmt->GetNumType() == SVX_NUM_CHAR_SPECIAL )
        aBulletFont = *pFmt->GetBulletFont();
    else
        aBulletFont = pEditEngine->GetStandardFont( nPara );

    aBulletFont.SetAlign( ALIGN_BOTTOM );
    aBulletFont.SetSize( Size( 0, nScaledLineHeight ) );
    BOOL bVertical = IsVertical();
    aBulletFont.SetVertical( bVertical );
    aBulletFont.SetOrientation( bVertical ? 2700 : 0 );

    Color aColor( COL_BLACK );
    if( !pEditEngine->IsFlatMode() && !( pEditEngine->GetControlWord() & EE_CNTRL_NOCOLORS ) )
        aColor = pFmt->GetBulletColor();
    aBulletFont.SetColor( aColor );
    return aBulletFont;
}

void Outliner::PaintBullet( USHORT nPara, const Point& rStartPos,
    const Point& rOrigin, short nOrientation, OutputDevice* pOutDev )
{
    DBG_CHKTHIS(Outliner,0);

    if ( ImplHasBullet( nPara ) )
    {
        BOOL bVertical = IsVertical();

        Rectangle aBulletArea( ImpCalcBulletArea( nPara, TRUE ) );
        if ( !bVertical )
            aBulletArea.Move( rStartPos.X(), rStartPos.Y() );
        else
            aBulletArea.Move( rStartPos.Y(), rStartPos.X()-2*aBulletArea.Top() );

        Paragraph* pPara = pParaList->GetParagraph( nPara );
        const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&) pEditEngine->GetParaAttrib( nPara, EE_PARA_NUMBULLET );
        const SvxNumberFormat* pFmt = rNumBullet.GetNumRule()->Get( pPara->GetDepth() );
        if ( pFmt )
        {
            if( pFmt->GetNumType() != SVX_NUM_BITMAP )
            {
                Font aBulletFont( ImpCalcBulletFont( nPara ) );
                Font aOldFont = pOutDev->GetFont();
                pOutDev->SetFont( aBulletFont );

                Point aTextPos;
                if ( !bVertical )
                    aTextPos = aBulletArea.BottomLeft();
                else
                {
                    aTextPos.X() = aBulletArea.Bottom();
                    aTextPos.Y() = aBulletArea.Left();
                }

                if ( !bStrippingPortions )
                {
                    if ( nOrientation )
                    {
                        // Sowohl TopLeft als auch BottomLeft nicht ganz richtig, da
                        // in EditEngine BaseLine...
                        double nRealOrientation = nOrientation*F_PI1800;
                        double nCos = cos( nRealOrientation );
                        double nSin = sin( nRealOrientation );
                        Point aRotatedPos;
                        // Translation...
                        aTextPos -= rOrigin;
                        // Rotation...
                        aRotatedPos.X()=(long)   (nCos*aTextPos.X() + nSin*aTextPos.Y());
                        aRotatedPos.Y()=(long) - (nSin*aTextPos.X() - nCos*aTextPos.Y());
                        aTextPos = aRotatedPos;
                        // Translation...
                        aTextPos += rOrigin;
                        Font aRotatedFont( aBulletFont );
                        aRotatedFont.SetLineOrientation( nOrientation );
                        pOutDev->SetFont( aRotatedFont );
                    }

                    pOutDev->DrawText( aTextPos, pPara->GetText() );

                    // HACK #47227#
                    // Seitennummer im Gliederungsmodus ausgeben...
                    if ( (pPara->GetDepth() == 0 ) && ( pEditEngine->GetControlWord() & EE_CNTRL_OUTLINER ) )
                    {
                        long nPage = nFirstPage-1;
                        for ( USHORT n = 0; n <= nPara; n++ )
                        {
                            Paragraph* p = pParaList->GetParagraph( n );
                            if ( p->GetDepth() == 0 )
                                nPage++;
                        }
                        const SvxFontHeightItem& rFH = (const SvxFontHeightItem&)pEditEngine->GetParaAttrib( nPara, EE_CHAR_FONTHEIGHT );
                        Size aFontSz( 0, rFH.GetHeight() );
                        aFontSz.Height() /= 5;

                        Font aNewFont( System::GetStandardFont( STDFONT_SWISS ) );
                        aNewFont.SetSize( aFontSz );
                        aNewFont.SetAlign( ALIGN_BOTTOM );
                        aNewFont.SetVertical( bVertical );
                        aNewFont.SetOrientation( bVertical ? 2700 : 0 );
                        pOutDev->SetFont( aNewFont );
                        String aPageText = String::CreateFromInt32( nPage );
                        Size aTextSz;
                        aTextSz.Width() = pOutDev->GetTextWidth( aPageText );
                        aTextSz.Height() = pOutDev->GetTextHeight();
                        long nBulletHeight = aBulletArea.GetHeight();
                        if ( !bVertical )
                        {
                            aTextPos.X() -= aTextSz.Width();
                            aTextPos.X() -= aTextSz.Height() / 8;
                            aTextPos.Y() -= nBulletHeight / 2;
                            aTextPos.Y() += aTextSz.Height() / 2;
                        }
                        else
                        {
                            aTextPos.Y() -= aTextSz.Width();
                            aTextPos.Y() -= aTextSz.Height() / 8;
                            aTextPos.X() += nBulletHeight / 2;
                            aTextPos.X() -= aTextSz.Height() / 2;
                        }
                        pOutDev->DrawText( aTextPos, aPageText );
                    }
                }
                else
                {
                    Font aSvxFont( aBulletFont );
                    long* pBuf = new long[ pPara->GetText().Len() ];
                    pOutDev->GetTextArray( pPara->GetText(), pBuf );
                    // aTextPos ist Bottom, jetzt die Baseline liefern:
                    FontMetric aMetric( pOutDev->GetFontMetric() );
                    aTextPos.Y() -= aMetric.GetDescent();
                    DrawingText( aTextPos, pPara->GetText(), pBuf, aSvxFont, nPara, 0xFFFF );
                    delete pBuf;
                }
                pOutDev->SetFont( aOldFont );
            }
            else
            {
                if ( !bStrippingPortions )
                {
                    // MT: Remove CAST when KA made the Draw-Method const
                    if ( pFmt->GetBrush()->GetGraphicObject() )
                        ((GraphicObject*)pFmt->GetBrush()->GetGraphicObject())->Draw( pOutDev, aBulletArea.TopLeft(), pPara->aBulSize );
                }
            }
        }

        // Bei zusammengeklappten Absaetzen einen Strich vor den Text malen.
        if( pParaList->HasChilds(pPara) && !pParaList->HasVisibleChilds(pPara) &&
                !bStrippingPortions && !nOrientation )
        {
            Point aStartPoint( aBulletArea.BottomLeft() );
            aStartPoint.X() += aBulletArea.GetWidth();
            Point aEndPoint( aStartPoint );
            aEndPoint.X() += pOutDev->PixelToLogic( Size( 10, 0 ) ).Width();

            const Color& rOldLineColor = pOutDev->GetLineColor();
            pOutDev->SetLineColor( Color( COL_BLACK ) );
            pOutDev->DrawLine( aStartPoint, aEndPoint );
            pOutDev->SetLineColor( rOldLineColor );
        }
    }
}

void Outliner::InvalidateBullet( Paragraph* pPara, ULONG nPara )
{
    DBG_CHKTHIS(Outliner,0);

    long nLineHeight = (long)pEditEngine->GetLineHeight((USHORT)nPara );
    OutlinerView* pView = aViewList.First();
    while( pView )
    {
        Point aPos( pView->pEditView->GetWindowPosTopLeft((USHORT)nPara ) );
        Rectangle aRect( pView->GetOutputArea() );
        aRect.Right() = aPos.X();
        aRect.Top() = aPos.Y();
        aRect.Bottom() = aPos.Y();
        aRect.Bottom() += nLineHeight;

        pView->GetWindow()->Invalidate( aRect );
        pView = aViewList.Next();
    }
}

ULONG Outliner::Read( SvStream& rInput, USHORT eFormat, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    DBG_CHKTHIS(Outliner,0);

    BOOL bOldUndo = pEditEngine->IsUndoEnabled();
    EnableUndo( FALSE );

    BOOL bUpdate = pEditEngine->GetUpdateMode();
    pEditEngine->SetUpdateMode( FALSE );

    Clear();

    bBlockInsCallback = TRUE;
    ULONG nRet = pEditEngine->Read( rInput, (EETextFormat)eFormat, pHTTPHeaderAttrs );
    bBlockInsCallback = FALSE;

    bFirstParaIsEmpty = FALSE;

    USHORT nParas = pEditEngine->GetParagraphCount();
     pParaList->Clear( TRUE );
    USHORT n;
    for ( n = 0; n < nParas; n++ )
    {
        Paragraph* pPara = new Paragraph( 0 );
        pParaList->Insert( pPara, LIST_APPEND );

        if ( eFormat == EE_FORMAT_BIN )
        {
            USHORT nDepth = 0;
            const SfxItemSet& rAttrs = pEditEngine->GetParaAttribs( n );
            const SfxUInt16Item& rLevel = (const SfxUInt16Item&) rAttrs.Get( EE_PARA_OUTLLEVEL );
            nDepth = rLevel.GetValue();
            ImplInitDepth( n, nDepth, FALSE );
        }
    }

    if ( eFormat != EE_FORMAT_BIN )
    {
        ImpFilterIndents( 0, nParas-1 );
    }

    for ( n = 0; n < nParas; n++ )
    {
        // Handler rufen, Outliner war leer. Aber nicht fuer den ersten
        pHdlParagraph = pParaList->GetParagraph( n );
        ParagraphInsertedHdl();
    }

    pEditEngine->SetUpdateMode( bUpdate );
    EnableUndo( bOldUndo );

    return nRet;
}


void Outliner::ImpFilterIndents( ULONG nFirstPara, ULONG nLastPara )
{
    DBG_CHKTHIS(Outliner,0);

    BOOL bUpdate = pEditEngine->GetUpdateMode();
    pEditEngine->SetUpdateMode( FALSE );

    Paragraph* pLastConverted = NULL;
    for( ULONG nPara = nFirstPara; nPara <= nLastPara; nPara++ )
    {
        Paragraph* pPara = pParaList->GetParagraph( nPara );
        if( ImpConvertEdtToOut( pPara, nPara ) )
        {
            pLastConverted = pPara;
        }
        else if ( pLastConverted )
        {
            // Normale Absaetze unter der Ueberschrift anordnen...
            pPara->SetDepth( pLastConverted->GetDepth() );
        }

        ImplInitDepth( (USHORT)nPara, pPara->GetDepth(), FALSE );
    }

    pEditEngine->SetUpdateMode( bUpdate );
}


SfxUndoManager& Outliner::GetUndoManager()
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetUndoManager();
}

void Outliner::ImpTextPasted( ULONG nStartPara, USHORT nCount )
{
    DBG_CHKTHIS(Outliner,0);

    BOOL bUpdate = pEditEngine->GetUpdateMode();
    pEditEngine->SetUpdateMode( FALSE );

    const ULONG nStart = nStartPara;

    Paragraph* pPara = pParaList->GetParagraph( nStartPara );
    Paragraph* pLastConverted = NULL;
    while( nCount && pPara )
    {
        if( ImplGetOutlinerMode() != OUTLINERMODE_TEXTOBJECT )
        {
            USHORT nPrevDepth = pPara->GetDepth();
            if( ImpConvertEdtToOut( pPara, nStartPara ) )
                pLastConverted = pPara;
            else if ( pLastConverted )
            {
                // Normale Absaetze unter der Ueberschrift anordnen...
                pPara->SetDepth( pLastConverted->GetDepth() );
            }
            // Tiefe des Absatzes hat sich geaendert.
            if( pPara->GetDepth() != nPrevDepth )
            {
                pHdlParagraph = pPara;
                if( nPrevDepth == 0xffff )
                {
                    // neuer Absatz -> InsertedHdl rufen
                    ParagraphInsertedHdl();
                }
                else
                {
                    if( nStartPara == nStart )
                        // die Einrueckebene des Startabsatzes bleibt
                        // immer erhalten (leere Absaetze bekommen
                        // die Absatzattribute des eingefuegten Absatzes,
                        // diese sind aber erst nach dem Paste bekannt,
                        // d.h. es muesste dann ggf. der ParagraphIndenting
                        // Handler gerufen werden (also u.U. vor UND nach
                        // dem Paste)
                        pPara->SetDepth( nPrevDepth );
                    else if( (!nStartPara && pPara->GetDepth()) ||
                             (pPara->GetDepth() < nMinDepth) )
                        pPara->SetDepth( nMinDepth );

                    if( pPara->GetDepth() != nPrevDepth )
                    {
                        // alter Absatz -> DepthChangedHdl rufen
                        nDepthChangedHdlPrevDepth = nPrevDepth;
                        DepthChangedHdl();
                    }
                }
                ImplInitDepth( (USHORT)nStartPara, pPara->GetDepth(), FALSE );
            }
            else if( nStartPara == nStart )
                ImplInitDepth( (USHORT)nStartPara, pPara->GetDepth(), FALSE );
        }
        else // EditEngine-Modus
        {
            USHORT nDepth = 0;
            const SfxItemSet& rAttrs = pEditEngine->GetParaAttribs( (USHORT)nStartPara );
            if ( rAttrs.GetItemState( EE_PARA_OUTLLEVEL ) == SFX_ITEM_ON )
            {
                const SfxUInt16Item& rLevel = (const SfxUInt16Item&) rAttrs.Get( EE_PARA_OUTLLEVEL );
                nDepth = rLevel.GetValue();
            }
            ImplInitDepth( (USHORT)nStartPara, nDepth, FALSE );
        }

        nCount--;
        nStartPara++;
        pPara = pParaList->GetParagraph( nStartPara );
    }

    pEditEngine->SetUpdateMode( bUpdate );

    DBG_ASSERT(pParaList->GetParagraphCount()==pEditEngine->GetParagraphCount(),"ImpTextPasted failed");
}



void Outliner::ImpDropped( OutlinerView* pOutlView )
{
    DBG_CHKTHIS(Outliner,0);

    // MT 06/00 - Was soll das hier?
    // Diese Methode muss mal ueberarbeitet werden...
    EditView* pView = pOutlView->pEditView;

    ULONG nParas = pParaList->GetParagraphCount();
    for ( ULONG nPara = 0; nPara < nParas; nPara++ )
    {
        Paragraph* pPara = pParaList->GetParagraph( nPara );
        USHORT nCurDepth = pPara->GetDepth();
        if( nCurDepth == 0xffff )
        {
            ImpConvertEdtToOut( pPara, nPara, pView );
            pHdlParagraph = pPara;
            ParagraphInsertedHdl();
            ImplInitDepth( (USHORT)nPara, pPara->GetDepth(), FALSE );
        }
        else if( pPara->nFlags & PARAFLAG_DROPTARGET )
        {
            ImpConvertEdtToOut( pPara, nPara, pView );
            if( pPara->nDepth != nCurDepth ||
                (pPara->nFlags & PARAFLAG_DROPTARGET_EMPTY) )
            {
                // auf alten Wert zuruecksetzen
                pPara->SetDepth( nCurDepth );

                // und neu attributieren
                ImplInitDepth( (USHORT)nPara, pPara->GetDepth(), FALSE );
            }
            pPara->nFlags &= (~PARAFLAG_DROPTARGET|PARAFLAG_DROPTARGET_EMPTY);
        }
    }
}

long Outliner::IndentingPagesHdl( OutlinerView* pView )
{
    DBG_CHKTHIS(Outliner,0);
    if( !aIndentingPagesHdl.IsSet() )
        return 1;
    return aIndentingPagesHdl.Call( pView );
}

BOOL Outliner::ImpCanIndentSelectedPages( OutlinerView* pCurView )
{
    DBG_CHKTHIS(Outliner,0);
    // Die selektierten Seiten muessen vorher durch ImpCalcSelectedPages
    // schon eingestellt sein

    // Wenn der erste Absatz auf Ebene 0 liegt darf er auf keinen Fall
    // eingerueckt werden, evtl folgen aber weitere auf Ebene 0.
    if ( (ULONG)pHdlParagraph == 0 )
    {
        if ( nDepthChangedHdlPrevDepth == 1 )   // ist die einzige Seite
            return FALSE;
        else
            pCurView->ImpCalcSelectedPages( FALSE );    // ohne die erste
    }
    return (BOOL)IndentingPagesHdl( pCurView );
}


BOOL Outliner::ImpCanDeleteSelectedPages( OutlinerView* pCurView )
{
    DBG_CHKTHIS(Outliner,0);
    // Die selektierten Seiten muessen vorher durch ImpCalcSelectedPages
    // schon eingestellt sein
    return (BOOL)RemovingPagesHdl( pCurView );
}

Outliner::Outliner( SfxItemPool* pPool, USHORT nMode )
{
    DBG_CTOR( Outliner, 0 );

    bStrippingPortions  = FALSE;
    bPasting            = FALSE;

    nFirstPage          = 1;
    bBlockInsCallback   = FALSE;

    nMinDepth           = 0;
    nMaxDepth           = 9;

    pParaList = new ParagraphList;
    pParaList->SetVisibleStateChangedHdl( LINK( this, Outliner, ParaVisibleStateChangedHdl ) );
    Paragraph* pPara = new Paragraph( 0 );
    pParaList->Insert( pPara, LIST_APPEND );
    bFirstParaIsEmpty = TRUE;

    pEditEngine = new OutlinerEditEng( this, pPool );

    Init( nMode );
}

Outliner::~Outliner()
{
    DBG_DTOR(Outliner,0);

    pParaList->Clear( TRUE );
    delete pParaList;
    delete pEditEngine;
}

ULONG Outliner::InsertView( OutlinerView* pView, ULONG nIndex )
{
    DBG_CHKTHIS(Outliner,0);

    aViewList.Insert( pView, nIndex );
    pEditEngine->InsertView(  pView->pEditView, (USHORT)nIndex );
    return aViewList.GetPos( pView );
}

OutlinerView* Outliner::RemoveView( OutlinerView* pView )
{
    DBG_CHKTHIS(Outliner,0);

    ULONG nPos = aViewList.GetPos( pView );
    if ( nPos != LIST_ENTRY_NOTFOUND )
    {
        pView->pEditView->HideCursor(); // HACK wg. BugId 10006
        pEditEngine->RemoveView(  pView->pEditView );
        aViewList.Remove( nPos );
    }
    return NULL;    // MT: return ueberfluessig
}

OutlinerView* Outliner::RemoveView( ULONG nIndex )
{
    DBG_CHKTHIS(Outliner,0);

    EditView* pEditView = pEditEngine->GetView( (USHORT)nIndex );
    pEditView->HideCursor(); // HACK wg. BugId 10006

    pEditEngine->RemoveView( (USHORT)nIndex );
    aViewList.Remove( nIndex );
    return NULL;    // MT: return ueberfluessig
}


OutlinerView* Outliner::GetView( ULONG nIndex ) const
{
    DBG_CHKTHIS(Outliner,0);
    return aViewList.GetObject( nIndex );
}

ULONG Outliner::GetViewCount() const
{
    DBG_CHKTHIS(Outliner,0);
    return aViewList.Count();
}

void Outliner::ParagraphInsertedHdl()
{
    DBG_CHKTHIS(Outliner,0);
    aParaInsertedHdl.Call( this );
}


void Outliner::ParagraphRemovingHdl()
{
    DBG_CHKTHIS(Outliner,0);
    aParaRemovingHdl.Call( this );
}


void Outliner::DepthChangedHdl()
{
    DBG_CHKTHIS(Outliner,0);
    aDepthChangedHdl.Call( this );
}


ULONG Outliner::GetAbsPos( Paragraph* pPara )
{
    DBG_CHKTHIS(Outliner,0);
    DBG_ASSERT(pPara,"GetAbsPos:No Para")
    return pParaList->GetAbsPos( pPara );
}

void Outliner::ParagraphHeightChanged( USHORT )
{
    DBG_CHKTHIS(Outliner,0);
    // MT: Kann wohl weg...
}

ULONG Outliner::GetParagraphCount() const
{
    DBG_CHKTHIS(Outliner,0);
    return pParaList->GetParagraphCount();
}

Paragraph* Outliner::GetParagraph( ULONG nAbsPos ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pParaList->GetParagraph( nAbsPos );
}

BOOL Outliner::HasChilds( Paragraph* pParagraph ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pParaList->HasChilds( pParagraph );
}

BOOL Outliner::ImplHasBullet( USHORT nPara ) const
{
    const SfxUInt16Item& rBulletState = (const SfxUInt16Item&) pEditEngine->GetParaAttrib( nPara, EE_PARA_BULLETSTATE );
    BOOL bBullet = rBulletState.GetValue() ? TRUE : FALSE;
    if ( !pParaList->GetParagraph( nPara )->GetDepth() && ( pEditEngine->GetControlWord() & EE_CNTRL_OUTLINER ) )
    {
        // Im Gliederungsmodus immer ein Bullet auf Ebene 0!
        bBullet = TRUE;
    }
    return bBullet;
}

Size Outliner::ImplGetBulletSize( USHORT nPara )
{
    Paragraph* pPara = pParaList->GetParagraph( nPara );

    if( pPara->aBulSize.Width() == -1 )
    {
        const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&) pEditEngine->GetParaAttrib( nPara, EE_PARA_NUMBULLET );
        const SvxNumberFormat* pFmt = rNumBullet.GetNumRule()->Get( pPara->GetDepth() );
        DBG_ASSERT( pFmt, "ImplGetBulletSize - no Bullet!" );

        if( pFmt->GetNumType() != SVX_NUM_BITMAP )
        {
            String aBulletText = ImplGetBulletText( nPara );
            OutputDevice* pRefDev = pEditEngine->GetRefDevice();
            Font aBulletFont( ImpCalcBulletFont( nPara ) );
            Font aRefFont( pRefDev->GetFont());
            pRefDev->SetFont( aBulletFont );
            pPara->aBulSize.Width() = pRefDev->GetTextWidth( aBulletText );
            pPara->aBulSize.Height() = pRefDev->GetTextHeight();
            pRefDev->SetFont( aRefFont );
        }
        else
        {
            pPara->aBulSize = OutputDevice::LogicToLogic( pFmt->GetGraphicSize(), MAP_100TH_MM, pEditEngine->GetRefDevice()->GetMapMode() );
        }
    }

    return pPara->aBulSize;
}

void Outliner::ImplCheckParagraphs( USHORT nStart, USHORT nEnd )
{
    DBG_CHKTHIS( Outliner, 0 );

    // Sicherstellen dass es ein Bullet und ein LR-Space gibt.

    for ( USHORT n = nStart; n <= nEnd; n++ )
    {
        Paragraph* pPara = pParaList->GetParagraph( n );
        BOOL bHasLRSpace = pEditEngine->HasParaAttrib( n, EE_PARA_OUTLLRSPACE );
        BOOL bHasLevel = pEditEngine->HasParaAttrib( n, EE_PARA_OUTLLEVEL );
        if ( !bHasLRSpace || !bHasLevel )
        {
            SfxItemSet aAttrs( pEditEngine->GetParaAttribs( n ) );

            // MT 05/00: Default-Item muss erstmal richtig sein => Dann koennen diese ganzen komischen Defaults weg!!!
//          const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&) pEditEngine->GetParaAttrib( n, EE_PARA_NUMBULLET );
//          const SvxNumberFormat* pFmt = NULL;
//          if ( ( rNumBullet.GetNumRule()->GetLevelCount() > pPara->GetDepth() ) &&
//               ( ( pFtm = rNumBullet.GetNumRule()->Get( pPara->GetDepth() ) != NULL ) )
//          {
//          }
            if ( !bHasLRSpace )
            {
                SvxLRSpaceItem aLRSpaceItem = lcl_ImplGetDefLRSpaceItem( pPara->GetDepth(), GetRefMapMode().GetMapUnit() );
                aAttrs.Put( aLRSpaceItem );
            }
            if ( !bHasLevel )
            {
                SfxUInt16Item aLevelItem( EE_PARA_OUTLLEVEL, pPara->GetDepth() );
                aAttrs.Put( aLevelItem );
            }
            pEditEngine->SetParaAttribs( n, aAttrs );
        }

        pPara->Invalidate();
        ImplCalcBulletText( n, FALSE, FALSE );
    }
}

void Outliner::SetRefDevice( OutputDevice* pRefDev )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetRefDevice( pRefDev );
    for ( USHORT n = (USHORT) pParaList->GetParagraphCount(); n; )
    {
        Paragraph* pPara = pParaList->GetParagraph( --n );
        pPara->Invalidate();
    }
}

void Outliner::ParaAttribsChanged( USHORT nPara )
{
    DBG_CHKTHIS(Outliner,0);

    // Der Outliner hat kein eigenes Undo, wenn Absaetz getrennt/verschmolzen werden.
    // Beim ParagraphInserted ist das Attribut EE_PARA_OUTLLEVEL
    // ggf. noch nicht eingestellt, dies wird aber benoetigt um die Tiefe
    // des Absatzes zu bestimmen.

    if( pEditEngine->IsInUndo() )
    {
        if ( pParaList->GetParagraphCount() == pEditEngine->GetParagraphCount() )
        {
            Paragraph* pPara = pParaList->GetParagraph( nPara );
            const SfxUInt16Item& rLevel = (const SfxUInt16Item&) pEditEngine->GetParaAttrib( nPara, EE_PARA_OUTLLEVEL );
            if ( pPara->GetDepth() != rLevel.GetValue() )
            {
                USHORT nMin = Min( pPara->GetDepth(), (USHORT)rLevel.GetValue() );
                pPara->SetDepth( rLevel.GetValue() );
                ImplCalcBulletText( nPara, TRUE, TRUE );
            }
        }
    }
}

void Outliner::StyleSheetChanged( SfxStyleSheet* pStyle )
{
    DBG_CHKTHIS(Outliner,0);

    // Die EditEngine ruft StyleSheetChanged auch fuer abgeleitete Styles.
    // MT: Hier wurde frueher alle Absaetze durch ein ImpRecalcParaAttribs
    // gejagt, die die besagte Vorlage haben, warum?
    // => Eigentlich kann sich nur die Bullet-Repraesentation aendern...

    USHORT nParas = (USHORT)pParaList->GetParagraphCount();
    for( USHORT nPara = 0; nPara < nParas; nPara++ )
    {
        if ( pEditEngine->GetStyleSheet( nPara ) == pStyle )
        {
            ImplCheckNumBulletItem( nPara );
            ImplCalcBulletText( nPara, FALSE, FALSE );
        }
    }
}

Rectangle Outliner::ImpCalcBulletArea( USHORT nPara, BOOL bAdjust )
{
    // Bullet-Bereich innerhalb des Absatzes...
    Rectangle aBulletArea;

    const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&) pEditEngine->GetParaAttrib( nPara, EE_PARA_NUMBULLET );
    Paragraph* pPara = pParaList->GetParagraph( nPara );
    const SvxNumberFormat* pFmt = rNumBullet.GetNumRule()->Get( pPara->GetDepth() );

    if ( pFmt )
    {
        Point aTopLeft;
        Size aBulletSize( ImplGetBulletSize( nPara ) );

        const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&) pEditEngine->GetParaAttrib( nPara, EE_PARA_LRSPACE );
        aTopLeft.X() = rLR.GetTxtLeft() + rLR.GetTxtFirstLineOfst();


        long nBulletWidth = Max( (long) -rLR.GetTxtFirstLineOfst(), (long) ((-pFmt->GetFirstLineOffset()) + pFmt->GetCharTextDistance()) );
        if ( nBulletWidth < aBulletSize.Width() )   // Bullet macht sich Platz
            nBulletWidth = aBulletSize.Width();

        if ( bAdjust && ( !( pEditEngine->GetControlWord() & EE_CNTRL_OUTLINER ) ) )
        {
            // Bei zentriert/rechtsbuendig anpassen
            const SvxAdjustItem& rItem = (const SvxAdjustItem&)pEditEngine->GetParaAttrib( nPara, EE_PARA_JUST );
            if ( rItem.GetAdjust() != SVX_ADJUST_LEFT )
                aTopLeft.X() = pEditEngine->GetFirstLineStartX( nPara ) - nBulletWidth;
        }

        // Vertikal:
        ParagraphInfos aInfos = pEditEngine->GetParagraphInfos( nPara );
        if ( aInfos.bValid )
        {
            aTopLeft.Y() = aInfos.nFirstLineOffset
                            + aInfos.nFirstLineHeight - aInfos.nFirstLineTextHeight
                            + aInfos.nFirstLineTextHeight / 2
                            - aBulletSize.Height() / 2;
            // ggf. lieber auf der Baseline ausgeben...
            if( ( pFmt->GetNumType() != SVX_NUM_BITMAP ) && ( pFmt->GetNumType() != SVX_NUM_CHAR_SPECIAL ) )
            {
                Font aBulletFont( ImpCalcBulletFont( nPara ) );
                if ( aBulletFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL )
                {
                    OutputDevice* pRefDev = pEditEngine->GetRefDevice();
                    Font aOldFont = pRefDev->GetFont();
                    pRefDev->SetFont( aBulletFont );
                    FontMetric aMetric( pRefDev->GetFontMetric() );
                    // Leading der ersten Zeile...
                    aTopLeft.Y() = aInfos.nFirstLineOffset + aInfos.nFirstLineMaxAscent;
                    aTopLeft.Y() -= aMetric.GetAscent();
                    pRefDev->SetFont( aOldFont );
                }
            }
        }

        // Horizontal:
        if( pFmt->GetNumAdjust() == SVX_ADJUST_RIGHT )
        {
            aTopLeft.X() += nBulletWidth - aBulletSize.Width();
        }
        else if( pFmt->GetNumAdjust() == SVX_ADJUST_CENTER )
        {
            aTopLeft.X() += ( nBulletWidth - aBulletSize.Width() ) / 2;
        }

        if ( aTopLeft.X() < 0 )     // dann draengeln
            aTopLeft.X() = 0;

        aBulletArea = Rectangle( aTopLeft, aBulletSize );
    }
    return aBulletArea;
}

void Outliner::ExpandHdl()
{
    DBG_CHKTHIS(Outliner,0);
    aExpandHdl.Call( this );
}

XubString Outliner::GetText( Paragraph* pParagraph, ULONG nCount ) const
{
    DBG_CHKTHIS(Outliner,0);

    XubString aText;
    USHORT nStartPara = (USHORT) pParaList->GetAbsPos( pParagraph );
    for ( USHORT n = 0; n < nCount; n++ )
    {
        aText += pEditEngine->GetText( nStartPara + n );
        if ( (n+1) < (USHORT)nCount )
            aText += '\n';
    }
    return aText;
}

void Outliner::Remove( Paragraph* pPara, ULONG nParaCount )
{
    DBG_CHKTHIS(Outliner,0);

    ULONG nPos = pParaList->GetAbsPos( pPara );
    if( !nPos && ( nParaCount >= pParaList->GetParagraphCount() ) )
    {
        Clear();
    }
    else
    {
        for( USHORT n = 0; n < (USHORT)nParaCount; n++ )
            pEditEngine->RemoveParagraph( (USHORT) nPos );
    }
}

void Outliner::StripPortions()
{
    DBG_CHKTHIS(Outliner,0);
    bStrippingPortions = TRUE;
    pEditEngine->StripPortions();
    bStrippingPortions = FALSE;
}

void Outliner::DrawingText( const Point& rStartPos, const XubString& rText, const long* pDXArray,const SvxFont& rFont, USHORT nPara, USHORT nIndex )
{
    DBG_CHKTHIS(Outliner,0);
    DrawPortionInfo aInfo( rStartPos, rText, rFont, nPara, nIndex, pDXArray );
    aDrawPortionHdl.Call( &aInfo );
}

long Outliner::RemovingPagesHdl( OutlinerView* pView )
{
    DBG_CHKTHIS(Outliner,0);
    return aRemovingPagesHdl.IsSet() ? aRemovingPagesHdl.Call( pView ) : TRUE;
}

BOOL Outliner::ImpCanDeleteSelectedPages( OutlinerView* pCurView, USHORT nFirstPage, USHORT nPages )
{
    DBG_CHKTHIS(Outliner,0);

    nDepthChangedHdlPrevDepth = nPages;
    pHdlParagraph = (Paragraph*)nFirstPage;
    return (BOOL)RemovingPagesHdl( pCurView );
}

SfxItemSet Outliner::GetParaAttribs( ULONG nPara )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetParaAttribs( (USHORT)nPara );
}

IMPL_LINK( Outliner, ParaVisibleStateChangedHdl, Paragraph*, pPara )
{
    DBG_CHKTHIS(Outliner,0);

    ULONG nPara = pParaList->GetAbsPos( pPara );
    pEditEngine->ShowParagraph( (USHORT)nPara, pPara->IsVisible() );

    return 0;
}

void Outliner::ImplCalcBulletText( USHORT nPara, BOOL bRecalcLevel, BOOL bRecalcChilds )
{
    DBG_CHKTHIS(Outliner,0);

    Paragraph* pPara = pParaList->GetParagraph( nPara );
    USHORT nRelPos = 0xFFFF;

    while ( pPara )
    {
        XubString aBulletText;
        const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&) pEditEngine->GetParaAttrib( nPara, EE_PARA_NUMBULLET );
        const SvxNumberFormat* pFmt = rNumBullet.GetNumRule()->Get( pPara->GetDepth() );
        if( pFmt && ( pFmt->GetNumType() != SVX_NUM_BITMAP ) )
        {
            aBulletText += pFmt->GetPrefix();
            if( pFmt->GetNumType() != SVX_NUM_CHAR_SPECIAL )
            {
                if ( nRelPos == 0xFFFF )
                    pParaList->GetParent( pPara, nRelPos );
                USHORT nNumber = nRelPos + pFmt->GetStart();
                aBulletText += pFmt->GetNumStr( nNumber );
            }
            else
            {
                aBulletText += pFmt->GetBulletChar();
            }
            aBulletText += pFmt->GetSuffix();
        }

        if( aBulletText != pPara->GetText() )
            pPara->SetText( aBulletText );

        pPara->nFlags &= (~PARAFLAG_SETBULLETTEXT);

        if ( bRecalcLevel )
        {
            if ( nRelPos != 0xFFFF )
                nRelPos++;

            USHORT nDepth = pPara->GetDepth();
            pPara = pParaList->GetParagraph( ++nPara );
            if ( !bRecalcChilds )
            {
                while ( pPara && ( pPara->GetDepth() > nDepth ) )
                    pPara = pParaList->GetParagraph( ++nPara );
            }

            if ( pPara && ( pPara->GetDepth() < nDepth ) )
                pPara = NULL;
        }
        else
        {
            pPara = NULL;
        }
    }
}

void Outliner::Clear()
{
    DBG_CHKTHIS(Outliner,0);

    if( !bFirstParaIsEmpty )
    {
        bBlockInsCallback = TRUE;
        pEditEngine->Clear();
        bBlockInsCallback = FALSE;

        pParaList->Clear( TRUE );
        pParaList->Insert( new Paragraph( nMinDepth ), LIST_APPEND );
        bFirstParaIsEmpty = TRUE;
    }
    else
    {
        pParaList->GetParagraph( 0 )->SetDepth( nMinDepth );
    }
}

void Outliner::SetFlatMode( BOOL bFlat )
{
    DBG_CHKTHIS(Outliner,0);

    if( bFlat != pEditEngine->IsFlatMode() )
    {
        for ( USHORT nPara = (USHORT)pParaList->GetParagraphCount(); nPara; )
            pParaList->GetParagraph( --nPara )->aBulSize.Width() = -1;

        pEditEngine->SetFlatMode( bFlat );
    }
}

String Outliner::ImplGetBulletText( USHORT nPara )
{
    Paragraph* pPara = pParaList->GetParagraph( nPara );
    // MT: Optimierung mal wieder aktivieren...
//  if( pPara->nFlags & PARAFLAG_SETBULLETTEXT )
        ImplCalcBulletText( nPara, FALSE, FALSE );
    return pPara->GetText();
}
