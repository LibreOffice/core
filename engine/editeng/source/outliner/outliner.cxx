/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <comphelper/kit.hxx>
#include <comphelper/string.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editdata.hxx>
#include <editeng/lrspitem.hxx>

#include <svl/style.hxx>
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <ParagraphPortionList.hxx>
#include <outlundo.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editstat.hxx>
#include <editeng/overflowingtxt.hxx>
#include <editeng/editobj.hxx>
#include <svl/itemset.hxx>
#include <vcl/metric.hxx>
#include <editeng/numitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/svxfont.hxx>
#include <editeng/brushitem.hxx>
#include <svl/itempool.hxx>
#include <libxml/xmlwriter.h>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <editeng/StripPortionsHelper.hxx>

#include <memory>


// Outliner


void Outliner::ImplCheckDepth( sal_Int16& rnDepth ) const
{
    if( rnDepth < gnMinDepth )
        rnDepth = gnMinDepth;
    else if( rnDepth > m_nMaxDepth )
        rnDepth = m_nMaxDepth;
}

sal_Int32 Outliner::Insert(const OUString& rText, sal_Int32 nAbsPos, sal_Int16 nDepth)
{
    sal_Int32 nPara;

    ImplCheckDepth( nDepth );

    sal_Int32 nParagraphCount = m_pEditEngine->GetParagraphCount();
    if( nAbsPos > nParagraphCount )
        nAbsPos = nParagraphCount;

    if( m_bFirstParaIsEmpty )
    {
        nPara = 0;
        sal_Int32 nNumberingDepth = m_pEditEngine->GetNumberingDepth(0);
        ParaFlag nPrevFlags = m_pEditEngine->GetParaFlag(0);
        if( nNumberingDepth != nDepth )
        {
            m_nDepthChangedHdlPrevDepth = nNumberingDepth;
            m_pEditEngine->SetNumberingDepth(0, nDepth);
            DepthChangedHdl(nPrevFlags, 0);
        }
        m_pEditEngine->ClearAndSetParaFlag(0, nPrevFlags | ParaFlag::HOLDDEPTH);
        SetText( rText, nPara );
    }
    else
    {
        bool bUpdate = m_pEditEngine->SetUpdateLayout( false );
        ImplBlockInsertionCallbacks( true );
        nPara = nAbsPos;
        m_pEditEngine->InsertParagraph( nAbsPos, OUString() );
        m_pEditEngine->SetNumberingDepth(nAbsPos, nDepth);
        ImplInitDepth( nAbsPos, nDepth, false );
        ParagraphInsertedHdl(nAbsPos);
        m_pEditEngine->SetParaFlag(nAbsPos, m_pEditEngine->GetParaFlag(nAbsPos) | ParaFlag::HOLDDEPTH);
        SetText( rText, nPara );
        ImplBlockInsertionCallbacks( false );
        m_pEditEngine->SetUpdateLayout( bUpdate );
    }
    m_bFirstParaIsEmpty = false;
    return nPara;
}


void Outliner::ParagraphInserted( sal_Int32 nPara )
{

    if ( m_nBlockInsCallback )
        return;

    if( m_bPasting || m_pEditEngine->IsInUndo() )
    {
        if( m_pEditEngine->IsInUndo() )
        {
            m_pEditEngine->SetBulletVisible(nPara, true);
            const SfxInt16Item& rLevel = m_pEditEngine->GetParaAttrib( nPara, EE_PARA_OUTLLEVEL );
            m_pEditEngine->SetNumberingDepth( nPara, rLevel.GetValue() );
        }
    }
    else
    {
        sal_Int16 nDepth = -1;
        if (nPara - 1 >= 0)
            nDepth = m_pEditEngine->GetNumberingDepth(nPara-1);

        m_pEditEngine->SetNumberingDepth( nPara, nDepth );

        if( !m_pEditEngine->IsInUndo() )
        {
            ImplCalcBulletText( nPara, true, false );
            ParagraphInsertedHdl(nPara);
        }
    }
}

/**
 * nPara is the index of the paragraph that __was__ deleted, so it actually
 * now indexes to the __next__ paragraph.
 */
void Outliner::ParagraphDeleted( sal_Int32 nPara, sal_Int16 nDepth, bool bIsPage )
{

    if (m_nBlockInsCallback || (nPara == EE_PARA_MAX))
        return;

    if( !m_pEditEngine->IsInUndo() )
    {
        m_aParaRemovingHdl.Call( { this, nPara, bIsPage } );
    }

    if( m_pEditEngine->IsInUndo() || m_bPasting )
        return;

    // is next paragraph of greater depth?
    if ( nPara < m_pEditEngine->GetParagraphCount() && m_pEditEngine->GetNumberingDepth(nPara) > nDepth )
    {
        ImplCalcBulletText( nPara, true, false );
        // Search for next on the this level ...
        while ( nPara < m_pEditEngine->GetParagraphCount() && m_pEditEngine->GetNumberingDepth(nPara) > nDepth )
            ++nPara;
    }

    if ( nPara < m_pEditEngine->GetParagraphCount() && ( m_pEditEngine->GetNumberingDepth(nPara) == nDepth ) )
        ImplCalcBulletText( nPara, true, false );
}

void Outliner::Init( OutlinerMode nMode )
{
    m_nOutlinerMode = nMode;

    Clear();

    EEControlBits nCtrl = m_pEditEngine->GetControlWord();
    nCtrl &= ~EEControlBits(EEControlBits::OUTLINER|EEControlBits::OUTLINER2);

    SetMaxDepth( 9 );

    switch ( GetOutlinerMode() )
    {
        case OutlinerMode::TextObject:
        case OutlinerMode::TitleObject:
            break;

        case OutlinerMode::OutlineObject:
            nCtrl |= EEControlBits::OUTLINER2;
            break;
        case OutlinerMode::OutlineView:
            nCtrl |= EEControlBits::OUTLINER;
            break;

        default: OSL_FAIL( "Outliner::Init - Invalid Mode!" );
    }

    m_pEditEngine->SetControlWord( nCtrl );
    m_pEditEngine->setTextFittingHeight(0);

    const bool bWasUndoEnabled(IsUndoEnabled());
    EnableUndo(false);
    ImplInitDepth( 0, -1, false );
    GetUndoManager().Clear();
    EnableUndo(bWasUndoEnabled);
}

void Outliner::SetMaxDepth( sal_Int16 nDepth )
{
    if( m_nMaxDepth != nDepth )
    {
        m_nMaxDepth = std::min( nDepth, sal_Int16(SVX_MAX_NUM-1) );
    }
}

sal_Int16 Outliner::GetDepth( sal_Int32 nPara ) const
{
    return m_pEditEngine->GetNumberingDepth(nPara);
}

void Outliner::SetDepth( sal_Int32 nPara, sal_Int16 nNewDepth )
{
    const sal_Int16 nOldDepth = m_pEditEngine->GetNumberingDepth(nPara);

    ImplCheckDepth( nNewDepth );

    if ( nNewDepth == nOldDepth )
        return;

    m_nDepthChangedHdlPrevDepth = nOldDepth;
    ParaFlag nPrevFlags = m_pEditEngine->GetParaFlag(nPara);

    ImplInitDepth( nPara, nNewDepth, true );
    ImplCalcBulletText( nPara, false, false );

    if ( GetOutlinerMode() == OutlinerMode::OutlineObject )
        ImplSetLevelDependentStyleSheet( nPara );

    DepthChangedHdl(nPrevFlags, nPara);
}

sal_Int16 Outliner::GetNumberingStartValue( sal_Int32 nPara ) const
{
    return m_pEditEngine->GetNumberingStartValue(nPara);
}

void Outliner::SetNumberingStartValue( sal_Int32 nPara, sal_Int16 nNumberingStartValue )
{
    if(  m_pEditEngine->GetNumberingStartValue(nPara) != nNumberingStartValue )
    {
        if( IsUndoEnabled() && !IsInUndo() )
            InsertUndo( std::make_unique<OutlinerUndoChangeParaNumberingRestart>( this, nPara,
                m_pEditEngine->GetNumberingStartValue(nPara), nNumberingStartValue,
                m_pEditEngine->IsNumberingRestart(nPara), m_pEditEngine->IsNumberingRestart(nPara) ) );

        m_pEditEngine->SetNumberingStartValue( nPara, nNumberingStartValue );
        ImplCheckParagraphs( nPara, m_pEditEngine->GetParagraphCount() );
        m_pEditEngine->SetModified();
    }
}

bool Outliner::IsParaIsNumberingRestart( sal_Int32 nPara ) const
{
    return m_pEditEngine->IsNumberingRestart(nPara);
}

void Outliner::SetParaIsNumberingRestart( sal_Int32 nPara, bool bParaIsNumberingRestart )
{
    if( m_pEditEngine->IsNumberingRestart(nPara) != bParaIsNumberingRestart )
    {
        if( IsUndoEnabled() && !IsInUndo() )
            InsertUndo( std::make_unique<OutlinerUndoChangeParaNumberingRestart>( this, nPara,
                m_pEditEngine->GetNumberingStartValue(nPara), m_pEditEngine->GetNumberingStartValue(nPara),
                m_pEditEngine->IsNumberingRestart(nPara), bParaIsNumberingRestart ) );

        m_pEditEngine->SetNumberingRestart( nPara, bParaIsNumberingRestart );
        ImplCheckParagraphs( nPara, m_pEditEngine->GetParagraphCount() );
        m_pEditEngine->SetModified();
    }
}

