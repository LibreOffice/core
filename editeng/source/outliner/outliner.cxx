/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <comphelper/string.hxx>
#include <svl/intitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>

#include <math.h>
#include <svl/style.hxx>
#include <vcl/wrkwin.hxx>
#define _OUTLINER_CXX
#include <editeng/outliner.hxx>
#include <paralist.hxx>
#include <editeng/outlobj.hxx>
#include <outleeng.hxx>
#include <outlundo.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editstat.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/editobj.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>
#include <vcl/metric.hxx>
#include <editeng/numitem.hxx>
#include <editeng/adjitem.hxx>
#include <vcl/graph.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <svtools/grfmgr.hxx>
#include <editeng/svxfont.hxx>
#include <editeng/brshitem.hxx>
#include <svl/itempool.hxx>

// calculate if it's RTL or not
#include <unicode/ubidi.h>
#include <cassert>
using ::std::advance;

static const sal_uInt16 nDefStyles = 3; // Special treatment for the first 3 levels
static const sal_uInt16 nDefBulletIndent = 800;
static const sal_uInt16 nDefBulletWidth = 700;
static const sal_uInt16 pDefBulletIndents[nDefStyles]=  { 1400, 800, 800 };
static const sal_uInt16 pDefBulletWidths[nDefStyles] =  { 1000, 850, 700 };

// ----------------------------------------------------------------------
// Outliner
// ----------------------------------------------------------------------
DBG_NAME(Outliner);

void Outliner::ImplCheckDepth( sal_Int16& rnDepth ) const
{
    if( rnDepth < nMinDepth )
        rnDepth = nMinDepth;
    else if( rnDepth > nMaxDepth )
        rnDepth = nMaxDepth;
}

Paragraph* Outliner::Insert(const XubString& rText, sal_uLong nAbsPos, sal_Int16 nDepth)
{
    DBG_CHKTHIS(Outliner,0);
    DBG_ASSERT(pParaList->GetParagraphCount(),"Insert:No Paras");

    Paragraph* pPara;

    ImplCheckDepth( nDepth );

    sal_uLong nParagraphCount = pParaList->GetParagraphCount();
    if( nAbsPos > nParagraphCount )
        nAbsPos = nParagraphCount;

    if( bFirstParaIsEmpty )
    {
        pPara = pParaList->GetParagraph( 0 );
        if( pPara->GetDepth() != nDepth )
        {
            nDepthChangedHdlPrevDepth = pPara->GetDepth();
            mnDepthChangeHdlPrevFlags = pPara->nFlags;
            pPara->SetDepth( nDepth );
            pHdlParagraph = pPara;
            DepthChangedHdl();
        }
        pPara->nFlags |= PARAFLAG_HOLDDEPTH;
        SetText( rText, pPara );
    }
    else
    {
        sal_Bool bUpdate = pEditEngine->GetUpdateMode();
        pEditEngine->SetUpdateMode( sal_False );
        ImplBlockInsertionCallbacks( sal_True );
        pPara = new Paragraph( nDepth );
        pParaList->Insert( pPara, nAbsPos );
        pEditEngine->InsertParagraph( (sal_uInt16)nAbsPos, String() );
        DBG_ASSERT(pPara==pParaList->GetParagraph(nAbsPos),"Insert:Failed");
        ImplInitDepth( (sal_uInt16)nAbsPos, nDepth, sal_False );
        pHdlParagraph = pPara;
        ParagraphInsertedHdl();
        pPara->nFlags |= PARAFLAG_HOLDDEPTH;
        SetText( rText, pPara );
        ImplBlockInsertionCallbacks( sal_False );
        pEditEngine->SetUpdateMode( bUpdate );
    }
    bFirstParaIsEmpty = sal_False;
    DBG_ASSERT(pEditEngine->GetParagraphCount()==pParaList->GetParagraphCount(),"SetText failed");
    return pPara;
}


void Outliner::ParagraphInserted( sal_uInt16 nPara )
{
    DBG_CHKTHIS(Outliner,0);

    if ( bBlockInsCallback )
        return;

    if( bPasting || pEditEngine->IsInUndo() )
    {
        Paragraph* pPara = new Paragraph( -1 );
        pParaList->Insert( pPara, nPara );
        if( pEditEngine->IsInUndo() )
        {
            pPara->nFlags = PARAFLAG_SETBULLETTEXT;
            pPara->bVisible = sal_True;
            const SfxInt16Item& rLevel = (const SfxInt16Item&) pEditEngine->GetParaAttrib( nPara, EE_PARA_OUTLLEVEL );
            pPara->SetDepth( rLevel.GetValue() );
        }
    }
    else
    {
        sal_Int16 nDepth = -1;
        Paragraph* pParaBefore = pParaList->GetParagraph( nPara-1 );
        if ( pParaBefore )
            nDepth = pParaBefore->GetDepth();

        Paragraph* pPara = new Paragraph( nDepth );
        pParaList->Insert( pPara, nPara );

        if( !pEditEngine->IsInUndo() )
        {
            ImplCalcBulletText( nPara, sal_True, sal_False );
            pHdlParagraph = pPara;
            ParagraphInsertedHdl();
        }
    }
}

void Outliner::ParagraphDeleted( sal_uInt16 nPara )
{
    DBG_CHKTHIS(Outliner,0);

    if ( bBlockInsCallback || ( nPara == EE_PARA_ALL ) )
        return;

    Paragraph* pPara = pParaList->GetParagraph( nPara );
        if (!pPara)
            return;

    sal_Int16 nDepth = pPara->GetDepth();

    if( !pEditEngine->IsInUndo() )
    {
        pHdlParagraph = pPara;
        ParagraphRemovingHdl();
    }

    pParaList->Remove( nPara );
    delete pPara;

    if( !pEditEngine->IsInUndo() && !bPasting )
    {
        pPara = pParaList->GetParagraph( nPara );
        if ( pPara && ( pPara->GetDepth() > nDepth ) )
        {
            ImplCalcBulletText( nPara, sal_True, sal_False );
            // Search for next on the this level ...
            while ( pPara && pPara->GetDepth() > nDepth )
                pPara = pParaList->GetParagraph( ++nPara );
        }

        if ( pPara && ( pPara->GetDepth() == nDepth ) )
            ImplCalcBulletText( nPara, sal_True, sal_False );
    }
}

void Outliner::Init( sal_uInt16 nMode )
{
    nOutlinerMode = nMode;

    Clear();

    sal_uLong nCtrl = pEditEngine->GetControlWord();
    nCtrl &= ~(EE_CNTRL_OUTLINER|EE_CNTRL_OUTLINER2);

    SetMaxDepth( 9 );

    switch ( ImplGetOutlinerMode() )
    {
        case OUTLINERMODE_TEXTOBJECT:
        case OUTLINERMODE_TITLEOBJECT:
            break;

        case OUTLINERMODE_OUTLINEOBJECT:
            nCtrl |= EE_CNTRL_OUTLINER2;
            break;
        case OUTLINERMODE_OUTLINEVIEW:
            nCtrl |= EE_CNTRL_OUTLINER;
            break;

        default: OSL_FAIL( "Outliner::Init - Invalid Mode!" );
    }

    pEditEngine->SetControlWord( nCtrl );

    ImplInitDepth( 0, GetMinDepth(), sal_False );

    GetUndoManager().Clear();
}

void Outliner::SetMaxDepth( sal_Int16 nDepth, sal_Bool bCheckParagraphs )
{
    if( nMaxDepth != nDepth )
    {
        nMaxDepth = Min( nDepth, (sal_Int16)(SVX_MAX_NUM-1) );

        if( bCheckParagraphs )
        {
            sal_uInt16 nParagraphs = (sal_uInt16)pParaList->GetParagraphCount();
            for ( sal_uInt16 nPara = 0; nPara < nParagraphs; nPara++ )
            {
                Paragraph* pPara = pParaList->GetParagraph( nPara );
                if( pPara && pPara->GetDepth() > nMaxDepth )
                {
                    SetDepth( pPara, nMaxDepth );
                }
            }
        }
    }
}

sal_Int16 Outliner::GetDepth( sal_uLong nPara ) const
{
    Paragraph* pPara = pParaList->GetParagraph( nPara );
    DBG_ASSERT( pPara, "Outliner::GetDepth - Paragraph not found!" );
    return pPara ? pPara->GetDepth() : -1;
}

void Outliner::SetDepth( Paragraph* pPara, sal_Int16 nNewDepth )
{
    DBG_CHKTHIS(Outliner,0);

    ImplCheckDepth( nNewDepth );

    if ( nNewDepth != pPara->GetDepth() )
    {
        nDepthChangedHdlPrevDepth = pPara->GetDepth();
        mnDepthChangeHdlPrevFlags = pPara->nFlags;
        pHdlParagraph = pPara;

        sal_uInt16 nPara = (sal_uInt16)GetAbsPos( pPara );
        ImplInitDepth( nPara, nNewDepth, sal_True );
        ImplCalcBulletText( nPara, sal_False, sal_False );

        if ( ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEOBJECT )
            ImplSetLevelDependendStyleSheet( nPara );

        DepthChangedHdl();
    }
}

sal_Int16 Outliner::GetNumberingStartValue( sal_uInt16 nPara )
{
    Paragraph* pPara = pParaList->GetParagraph( nPara );
    DBG_ASSERT( pPara, "Outliner::GetNumberingStartValue - Paragraph not found!" );
    return pPara ? pPara->GetNumberingStartValue() : -1;
}