sal_Int32 Outliner::GetBulletsNumberingStatus(
    const sal_Int32 nParaStart,
    const sal_Int32 nParaEnd ) const
{
    if ( nParaStart > nParaEnd
         || nParaEnd >= m_pEditEngine->GetParagraphCount() )
    {
        SAL_WARN("editeng", "<Outliner::GetBulletsNumberingStatus> - unexpected parameter values" );
        return 2;
    }

    sal_Int32 nBulletsCount = 0;
    sal_Int32 nNumberingCount = 0;
    for (sal_Int32 nPara = nParaStart; nPara <= nParaEnd; ++nPara)
    {
        const SvxNumberFormat* pFmt = GetNumberFormat(nPara);
        if (!pFmt)
        {
            // At least, exists one paragraph that has no Bullets/Numbering.
            break;
        }
        else if ((pFmt->GetNumberingType() == SVX_NUM_BITMAP) || (pFmt->GetNumberingType() == SVX_NUM_CHAR_SPECIAL))
        {
            // Having Bullets in this paragraph.
            nBulletsCount++;
        }
        else
        {
            // Having Numbering in this paragraph.
            nNumberingCount++;
        }
    }

    const sal_Int32 nParaCount = nParaEnd - nParaStart + 1;
    if ( nBulletsCount == nParaCount )
    {
        return 0;
    }
    else if ( nNumberingCount == nParaCount )
    {
        return 1;
    }
    return 2;
}

sal_Int32 Outliner::GetBulletsNumberingStatus() const
{
    return m_pEditEngine->GetParagraphCount() > 0
           ? GetBulletsNumberingStatus( 0, m_pEditEngine->GetParagraphCount()-1 )
           : 2;
}

std::optional<OutlinerParaObject> Outliner::CreateParaObject( sal_Int32 nStartPara, sal_Int32 nCount ) const
{
    if ( static_cast<sal_uInt64>(nStartPara) + nCount >
            o3tl::make_unsigned(m_pEditEngine->GetParagraphCount()) )
        nCount = m_pEditEngine->GetParagraphCount() - nStartPara;

    // When a new OutlinerParaObject is created because a paragraph is just being deleted,
    // it can happen that the ParaList is not updated yet...
    if ( ( nStartPara + nCount ) > m_pEditEngine->GetParagraphCount() )
        nCount = m_pEditEngine->GetParagraphCount() - nStartPara;

    if (nCount <= 0)
        return std::nullopt;

    EditTextObject aText = m_pEditEngine->CreateTextObject( nStartPara, nCount );
    const sal_Int32 nLastPara(nStartPara + nCount - 1);

    for(sal_Int32 nPara(nStartPara); nPara <= nLastPara; nPara++)
    {
        aText.SetNumberingDepth(nPara - nStartPara, m_pEditEngine->GetNumberingDepth(nPara));
        aText.SetNumberingStartValue(nPara - nStartPara, m_pEditEngine->GetNumberingStartValue(nPara));
        aText.SetNumberingRestart(nPara - nStartPara, m_pEditEngine->IsNumberingRestart(nPara));
    }

    aText.ClearPortionInfo(); // tdf#147166 the PortionInfo is unwanted here
    OutlinerParaObject aPObj(std::move(aText));
    aPObj.SetOutlinerMode(GetOutlinerMode());

    return aPObj;
}

void Outliner::SetToEmptyText()
{
    SetText(GetEmptyParaObject());
}

void Outliner::SetText( const OUString& rText, sal_Int32 nPara )
{
    if (m_pEditEngine->GetText( nPara ) == rText)
    {
        // short-circuit logic to improve performance
        m_bFirstParaIsEmpty = false;
        return;
    }

    const bool bUpdate = m_pEditEngine->SetUpdateLayout( false );
    ImplBlockInsertionCallbacks( true );

    if (rText.isEmpty())
    {
        m_pEditEngine->SetText( nPara, rText );
        const sal_Int16 nDepth = m_pEditEngine->GetNumberingDepth(nPara);
        ImplInitDepth( nPara, nDepth, false );
    }
    else
    {
        const OUString aText(convertLineEnd(rText, LINEEND_LF));

        sal_Int32 nPos = 0;
        sal_Int32 nInsPos = nPara+1;
        sal_Int32 nIdx {0};
        // Loop over all tokens, but ignore the last one if empty
        // (i.e. if strings ends with the delimiter, detected by
        // checking nIdx against string length). This check also
        // handle empty strings.
        while( nIdx>=0 && nIdx<aText.getLength() )
        {
            std::u16string_view aStr = o3tl::getToken(aText, 0, '\x0A', nIdx );

            sal_Int16 nCurDepth;
            ParaFlag nFlag;
            if( nPos )
            {
                nCurDepth = -1;
                nFlag = ParaFlag::NONE;
            }
            else
            {
                nCurDepth = m_pEditEngine->GetNumberingDepth(nPara);
                nFlag = m_pEditEngine->GetParaFlag(nPara);
            }

            // In the outliner mode, filter the tabs and set the indentation
            // about a LRSpaceItem. In EditEngine mode intend over old tabs
            if( ( GetOutlinerMode() == OutlinerMode::OutlineObject ) ||
                ( GetOutlinerMode() == OutlinerMode::OutlineView ) )
            {
                // Extract Tabs
                size_t nTabs = 0;
                while ( ( nTabs < aStr.size() ) && ( aStr[nTabs] == '\t' ) )
                    nTabs++;
                if ( nTabs )
                    aStr = aStr.substr(nTabs);

                // Keep depth?  (see Outliner::Insert)
                if( !(nFlag & ParaFlag::HOLDDEPTH) )
                {
                    nCurDepth = nTabs-1; //TODO: sal_Int32 -> sal_Int16!
                    ImplCheckDepth( nCurDepth );
                }
            }
            if( nPos ) // not with the first paragraph
            {
                m_pEditEngine->InsertParagraph( nInsPos, OUString(aStr) );
                ParagraphInsertedHdl(nInsPos);
            }
            else
            {
                nInsPos--;
                m_pEditEngine->SetText( nInsPos, OUString(aStr) );
            }
            ImplInitDepth( nInsPos, nCurDepth, false );
            m_pEditEngine->SetNumberingDepth( nInsPos, nCurDepth );
            nInsPos++;
            nPos++;
        }
    }

    m_bFirstParaIsEmpty = false;
    ImplBlockInsertionCallbacks( false );
    // Restore the update mode.
    m_pEditEngine->SetUpdateLayout(bUpdate, /*bRestoring=*/true);
}

// pView == 0 -> Ignore tabs

bool Outliner::ImpConvertEdtToOut( sal_Int32 nPara )
{

    bool bConverted = false;
    sal_Int32 nTabs = 0;
    ESelection aDelSel;

    OUString aName;

    OUString aStr( m_pEditEngine->GetText( nPara ) );
    const sal_Unicode* pPtr = aStr.getStr();

    sal_Int32 nHeadingNumberStart = 0;
    sal_Int32 nNumberingNumberStart = 0;
    SfxStyleSheet* pStyle= m_pEditEngine->GetStyleSheet( nPara );
    if( pStyle )
    {
        OUString aHeading_US( u"heading"_ustr );
        OUString aNumber_US( u"Numbering"_ustr );
        aName = pStyle->GetName();
        sal_Int32 nSearch;
        if ( ( nSearch = aName.indexOf( aHeading_US ) ) != -1 )
            nHeadingNumberStart = nSearch + aHeading_US.getLength();
        else if ( ( nSearch = aName.indexOf( aNumber_US ) ) != -1 )
            nNumberingNumberStart = nSearch + aNumber_US.getLength();
    }

    if ( nHeadingNumberStart || nNumberingNumberStart )
    {
        // PowerPoint import ?
        if( nHeadingNumberStart && ( aStr.getLength() >= 2 ) &&
                ( pPtr[0] != '\t' ) && ( pPtr[1] == '\t' ) )
        {
            // Extract Bullet and Tab
            aDelSel = ESelection( nPara, 0, nPara, 2 );
        }

        sal_Int32 nPos = nHeadingNumberStart ? nHeadingNumberStart : nNumberingNumberStart;
        std::u16string_view aLevel = comphelper::string::stripStart(aName.subView(nPos), ' ');
        nTabs = o3tl::toInt32(aLevel);
        if( nTabs )
            nTabs--; // Level 0 = "heading 1"
        bConverted = true;
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
            aDelSel = ESelection( nPara, 0, nPara, nTabs );
    }

    if ( aDelSel.HasRange() )
    {
        m_pEditEngine->QuickDelete( aDelSel );
    }

    const SfxInt16Item& rLevel = m_pEditEngine->GetParaAttrib( nPara, EE_PARA_OUTLLEVEL );
    sal_Int16 nOutlLevel = rLevel.GetValue();

    ImplCheckDepth( nOutlLevel );
    ImplInitDepth( nPara, nOutlLevel, false );

    return bConverted;
}

void Outliner::SetText( const OutlinerParaObject& rPObj )
{
    bool bUpdate = m_pEditEngine->SetUpdateLayout( false );

    bool bUndo = m_pEditEngine->IsUndoEnabled();
    EnableUndo( false );

    Init( rPObj.GetOutlinerMode() );

    ImplBlockInsertionCallbacks( true );
    m_pEditEngine->SetText(rPObj.GetTextObject());

    m_bFirstParaIsEmpty = false;

    for( sal_Int32 nCurPara = 0; nCurPara < rPObj.Count(); nCurPara++ )
    {
        sal_Int16 nDepth = rPObj.GetNumberingDepth(nCurPara);
        ImplCheckDepth( nDepth );
        m_pEditEngine->SetNumberingDepth(nCurPara, nDepth);
        m_pEditEngine->SetNumberingStartValue(nCurPara, rPObj.GetNumberingStartValue(nCurPara));
        m_pEditEngine->SetNumberingRestart(nCurPara, rPObj.IsNumberingRestart(nCurPara));

        ImplCheckNumBulletItem( nCurPara );
    }

    ImplCheckParagraphs( 0, m_pEditEngine->GetParagraphCount() );

    EnableUndo( bUndo );
    ImplBlockInsertionCallbacks( false );
    m_pEditEngine->SetUpdateLayout( bUpdate );

    DBG_ASSERT( m_pEditEngine->GetParagraphCount()==rPObj.Count(),"SetText failed");
}

void Outliner::AddText( const OutlinerParaObject& rPObj, bool bAppend )
{
    bool bUpdate = m_pEditEngine->SetUpdateLayout( false );

    ImplBlockInsertionCallbacks( true );
    sal_Int32 nPara;
    if( m_bFirstParaIsEmpty )
    {
        m_pEditEngine->SetText(rPObj.GetTextObject());
        nPara = 0;
        bAppend = false;
    }
    else
    {
        nPara = m_pEditEngine->GetParagraphCount();
        m_pEditEngine->InsertParagraph(EE_PARA_MAX, rPObj.GetTextObject(), bAppend);
    }
    m_bFirstParaIsEmpty = false;

    for( sal_Int32 n = 0; n < rPObj.Count(); n++ )
    {
        if ( n == 0 && bAppend )
        {
            // This first "paragraph" was just appended to an existing (incomplete) paragraph.
            // Since no new paragraph will be added, the assumed increase-by-1 also won't happen.
            --nPara;
            continue;
        }

        const sal_Int16 nDepth = rPObj.GetNumberingDepth(n);
        sal_Int32 nP = nPara+n;
        m_pEditEngine->SetNumberingDepth(nP, nDepth);
        m_pEditEngine->SetNumberingStartValue(nP, rPObj.GetNumberingStartValue(n));
        m_pEditEngine->SetNumberingRestart(nP, rPObj.IsNumberingRestart(n));
        ImplInitDepth( nP, nDepth, false );
    }

    ImplCheckParagraphs( nPara, m_pEditEngine->GetParagraphCount() );

    ImplBlockInsertionCallbacks( false );
    m_pEditEngine->SetUpdateLayout( bUpdate );
}

OUString Outliner::CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, std::optional<Color>& rpTxtColor, std::optional<Color>& rpFldColor, std::optional<FontLineStyle>& rpFldLineStyle )
{
    if ( !maCalcFieldValueHdl.IsSet() && !maSdrCalcFieldValueHdl.IsSet() )
        return OUString( ' ' );

    EditFieldInfo aFldInfo( this, rField, nPara, nPos );
    // The FldColor is preset with COL_LIGHTGRAY.
    if ( rpFldColor )
        aFldInfo.SetFieldColor( *rpFldColor );

    if (maSdrCalcFieldValueHdl.IsSet())
        maSdrCalcFieldValueHdl.Call( &aFldInfo );

    // if the SdrOutliner handler did not generate a result, and the other handler exists, call it
    if (aFldInfo.GetRepresentation().isEmpty() && maCalcFieldValueHdl.IsSet())
        maCalcFieldValueHdl.Call( &aFldInfo );

    if ( aFldInfo.GetTextColor() )
    {
        rpTxtColor = *aFldInfo.GetTextColor();
    }

    if ( aFldInfo.GetFontLineStyle() )
    {
        rpFldLineStyle = *aFldInfo.GetFontLineStyle();
    }

    if (aFldInfo.GetFieldColor())
        rpFldColor = *aFldInfo.GetFieldColor();
    else
        rpFldColor.reset();

    return aFldInfo.GetRepresentation();
}

void Outliner::SetStyleSheet( sal_Int32 nPara, SfxStyleSheet* pStyle )
{
    m_pEditEngine->SetStyleSheet( nPara, pStyle );
    ImplCheckNumBulletItem(  nPara );
}

void Outliner::ImplCheckNumBulletItem( sal_Int32 nPara )
{
    m_pEditEngine->InvalidateBulletSize( nPara );
}

void Outliner::ImplSetLevelDependentStyleSheet( sal_Int32 nPara )
{

    DBG_ASSERT( ( GetOutlinerMode() == OutlinerMode::OutlineObject ) || ( GetOutlinerMode() == OutlinerMode::OutlineView ), "SetLevelDependentStyleSheet: Wrong Mode!" );

    SfxStyleSheet* pStyle = GetStyleSheet( nPara );

    if ( !pStyle )
        return;

    sal_Int16 nDepth = GetDepth( nPara );
    if( nDepth < 0 )
        nDepth = 0;

    OUString aNewStyleSheetName( pStyle->GetName() );
    aNewStyleSheetName = aNewStyleSheetName.subView( 0, aNewStyleSheetName.getLength()-1 ) +
        OUString::number( nDepth+1 );
    SfxStyleSheet* pNewStyle = static_cast<SfxStyleSheet*>(GetStyleSheetPool()->Find( aNewStyleSheetName, pStyle->GetFamily() ));
    DBG_ASSERT( pNewStyle, "AutoStyleSheetName - Style not found!" );
    if ( pNewStyle && ( pNewStyle != GetStyleSheet( nPara ) ) )
    {
        SfxItemSet aOldAttrs( GetParaAttribs( nPara ) );
        SetStyleSheet( nPara, pNewStyle );
        if ( aOldAttrs.GetItemState( EE_PARA_NUMBULLET ) == SfxItemState::SET )
        {
            SfxItemSet aAttrs( GetParaAttribs( nPara ) );
            aAttrs.Put( aOldAttrs.Get( EE_PARA_NUMBULLET ) );
            SetParaAttribs( nPara, aAttrs );
        }
    }
}

void Outliner::ImplInitDepth( sal_Int32 nPara, sal_Int16 nDepth, bool bCreateUndo )
{

    DBG_ASSERT( ( nDepth >= gnMinDepth ) && ( nDepth <= m_nMaxDepth ), "ImplInitDepth - Depth is invalid!" );

    sal_Int16 nOldDepth = m_pEditEngine->GetNumberingDepth(nPara);
    m_pEditEngine->SetNumberingDepth( nPara, nDepth );

    // For IsInUndo attributes and style do not have to be set, there
    // the old values are restored by the EditEngine.
    if( IsInUndo() )
        return;

    bool bUpdate = m_pEditEngine->SetUpdateLayout( false );

    bool bUndo = bCreateUndo && IsUndoEnabled();

    SfxItemSet aAttrs( m_pEditEngine->GetParaAttribs( nPara ) );
    aAttrs.Put( SfxInt16Item( EE_PARA_OUTLLEVEL, nDepth ) );
    m_pEditEngine->SetParaAttribs( nPara, aAttrs );
    ImplCheckNumBulletItem( nPara );
    ImplCalcBulletText( nPara, false, false );

    if ( bUndo )
    {
        InsertUndo( std::make_unique<OutlinerUndoChangeDepth>( this, nPara, nOldDepth, nDepth ) );
    }

    m_pEditEngine->SetUpdateLayout( bUpdate );
}

void Outliner::SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet )
{
    m_pEditEngine->SetParaAttribs( nPara, rSet );
}

void Outliner::SetCharAttribs(sal_Int32 nPara, const SfxItemSet& rSet)
{
    m_pEditEngine->SetCharAttribs(nPara, rSet);
}

bool Outliner::Expand( sal_Int32 nParentPos )
{
    if ( !HasHiddenChildren( nParentPos ) )
        return false;

    std::unique_ptr<OLUndoExpand> pUndo;
    bool bUndo = IsUndoEnabled() && !IsInUndo();
    if( bUndo )
    {
        UndoActionStart( OLUNDO_EXPAND );
        pUndo.reset( new OLUndoExpand( this, OLUNDO_EXPAND ) );
        pUndo->nCount = nParentPos;
    }

    sal_Int32 nChildCount = GetChildCount( nParentPos );
    for ( sal_Int32 n = 1; n <= nChildCount; n++  )
    {
        if ( !( m_pEditEngine->IsBulletVisible(nParentPos + n) ) )
        {
            m_pEditEngine->SetBulletVisible(nParentPos + n, true);
            m_pEditEngine->ShowParagraph(nParentPos + n, true);
        }
    }

    InvalidateBullet(nParentPos);
    if( bUndo )
    {
        InsertUndo( std::move(pUndo) );
        UndoActionEnd();
    }
    return true;
}

bool Outliner::Collapse( sal_Int32 nParentPos )
{
    if ( !HasVisibleChildren( nParentPos ) ) // collapsed
        return false;

    std::unique_ptr<OLUndoExpand> pUndo;
    bool bUndo = false;

    if( !IsInUndo() && IsUndoEnabled() )
        bUndo = true;
    if( bUndo )
    {
        UndoActionStart( OLUNDO_COLLAPSE );
        pUndo.reset( new OLUndoExpand( this, OLUNDO_COLLAPSE ) );
        pUndo->nCount = nParentPos;
    }

    sal_Int32 nChildCount = GetChildCount( nParentPos );
    for ( sal_Int32 n = 1; n <= nChildCount; n++  )
    {
        if ( m_pEditEngine->IsBulletVisible(nParentPos+n) )
        {
            m_pEditEngine->SetBulletVisible(nParentPos+n, false);
            m_pEditEngine->ShowParagraph(nParentPos + n, false);
        }
    }

    InvalidateBullet(nParentPos);
    if( bUndo )
    {
        InsertUndo( std::move(pUndo) );
        UndoActionEnd();
    }
    return true;
}