void Outliner::SetNumberingStartValue( sal_uInt16 nPara, sal_Int16 nNumberingStartValue )
{
    Paragraph* pPara = pParaList->GetParagraph( nPara );
    DBG_ASSERT( pPara, "Outliner::GetNumberingStartValue - Paragraph not found!" );
    if( pPara && pPara->GetNumberingStartValue() != nNumberingStartValue )
    {
        if( IsUndoEnabled() && !IsInUndo() )
            InsertUndo( new OutlinerUndoChangeParaNumberingRestart( this, nPara,
                pPara->GetNumberingStartValue(), nNumberingStartValue,
                pPara->IsParaIsNumberingRestart(), pPara->IsParaIsNumberingRestart() ) );

        pPara->SetNumberingStartValue( nNumberingStartValue );
        // #i100014#
        // It is not a good idea to substract 1 from a count and cast the result
        // to USHORT without check, if the count is 0.
        ImplCheckParagraphs( nPara, (sal_uInt16) (pParaList->GetParagraphCount()) );
        pEditEngine->SetModified();
    }
}

sal_Bool Outliner::IsParaIsNumberingRestart( sal_uInt16 nPara )
{
    Paragraph* pPara = pParaList->GetParagraph( nPara );
    DBG_ASSERT( pPara, "Outliner::IsParaIsNumberingRestart - Paragraph not found!" );
    return pPara ? pPara->IsParaIsNumberingRestart() : sal_False;
}

void Outliner::SetParaIsNumberingRestart( sal_uInt16 nPara, sal_Bool bParaIsNumberingRestart )
{
    Paragraph* pPara = pParaList->GetParagraph( nPara );
    DBG_ASSERT( pPara, "Outliner::SetParaIsNumberingRestart - Paragraph not found!" );
    if( pPara && (pPara->IsParaIsNumberingRestart() != bParaIsNumberingRestart) )
    {
        if( IsUndoEnabled() && !IsInUndo() )
            InsertUndo( new OutlinerUndoChangeParaNumberingRestart( this, nPara,
                pPara->GetNumberingStartValue(), pPara->GetNumberingStartValue(),
                pPara->IsParaIsNumberingRestart(), bParaIsNumberingRestart ) );

        pPara->SetParaIsNumberingRestart( bParaIsNumberingRestart );
        // #i100014#
        // It is not a good idea to substract 1 from a count and cast the result
        // to USHORT without check, if the count is 0.
        ImplCheckParagraphs( nPara, (sal_uInt16) (pParaList->GetParagraphCount()) );
        pEditEngine->SetModified();
    }
}

OutlinerParaObject* Outliner::CreateParaObject( sal_uInt16 nStartPara, sal_uInt16 nCount ) const
{
    DBG_CHKTHIS(Outliner,0);

    if ( sal::static_int_cast< sal_uLong >( nStartPara + nCount ) >
         pParaList->GetParagraphCount() )
        nCount = sal::static_int_cast< sal_uInt16 >(
            pParaList->GetParagraphCount() - nStartPara );

    // When a new OutlinerParaObject is created because a paragraph is just beeing deleted,
    // it can happen that the ParaList is not updated yet...
    if ( ( nStartPara + nCount ) > pEditEngine->GetParagraphCount() )
        nCount = pEditEngine->GetParagraphCount() - nStartPara;

    if( !nCount )
        return NULL;

    EditTextObject* pText = pEditEngine->CreateTextObject( nStartPara, nCount );
    const bool bIsEditDoc(OUTLINERMODE_TEXTOBJECT == ImplGetOutlinerMode());
    ParagraphDataVector aParagraphDataVector(nCount);
    const sal_uInt16 nLastPara(nStartPara + nCount - 1);

    for(sal_uInt16 nPara(nStartPara); nPara <= nLastPara; nPara++)
    {
        aParagraphDataVector[nPara-nStartPara] = *GetParagraph(nPara);
    }

    OutlinerParaObject* pPObj = new OutlinerParaObject(*pText, aParagraphDataVector, bIsEditDoc);
    pPObj->SetOutlinerMode(GetMode());
    delete pText;

    return pPObj;
}

void Outliner::SetText( const XubString& rText, Paragraph* pPara )
{
    DBG_CHKTHIS(Outliner,0);
    DBG_ASSERT(pPara,"SetText:No Para");

    sal_Bool bUpdate = pEditEngine->GetUpdateMode();
    pEditEngine->SetUpdateMode( sal_False );
    ImplBlockInsertionCallbacks( sal_True );

    sal_uInt16 nPara = (sal_uInt16)pParaList->GetAbsPos( pPara );

    if( !rText.Len() )
    {
        pEditEngine->SetText( nPara, rText );
        ImplInitDepth( nPara, pPara->GetDepth(), sal_False );
    }
    else
    {
        XubString aText(convertLineEnd(rText, LINEEND_LF));

        if( aText.GetChar( aText.Len()-1 ) == '\x0A' )
            aText.Erase( aText.Len()-1, 1 ); // Delete the last break

        sal_uInt16 nCount = comphelper::string::getTokenCount(aText, '\x0A');
        sal_uInt16 nPos = 0;
        sal_uInt16 nInsPos = nPara+1;
        while( nCount > nPos )
        {
            XubString aStr = aText.GetToken( nPos, '\x0A' );

            sal_Int16 nCurDepth;
            if( nPos )
            {
                pPara = new Paragraph( -1 );
                nCurDepth = -1;
            }
            else
                nCurDepth = pPara->GetDepth();

            // In the outliner mode, filter the tabs and set the indentation
            // about a LRSpaceItem. In EditEngine mode intend over old tabs
            if( ( ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEOBJECT ) ||
                ( ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEVIEW ) )
            {
                // Extract Tabs
                sal_uInt16 nTabs = 0;
                while ( ( nTabs < aStr.Len() ) && ( aStr.GetChar( nTabs ) == '\t' ) )
                    nTabs++;
                if ( nTabs )
                    aStr.Erase( 0, nTabs );

                // Keep depth?  (see Outliner::Insert)
                if( !(pPara->nFlags & PARAFLAG_HOLDDEPTH) )
                {
                    nCurDepth = nTabs-1;
                    ImplCheckDepth( nCurDepth );
                    pPara->SetDepth( nCurDepth );
                    pPara->nFlags &= (~PARAFLAG_HOLDDEPTH);
                }
            }
            if( nPos ) // not with the first paragraph
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
            ImplInitDepth( nInsPos, nCurDepth, sal_False );
            nInsPos++;
            nPos++;
        }
    }

    DBG_ASSERT(pParaList->GetParagraphCount()==pEditEngine->GetParagraphCount(),"SetText failed!");
    bFirstParaIsEmpty = sal_False;
    ImplBlockInsertionCallbacks( sal_False );
    pEditEngine->SetUpdateMode( bUpdate );
}

// pView == 0 -> Ignore tabs