vcl::Font Outliner::ImpCalcBulletFont( sal_Int32 nPara ) const
{
    const SvxNumberFormat* pFmt = GetNumberFormat( nPara );
    assert(pFmt && "ImpCalcBulletFont: Missing!");
    DBG_ASSERT(( pFmt->GetNumberingType() != SVX_NUM_BITMAP ) && ( pFmt->GetNumberingType() != SVX_NUM_NUMBER_NONE ), "ImpCalcBulletFont: BitmapBullet!" );

    vcl::Font aStdFont;
    if ( !m_pEditEngine->IsFlatMode() )
    {
        ESelection aSel(nPara, 0);
        aStdFont = EditEngine::CreateFontFromItemSet( m_pEditEngine->GetAttribs( aSel ), m_pEditEngine->GetScriptType( aSel ) );
    }
    else
    {
        aStdFont = m_pEditEngine->GetStandardFont( nPara );
    }

    vcl::Font aBulletFont;
    std::optional<vcl::Font> pSourceFont;
    if ( pFmt->GetNumberingType() == SVX_NUM_CHAR_SPECIAL )
    {
        pSourceFont = pFmt->GetBulletFont();
    }

    if (pSourceFont)
    {
        aBulletFont = *pSourceFont;
    }
    else
    {
        aBulletFont = aStdFont;
        aBulletFont.SetUnderline( LINESTYLE_NONE );
        aBulletFont.SetOverline( LINESTYLE_NONE );
        aBulletFont.SetStrikeout( STRIKEOUT_NONE );
        aBulletFont.SetEmphasisMark( FontEmphasisMark::NONE );
        aBulletFont.SetRelief( FontRelief::NONE );
    }

    // Use original scale...

    double fFontScaleY = pFmt->GetBulletRelSize() / 100.0 * getScalingParameters().fFontY;
    double fScaledLineHeight = aStdFont.GetFontSize().Height() * fFontScaleY;

    aBulletFont.SetAlignment( ALIGN_BOTTOM );
    aBulletFont.SetFontSize(Size(0, basegfx::fround(fScaledLineHeight)));
    bool bVertical = IsVertical();
    aBulletFont.SetVertical( bVertical );
    aBulletFont.SetOrientation( Degree10(bVertical ? (IsTopToBottom() ? 2700 : 900) : 0) );

    Color aColor( COL_AUTO );
    if( !m_pEditEngine->IsFlatMode() && !( m_pEditEngine->GetControlWord() & EEControlBits::NOCOLORS ) )
    {
        aColor = pFmt->GetBulletColor();
    }

    // Vector rendering leaves a bullet in the automatic color, the same as the text.
    if (comphelper::COKit::isVectorRendering() && (aColor == COL_AUTO || IsForceAutoColor()))
        aColor = COL_AUTO;
    else if ( ( aColor == COL_AUTO ) || ( IsForceAutoColor() ) )
        aColor = m_pEditEngine->GetAutoColor();

    aBulletFont.SetColor( aColor );
    return aBulletFont;
}

void Outliner::StripBullet(
    sal_Int32 nPara, const Point& rStartPos, OutputDevice& rOutDev, StripPortionsHelper& rStripPortionsHelper)
{

    bool bDrawBullet = false;
    if (m_pEditEngine)
    {
        const SfxBoolItem& rBulletState = m_pEditEngine->GetParaAttrib( nPara, EE_PARA_BULLETSTATE );
        bDrawBullet = rBulletState.GetValue();
    }

    if (!(bDrawBullet && ImplHasNumberFormat(nPara)))
        return;

    bool bVertical = IsVertical();
    bool bTopToBottom = IsTopToBottom();

    bool bRightToLeftPara = m_pEditEngine->IsRightToLeft( nPara );

    tools::Rectangle aBulletArea( ImpCalcBulletArea( nPara, true, false ) );

    double fSpacingFactorX = getScalingParameters().fSpacingX;

    tools::Long nStretchBulletX = basegfx::fround<tools::Long>(double(aBulletArea.Left()) * fSpacingFactorX);
    tools::Long nStretchBulletWidth = basegfx::fround<tools::Long>(double(aBulletArea.GetWidth()) * fSpacingFactorX);
    aBulletArea = tools::Rectangle(Point(nStretchBulletX, aBulletArea.Top()),
                             Size(nStretchBulletWidth, aBulletArea.GetHeight()) );

    const SvxNumberFormat* pFmt = GetNumberFormat( nPara );
    if ( pFmt && ( pFmt->GetNumberingType() != SVX_NUM_NUMBER_NONE ) )
    {
        if( pFmt->GetNumberingType() != SVX_NUM_BITMAP )
        {
            vcl::Font aBulletFont( ImpCalcBulletFont( nPara ) );
            // Use baseline
            bool bSymbol = pFmt->GetNumberingType() == SVX_NUM_CHAR_SPECIAL;
            aBulletFont.SetAlignment( bSymbol ? ALIGN_BOTTOM : ALIGN_BASELINE );
            vcl::Font aOldFont = rOutDev.GetFont();
            rOutDev.SetFont( aBulletFont );

            ParagraphInfos  aParaInfos = m_pEditEngine->GetParagraphInfos( nPara );
            Point aTextPos;
            if ( !bVertical )
            {
//                  aTextPos.Y() = rStartPos.Y() + aBulletArea.Bottom();
                aTextPos.setY( rStartPos.Y() + ( bSymbol ? aBulletArea.Bottom() : aParaInfos.nFirstLineMaxAscent ) );
                if ( !bRightToLeftPara )
                    aTextPos.setX( rStartPos.X() + aBulletArea.Left() );
                else
                    aTextPos.setX( rStartPos.X() + GetPaperSize().Width() - aBulletArea.Right() );
            }
            else
            {
                if (bTopToBottom)
                {
//                      aTextPos.X() = rStartPos.X() - aBulletArea.Bottom();
                    aTextPos.setX( rStartPos.X() - (bSymbol ? aBulletArea.Bottom() : aParaInfos.nFirstLineMaxAscent) );
                    aTextPos.setY( rStartPos.Y() + aBulletArea.Left() );
                }
                else
                {
                    aTextPos.setX( rStartPos.X() + (bSymbol ? aBulletArea.Bottom() : aParaInfos.nFirstLineMaxAscent) );
                    aTextPos.setY( rStartPos.Y() + aBulletArea.Left() );
                }
            }

            // VCL will take care of brackets and so on...
            vcl::text::ComplexTextLayoutFlags nLayoutMode = rOutDev.GetLayoutMode();
            nLayoutMode &= ~vcl::text::ComplexTextLayoutFlags(vcl::text::ComplexTextLayoutFlags::BiDiRtl|vcl::text::ComplexTextLayoutFlags::BiDiStrong);
            if ( bRightToLeftPara )
                nLayoutMode |= vcl::text::ComplexTextLayoutFlags::BiDiRtl | vcl::text::ComplexTextLayoutFlags::TextOriginLeft | vcl::text::ComplexTextLayoutFlags::BiDiStrong;
            rOutDev.SetLayoutMode( nLayoutMode );

            const SvxFont aSvxFont(rOutDev.GetFont());
            KernArray aBuf;
            OUString sBulletText = m_pEditEngine->GetBulletText(nPara);
            rOutDev.GetTextArray( sBulletText, &aBuf );

            if(bSymbol)
            {
                // aTextPos is Bottom, go to Baseline
                FontMetric aMetric(rOutDev.GetFontMetric());
                aTextPos.AdjustY( -(aMetric.GetDescent()) );
            }

            const DrawPortionInfo aInfo(
                aTextPos, sBulletText, 0, sBulletText.getLength(), aBuf, {},
                aSvxFont, nPara, bRightToLeftPara ? 1 : 0, nullptr, nullptr, false, false, true, nullptr, Color(), Color());
            rStripPortionsHelper.processDrawPortionInfo(aInfo);

            rOutDev.SetFont( aOldFont );
        }
        else
        {
            if ( pFmt->GetBrush()->GetGraphicObject() )
            {
                Point aBulletPos;
                if ( !bVertical )
                {
                    aBulletPos.setY( rStartPos.Y() + aBulletArea.Top() );
                    if ( !bRightToLeftPara )
                        aBulletPos.setX( rStartPos.X() + aBulletArea.Left() );
                    else
                        aBulletPos.setX( rStartPos.X() + GetPaperSize().Width() - aBulletArea.Right() );
                }
                else
                {
                    if (bTopToBottom)
                    {
                        aBulletPos.setX( rStartPos.X() - aBulletArea.Bottom() );
                        aBulletPos.setY( rStartPos.Y() + aBulletArea.Left() );
                    }
                    else
                    {
                        aBulletPos.setX( rStartPos.X() + aBulletArea.Top() );
                        aBulletPos.setY( rStartPos.Y() - aBulletArea.Right() );
                    }
                }

                // call something analog to aDrawPortionHdl (if set) and feed it something
                // analog to DrawPortionInfo...
                // created aDrawBulletHdl, Set/GetDrawBulletHdl.
                // created DrawBulletInfo and added handling to sdrtextdecomposition.cxx
                DrawBulletInfo aDrawBulletInfo(
                    *pFmt->GetBrush()->GetGraphicObject(),
                    aBulletPos,
                    m_pEditEngine->GetBulletSize(nPara));
                rStripPortionsHelper.processDrawBulletInfo(aDrawBulletInfo);
            }
        }
    }
}

void Outliner::InvalidateBullet(sal_Int32 nPara)
{
    tools::Long nLineHeight = static_cast<tools::Long>(m_pEditEngine->GetLineHeight(nPara ));
    for (OutlinerView* pView : m_aViewList)
    {
        Point aPos( pView->pEditView->GetWindowPosTopLeft(nPara ) );
        tools::Rectangle aRect( pView->GetOutputArea() );
        aRect.SetRight( aPos.X() );
        aRect.SetTop( aPos.Y() );
        aRect.SetBottom( aPos.Y() );
        aRect.AdjustBottom(nLineHeight );

        pView->pEditView->InvalidateWindow(aRect);
    }
}