bool Outliner::ImpConvertEdtToOut( sal_uInt32 nPara,EditView* pView)
{
    DBG_CHKTHIS(Outliner,0);

    bool bConverted = false;
    sal_uInt16 nTabs = 0;
    ESelection aDelSel;

//  const SfxItemSet& rAttrs = pEditEngine->GetParaAttribs( (sal_uInt16)nPara );
//  bool bAlreadyOutliner = rAttrs.GetItemState( EE_PARA_OUTLLRSPACE ) == SFX_ITEM_ON ? true : false;

    XubString aName;
    XubString aHeading_US( RTL_CONSTASCII_USTRINGPARAM( "heading" ) );
    XubString aNumber_US( RTL_CONSTASCII_USTRINGPARAM( "Numbering" ) );

    XubString aStr( pEditEngine->GetText( (sal_uInt16)nPara ) );
    sal_Unicode* pPtr = (sal_Unicode*)aStr.GetBuffer();

    sal_uInt16 nHeadingNumberStart = 0;
    sal_uInt16 nNumberingNumberStart = 0;
    SfxStyleSheet* pStyle= pEditEngine->GetStyleSheet( (sal_uInt16)nPara );
    if( pStyle )
    {
        aName = pStyle->GetName();
        sal_uInt16 nSearch;
        if ( ( nSearch = aName.Search( aHeading_US ) ) != STRING_NOTFOUND )
            nHeadingNumberStart = nSearch + aHeading_US.Len();
        else if ( ( nSearch = aName.Search( aNumber_US ) ) != STRING_NOTFOUND )
            nNumberingNumberStart = nSearch + aNumber_US.Len();
    }

    if ( nHeadingNumberStart || nNumberingNumberStart )
    {
        // PowerPoint import ?
        if( nHeadingNumberStart && ( aStr.Len() >= 2 ) &&
                ( pPtr[0] != '\t' ) && ( pPtr[1] == '\t' ) )
        {
            // Extract Bullet and Tab
            aDelSel = ESelection( (sal_uInt16)nPara, 0, (sal_uInt16)nPara, 2 );
        }

        sal_uInt16 nPos = nHeadingNumberStart ? nHeadingNumberStart : nNumberingNumberStart;
        String aLevel = comphelper::string::stripStart(aName.Copy(nPos), ' ');
        nTabs = sal::static_int_cast< sal_uInt16 >(aLevel.ToInt32());
        if( nTabs )
            nTabs--; // Level 0 = "heading 1"
        bConverted = sal_True;
    }
    else
    {
        // filter leading tabs
        while( *pPtr == '\t' )
        {
            pPtr++;
            nTabs++;
        }
        // Remove tabs from the text
        if( nTabs )
            aDelSel = ESelection( (sal_uInt16)nPara, 0, (sal_uInt16)nPara, nTabs );
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

    const SfxInt16Item& rLevel = (const SfxInt16Item&) pEditEngine->GetParaAttrib( sal::static_int_cast< sal_uInt16 >(nPara), EE_PARA_OUTLLEVEL );
    sal_Int16 nOutlLevel = rLevel.GetValue();

    ImplCheckDepth( nOutlLevel );
    ImplInitDepth( sal::static_int_cast< sal_uInt16 >(nPara), nOutlLevel, sal_False );

    return bConverted;
}

void Outliner::SetText( const OutlinerParaObject& rPObj )
{
    DBG_CHKTHIS(Outliner,0);

    sal_Bool bUpdate = pEditEngine->GetUpdateMode();
    pEditEngine->SetUpdateMode( sal_False );

    sal_Bool bUndo = pEditEngine->IsUndoEnabled();
    EnableUndo( sal_False );

    Init( rPObj.GetOutlinerMode() );

    ImplBlockInsertionCallbacks( sal_True );
    pEditEngine->SetText(rPObj.GetTextObject());
    if( rPObj.Count() != pEditEngine->GetParagraphCount() )
    {
        int nop=0;nop++;
    }

    bFirstParaIsEmpty = sal_False;

    pParaList->Clear( sal_True );
    for( sal_uInt16 nCurPara = 0; nCurPara < rPObj.Count(); nCurPara++ )
    {
        Paragraph* pPara = new Paragraph( rPObj.GetParagraphData(nCurPara));
        ImplCheckDepth( pPara->nDepth );

        pParaList->Append(pPara);
        ImplCheckNumBulletItem( nCurPara );
    }

    // #i100014#
    // It is not a good idea to substract 1 from a count and cast the result
    // to USHORT without check, if the count is 0.
    ImplCheckParagraphs( 0, (sal_uInt16) (pParaList->GetParagraphCount()) );

    EnableUndo( bUndo );
    ImplBlockInsertionCallbacks( sal_False );
    pEditEngine->SetUpdateMode( bUpdate );

    DBG_ASSERT( pParaList->GetParagraphCount()==rPObj.Count(),"SetText failed");
    DBG_ASSERT( pEditEngine->GetParagraphCount()==rPObj.Count(),"SetText failed");
}

void Outliner::AddText( const OutlinerParaObject& rPObj )
{
    DBG_CHKTHIS(Outliner,0);
    Paragraph* pPara;

    sal_Bool bUpdate = pEditEngine->GetUpdateMode();
    pEditEngine->SetUpdateMode( sal_False );

    ImplBlockInsertionCallbacks( sal_True );
    sal_uLong nPara;
    if( bFirstParaIsEmpty )
    {
        pParaList->Clear( sal_True );
        pEditEngine->SetText(rPObj.GetTextObject());
        nPara = 0;
    }
    else
    {
        nPara = pParaList->GetParagraphCount();
        pEditEngine->InsertParagraph( EE_PARA_APPEND, rPObj.GetTextObject() );
    }
    bFirstParaIsEmpty = sal_False;

    for( sal_uInt16 n = 0; n < rPObj.Count(); n++ )
    {
        pPara = new Paragraph( rPObj.GetParagraphData(n) );
        pParaList->Append(pPara);
        sal_uInt16 nP = sal::static_int_cast< sal_uInt16 >(nPara+n);
        DBG_ASSERT(pParaList->GetAbsPos(pPara)==nP,"AddText:Out of sync");
        ImplInitDepth( nP, pPara->GetDepth(), sal_False );
    }
    DBG_ASSERT( pEditEngine->GetParagraphCount()==pParaList->GetParagraphCount(), "SetText: OutOfSync" );

    // #i100014#
    // It is not a good idea to substract 1 from a count and cast the result
    // to USHORT without check, if the count is 0.
    ImplCheckParagraphs( (sal_uInt16)nPara, (sal_uInt16) (pParaList->GetParagraphCount()) );

    ImplBlockInsertionCallbacks( sal_False );
    pEditEngine->SetUpdateMode( bUpdate );
}

void Outliner::FieldClicked( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos )
{
    DBG_CHKTHIS(Outliner,0);

    if ( aFieldClickedHdl.IsSet() )
    {
        EditFieldInfo aFldInfo( this, rField, nPara, nPos );
        aFldInfo.SetSimpleClick( sal_True );
        aFieldClickedHdl.Call( &aFldInfo );
    }
}


void Outliner::FieldSelected( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos )
{
    DBG_CHKTHIS(Outliner,0);
    if ( !aFieldClickedHdl.IsSet() )
        return;

    EditFieldInfo aFldInfo( this, rField, nPara, nPos );
    aFldInfo.SetSimpleClick( sal_False );
    aFieldClickedHdl.Call( &aFldInfo );
}


XubString Outliner::CalcFieldValue( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    DBG_CHKTHIS(Outliner,0);
    if ( !aCalcFieldValueHdl.IsSet() )
        return rtl::OUString( ' ' );

    EditFieldInfo aFldInfo( this, rField, nPara, nPos );
    // The FldColor is preset with COL_LIGHTGRAY.
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

void Outliner::SetStyleSheet( sal_uLong nPara, SfxStyleSheet* pStyle )
{
    DBG_CHKTHIS(Outliner,0);
    Paragraph* pPara = pParaList->GetParagraph( nPara );
        if (pPara)
        {
            pEditEngine->SetStyleSheet( (sal_uInt16)nPara, pStyle );
            pPara->nFlags |= PARAFLAG_SETBULLETTEXT;
            ImplCheckNumBulletItem( (sal_uInt16) nPara );
        }
}

void Outliner::ImplCheckNumBulletItem( sal_uInt16 nPara )
{
    Paragraph* pPara = pParaList->GetParagraph( nPara );
        if (pPara)
            pPara->aBulSize.Width() = -1;
}

void Outliner::ImplSetLevelDependendStyleSheet( sal_uInt16 nPara, SfxStyleSheet* pLevelStyle )
{
    DBG_CHKTHIS(Outliner,0);

    DBG_ASSERT( ( ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEOBJECT ) || ( ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEVIEW ), "SetLevelDependendStyleSheet: Wrong Mode!" );

    SfxStyleSheet* pStyle = pLevelStyle;
    if ( !pStyle )
        pStyle = GetStyleSheet( nPara );

    if ( pStyle )
    {
        sal_Int16 nDepth = GetDepth( nPara );
        if( nDepth < 0 )
            nDepth = 0;

        String aNewStyleSheetName( pStyle->GetName() );
        aNewStyleSheetName.Erase( aNewStyleSheetName.Len()-1, 1 );
        aNewStyleSheetName += String::CreateFromInt32( nDepth+1 );
        SfxStyleSheet* pNewStyle = (SfxStyleSheet*)GetStyleSheetPool()->Find( aNewStyleSheetName, pStyle->GetFamily() );
        DBG_ASSERT( pNewStyle, "AutoStyleSheetName - Style not found!" );
        if ( pNewStyle && ( pNewStyle != GetStyleSheet( nPara ) ) )
        {
             SfxItemSet aOldAttrs( GetParaAttribs( nPara ) );
            SetStyleSheet( nPara, pNewStyle );
            if ( aOldAttrs.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
            {
                SfxItemSet aAttrs( GetParaAttribs( nPara ) );
                aAttrs.Put( aOldAttrs.Get( EE_PARA_NUMBULLET ) );
                SetParaAttribs( nPara, aAttrs );
            }
        }
    }
}

void Outliner::ImplInitDepth( sal_uInt16 nPara, sal_Int16 nDepth, sal_Bool bCreateUndo, sal_Bool bUndoAction )
{
    DBG_CHKTHIS(Outliner,0);

    DBG_ASSERT( ( nDepth >= nMinDepth ) && ( nDepth <= nMaxDepth ), "ImplInitDepth - Depth is invalid!" );

    Paragraph* pPara = pParaList->GetParagraph( nPara );
        if (!pPara)
            return;
    sal_Int16 nOldDepth = pPara->GetDepth();
    pPara->SetDepth( nDepth );

    // For IsInUndo attributes and style do not have to be set, there
    // the old values are restored by the EditEngine.
    if( !IsInUndo() )
    {
        sal_Bool bUpdate = pEditEngine->GetUpdateMode();
        pEditEngine->SetUpdateMode( sal_False );

        sal_Bool bUndo = bCreateUndo && IsUndoEnabled();
        if ( bUndo && bUndoAction )
            UndoActionStart( OLUNDO_DEPTH );

        SfxItemSet aAttrs( pEditEngine->GetParaAttribs( nPara ) );
        aAttrs.Put( SfxInt16Item( EE_PARA_OUTLLEVEL, nDepth ) );
        pEditEngine->SetParaAttribs( nPara, aAttrs );
        ImplCheckNumBulletItem( nPara );
        ImplCalcBulletText( nPara, sal_False, sal_False );

        if ( bUndo )
        {
            InsertUndo( new OutlinerUndoChangeDepth( this, nPara, nOldDepth, nDepth ) );
            if ( bUndoAction )
                UndoActionEnd( OLUNDO_DEPTH );
        }

        pEditEngine->SetUpdateMode( bUpdate );
    }
}

void Outliner::SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rSet )
{
    DBG_CHKTHIS(Outliner,0);

    pEditEngine->SetParaAttribs( nPara, rSet );
}

sal_Bool Outliner::Expand( Paragraph* pPara )
{
    DBG_CHKTHIS(Outliner,0);

    if ( pParaList->HasHiddenChildren( pPara ) )
    {
        OLUndoExpand* pUndo = 0;
        sal_Bool bUndo = IsUndoEnabled() && !IsInUndo();
        if( bUndo )
        {
            UndoActionStart( OLUNDO_EXPAND );
            pUndo = new OLUndoExpand( this, OLUNDO_EXPAND );
            pUndo->pParas = 0;
            pUndo->nCount = (sal_uInt16)pParaList->GetAbsPos( pPara );
        }
        pHdlParagraph = pPara;
        bIsExpanding = sal_True;
        pParaList->Expand( pPara );
        ExpandHdl();
        InvalidateBullet( pPara, pParaList->GetAbsPos(pPara) );
        if( bUndo )
        {
            InsertUndo( pUndo );
            UndoActionEnd( OLUNDO_EXPAND );
        }
        return sal_True;
    }
    return sal_False;
}


sal_Bool Outliner::Collapse( Paragraph* pPara )
{
    DBG_CHKTHIS(Outliner,0);
    if ( pParaList->HasVisibleChildren( pPara ) ) // expanded
    {
        OLUndoExpand* pUndo = 0;
        sal_Bool bUndo = sal_False;

        if( !IsInUndo() && IsUndoEnabled() )
            bUndo = sal_True;
        if( bUndo )
        {
            UndoActionStart( OLUNDO_COLLAPSE );
            pUndo = new OLUndoExpand( this, OLUNDO_COLLAPSE );
            pUndo->pParas = 0;
            pUndo->nCount = (sal_uInt16)pParaList->GetAbsPos( pPara );
        }

        pHdlParagraph = pPara;
        bIsExpanding = sal_False;
        pParaList->Collapse( pPara );
        ExpandHdl();
        InvalidateBullet( pPara, pParaList->GetAbsPos(pPara) );
        if( bUndo )
        {
            InsertUndo( pUndo );
            UndoActionEnd( OLUNDO_COLLAPSE );
        }
        return sal_True;
    }
    return sal_False;
}


Font Outliner::ImpCalcBulletFont( sal_uInt16 nPara ) const
{
    const SvxNumberFormat* pFmt = GetNumberFormat( nPara );
    DBG_ASSERT( pFmt && ( pFmt->GetNumberingType() != SVX_NUM_BITMAP ) && ( pFmt->GetNumberingType() != SVX_NUM_NUMBER_NONE ), "ImpCalcBulletFont: Missing or BitmapBullet!" );

    Font aStdFont;
    if ( !pEditEngine->IsFlatMode() )
    {
        ESelection aSel( nPara, 0, nPara, 0 );
        aStdFont = EditEngine::CreateFontFromItemSet( pEditEngine->GetAttribs( aSel ), GetScriptType( aSel ) );
    }
    else
    {
        aStdFont = pEditEngine->GetStandardFont( nPara );
    }

    Font aBulletFont;
    if ( pFmt->GetNumberingType() == SVX_NUM_CHAR_SPECIAL )
    {
        aBulletFont = *pFmt->GetBulletFont();
    }
    else
    {
        aBulletFont = aStdFont;
        aBulletFont.SetUnderline( UNDERLINE_NONE );
        aBulletFont.SetOverline( UNDERLINE_NONE );
        aBulletFont.SetStrikeout( STRIKEOUT_NONE );
        aBulletFont.SetEmphasisMark( EMPHASISMARK_NONE );
        aBulletFont.SetRelief( RELIEF_NONE );
    }

    // Use original scale...
    sal_uInt16 nStretchX, nStretchY;
    const_cast<Outliner*>(this)->GetGlobalCharStretching(nStretchX, nStretchY);

    sal_uInt16 nScale = pFmt->GetBulletRelSize() * nStretchY / 100;
    sal_uLong nScaledLineHeight = aStdFont.GetSize().Height();
    nScaledLineHeight *= nScale*10;
    nScaledLineHeight /= 1000;

    aBulletFont.SetAlign( ALIGN_BOTTOM );
    aBulletFont.SetSize( Size( 0, nScaledLineHeight ) );
    sal_Bool bVertical = IsVertical();
    aBulletFont.SetVertical( bVertical );
    aBulletFont.SetOrientation( bVertical ? 2700 : 0 );

    Color aColor( COL_AUTO );
    if( !pEditEngine->IsFlatMode() && !( pEditEngine->GetControlWord() & EE_CNTRL_NOCOLORS ) )
    {
        aColor = pFmt->GetBulletColor();
    }

    if ( ( aColor == COL_AUTO ) || ( IsForceAutoColor() ) )
        aColor = pEditEngine->GetAutoColor();

    aBulletFont.SetColor( aColor );
    return aBulletFont;
}

void Outliner::PaintBullet( sal_uInt16 nPara, const Point& rStartPos,
    const Point& rOrigin, short nOrientation, OutputDevice* pOutDev )
{
    DBG_CHKTHIS(Outliner,0);

    bool bDrawBullet = false;
    if (pEditEngine)
    {
        const SfxBoolItem& rBulletState = (const SfxBoolItem&) pEditEngine->GetParaAttrib( nPara, EE_PARA_BULLETSTATE );
        bDrawBullet = rBulletState.GetValue() ? true : false;
    }

    if ( ImplHasBullet( nPara ) && bDrawBullet)
    {
        sal_Bool bVertical = IsVertical();

        sal_Bool bRightToLeftPara = pEditEngine->IsRightToLeft( nPara );

        Rectangle aBulletArea( ImpCalcBulletArea( nPara, sal_True, sal_False ) );
        sal_uInt16 nStretchX, nStretchY;
        GetGlobalCharStretching(nStretchX, nStretchY);
        aBulletArea = Rectangle( Point(aBulletArea.Left()*nStretchX/100,
                                       aBulletArea.Top()),
                                 Size(aBulletArea.GetWidth()*nStretchX/100,
                                      aBulletArea.GetHeight()) );

        Paragraph* pPara = pParaList->GetParagraph( nPara );
        const SvxNumberFormat* pFmt = GetNumberFormat( nPara );
        if ( pFmt && ( pFmt->GetNumberingType() != SVX_NUM_NUMBER_NONE ) )
        {
            if( pFmt->GetNumberingType() != SVX_NUM_BITMAP )
            {
                Font aBulletFont( ImpCalcBulletFont( nPara ) );
                // Use baseline
                sal_Bool bSymbol = pFmt->GetNumberingType() == SVX_NUM_CHAR_SPECIAL;
                aBulletFont.SetAlign( bSymbol ? ALIGN_BOTTOM : ALIGN_BASELINE );
                Font aOldFont = pOutDev->GetFont();
                pOutDev->SetFont( aBulletFont );

                ParagraphInfos  aParaInfos = pEditEngine->GetParagraphInfos( nPara );
                Point aTextPos;
                if ( !bVertical )
                {
//                  aTextPos.Y() = rStartPos.Y() + aBulletArea.Bottom();
                    aTextPos.Y() = rStartPos.Y() + ( bSymbol ? aBulletArea.Bottom() : aParaInfos.nFirstLineMaxAscent );
                    if ( !bRightToLeftPara )
                        aTextPos.X() = rStartPos.X() + aBulletArea.Left();
                    else
                        aTextPos.X() = rStartPos.X() + GetPaperSize().Width() - aBulletArea.Left();
                }
                else
                {
//                  aTextPos.X() = rStartPos.X() - aBulletArea.Bottom();
                    aTextPos.X() = rStartPos.X() - ( bSymbol ? aBulletArea.Bottom() : aParaInfos.nFirstLineMaxAscent );
                    aTextPos.Y() = rStartPos.Y() + aBulletArea.Left();
                }

                if ( nOrientation )
                {
                    // Both TopLeft and bottom left is not quite correct,
                    // since in EditEngine baseline ...
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
                    aRotatedFont.SetOrientation( nOrientation );
                    pOutDev->SetFont( aRotatedFont );
                }

                // VCL will take care of brackets and so on...
                sal_uLong nLayoutMode = pOutDev->GetLayoutMode();
                nLayoutMode &= ~(TEXT_LAYOUT_BIDI_RTL|TEXT_LAYOUT_COMPLEX_DISABLED|TEXT_LAYOUT_BIDI_STRONG);
                if ( bRightToLeftPara )
                    nLayoutMode |= TEXT_LAYOUT_BIDI_RTL;
                pOutDev->SetLayoutMode( nLayoutMode );

                if(bStrippingPortions)
                {
                    const Font aSvxFont(pOutDev->GetFont());
                    sal_Int32* pBuf = new sal_Int32[ pPara->GetText().getLength() ];
                    pOutDev->GetTextArray( pPara->GetText(), pBuf );

                    if(bSymbol)
                    {
                        // aTextPos is Bottom, go to Baseline
                        FontMetric aMetric(pOutDev->GetFontMetric());
                        aTextPos.Y() -= aMetric.GetDescent();
                    }

                    DrawingText(aTextPos, pPara->GetText(), 0, pPara->GetText().getLength(), pBuf,
                        aSvxFont, nPara, 0xFFFF, 0xFF, 0, 0, false, false, true, 0, Color(), Color());

                    delete[] pBuf;
                }
                else
                {
                    pOutDev->DrawText( aTextPos, pPara->GetText() );
                }

                pOutDev->SetFont( aOldFont );
            }
            else
            {
                if ( pFmt->GetBrush()->GetGraphicObject() )
                {
                    Point aBulletPos;
                    if ( !bVertical )
                    {
                        aBulletPos.Y() = rStartPos.Y() + aBulletArea.Top();
                        if ( !bRightToLeftPara )
                            aBulletPos.X() = rStartPos.X() + aBulletArea.Left();
                        else
                            aBulletPos.X() = rStartPos.X() + GetPaperSize().Width() - aBulletArea.Right();
                    }
                    else
                    {
                        aBulletPos.X() = rStartPos.X() - aBulletArea.Bottom();
                        aBulletPos.Y() = rStartPos.Y() + aBulletArea.Left();
                    }

                    if(bStrippingPortions)
                    {
                        if(aDrawBulletHdl.IsSet())
                        {
                            // call something analog to aDrawPortionHdl (if set) and feed it something
                            // analog to DrawPortionInfo...
                            // created aDrawBulletHdl, Set/GetDrawBulletHdl.
                            // created DrawBulletInfo and added handling to sdrtextdecomposition.cxx
                            DrawBulletInfo aDrawBulletInfo(
                                *pFmt->GetBrush()->GetGraphicObject(),
                                aBulletPos,
                                pPara->aBulSize);

                            aDrawBulletHdl.Call(&aDrawBulletInfo);
                        }
                    }
                    else
                    {
                        // Remove CAST when KA made the Draw-Method const
                        ((GraphicObject*)pFmt->GetBrush()->GetGraphicObject())->Draw( pOutDev, aBulletPos, pPara->aBulSize );
                    }
                }
            }
        }

        // In case of collapsed subparagraphs paint a line before the text.
        if( pParaList->HasChildren(pPara) && !pParaList->HasVisibleChildren(pPara) &&
                !bStrippingPortions && !nOrientation )
        {
            long nWidth = pOutDev->PixelToLogic( Size( 10, 0 ) ).Width();

            Point aStartPos, aEndPos;
            if ( !bVertical )
            {
                aStartPos.Y() = rStartPos.Y() + aBulletArea.Bottom();
                if ( !bRightToLeftPara )
                    aStartPos.X() = rStartPos.X() + aBulletArea.Right();
                else
                    aStartPos.X() = rStartPos.X() + GetPaperSize().Width() - aBulletArea.Left();
                aEndPos = aStartPos;
                aEndPos.X() += nWidth;
            }
            else
            {
                aStartPos.X() = rStartPos.X() - aBulletArea.Bottom();
                aStartPos.Y() = rStartPos.Y() + aBulletArea.Right();
                aEndPos = aStartPos;
                aEndPos.Y() += nWidth;
            }

            const Color& rOldLineColor = pOutDev->GetLineColor();
            pOutDev->SetLineColor( Color( COL_BLACK ) );
            pOutDev->DrawLine( aStartPos, aEndPos );
            pOutDev->SetLineColor( rOldLineColor );
        }
    }
}

void Outliner::InvalidateBullet( Paragraph* /*pPara*/, sal_uLong nPara )
{
    DBG_CHKTHIS(Outliner,0);

    long nLineHeight = (long)pEditEngine->GetLineHeight((sal_uInt16)nPara );
    for ( size_t i = 0, n = aViewList.size(); i < n; ++i )
    {
        OutlinerView* pView = aViewList[ i ];
        Point aPos( pView->pEditView->GetWindowPosTopLeft((sal_uInt16)nPara ) );
        Rectangle aRect( pView->GetOutputArea() );
        aRect.Right() = aPos.X();
        aRect.Top() = aPos.Y();
        aRect.Bottom() = aPos.Y();
        aRect.Bottom() += nLineHeight;

        pView->GetWindow()->Invalidate( aRect );
    }
}

sal_uLong Outliner::Read( SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    DBG_CHKTHIS(Outliner,0);

    sal_Bool bOldUndo = pEditEngine->IsUndoEnabled();
    EnableUndo( sal_False );

    sal_Bool bUpdate = pEditEngine->GetUpdateMode();
    pEditEngine->SetUpdateMode( sal_False );

    Clear();

    ImplBlockInsertionCallbacks( sal_True );
    sal_uLong nRet = pEditEngine->Read( rInput, rBaseURL, (EETextFormat)eFormat, pHTTPHeaderAttrs );

    bFirstParaIsEmpty = sal_False;

    sal_uInt16 nParas = pEditEngine->GetParagraphCount();
     pParaList->Clear( sal_True );
    sal_uInt16 n;
    for ( n = 0; n < nParas; n++ )
    {
        Paragraph* pPara = new Paragraph( 0 );
        pParaList->Append(pPara);

        if ( eFormat == EE_FORMAT_BIN )
        {
            const SfxItemSet& rAttrs = pEditEngine->GetParaAttribs( n );
            const SfxInt16Item& rLevel = (const SfxInt16Item&) rAttrs.Get( EE_PARA_OUTLLEVEL );
            sal_Int16 nDepth = rLevel.GetValue();
            ImplInitDepth( n, nDepth, sal_False );
        }
    }

    if ( eFormat != EE_FORMAT_BIN )
    {
        ImpFilterIndents( 0, nParas-1 );
    }

    ImplBlockInsertionCallbacks( sal_False );
    pEditEngine->SetUpdateMode( bUpdate );
    EnableUndo( bOldUndo );

    return nRet;
}


void Outliner::ImpFilterIndents( sal_uLong nFirstPara, sal_uLong nLastPara )
{
    DBG_CHKTHIS(Outliner,0);

    sal_Bool bUpdate = pEditEngine->GetUpdateMode();
    pEditEngine->SetUpdateMode( sal_False );

    Paragraph* pLastConverted = NULL;
    for( sal_uLong nPara = nFirstPara; nPara <= nLastPara; nPara++ )
    {
        Paragraph* pPara = pParaList->GetParagraph( nPara );
                if (pPara)
                {
                    if( ImpConvertEdtToOut( nPara ) )
                    {
                            pLastConverted = pPara;
                    }
                    else if ( pLastConverted )
                    {
                            // Arrange normal paragraphs below the heading ...
                            pPara->SetDepth( pLastConverted->GetDepth() );
                    }

                    ImplInitDepth( (sal_uInt16)nPara, pPara->GetDepth(), sal_False );
        }
    }

    pEditEngine->SetUpdateMode( bUpdate );
}

::svl::IUndoManager& Outliner::GetUndoManager()
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetUndoManager();
}

void Outliner::ImpTextPasted( sal_uLong nStartPara, sal_uInt16 nCount )
{
    DBG_CHKTHIS(Outliner,0);

    sal_Bool bUpdate = pEditEngine->GetUpdateMode();
    pEditEngine->SetUpdateMode( sal_False );

    const sal_uLong nStart = nStartPara;

    Paragraph* pPara = pParaList->GetParagraph( nStartPara );

    while( nCount && pPara )
    {
        if( ImplGetOutlinerMode() != OUTLINERMODE_TEXTOBJECT )
        {
            nDepthChangedHdlPrevDepth = pPara->GetDepth();
            mnDepthChangeHdlPrevFlags = pPara->nFlags;

            ImpConvertEdtToOut( nStartPara );

            pHdlParagraph = pPara;

            if( nStartPara == nStart )
            {
                // the existing paragraph has changed depth or flags
                if( (pPara->GetDepth() != nDepthChangedHdlPrevDepth) || (pPara->nFlags != mnDepthChangeHdlPrevFlags) )
                    DepthChangedHdl();
            }
        }
        else // EditEngine mode
        {
            sal_Int16 nDepth = -1;
            const SfxItemSet& rAttrs = pEditEngine->GetParaAttribs( (sal_uInt16)nStartPara );
            if ( rAttrs.GetItemState( EE_PARA_OUTLLEVEL ) == SFX_ITEM_ON )
            {
                const SfxInt16Item& rLevel = (const SfxInt16Item&) rAttrs.Get( EE_PARA_OUTLLEVEL );
                nDepth = rLevel.GetValue();
            }
            if ( nDepth != GetDepth( nStartPara ) )
                ImplInitDepth( (sal_uInt16)nStartPara, nDepth, sal_False );
        }

        nCount--;
        nStartPara++;
        pPara = pParaList->GetParagraph( nStartPara );
    }

    pEditEngine->SetUpdateMode( bUpdate );

    DBG_ASSERT(pParaList->GetParagraphCount()==pEditEngine->GetParagraphCount(),"ImpTextPasted failed");
}

long Outliner::IndentingPagesHdl( OutlinerView* pView )
{
    DBG_CHKTHIS(Outliner,0);
    if( !aIndentingPagesHdl.IsSet() )
        return 1;
    return aIndentingPagesHdl.Call( pView );
}

sal_Bool Outliner::ImpCanIndentSelectedPages( OutlinerView* pCurView )
{
    DBG_CHKTHIS(Outliner,0);
    // The selected pages must already be set in advance through
    // ImpCalcSelectedPages

    // If the first paragraph is on level 0 it can not indented in any case,
    // possible there might be indentations in the following on the 0 level.
    if ( ( mnFirstSelPage == 0 ) && ( ImplGetOutlinerMode() != OUTLINERMODE_TEXTOBJECT ) )
    {
        if ( nDepthChangedHdlPrevDepth == 1 )   // is the only page
            return sal_False;
        else
            pCurView->ImpCalcSelectedPages( sal_False );  // without the first
    }
    return (sal_Bool)IndentingPagesHdl( pCurView );
}


sal_Bool Outliner::ImpCanDeleteSelectedPages( OutlinerView* pCurView )
{
    DBG_CHKTHIS(Outliner,0);
    // The selected pages must already be set in advance through
    // ImpCalcSelectedPages
    return (sal_Bool)RemovingPagesHdl( pCurView );
}

Outliner::Outliner( SfxItemPool* pPool, sal_uInt16 nMode )
: nMinDepth( -1 )
{
    DBG_CTOR( Outliner, 0 );

    bStrippingPortions  = sal_False;
    bPasting            = sal_False;

    nFirstPage          = 1;
    bBlockInsCallback   = sal_False;

    nMaxDepth           = 9;

    pParaList = new ParagraphList;
    pParaList->SetVisibleStateChangedHdl( LINK( this, Outliner, ParaVisibleStateChangedHdl ) );
    Paragraph* pPara = new Paragraph( 0 );
    pParaList->Append(pPara);
    bFirstParaIsEmpty = sal_True;

    pEditEngine = new OutlinerEditEng( this, pPool );
    pEditEngine->SetBeginMovingParagraphsHdl( LINK( this, Outliner, BeginMovingParagraphsHdl ) );
    pEditEngine->SetEndMovingParagraphsHdl( LINK( this, Outliner, EndMovingParagraphsHdl ) );
    pEditEngine->SetBeginPasteOrDropHdl( LINK( this, Outliner, BeginPasteOrDropHdl ) );
    pEditEngine->SetEndPasteOrDropHdl( LINK( this, Outliner, EndPasteOrDropHdl ) );

    Init( nMode );
}

Outliner::~Outliner()
{
    DBG_DTOR(Outliner,0);

    pParaList->Clear( sal_True );
    delete pParaList;
    delete pEditEngine;
}

size_t Outliner::InsertView( OutlinerView* pView, size_t nIndex )
{
    DBG_CHKTHIS(Outliner,0);
    size_t ActualIndex;

    if ( nIndex >= aViewList.size() )
    {
        aViewList.push_back( pView );
        ActualIndex = aViewList.size() - 1;
    }
    else
    {
        ViewList::iterator it = aViewList.begin();
        advance( it, nIndex );
        ActualIndex = nIndex;
    }
    pEditEngine->InsertView(  pView->pEditView, (sal_uInt16)nIndex );
    return ActualIndex;
}

OutlinerView* Outliner::RemoveView( OutlinerView* pView )
{
    DBG_CHKTHIS(Outliner,0);

    for ( ViewList::iterator it = aViewList.begin(); it != aViewList.end(); ++it )
    {
        if ( *it == pView )
        {
            pView->pEditView->HideCursor(); // HACK
            pEditEngine->RemoveView(  pView->pEditView );
            aViewList.erase( it );
            break;
        }
    }
    return NULL;    // return superfluous
}

OutlinerView* Outliner::RemoveView( size_t nIndex )
{
    DBG_CHKTHIS(Outliner,0);

    EditView* pEditView = pEditEngine->GetView( (sal_uInt16)nIndex );
    pEditView->HideCursor(); // HACK

    pEditEngine->RemoveView( (sal_uInt16)nIndex );

    {
        ViewList::iterator it = aViewList.begin();
        advance( it, nIndex );
        aViewList.erase( it );
    }

    return NULL;    // return superfluous
}


OutlinerView* Outliner::GetView( size_t nIndex ) const
{
    DBG_CHKTHIS(Outliner,0);
    return ( nIndex >= aViewList.size() ) ? NULL : aViewList[ nIndex ];
}

size_t Outliner::GetViewCount() const
{
    DBG_CHKTHIS(Outliner,0);
    return aViewList.size();
}

void Outliner::ParagraphInsertedHdl()
{
    DBG_CHKTHIS(Outliner,0);
    if( !IsInUndo() )
        aParaInsertedHdl.Call( this );
}


void Outliner::ParagraphRemovingHdl()
{
    DBG_CHKTHIS(Outliner,0);
    if( !IsInUndo() )
        aParaRemovingHdl.Call( this );
}


void Outliner::DepthChangedHdl()
{
    DBG_CHKTHIS(Outliner,0);
    if( !IsInUndo() )
        aDepthChangedHdl.Call( this );
}


sal_uLong Outliner::GetAbsPos( Paragraph* pPara )
{
    DBG_CHKTHIS(Outliner,0);
    DBG_ASSERT(pPara,"GetAbsPos:No Para");
    return pParaList->GetAbsPos( pPara );
}

sal_uLong Outliner::GetParagraphCount() const
{
    DBG_CHKTHIS(Outliner,0);
    return pParaList->GetParagraphCount();
}

Paragraph* Outliner::GetParagraph( sal_uLong nAbsPos ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pParaList->GetParagraph( nAbsPos );
}

sal_Bool Outliner::HasChildren( Paragraph* pParagraph ) const
{
    DBG_CHKTHIS(Outliner,0);
    return pParaList->HasChildren( pParagraph );
}

sal_Bool Outliner::ImplHasBullet( sal_uInt16 nPara ) const
{
    return GetNumberFormat(nPara) != 0;
}

const SvxNumberFormat* Outliner::GetNumberFormat( sal_uInt16 nPara ) const
{
    const SvxNumberFormat* pFmt = NULL;

    Paragraph* pPara = pParaList->GetParagraph( nPara );
    if (pPara == NULL)
        return NULL;

    sal_Int16 nDepth = pPara? pPara->GetDepth() : -1;

    if( nDepth >= 0 )
    {
        const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&) pEditEngine->GetParaAttrib( nPara, EE_PARA_NUMBULLET );
        if ( rNumBullet.GetNumRule()->GetLevelCount() > nDepth )
            pFmt = rNumBullet.GetNumRule()->Get( nDepth );
    }

    return pFmt;
}