ErrCode Outliner::Read( SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat, SvKeyValueIterator* pHTTPHeaderAttrs )
{

    bool bOldUndo = m_pEditEngine->IsUndoEnabled();
    EnableUndo( false );

    bool bUpdate = m_pEditEngine->SetUpdateLayout( false );

    Clear();

    ImplBlockInsertionCallbacks( true );
    ErrCode nRet = m_pEditEngine->Read( rInput, rBaseURL, eFormat, pHTTPHeaderAttrs );

    m_bFirstParaIsEmpty = false;

    sal_Int32 nParas = m_pEditEngine->GetParagraphCount();
    for ( sal_Int32 n = 0; n < nParas; n++ )
    {
        m_pEditEngine->SetNumberingDepth(n, 0);
    }

    ImpFilterIndents( 0, nParas-1 );

    ImplBlockInsertionCallbacks( false );
    m_pEditEngine->SetUpdateLayout( bUpdate );
    EnableUndo( bOldUndo );

    return nRet;
}


void Outliner::ImpFilterIndents( sal_Int32 nFirstPara, sal_Int32 nLastPara )
{
    bool bUpdate = m_pEditEngine->SetUpdateLayout( false );

    sal_Int32 nLastConverted = -1;
    for( sal_Int32 nPara = nFirstPara; nPara <= nLastPara; nPara++ )
    {
        if( ImpConvertEdtToOut( nPara ) )
        {
            nLastConverted = nPara;
        }
        else if ( nLastConverted != -1 )
        {
            // Arrange normal paragraphs below the heading ...
            m_pEditEngine->SetNumberingDepth( nPara, m_pEditEngine->GetNumberingDepth(nLastConverted) );
        }

        ImplInitDepth( nPara, m_pEditEngine->GetNumberingDepth(nPara), false );
    }

    m_pEditEngine->SetUpdateLayout( bUpdate );
}

EditUndoManager& Outliner::GetUndoManager()
{
    return m_pEditEngine->GetUndoManager();
}

EditUndoManager* Outliner::SetUndoManager(EditUndoManager* pNew)
{
    return m_pEditEngine->SetUndoManager(pNew);
}

void Outliner::ImpTextPasted( sal_Int32 nStartPara, sal_Int32 nCount )
{
    bool bUpdate = m_pEditEngine->SetUpdateLayout( false );

    const sal_Int32 nStart = nStartPara;

    while( nCount )
    {
        if( GetOutlinerMode() != OutlinerMode::TextObject )
        {
            m_nDepthChangedHdlPrevDepth = m_pEditEngine->GetNumberingDepth(nStartPara);
            ParaFlag nPrevFlags = m_pEditEngine->GetParaFlag(nStartPara);

            ImpConvertEdtToOut( nStartPara );

            if( nStartPara == nStart )
            {
                // the existing paragraph has changed depth or flags
                if( (m_pEditEngine->GetNumberingDepth(nStartPara) != m_nDepthChangedHdlPrevDepth)
                    || (m_pEditEngine->GetParaFlag(nStartPara) != nPrevFlags) )
                    DepthChangedHdl(nPrevFlags, nStartPara);
            }
        }
        else // EditEngine mode
        {
            sal_Int16 nDepth = -1;
            const SfxItemSet& rAttrs = m_pEditEngine->GetParaAttribs( nStartPara );
            if ( rAttrs.GetItemState( EE_PARA_OUTLLEVEL ) == SfxItemState::SET )
            {
                const SfxInt16Item& rLevel = rAttrs.Get( EE_PARA_OUTLLEVEL );
                nDepth = rLevel.GetValue();
            }
            if ( nDepth != GetDepth( nStartPara ) )
                ImplInitDepth( nStartPara, nDepth, false );
        }

        nCount--;
        nStartPara++;
    }

    m_pEditEngine->SetUpdateLayout( bUpdate );
}

bool Outliner::IndentingPagesHdl( OutlinerView* pView )
{
    if( !m_aIndentingPagesHdl.IsSet() )
        return true;
    return m_aIndentingPagesHdl.Call( pView );
}

bool Outliner::ImpCanIndentSelectedPages( OutlinerView* pCurView )
{
    // The selected pages must already be set in advance through
    // ImpCalcSelectedPages

    // If the first paragraph is on level 0 it can not indented in any case,
    // possible there might be indentations in the following on the 0 level.
    if ( ( mnFirstSelPage == 0 ) && ( GetOutlinerMode() != OutlinerMode::TextObject ) )
    {
        if ( m_nDepthChangedHdlPrevDepth == 1 )   // is the only page
            return false;
        else
            (void)pCurView->ImpCalcSelectedPages( false );  // without the first
    }
    return IndentingPagesHdl( pCurView );
}


bool Outliner::ImpCanDeleteSelectedPages( OutlinerView* pCurView )
{
    // The selected pages must already be set in advance through
    // ImpCalcSelectedPages
    return RemovingPagesHdl( pCurView );
}

Outliner::Outliner(SfxItemPool* pPool, OutlinerMode nMode)
    : m_pEditEngine(new EditEngine(this, pPool))
    , mnFirstSelPage(0)
    , m_nDepthChangedHdlPrevDepth(0)
    , m_nMaxDepth(9)
    , m_bFirstParaIsEmpty(true)
    , m_nBlockInsCallback(0)
    , m_bPasting(false)
{
    m_pEditEngine->SetNumberingDepth(0, 0);

    m_pEditEngine->SetBeginMovingParagraphsHdl( LINK( this, Outliner, BeginMovingParagraphsHdl ) );
    m_pEditEngine->SetEndMovingParagraphsHdl( LINK( this, Outliner, EndMovingParagraphsHdl ) );
    m_pEditEngine->SetBeginPasteOrDropHdl( LINK( this, Outliner, BeginPasteOrDropHdl ) );
    m_pEditEngine->SetEndPasteOrDropHdl( LINK( this, Outliner, EndPasteOrDropHdl ) );

    Init( nMode );
}

Outliner::~Outliner()
{
    m_pEditEngine.reset();
}

size_t Outliner::InsertView( OutlinerView* pView, size_t nIndex )
{
    size_t nActualIndex = std::min( nIndex, m_aViewList.size() );
    m_aViewList.insert( m_aViewList.begin() + nActualIndex, pView );
    m_pEditEngine->InsertView( pView->pEditView.get(), nActualIndex );
    SAL_WARN_IF(m_aViewList.size() != m_pEditEngine->GetViewCount(), "editeng",
                "mismatch in view counts between outliner and editengine: " << m_aViewList.size()
                    << " vs " << m_pEditEngine->GetViewCount());
    return nActualIndex;
}

void Outliner::RemoveView( OutlinerView const * pView )
{
    ViewList::iterator it = std::find(m_aViewList.begin(), m_aViewList.end(), pView);
    if (it != m_aViewList.end())
    {
        pView->pEditView->HideCursor(); // HACK
        m_pEditEngine->RemoveView(  pView->pEditView.get() );
        m_aViewList.erase( it );
        SAL_WARN_IF(m_aViewList.size() != m_pEditEngine->GetViewCount(), "editeng",
                    "mismatch in view counts between outliner and editengine: "
                        << m_aViewList.size() << " vs " << m_pEditEngine->GetViewCount());
    }
}

OutlinerView* Outliner::GetView( size_t nIndex ) const
{
    return ( nIndex >= m_aViewList.size() ) ? nullptr : m_aViewList[ nIndex ];
}

size_t Outliner::GetViewCount() const
{
    return m_aViewList.size();
}

void Outliner::ParagraphInsertedHdl(sal_Int32 nParaPos)
{
    if( !IsInUndo() )
        m_aParaInsertedHdl.Call( { this, nParaPos, m_pEditEngine->HasParaFlag( nParaPos, ParaFlag::ISPAGE ) } );
}


void Outliner::DepthChangedHdl(ParaFlag nPrevFlags, sal_Int32 nParaPos)
{
    if( !IsInUndo() )
        m_aDepthChangedHdl.Call( { this, nPrevFlags, nParaPos } );
}


sal_Int32 Outliner::GetParagraphCount() const
{
    return m_pEditEngine->GetParagraphCount();
}

bool Outliner::HasChildren( sal_Int32 nPara ) const
{
    sal_Int32 nParaNext = nPara+1;
    if (nParaNext >= m_pEditEngine->GetParagraphCount())
        return false;
    return m_pEditEngine->GetNumberingDepth(nParaNext) > m_pEditEngine->GetNumberingDepth(nPara);
}

bool Outliner::ImplHasNumberFormat( sal_Int32 nPara ) const
{
    return GetNumberFormat(nPara) != nullptr;
}

const SvxNumberFormat* Outliner::GetNumberFormat( sal_Int32 nPara ) const
{
    const SvxNumberFormat* pFmt = nullptr;

    sal_Int16 nDepth = m_pEditEngine->GetNumberingDepth(nPara);

    if( nDepth >= 0 )
    {
        const SvxNumBulletItem& rNumBullet = m_pEditEngine->GetParaAttrib( nPara, EE_PARA_NUMBULLET );
        if ( rNumBullet.GetNumRule().GetLevelCount() > nDepth )
            pFmt = rNumBullet.GetNumRule().Get( nDepth );
    }

    return pFmt;
}

Size Outliner::ImplGetBulletSize( sal_Int32 nPara )
{
    auto aScalingParameters = getScalingParameters();
    Size aSize;

    if (m_pEditEngine->IsBulletInvalid(nPara, aScalingParameters))
    {
        const SvxNumberFormat* pFmt = GetNumberFormat( nPara );
        assert(pFmt && "ImplGetBulletSize - no Bullet!");
        if ( pFmt->GetNumberingType() == SVX_NUM_NUMBER_NONE )
        {
            aSize = Size(0, 0);
        }
        else if( pFmt->GetNumberingType() != SVX_NUM_BITMAP )
        {
            OUString aBulletText = ImplGetBulletText( nPara );
            OutputDevice* pRefDev = m_pEditEngine->GetRefDevice();
            vcl::Font aBulletFont( ImpCalcBulletFont( nPara ) );
            vcl::Font aRefFont( pRefDev->GetFont());
            pRefDev->SetFont( aBulletFont );
            tools::Long x = pRefDev->GetTextWidth(aBulletText);
            tools::Long y = pRefDev->GetTextHeight();
            aSize = Size(x, y);
            pRefDev->SetFont( aRefFont );
        }
        else
        {
            aSize = OutputDevice::LogicToLogic(pFmt->GetGraphicSize(),
                    MapMode(MapUnit::Map100thMM),
                    m_pEditEngine->GetRefDevice()->GetMapMode());
        }

        m_pEditEngine->SetBulletSize(nPara, aSize, aScalingParameters);
    }

    return m_pEditEngine->GetBulletSize(nPara);
}