Size Outliner::ImplGetBulletSize( sal_uInt16 nPara )
{
    Paragraph* pPara = pParaList->GetParagraph( nPara );
        if (!pPara)
            return Size();

    if( pPara->aBulSize.Width() == -1 )
    {
        const SvxNumberFormat* pFmt = GetNumberFormat( nPara );
        DBG_ASSERT( pFmt, "ImplGetBulletSize - no Bullet!" );

        if ( pFmt->GetNumberingType() == SVX_NUM_NUMBER_NONE )
        {
            pPara->aBulSize = Size( 0, 0 );
        }
        else if( pFmt->GetNumberingType() != SVX_NUM_BITMAP )
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

void Outliner::ImplCheckParagraphs( sal_uInt16 nStart, sal_uInt16 nEnd )
{
    DBG_CHKTHIS( Outliner, 0 );

    // i100014#
    // assure that the following for-loop does not loop forever
    for ( sal_uInt16 n = nStart; n < nEnd; n++ )
    {
        Paragraph* pPara = pParaList->GetParagraph( n );
        if (pPara)
        {
            pPara->Invalidate();
            ImplCalcBulletText( n, sal_False, sal_False );
        }
    }
}

void Outliner::SetRefDevice( OutputDevice* pRefDev )
{
    DBG_CHKTHIS(Outliner,0);
    pEditEngine->SetRefDevice( pRefDev );
    for ( sal_uInt16 n = (sal_uInt16) pParaList->GetParagraphCount(); n; )
    {
        Paragraph* pPara = pParaList->GetParagraph( --n );
        pPara->Invalidate();
    }
}

void Outliner::ParaAttribsChanged( sal_uInt16 nPara )
{
    DBG_CHKTHIS(Outliner,0);

    // The Outliner does not have an undo of its own, when paragraphs are
    // separated/merged. When ParagraphInserted the attribute EE_PARA_OUTLLEVEL
    // may not be set, this is however needed when the depth of the paragraph
    // is to be determined.
    if( pEditEngine->IsInUndo() )
    {
        if ( pParaList->GetParagraphCount() == pEditEngine->GetParagraphCount() )
        {
            Paragraph* pPara = pParaList->GetParagraph( nPara );
            const SfxInt16Item& rLevel = (const SfxInt16Item&) pEditEngine->GetParaAttrib( nPara, EE_PARA_OUTLLEVEL );
            if ( pPara && pPara->GetDepth() != rLevel.GetValue() )
            {
                pPara->SetDepth( rLevel.GetValue() );
                ImplCalcBulletText( nPara, sal_True, sal_True );
            }
        }
    }
}

void Outliner::StyleSheetChanged( SfxStyleSheet* pStyle )
{
    DBG_CHKTHIS(Outliner,0);

    // The EditEngine calls StyleSheetChanged also for derived styles.
    // Here all the paragraphs, which had the said template, used to be
    // hunted by a ImpRecalcParaAttribs, why?
    // => only the Bullet-representation can really change...
    sal_uInt16 nParas = (sal_uInt16)pParaList->GetParagraphCount();
    for( sal_uInt16 nPara = 0; nPara < nParas; nPara++ )
    {
        if ( pEditEngine->GetStyleSheet( nPara ) == pStyle )
        {
            ImplCheckNumBulletItem( nPara );
            ImplCalcBulletText( nPara, sal_False, sal_False );
            // EditEngine formats changed paragraphs before calling this method,
            // so they are not reformatted now and use wrong bullet indent
            pEditEngine->QuickMarkInvalid( ESelection( nPara, 0, nPara, 0 ) );
        }
    }
}

Rectangle Outliner::ImpCalcBulletArea( sal_uInt16 nPara, sal_Bool bAdjust, sal_Bool bReturnPaperPos )
{
    // Bullet area within the paragraph ...
    Rectangle aBulletArea;

    const SvxNumberFormat* pFmt = GetNumberFormat( nPara );
    if ( pFmt )
    {
        Point aTopLeft;
        Size aBulletSize( ImplGetBulletSize( nPara ) );

        sal_Bool bOutlineMode = ( pEditEngine->GetControlWord() & EE_CNTRL_OUTLINER ) != 0;

        // the ODF attribut text:space-before which holds the spacing to add to the left of the label
        const short nSpaceBefore = pFmt->GetAbsLSpace() + pFmt->GetFirstLineOffset();

        const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&) pEditEngine->GetParaAttrib( nPara, bOutlineMode ? EE_PARA_OUTLLRSPACE : EE_PARA_LRSPACE );
        aTopLeft.X() = rLR.GetTxtLeft() + rLR.GetTxtFirstLineOfst() + nSpaceBefore;

        long nBulletWidth = Max( (long) -rLR.GetTxtFirstLineOfst(), (long) ((-pFmt->GetFirstLineOffset()) + pFmt->GetCharTextDistance()) );
        if ( nBulletWidth < aBulletSize.Width() )   // The Bullet creates its space
            nBulletWidth = aBulletSize.Width();

        if ( bAdjust && !bOutlineMode )
        {
            // Adjust when centered or align right
            const SvxAdjustItem& rItem = (const SvxAdjustItem&)pEditEngine->GetParaAttrib( nPara, EE_PARA_JUST );
            if ( ( !pEditEngine->IsRightToLeft( nPara ) && ( rItem.GetAdjust() != SVX_ADJUST_LEFT ) ) ||
                 ( pEditEngine->IsRightToLeft( nPara ) && ( rItem.GetAdjust() != SVX_ADJUST_RIGHT ) ) )
            {
                aTopLeft.X() = pEditEngine->GetFirstLineStartX( nPara ) - nBulletWidth;
            }
        }

        // Vertical:
        ParagraphInfos aInfos = pEditEngine->GetParagraphInfos( nPara );
        if ( aInfos.bValid )
        {
            aTopLeft.Y() = /* aInfos.nFirstLineOffset + */ // nFirstLineOffset is already added to the StartPos (PaintBullet) from the EditEngine
                            aInfos.nFirstLineHeight - aInfos.nFirstLineTextHeight
                            + aInfos.nFirstLineTextHeight / 2
                            - aBulletSize.Height() / 2;
            // may prefer to print out on the baseline ...
            if( ( pFmt->GetNumberingType() != SVX_NUM_NUMBER_NONE ) && ( pFmt->GetNumberingType() != SVX_NUM_BITMAP ) && ( pFmt->GetNumberingType() != SVX_NUM_CHAR_SPECIAL ) )
            {
                Font aBulletFont( ImpCalcBulletFont( nPara ) );
                if ( aBulletFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL )
                {
                    OutputDevice* pRefDev = pEditEngine->GetRefDevice();
                    Font aOldFont = pRefDev->GetFont();
                    pRefDev->SetFont( aBulletFont );
                    FontMetric aMetric( pRefDev->GetFontMetric() );
                    // Leading on the first line ...
                    aTopLeft.Y() = /* aInfos.nFirstLineOffset + */ aInfos.nFirstLineMaxAscent;
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

        if ( aTopLeft.X() < 0 )     // then push
            aTopLeft.X() = 0;

        aBulletArea = Rectangle( aTopLeft, aBulletSize );
    }
    if ( bReturnPaperPos )
    {
        Size aBulletSize( aBulletArea.GetSize() );
        Point aBulletDocPos( aBulletArea.TopLeft() );
        aBulletDocPos.Y() += pEditEngine->GetDocPosTopLeft( nPara ).Y();
        Point aBulletPos( aBulletDocPos );

        if ( IsVertical() )
        {
            aBulletPos.Y() = aBulletDocPos.X();
            aBulletPos.X() = GetPaperSize().Width() - aBulletDocPos.Y();
            // Rotate:
            aBulletPos.X() -= aBulletSize.Height();
            Size aSz( aBulletSize );
            aBulletSize.Width() = aSz.Height();
            aBulletSize.Height() = aSz.Width();
        }
        else if ( pEditEngine->IsRightToLeft( nPara ) )
        {
            aBulletPos.X() = GetPaperSize().Width() - aBulletDocPos.X() - aBulletSize.Width();
        }

        aBulletArea = Rectangle( aBulletPos, aBulletSize );
    }
    return aBulletArea;
}

void Outliner::ExpandHdl()
{
    DBG_CHKTHIS(Outliner,0);
    aExpandHdl.Call( this );
}

EBulletInfo Outliner::GetBulletInfo( sal_uInt16 nPara )
{
    EBulletInfo aInfo;

    aInfo.nParagraph = nPara;
    aInfo.bVisible = ImplHasBullet( nPara );

    const SvxNumberFormat* pFmt = GetNumberFormat( nPara );
    aInfo.nType = pFmt ? pFmt->GetNumberingType() : 0;

    if( pFmt )
    {
        if( pFmt->GetNumberingType() != SVX_NUM_BITMAP )
        {
            aInfo.aText = ImplGetBulletText( nPara );

            if( pFmt->GetBulletFont() )
                aInfo.aFont = *pFmt->GetBulletFont();
        }
        else if ( pFmt->GetBrush()->GetGraphicObject() )
        {
            aInfo.aGraphic = pFmt->GetBrush()->GetGraphicObject()->GetGraphic();
        }
    }

    if ( aInfo.bVisible )
    {
        aInfo.aBounds = ImpCalcBulletArea( nPara, sal_True, sal_True );
    }

    return aInfo;
}

XubString Outliner::GetText( Paragraph* pParagraph, sal_uLong nCount ) const
{
    DBG_CHKTHIS(Outliner,0);

    XubString aText;
    sal_uInt16 nStartPara = (sal_uInt16) pParaList->GetAbsPos( pParagraph );
    for ( sal_uInt16 n = 0; n < nCount; n++ )
    {
        aText += pEditEngine->GetText( nStartPara + n );
        if ( (n+1) < (sal_uInt16)nCount )
            aText += '\n';
    }
    return aText;
}

void Outliner::Remove( Paragraph* pPara, sal_uLong nParaCount )
{
    DBG_CHKTHIS(Outliner,0);

    sal_uLong nPos = pParaList->GetAbsPos( pPara );
    if( !nPos && ( nParaCount >= pParaList->GetParagraphCount() ) )
    {
        Clear();
    }
    else
    {
        for( sal_uInt16 n = 0; n < (sal_uInt16)nParaCount; n++ )
            pEditEngine->RemoveParagraph( (sal_uInt16) nPos );
    }
}

void Outliner::StripPortions()
{
    DBG_CHKTHIS(Outliner,0);
    bStrippingPortions = sal_True;
    pEditEngine->StripPortions();
    bStrippingPortions = sal_False;
}

void Outliner::DrawingText( const Point& rStartPos, const XubString& rText, sal_uInt16 nTextStart, sal_uInt16 nTextLen, const sal_Int32* pDXArray,const SvxFont& rFont,
    sal_uInt16 nPara, sal_uInt16 nIndex, sal_uInt8 nRightToLeft,
    const EEngineData::WrongSpellVector* pWrongSpellVector,
    const SvxFieldData* pFieldData,
    bool bEndOfLine,
    bool bEndOfParagraph,
    bool bEndOfBullet,
    const ::com::sun::star::lang::Locale* pLocale,
    const Color& rOverlineColor,
    const Color& rTextLineColor)
{
    DBG_CHKTHIS(Outliner,0);

    if(aDrawPortionHdl.IsSet())
    {
        DrawPortionInfo aInfo( rStartPos, rText, nTextStart, nTextLen, rFont, nPara, nIndex, pDXArray, pWrongSpellVector,
            pFieldData, pLocale, rOverlineColor, rTextLineColor, nRightToLeft, false, 0, bEndOfLine, bEndOfParagraph, bEndOfBullet);

        aDrawPortionHdl.Call( &aInfo );
    }
}

void Outliner::DrawingTab( const Point& rStartPos, long nWidth, const String& rChar, const SvxFont& rFont,
    sal_uInt16 nPara, xub_StrLen nIndex, sal_uInt8 nRightToLeft, bool bEndOfLine, bool bEndOfParagraph,
    const Color& rOverlineColor, const Color& rTextLineColor)
{
    if(aDrawPortionHdl.IsSet())
    {
        DrawPortionInfo aInfo( rStartPos, rChar, 0, rChar.Len(), rFont, nPara, nIndex, NULL, NULL,
            NULL, NULL, rOverlineColor, rTextLineColor, nRightToLeft, true, nWidth, bEndOfLine, bEndOfParagraph, false);

        aDrawPortionHdl.Call( &aInfo );
    }
}

long Outliner::RemovingPagesHdl( OutlinerView* pView )
{
    DBG_CHKTHIS(Outliner,0);
    return aRemovingPagesHdl.IsSet() ? aRemovingPagesHdl.Call( pView ) : sal_True;
}

sal_Bool Outliner::ImpCanDeleteSelectedPages( OutlinerView* pCurView, sal_uInt16 _nFirstPage, sal_uInt16 nPages )
{
    DBG_CHKTHIS(Outliner,0);

    nDepthChangedHdlPrevDepth = nPages;
    mnFirstSelPage = _nFirstPage;
    pHdlParagraph = 0;
    return (sal_Bool)RemovingPagesHdl( pCurView );
}

SfxItemSet Outliner::GetParaAttribs( sal_uInt16 nPara )
{
    DBG_CHKTHIS(Outliner,0);
    return pEditEngine->GetParaAttribs( nPara );
}

IMPL_LINK( Outliner, ParaVisibleStateChangedHdl, Paragraph*, pPara )
{
    DBG_CHKTHIS(Outliner,0);

    sal_uLong nPara = pParaList->GetAbsPos( pPara );
    pEditEngine->ShowParagraph( (sal_uInt16)nPara, pPara->IsVisible() );

    return 0;
}

IMPL_LINK_NOARG(Outliner, BeginMovingParagraphsHdl)
{
    DBG_CHKTHIS(Outliner,0);

    if( !IsInUndo() )
        GetBeginMovingHdl().Call( this );

    return 0;
}

IMPL_LINK( Outliner, BeginPasteOrDropHdl, PasteOrDropInfos*, pInfos )
{
    UndoActionStart( EDITUNDO_DRAGANDDROP );
    maBeginPasteOrDropHdl.Call(pInfos);
    return 0;
}

IMPL_LINK( Outliner, EndPasteOrDropHdl, PasteOrDropInfos*, pInfos )
{
    bPasting = sal_False;
    ImpTextPasted( pInfos->nStartPara, pInfos->nEndPara - pInfos->nStartPara + 1 );
    maEndPasteOrDropHdl.Call( pInfos );
    UndoActionEnd( EDITUNDO_DRAGANDDROP );
    return 0;
}

IMPL_LINK( Outliner, EndMovingParagraphsHdl, MoveParagraphsInfo*, pInfos )
{
    DBG_CHKTHIS(Outliner,0);

    pParaList->MoveParagraphs( pInfos->nStartPara, pInfos->nDestPara, pInfos->nEndPara - pInfos->nStartPara + 1 );
    sal_uInt16 nChangesStart = Min( pInfos->nStartPara, pInfos->nDestPara );
    sal_uInt16 nParas = (sal_uInt16)pParaList->GetParagraphCount();
    for ( sal_uInt16 n = nChangesStart; n < nParas; n++ )
        ImplCalcBulletText( n, sal_False, sal_False );

    if( !IsInUndo() )
        aEndMovingHdl.Call( this );

    return 0;
}

static bool isSameNumbering( const SvxNumberFormat& rN1, const SvxNumberFormat& rN2 )
{
    if( rN1.GetNumberingType() != rN2.GetNumberingType() )
        return false;

    if( rN1.GetNumStr(1) != rN2.GetNumStr(1) )
        return false;

    if( (rN1.GetPrefix() != rN2.GetPrefix()) || (rN1.GetSuffix() != rN2.GetSuffix()) )
        return false;

    return true;
}

sal_uInt16 Outliner::ImplGetNumbering( sal_uInt16 nPara, const SvxNumberFormat* pParaFmt )
{
    sal_uInt16 nNumber = pParaFmt->GetStart() - 1;

    Paragraph* pPara = pParaList->GetParagraph( nPara );
    const sal_Int16 nParaDepth = pPara->GetDepth();

    do
    {
        pPara = pParaList->GetParagraph( nPara );
        const sal_Int16 nDepth = pPara->GetDepth();

        // ignore paragraphs that are below our paragraph or have no numbering
        if( (nDepth > nParaDepth) || (nDepth == -1) )
            continue;

        // stop on paragraphs that are above our paragraph
        if( nDepth < nParaDepth )
            break;

        const SvxNumberFormat* pFmt = GetNumberFormat( nPara );

        if( pFmt == 0 )
            continue; // ignore paragraphs without bullets

        // check if numbering is the same
        if( !isSameNumbering( *pFmt, *pParaFmt ) )
            break;

        const SfxBoolItem& rBulletState = (const SfxBoolItem&) pEditEngine->GetParaAttrib( nPara, EE_PARA_BULLETSTATE );

        if( rBulletState.GetValue() )
            nNumber += 1;

        // same depth, same number format, check for restart
        const sal_Int16 nNumberingStartValue = pPara->GetNumberingStartValue();
        if( (nNumberingStartValue != -1) || pPara->IsParaIsNumberingRestart() )
        {
            if( nNumberingStartValue != -1 )
                nNumber += nNumberingStartValue - 1;
            break;
        }
    }
    while( nPara-- );

    return nNumber;
}

void Outliner::ImplCalcBulletText( sal_uInt16 nPara, sal_Bool bRecalcLevel, sal_Bool bRecalcChildren )
{
    DBG_CHKTHIS(Outliner,0);

    Paragraph* pPara = pParaList->GetParagraph( nPara );
    sal_uInt16 nRelPos = 0xFFFF;

    while ( pPara )
    {
        XubString aBulletText;
        const SvxNumberFormat* pFmt = GetNumberFormat( nPara );
        if( pFmt && ( pFmt->GetNumberingType() != SVX_NUM_BITMAP ) )
        {
            aBulletText += pFmt->GetPrefix();
            if( pFmt->GetNumberingType() == SVX_NUM_CHAR_SPECIAL )
            {
                aBulletText += pFmt->GetBulletChar();
            }
            else if( pFmt->GetNumberingType() != SVX_NUM_NUMBER_NONE )
            {
                aBulletText += pFmt->GetNumStr( ImplGetNumbering( nPara, pFmt ) );
            }
            aBulletText += pFmt->GetSuffix();
        }

        if (!pPara->GetText().equals(aBulletText))
            pPara->SetText( aBulletText );

        pPara->nFlags &= (~PARAFLAG_SETBULLETTEXT);

        if ( bRecalcLevel )
        {
            if ( nRelPos != 0xFFFF )
                nRelPos++;

            sal_Int16 nDepth = pPara->GetDepth();
            pPara = pParaList->GetParagraph( ++nPara );
            if ( !bRecalcChildren )
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
        ImplBlockInsertionCallbacks( sal_True );
        pEditEngine->Clear();
        pParaList->Clear( sal_True );
        pParaList->Append( new Paragraph( nMinDepth ));
        bFirstParaIsEmpty = sal_True;
        ImplBlockInsertionCallbacks( sal_False );
    }
    else
    {
            Paragraph* pPara = pParaList->GetParagraph( 0 );
            if(pPara)
                pPara->SetDepth( nMinDepth );
    }
}

void Outliner::SetFlatMode( sal_Bool bFlat )
{
    DBG_CHKTHIS(Outliner,0);

    if( bFlat != pEditEngine->IsFlatMode() )
    {
        for ( sal_uInt16 nPara = (sal_uInt16)pParaList->GetParagraphCount(); nPara; )
            pParaList->GetParagraph( --nPara )->aBulSize.Width() = -1;

        pEditEngine->SetFlatMode( bFlat );
    }
}

String Outliner::ImplGetBulletText( sal_uInt16 nPara )
{
        String aRes;
    Paragraph* pPara = pParaList->GetParagraph( nPara );
        if (pPara)
        {
    // Enable optimization again ...
//  if( pPara->nFlags & PARAFLAG_SETBULLETTEXT )
        ImplCalcBulletText( nPara, sal_False, sal_False );
                aRes = pPara->GetText();
        }
    return aRes;
}

// this is needed for StarOffice Api
void Outliner::SetLevelDependendStyleSheet( sal_uInt16 nPara )
{
    SfxItemSet aOldAttrs( pEditEngine->GetParaAttribs( nPara ) );
    ImplSetLevelDependendStyleSheet( nPara );
    pEditEngine->SetParaAttribs( nPara, aOldAttrs );
}

void Outliner::ImplBlockInsertionCallbacks( sal_Bool b )
{
    if ( b )
    {
        bBlockInsCallback++;
    }
    else
    {
        DBG_ASSERT( bBlockInsCallback, "ImplBlockInsertionCallbacks ?!" );
        bBlockInsCallback--;
        if ( !bBlockInsCallback )
        {
            // Call blocked notify events...
            while(!pEditEngine->aNotifyCache.empty())
            {
                EENotify aNotify(pEditEngine->aNotifyCache.front());
                // Remove from list before calling, maybe we enter LeaveBlockNotifications while calling the handler...
                pEditEngine->aNotifyCache.erase(pEditEngine->aNotifyCache.begin());
                pEditEngine->aOutlinerNotifyHdl.Call( &aNotify );
            }
        }
    }
}

IMPL_LINK( Outliner, EditEngineNotifyHdl, EENotify*, pNotify )
{
    if ( !bBlockInsCallback )
        pEditEngine->aOutlinerNotifyHdl.Call( pNotify );
    else
        pEditEngine->aNotifyCache.push_back(*pNotify);

    return 0;
}

/** sets a link that is called at the beginning of a drag operation at an edit view */
void Outliner::SetBeginDropHdl( const Link& rLink )
{
    pEditEngine->SetBeginDropHdl( rLink );
}

/** sets a link that is called at the end of a drag operation at an edit view */
void Outliner::SetEndDropHdl( const Link& rLink )
{
    pEditEngine->SetEndDropHdl( rLink );
}

/** sets a link that is called before a drop or paste operation. */
void Outliner::SetBeginPasteOrDropHdl( const Link& rLink )
{
    maBeginPasteOrDropHdl = rLink;
}

/** sets a link that is called after a drop or paste operation. */
void Outliner::SetEndPasteOrDropHdl( const Link& rLink )
{
    maEndPasteOrDropHdl = rLink;
}

void Outliner::SetParaFlag( Paragraph* pPara,  sal_uInt16 nFlag )
{
    if( pPara && !pPara->HasFlag( nFlag ) )
    {
        if( IsUndoEnabled() && !IsInUndo() )
            InsertUndo( new OutlinerUndoChangeParaFlags( this, (sal_uInt16)GetAbsPos( pPara ), pPara->nFlags, pPara->nFlags|nFlag ) );

        pPara->SetFlag( nFlag );
    }
}

bool Outliner::HasParaFlag( const Paragraph* pPara, sal_uInt16 nFlag ) const
{
    return pPara && pPara->HasFlag( nFlag );
}


sal_Bool DrawPortionInfo::IsRTL() const
{
    if(0xFF == mnBiDiLevel)
    {
        // Use Bidi functions from icu 2.0 to calculate if this portion
        // is RTL or not.
        UErrorCode nError(U_ZERO_ERROR);
        UBiDi* pBidi = ubidi_openSized(mrText.Len(), 0, &nError);
        nError = U_ZERO_ERROR;

        // I do not have this info here. Is it necessary? I'll have to ask MT.
        const sal_uInt8 nDefaultDir = UBIDI_LTR; //IsRightToLeft( nPara ) ? UBIDI_RTL : UBIDI_LTR;

        ubidi_setPara(pBidi, reinterpret_cast<const UChar *>(mrText.GetBuffer()), mrText.Len(), nDefaultDir, NULL, &nError);    // UChar != sal_Unicode in MinGW
        nError = U_ZERO_ERROR;

        int32_t nStart(0);
        int32_t nEnd;
        UBiDiLevel nCurrDir;

        ubidi_getLogicalRun(pBidi, nStart, &nEnd, &nCurrDir);

        ubidi_close(pBidi);

        // remember on-demand calculated state
        ((DrawPortionInfo*)this)->mnBiDiLevel = nCurrDir;
    }

    return (1 == (mnBiDiLevel % 2));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