void Outliner::ImplCheckParagraphs( sal_Int32 nStart, sal_Int32 nEnd )
{

    for ( sal_Int32 n = nStart; n < nEnd; n++ )
    {
        m_pEditEngine->InvalidateBulletSize(n);
        ImplCalcBulletText( n, false, false );
    }
}

void Outliner::SetRefDevice( OutputDevice* pRefDev )
{
    m_pEditEngine->SetRefDevice( pRefDev );
    for ( sal_Int32 n = m_pEditEngine->GetParagraphCount(); n; )
    {
        --n;
        m_pEditEngine->InvalidateBulletSize(n);
    }
}

void Outliner::ParaAttribsChanged( sal_Int32 nPara )
{
    // The Outliner does not have an undo of its own, when paragraphs are
    // separated/merged. When ParagraphInserted the attribute EE_PARA_OUTLLEVEL
    // may not be set, this is however needed when the depth of the paragraph
    // is to be determined.
    if (!m_pEditEngine->IsInUndo())
        return;
    // tdf#100734: force update of bullet
    m_pEditEngine->InvalidateBulletSize(nPara);
    const SfxInt16Item& rLevel = m_pEditEngine->GetParaAttrib( nPara, EE_PARA_OUTLLEVEL );
    if (m_pEditEngine->GetNumberingDepth(nPara) == rLevel.GetValue())
        return;
    m_pEditEngine->SetNumberingDepth(nPara, rLevel.GetValue());
    ImplCalcBulletText(nPara, true, true);
}

void Outliner::StyleSheetChanged( SfxStyleSheet const * pStyle )
{

    // The EditEngine calls StyleSheetChanged also for derived styles.
    // Here all the paragraphs, which had the said template, used to be
    // hunted by an ImpRecalcParaAttribs, why?
    // => only the Bullet-representation can really change...
    sal_Int32 nParas = m_pEditEngine->GetParagraphCount();
    for( sal_Int32 nPara = 0; nPara < nParas; nPara++ )
    {
        if ( m_pEditEngine->GetStyleSheet( nPara ) == pStyle )
        {
            ImplCheckNumBulletItem( nPara );
            ImplCalcBulletText( nPara, false, false );
            // EditEngine formats changed paragraphs before calling this method,
            // so they are not reformatted now and use wrong bullet indent
            m_pEditEngine->QuickMarkInvalid(ESelection(nPara, 0));
        }
    }
}

// The space that the first line of the paragraph keeps free for its label, measured from
// the top left corner of that line. The bullet or the number is drawn inside this space.
tools::Rectangle Outliner::ImpCalcLabelArea( sal_Int32 nPara, bool bAdjust )
{
    const SvxNumberFormat* pFmt = GetNumberFormat( nPara );
    if ( !pFmt )
        return tools::Rectangle();

    const Size aBulletSize( ImplGetBulletSize( nPara ) );

    bool bOutlineMode = bool( m_pEditEngine->GetControlWord() & EEControlBits::OUTLINER );

    // the ODF attribute text:space-before which holds the spacing to add to the left of the label
    const auto nSpaceBefore = pFmt->GetAbsLSpace() + pFmt->GetFirstLineOffset();

    const SvxLRSpaceItem& rLR = m_pEditEngine->GetParaAttrib(
        nPara, bOutlineMode ? EE_PARA_OUTLLRSPACE : EE_PARA_LRSPACE );
    tools::Long nLeft = rLR.ResolveTextLeft({}) + rLR.ResolveTextFirstLineOffset({}) + nSpaceBefore;

    tools::Long nWidth = std::max(
        static_cast<tools::Long>(-rLR.ResolveTextFirstLineOffset({})),
        static_cast<tools::Long>((-pFmt->GetFirstLineOffset()) + pFmt->GetCharTextDistance()));
    if ( nWidth < aBulletSize.Width() )   // The Bullet creates its space
        nWidth = aBulletSize.Width();

    if ( bAdjust && !bOutlineMode )
    {
        // Adjust when centered or align right
        const SvxAdjustItem& rItem = m_pEditEngine->GetParaAttrib( nPara, EE_PARA_JUST );
        if ( ( !m_pEditEngine->IsRightToLeft( nPara ) && ( rItem.GetAdjust() != SvxAdjust::Left ) ) ||
             ( m_pEditEngine->IsRightToLeft( nPara ) && ( rItem.GetAdjust() != SvxAdjust::Right ) ) )
        {
            nLeft = m_pEditEngine->GetFirstLineStartX( nPara ) - nWidth;
        }
    }

    // The label is as tall as the line it belongs to.
    ParagraphInfos aInfos = m_pEditEngine->GetParagraphInfos( nPara );
    tools::Long nHeight = aInfos.bValid ? aInfos.nFirstLineHeight : aBulletSize.Height();

    return tools::Rectangle( Point( nLeft, 0 ), Size( nWidth, nHeight ) );
}

// The area where the mouse points at the bullet or the number of the paragraph. It is a strip
// as tall as the first line around the drawn label, wider than the label on each side by a
// tenth of the line height. A bullet drawn at a fraction of the size of the text is easy to
// hit that way, at any zoom and font size. The rest of the space kept for the label is left
// to the text, so a click there puts the cursor in front of the text.
tools::Rectangle Outliner::ImpCalcBulletHitArea( sal_Int32 nPara )
{
    const SvxNumberFormat* pFmt = GetNumberFormat( nPara );
    if ( !pFmt || pFmt->GetNumberingType() == SVX_NUM_NUMBER_NONE )
        return tools::Rectangle();

    // With the bullet switched off the paragraph keeps the space but draws nothing in it,
    // so there is nothing to point at.
    const SfxBoolItem& rBulletState = m_pEditEngine->GetParaAttrib( nPara, EE_PARA_BULLETSTATE );
    if ( !rBulletState.GetValue() )
        return tools::Rectangle();

    const tools::Rectangle aLabelArea( ImpCalcLabelArea( nPara, true ) );
    const tools::Rectangle aBulletArea( ImpCalcBulletArea( nPara, true, false ) );
    const tools::Long nMargin = aLabelArea.GetHeight() / 10;
    // A bullet taller than its line is drawn above or below it, and the strip covers it there.
    const tools::Rectangle aHitArea( aBulletArea.Left() - nMargin,
                                     std::min( aLabelArea.Top(), aBulletArea.Top() ),
                                     aBulletArea.Right() + nMargin,
                                     std::max( aLabelArea.Bottom(), aBulletArea.Bottom() ) );

    return ImpConvertToPaperPos( nPara, aHitArea );
}

// Turn a rectangle that is measured from the top left corner of the first line of the
// paragraph into a rectangle on the paper.
tools::Rectangle Outliner::ImpConvertToPaperPos( sal_Int32 nPara,
                                                 const tools::Rectangle& rParaArea )
{
    Size aSize( rParaArea.GetSize() );
    Point aDocPos( rParaArea.TopLeft() );
    aDocPos.AdjustY(m_pEditEngine->GetDocPosTopLeft( nPara ).Y() );
    // The paragraph starts above its first line by the space that it keeps above itself.
    const ParaPortion* pParaPortion = m_pEditEngine->GetParaPortions().SafeGetObject( nPara );
    if ( pParaPortion )
        aDocPos.AdjustY( pParaPortion->GetFirstLineOffset() );
    Point aPaperPos( aDocPos );

    if ( IsVertical() )
    {
        aPaperPos.setY( aDocPos.X() );
        aPaperPos.setX( GetPaperSize().Width() - aDocPos.Y() );
        // Rotate:
        aPaperPos.AdjustX( -(aSize.Height()) );
        Size aSz( aSize );
        aSize.setWidth( aSz.Height() );
        aSize.setHeight( aSz.Width() );
    }
    else if ( m_pEditEngine->IsRightToLeft( nPara ) )
    {
        aPaperPos.setX( GetPaperSize().Width() - aDocPos.X() - aSize.Width() );
    }

    return tools::Rectangle( aPaperPos, aSize );
}

tools::Rectangle Outliner::ImpCalcBulletArea( sal_Int32 nPara, bool bAdjust, bool bReturnPaperPos )
{
    // Bullet area within the paragraph ...
    tools::Rectangle aBulletArea;

    const SvxNumberFormat* pFmt = GetNumberFormat( nPara );
    if ( pFmt )
    {
        Size aBulletSize( ImplGetBulletSize( nPara ) );

        const tools::Rectangle aLabelArea( ImpCalcLabelArea( nPara, bAdjust ) );
        Point aTopLeft( aLabelArea.Left(), 0 );
        const tools::Long nBulletWidth = aLabelArea.GetWidth();

        // Vertical:
        ParagraphInfos aInfos = m_pEditEngine->GetParagraphInfos( nPara );
        if ( aInfos.bValid )
        {
            aTopLeft.setY( /* aInfos.nFirstLineOffset + */ // nFirstLineOffset is already added to the StartPos (StripBullet) from the EditEngine
                            aInfos.nFirstLineHeight - aInfos.nFirstLineTextHeight
                            + aInfos.nFirstLineTextHeight / 2
                            - aBulletSize.Height() / 2 );
            // may prefer to print out on the baseline ...
            // A bullet character sits on the baseline of the first line, as in the reference
            // program, unless the extra line height is kept below the baseline.
            const bool bCharOnBaseline = pFmt->GetNumberingType() == SVX_NUM_CHAR_SPECIAL
                                         && !m_pEditEngine->isLineSpacingBelowBaseline();
            if( ( pFmt->GetNumberingType() != SVX_NUM_NUMBER_NONE ) && ( pFmt->GetNumberingType() != SVX_NUM_BITMAP ) && ( pFmt->GetNumberingType() != SVX_NUM_CHAR_SPECIAL || bCharOnBaseline ) )
            {
                vcl::Font aBulletFont( ImpCalcBulletFont( nPara ) );
                if ( aBulletFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL )
                {
                    OutputDevice* pRefDev = m_pEditEngine->GetRefDevice();
                    vcl::Font aOldFont = pRefDev->GetFont();
                    pRefDev->SetFont( aBulletFont );
                    FontMetric aMetric( pRefDev->GetFontMetric() );
                    // Leading on the first line ...
                    aTopLeft.setY( /* aInfos.nFirstLineOffset + */ aInfos.nFirstLineMaxAscent );
                    aTopLeft.AdjustY( -(aMetric.GetAscent()) );
                    pRefDev->SetFont( aOldFont );
                }
            }
        }

        // Horizontal:
        if( pFmt->GetNumAdjust() == SvxAdjust::Right )
        {
            aTopLeft.AdjustX(nBulletWidth - aBulletSize.Width() );
        }
        else if( pFmt->GetNumAdjust() == SvxAdjust::Center )
        {
            aTopLeft.AdjustX(( nBulletWidth - aBulletSize.Width() ) / 2 );
        }

        if ( aTopLeft.X() < 0 )     // then push
            aTopLeft.setX( 0 );

        aBulletArea = tools::Rectangle( aTopLeft, aBulletSize );
    }
    if ( bReturnPaperPos )
        aBulletArea = ImpConvertToPaperPos( nPara, aBulletArea );
    return aBulletArea;
}

EBulletInfo Outliner::GetBulletInfo( sal_Int32 nPara )
{
    EBulletInfo aInfo;

    aInfo.nParagraph = nPara;
    aInfo.bVisible = ImplHasNumberFormat( nPara );

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
    }

    if ( aInfo.bVisible )
    {
        aInfo.aBounds = ImpCalcBulletArea( nPara, true, true );
    }

    return aInfo;
}

OUString Outliner::GetText( sal_Int32 nStartPara, sal_Int32 nCount ) const
{

    OUStringBuffer aText(128);
    for ( sal_Int32 n = 0; n < nCount; n++ )
    {
        aText.append(m_pEditEngine->GetText( nStartPara + n ));
        if ( (n+1) < nCount )
            aText.append("\n");
    }
    return aText.makeStringAndClear();
}

void Outliner::Remove( sal_Int32 nPos, sal_Int32 nParaCount )
{
    if( !nPos && ( nParaCount >= m_pEditEngine->GetParagraphCount() ) )
    {
        Clear();
    }
    else
    {
        for( sal_Int32 n = 0; n < nParaCount; n++ )
            m_pEditEngine->RemoveParagraph( nPos );
    }
}

void Outliner::StripPortions(StripPortionsHelper& rStripPortionsHelper)
{
    m_pEditEngine->StripPortions(rStripPortionsHelper);
}

bool Outliner::RemovingPagesHdl( OutlinerView* pView )
{
    return !m_aRemovingPagesHdl.IsSet() || m_aRemovingPagesHdl.Call( pView );
}

bool Outliner::ImpCanDeleteSelectedPages( OutlinerView* pCurView, sal_Int32 _nFirstPage, sal_Int32 nPages )
{

    m_nDepthChangedHdlPrevDepth = nPages;
    mnFirstSelPage = _nFirstPage;
    return RemovingPagesHdl( pCurView );
}

SfxItemSet const & Outliner::GetParaAttribs( sal_Int32 nPara ) const
{
    return m_pEditEngine->GetParaAttribs( nPara );
}

IMPL_LINK_NOARG(Outliner, BeginMovingParagraphsHdl, MoveParagraphsInfo&, void)
{
    if( !IsInUndo() )
        m_aBeginMovingHdl.Call( this );
}

IMPL_LINK( Outliner, BeginPasteOrDropHdl, PasteOrDropInfos&, rInfos, void )
{
    UndoActionStart( EDITUNDO_DRAGANDDROP );
    maBeginPasteOrDropHdl.Call(&rInfos);
}

IMPL_LINK( Outliner, EndPasteOrDropHdl, PasteOrDropInfos&, rInfos, void )
{
    m_bPasting = false;
    ImpTextPasted( rInfos.nStartPara, rInfos.nEndPara - rInfos.nStartPara + 1 );
    maEndPasteOrDropHdl.Call( &rInfos );
    UndoActionEnd();
}

IMPL_LINK( Outliner, EndMovingParagraphsHdl, MoveParagraphsInfo&, rInfos, void )
{
    sal_Int32 nChangesStart = std::min( rInfos.nStartPara, rInfos.nDestPara );
    sal_Int32 nParas = m_pEditEngine->GetParagraphCount();
    for ( sal_Int32 n = nChangesStart; n < nParas; n++ )
        ImplCalcBulletText( n, false, false );

    if( !IsInUndo() )
        m_aEndMovingHdl.Call( MoveParagraphsHdlParam{this, rInfos.nStartPara, rInfos.nEndPara, rInfos.nDestPara} );
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

sal_uInt16 Outliner::ImplGetNumbering( sal_Int32 nPara, const SvxNumberFormat* pParaFmt )
{
    sal_uInt16 nNumber = pParaFmt->GetStart() - 1;

    const sal_Int16 nParaDepth = m_pEditEngine->GetNumberingDepth(nPara);

    do
    {
        const sal_Int16 nDepth = m_pEditEngine->GetNumberingDepth(nPara);

        // ignore paragraphs that are below our paragraph or have no numbering
        if( (nDepth > nParaDepth) || (nDepth == -1) )
            continue;

        // stop on paragraphs that are above our paragraph
        if( nDepth < nParaDepth )
            break;

        const SvxNumberFormat* pFmt = GetNumberFormat( nPara );

        if( pFmt == nullptr )
            continue; // ignore paragraphs without bullets

        // check if numbering less than or equal to pParaFmt
        if( !isSameNumbering( *pFmt, *pParaFmt ) || ( pFmt->GetStart() < pParaFmt->GetStart() ) )
            break;

        if (  pFmt->GetStart() > pParaFmt->GetStart() )
        {
           nNumber += pFmt->GetStart() - pParaFmt->GetStart();
           pParaFmt = pFmt;
        }

        const SfxBoolItem& rBulletState = m_pEditEngine->GetParaAttrib( nPara, EE_PARA_BULLETSTATE );

        if( rBulletState.GetValue() )
            nNumber += 1;

        // same depth, same number format, check for restart
        const sal_Int16 nNumberingStartValue = m_pEditEngine->GetNumberingStartValue(nPara);
        if( (nNumberingStartValue != -1) || m_pEditEngine->IsNumberingRestart(nPara) )
        {
            if( nNumberingStartValue != -1 )
                nNumber += nNumberingStartValue - 1;
            break;
        }
    }
    while( nPara-- );

    return nNumber;
}

void Outliner::ImplCalcBulletText( sal_Int32 nPara, bool bRecalcLevel, bool bRecalcChildren )
{
    while ( nPara < m_pEditEngine->GetParagraphCount() )
    {
        OUString aBulletText;
        const SvxNumberFormat* pFmt = GetNumberFormat( nPara );
        if( pFmt && ( pFmt->GetNumberingType() != SVX_NUM_BITMAP ) )
        {
            aBulletText += pFmt->GetPrefix();
            if( pFmt->GetNumberingType() == SVX_NUM_CHAR_SPECIAL )
            {
                sal_UCS4 cChar = pFmt->GetBulletChar();
                aBulletText += OUString(&cChar, 1);
            }
            else if( pFmt->GetNumberingType() != SVX_NUM_NUMBER_NONE )
            {
                aBulletText += pFmt->GetNumStr( ImplGetNumbering( nPara, pFmt ) );
            }
            aBulletText += pFmt->GetSuffix();
        }

        if (m_pEditEngine->GetBulletText(nPara) != aBulletText)
            m_pEditEngine->SetBulletText( nPara, aBulletText );

        if ( bRecalcLevel )
        {
            sal_Int16 nDepth = m_pEditEngine->GetNumberingDepth(nPara);
            ++nPara;
            if ( !bRecalcChildren )
            {
                while ( nPara < m_pEditEngine->GetParagraphCount() && ( m_pEditEngine->GetNumberingDepth(nPara) > nDepth ) )
                    ++nPara;
            }

            if ( nPara < m_pEditEngine->GetParagraphCount() && ( m_pEditEngine->GetNumberingDepth(nPara) < nDepth ) )
                nPara = m_pEditEngine->GetParagraphCount();
        }
        else
        {
            nPara = m_pEditEngine->GetParagraphCount();
        }
    }
}

void Outliner::Clear()
{

    if( !m_bFirstParaIsEmpty )
    {
        ImplBlockInsertionCallbacks( true );
        m_pEditEngine->Clear();
        m_pEditEngine->SetNumberingDepth(0, gnMinDepth);
        m_bFirstParaIsEmpty = true;
        ImplBlockInsertionCallbacks( false );
    }
    else
    {
        m_pEditEngine->SetNumberingDepth( 0, gnMinDepth );
    }
}

void Outliner::SetFlatMode( bool bFlat )
{

    if( bFlat != m_pEditEngine->IsFlatMode() )
    {
        for ( sal_Int32 nPara = m_pEditEngine->GetParagraphCount(); nPara; )
            m_pEditEngine->InvalidateBulletSize(--nPara);

        m_pEditEngine->SetFlatMode( bFlat );
    }
}

OUString Outliner::ImplGetBulletText( sal_Int32 nPara )
{
    OUString aRes;
    ImplCalcBulletText( nPara, false, false );
    aRes = m_pEditEngine->GetBulletText(nPara);
    return aRes;
}

// this is needed for StarOffice Api
void Outliner::SetLevelDependentStyleSheet( sal_Int32 nPara )
{
    SfxItemSet aOldAttrs( m_pEditEngine->GetParaAttribs( nPara ) );
    ImplSetLevelDependentStyleSheet( nPara );
    m_pEditEngine->SetParaAttribs( nPara, aOldAttrs );
}

void Outliner::ImplBlockInsertionCallbacks( bool b )
{
    if ( b )
    {
        m_nBlockInsCallback++;
    }
    else
    {
        DBG_ASSERT( m_nBlockInsCallback, "ImplBlockInsertionCallbacks ?!" );
        m_nBlockInsCallback--;
        if ( !m_nBlockInsCallback )
        {
            // Call blocked notify events...
            while(!m_aNotifyCache.empty())
            {
                EENotify aNotify(m_aNotifyCache.front());
                // Remove from list before calling, maybe we enter LeaveBlockNotifications while calling the handler...
                m_aNotifyCache.erase(m_aNotifyCache.begin());
                m_aOutlinerNotifyHdl.Call( aNotify );
            }
        }
    }
}

IMPL_LINK( Outliner, EditEngineNotifyHdl, EENotify&, rNotify, void )
{
    if ( !m_nBlockInsCallback )
        m_aOutlinerNotifyHdl.Call( rNotify );
    else
        m_aNotifyCache.push_back(rNotify);
}

/** sets a link that is called at the beginning of a drag operation at an edit view */
void Outliner::SetBeginDropHdl( const Link<EditView*,void>& rLink )
{
    m_pEditEngine->SetBeginDropHdl( rLink );
}

/** sets a link that is called at the end of a drag operation at an edit view */
void Outliner::SetEndDropHdl( const Link<EditView*,void>& rLink )
{
    m_pEditEngine->SetEndDropHdl( rLink );
}

/** sets a link that is called before a drop or paste operation. */
void Outliner::SetBeginPasteOrDropHdl( const Link<PasteOrDropInfos*,void>& rLink )
{
    maBeginPasteOrDropHdl = rLink;
}

/** sets a link that is called after a drop or paste operation. */
void Outliner::SetEndPasteOrDropHdl( const Link<PasteOrDropInfos*,void>& rLink )
{
    maEndPasteOrDropHdl = rLink;
}

void Outliner::SetParaFlag( sal_Int32 nPara,  ParaFlag nFlag )
{
    if( !m_pEditEngine->HasParaFlag( nPara, nFlag ) )
    {
        if( IsUndoEnabled() && !IsInUndo() )
        {
            ParaFlag nFlags = m_pEditEngine->GetParaFlag(nPara);
            InsertUndo( std::make_unique<OutlinerUndoChangeParaFlags>( this, nPara,
                            nFlags, nFlags|nFlag ) );
        }

        m_pEditEngine->SetParaFlag( nPara, nFlag );
    }
}

bool Outliner::IsPageOverflow()
{
    return m_pEditEngine->IsPageOverflow();
}

std::optional<NonOverflowingText> Outliner::GetNonOverflowingText() const
{
    /* XXX:
     * nCount should be the number of paragraphs of the non overflowing text
     * nStart should be the starting paragraph of the non overflowing text (XXX: Always 0?)
    */

    if ( GetParagraphCount() < 1 )
        return {};

    // last non-overflowing paragraph is before the first overflowing one
    sal_Int32 nCount = m_pEditEngine->GetOverflowingParaNum();
    sal_Int32 nOverflowLine = m_pEditEngine->GetOverflowingLineNum(); // XXX: Unused for now

    // Defensive check: overflowing para index beyond actual # of paragraphs?
    if ( nCount > GetParagraphCount()-1) {
        SAL_INFO("editeng.chaining",
                 "[Overflowing] Ops, trying to retrieve para "
                 << nCount << " when max index is " << GetParagraphCount()-1 );
        return {};
    }

    if (nCount < 0)
    {
        SAL_INFO("editeng.chaining",
                 "[Overflowing] No Overflowing text but GetNonOverflowinText called?!");
        return {};
    }

    // NOTE: We want the selection of the overflowing text from here
    //       At the same time we may want to consider the beginning of such text
    //       in a more fine grained way (i.e. as GetNonOverflowingText did)

/*
    sal_Int32 nHeadPara = pEditEngine->GetOverflowingParaNum();
    sal_uInt32 nParaCount = GetParagraphCount();

    sal_uInt32 nLen = 0;
    for ( sal_Int32 nLine = 0;
          nLine < pEditEngine->GetOverflowingLineNum();
          nLine++) {
        nLen += GetLineLen(nHeadPara, nLine);
    }

    sal_uInt32 nOverflowingPara = pEditEngine->GetOverflowingParaNum();
    ESelection aOverflowingTextSel;
    sal_Int32 nLastPara = nParaCount-1;
    sal_Int32 nLastParaLen = GetText(GetParagraph(nLastPara)).getLength();
    aOverflowingTextSel = ESelection(nOverflowingPara, nLen,
                                     nLastPara, nLastParaLen);
    bool bLastParaInterrupted =
            pEditEngine->GetOverflowingLineNum() > 0;

    return new NonOverflowingText(aOverflowingTextSel, bLastParaInterrupted);
    **/


    // Only overflowing text, i.e. 1st line of 1st paragraph overflowing
    bool bItAllOverflew = nCount == 0 && nOverflowLine == 0;
    if ( bItAllOverflew )
    {
        ESelection aEmptySel;
        //EditTextObject *pTObj = pEditEngine->CreateTextObject(aEmptySel);
        bool const bLastParaInterrupted = true; // Last Para was interrupted since everything overflew
        return NonOverflowingText(aEmptySel, bLastParaInterrupted);
    } else { // Get the lines that of the overflowing para fit in the box

        sal_Int32 nOverflowingPara = nCount;
        sal_uInt32 nLen = 0;

        for ( sal_Int32 nLine = 0;
              nLine < m_pEditEngine->GetOverflowingLineNum();
              nLine++)
        {
            nLen += GetLineLen(nOverflowingPara, nLine);
        }

        //sal_Int32 nStartPara = 0;
        //sal_Int32 nStartPos = 0;
        ESelection aOverflowingTextSelection;

        const sal_Int32 nEndPara = GetParagraphCount()-1;
        const sal_Int32 nEndPos = m_pEditEngine->GetTextLen(nEndPara);

        if (nLen == 0) {
            // XXX: What happens inside this case might be dependent on the joining paragraph or not-thingy
            // Overflowing paragraph is empty or first line overflowing: it's not "Non-Overflowing" text then
            sal_Int32 nParaLen = GetText(nOverflowingPara-1).getLength();
            aOverflowingTextSelection =
                ESelection(nOverflowingPara-1, nParaLen, nEndPara, nEndPos);
        } else {
            // We take until we have to from the overflowing paragraph
            aOverflowingTextSelection =
                ESelection(nOverflowingPara, nLen, nEndPara, nEndPos);
        }
        //EditTextObject *pTObj = pEditEngine->CreateTextObject(aNonOverflowingTextSelection);

        //sal_Int32 nLastLine = GetLineCount(nOverflowingPara)-1;
        bool bLastParaInterrupted =
            m_pEditEngine->GetOverflowingLineNum() > 0;

        return NonOverflowingText(aOverflowingTextSelection, bLastParaInterrupted);
    }
}

OutlinerParaObject Outliner::GetEmptyParaObject() const
{
    EditTextObject aEmptyText = m_pEditEngine->GetEmptyTextObject();
    OutlinerParaObject aPObj( std::move(aEmptyText) );
    aPObj.SetOutlinerMode(GetOutlinerMode());
    return aPObj;
}

std::optional<OverflowingText> Outliner::GetOverflowingText() const
{
    if ( m_pEditEngine->GetOverflowingParaNum() < 0)
        return {};


    // Defensive check: overflowing para index beyond actual # of paragraphs?
    if ( m_pEditEngine->GetOverflowingParaNum() > GetParagraphCount()-1) {
        SAL_INFO("editeng.chaining",
                 "[Overflowing] Ops, trying to retrieve para "
                 << m_pEditEngine->GetOverflowingParaNum() << " when max index is "
                 << GetParagraphCount()-1 );
        return {};
    }

    sal_Int32 nHeadPara = m_pEditEngine->GetOverflowingParaNum();
    sal_uInt32 nParaCount = GetParagraphCount();

    sal_uInt32 nLen = 0;
    for ( sal_Int32 nLine = 0;
          nLine < m_pEditEngine->GetOverflowingLineNum();
          nLine++) {
        nLen += GetLineLen(nHeadPara, nLine);
    }

    sal_uInt32 nOverflowingPara = m_pEditEngine->GetOverflowingParaNum();
    ESelection aOverflowingTextSel;
    sal_Int32 nLastPara = nParaCount-1;
    sal_Int32 nLastParaLen = GetText(nLastPara).getLength();
    aOverflowingTextSel = ESelection(nOverflowingPara, nLen,
                                     nLastPara, nLastParaLen);
    return OverflowingText(m_pEditEngine->CreateTransferable(aOverflowingTextSel));

}

void Outliner::ClearOverflowingParaNum()
{
    m_pEditEngine->ClearOverflowingParaNum();
}

void Outliner::SetIgnoreBreakAfterMultilineField(bool b)
{
    m_pEditEngine->SetIgnoreBreakAfterMultilineField(b);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
