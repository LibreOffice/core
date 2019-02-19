/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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


#include <vcl/svapp.hxx>

#include <svl/srchitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/tstpitem.hxx>

#include "eertfpar.hxx"
#include <editeng/editeng.hxx>
#include "impedit.hxx"
#include <editeng/editview.hxx>
#include "eehtml.hxx"
#include "editobj2.hxx"
#include <i18nlangtag/lang.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <editxml.hxx>

#include <editeng/autokernitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include "textconv.hxx"
#include <rtl/tencinfo.h>
#include <svtools/rtfout.hxx>
#include <edtspell.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/unolingu.hxx>
#include <linguistic/lngprops.hxx>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/textsearch.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/help.hxx>
#include <svtools/rtfkeywd.hxx>
#include <editeng/edtdlg.hxx>

#include <memory>
#include <unordered_map>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;


EditPaM ImpEditEngine::Read(SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat, const EditSelection& rSel, SvKeyValueIterator* pHTTPHeaderAttrs)
{
    bool _bUpdate = GetUpdateMode();
    SetUpdateMode( false );
    EditPaM aPaM;
    if ( eFormat == EETextFormat::Text )
        aPaM = ReadText( rInput, rSel );
    else if ( eFormat == EETextFormat::Rtf )
        aPaM = ReadRTF( rInput, rSel );
    else if ( eFormat == EETextFormat::Xml )
        aPaM = ReadXML( rInput, rSel );
    else if ( eFormat == EETextFormat::Html )
        aPaM = ReadHTML( rInput, rBaseURL, rSel, pHTTPHeaderAttrs );
    else
    {
        OSL_FAIL( "Read: Unknown Format" );
    }

    FormatFullDoc();        // perhaps a simple format is enough?
    SetUpdateMode( _bUpdate );

    return aPaM;
}

EditPaM ImpEditEngine::ReadText( SvStream& rInput, EditSelection aSel )
{
    if ( aSel.HasRange() )
        aSel = ImpDeleteSelection( aSel );
    EditPaM aPaM = aSel.Max();

    OUString aTmpStr;
    bool bDone = rInput.ReadByteStringLine( aTmpStr, rInput.GetStreamCharSet() );
    while ( bDone )
    {
        if (aTmpStr.getLength() > MAXCHARSINPARA)
        {
            aTmpStr = aTmpStr.copy(0, MAXCHARSINPARA);
        }
        aPaM = ImpInsertText( EditSelection( aPaM, aPaM ), aTmpStr );
        aPaM = ImpInsertParaBreak( aPaM );
        bDone = rInput.ReadByteStringLine( aTmpStr, rInput.GetStreamCharSet() );
    }
    return aPaM;
}

EditPaM ImpEditEngine::ReadXML( SvStream& rInput, EditSelection aSel )
{
    if ( aSel.HasRange() )
        aSel = ImpDeleteSelection( aSel );

    ESelection aESel = CreateESel( aSel );

    return ::SvxReadXML( *GetEditEnginePtr(), rInput, aESel );
}

EditPaM ImpEditEngine::ReadRTF( SvStream& rInput, EditSelection aSel )
{
    if ( aSel.HasRange() )
        aSel = ImpDeleteSelection( aSel );

    // The SvRTF parser expects the Which-mapping passed on in the pool, not
    // dependent on a secondary.
    SfxItemPool* pPool = &aEditDoc.GetItemPool();
    while (pPool->GetSecondaryPool() && pPool->GetName() != "EditEngineItemPool")
   {
        pPool = pPool->GetSecondaryPool();

    }

    DBG_ASSERT(pPool && pPool->GetName() == "EditEngineItemPool",
        "ReadRTF: no EditEnginePool!");

    EditRTFParserRef xPrsr = new EditRTFParser(rInput, aSel, *pPool, pEditEngine);
    SvParserState eState = xPrsr->CallParser();
    if ( ( eState != SvParserState::Accepted ) && ( !rInput.GetError() ) )
    {
        rInput.SetError( EE_READWRITE_WRONGFORMAT );
        return aSel.Min();
    }
    return xPrsr->GetCurPaM();
}

EditPaM ImpEditEngine::ReadHTML( SvStream& rInput, const OUString& rBaseURL, EditSelection aSel, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    if ( aSel.HasRange() )
        aSel = ImpDeleteSelection( aSel );

    EditHTMLParserRef xPrsr = new EditHTMLParser( rInput, rBaseURL, pHTTPHeaderAttrs );
    SvParserState eState = xPrsr->CallParser(pEditEngine, aSel.Max());
    if ( ( eState != SvParserState::Accepted ) && ( !rInput.GetError() ) )
    {
        rInput.SetError( EE_READWRITE_WRONGFORMAT );
        return aSel.Min();
    }
    return xPrsr->GetCurSelection().Max();
}

void ImpEditEngine::Write(SvStream& rOutput, EETextFormat eFormat, const EditSelection& rSel)
{
    if ( !rOutput.IsWritable() )
        rOutput.SetError( SVSTREAM_WRITE_ERROR );

    if ( !rOutput.GetError() )
    {
        if ( eFormat == EETextFormat::Text )
            WriteText( rOutput, rSel );
        else if ( eFormat == EETextFormat::Rtf )
            WriteRTF( rOutput, rSel );
        else if ( eFormat == EETextFormat::Xml )
            WriteXML( rOutput, rSel );
        else if ( eFormat == EETextFormat::Html )
            ;
        else
        {
            OSL_FAIL( "Write: Unknown Format" );
        }
    }
}

ErrCode ImpEditEngine::WriteText( SvStream& rOutput, EditSelection aSel )
{
    sal_Int32 nStartNode, nEndNode;
    bool bRange = aSel.HasRange();
    if ( bRange )
    {
        aSel.Adjust( aEditDoc );
        nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
        nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );
    }
    else
    {
        nStartNode = 0;
        nEndNode = aEditDoc.Count()-1;
    }

    // iterate over the paragraphs ...
    for ( sal_Int32 nNode = nStartNode; nNode <= nEndNode; nNode++  )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        DBG_ASSERT( pNode, "Node not found: Search&Replace" );

        sal_Int32 nStartPos = 0;
        sal_Int32 nEndPos = pNode->Len();
        if ( bRange )
        {
            if ( nNode == nStartNode )
                nStartPos = aSel.Min().GetIndex();
            if ( nNode == nEndNode ) // can also be == nStart!
                nEndPos = aSel.Max().GetIndex();
        }
        OUString aTmpStr = EditDoc::GetParaAsString( pNode, nStartPos, nEndPos );
        rOutput.WriteByteStringLine( aTmpStr, rOutput.GetStreamCharSet() );
    }

    return rOutput.GetError();
}

bool ImpEditEngine::WriteItemListAsRTF( ItemList& rLst, SvStream& rOutput, sal_Int32 nPara, sal_Int32 nPos,
                        std::vector<SvxFontItem*>& rFontTable, SvxColorList& rColorList )
{
    const SfxPoolItem* pAttrItem = rLst.First();
    while ( pAttrItem )
    {
        WriteItemAsRTF( *pAttrItem, rOutput, nPara, nPos,rFontTable, rColorList );
        pAttrItem = rLst.Next();
    }
    return rLst.Count() != 0;
}

static void lcl_FindValidAttribs( ItemList& rLst, ContentNode* pNode, sal_Int32 nIndex, sal_uInt16 nScriptType )
{
    sal_uInt16 nAttr = 0;
    EditCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    while ( pAttr && ( pAttr->GetStart() <= nIndex ) )
    {
        // Start is checked in while ...
        if ( pAttr->GetEnd() > nIndex )
        {
            if ( IsScriptItemValid( pAttr->GetItem()->Which(), nScriptType ) )
                rLst.Insert( pAttr->GetItem() );
        }
        nAttr++;
        pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    }
}

void ImpEditEngine::WriteXML(SvStream& rOutput, const EditSelection& rSel)
{
    ESelection aESel = CreateESel(rSel);

    SvxWriteXML( *GetEditEnginePtr(), rOutput, aESel );
}

ErrCode ImpEditEngine::WriteRTF( SvStream& rOutput, EditSelection aSel )
{
    DBG_ASSERT( GetUpdateMode(), "WriteRTF for UpdateMode = sal_False!" );
    CheckIdleFormatter();
    if ( !IsFormatted() )
        FormatDoc();

    sal_Int32 nStartNode, nEndNode;
    aSel.Adjust( aEditDoc );

    nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    // RTF header ...
    rOutput.WriteChar( '{' ) ;

    rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_RTF );

    rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ANSI );
    rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252;

    // Generate and write out Font table  ...
    std::vector<SvxFontItem*> aFontTable;
    // default font must be up front, so DEF font in RTF
    aFontTable.push_back( new SvxFontItem( aEditDoc.GetItemPool().GetDefaultItem( EE_CHAR_FONTINFO ) ) );
    aFontTable.push_back( new SvxFontItem( aEditDoc.GetItemPool().GetDefaultItem( EE_CHAR_FONTINFO_CJK ) ) );
    aFontTable.push_back( new SvxFontItem( aEditDoc.GetItemPool().GetDefaultItem( EE_CHAR_FONTINFO_CTL ) ) );
    for ( sal_uInt16 nScriptType = 0; nScriptType < 3; nScriptType++ )
    {
        sal_uInt16 nWhich = EE_CHAR_FONTINFO;
        if ( nScriptType == 1 )
            nWhich = EE_CHAR_FONTINFO_CJK;
        else if ( nScriptType == 2 )
            nWhich = EE_CHAR_FONTINFO_CTL;

        auto const nFonts(aEditDoc.GetItemPool().GetItemCount2(nWhich));
        for (sal_uInt32 i = 0; i < nFonts; ++i)
        {
            SvxFontItem const*const pFontItem = static_cast<const SvxFontItem*>(
                    aEditDoc.GetItemPool().GetItem2(nWhich, i));
            if (!pFontItem)
            {
                continue;
            }
            bool bAlreadyExist = false;
            sal_uLong nTestMax = nScriptType ? aFontTable.size() : 1;
            for ( sal_uLong nTest = 0; !bAlreadyExist && ( nTest < nTestMax ); nTest++ )
            {
                bAlreadyExist = *aFontTable[ nTest ] == *pFontItem;
            }

            if ( !bAlreadyExist )
                aFontTable.push_back( new SvxFontItem( *pFontItem ) );
        }
    }

    rOutput << endl;
    rOutput.WriteChar( '{' ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_FONTTBL );
    for ( std::vector<SvxFontItem*>::size_type j = 0; j < aFontTable.size(); j++ )
    {
        SvxFontItem* pFontItem = aFontTable[ j ];
        rOutput.WriteChar( '{' );
        rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_F );
        rOutput.WriteUInt32AsString( j );
        switch ( pFontItem->GetFamily()  )
        {
            case FAMILY_DONTKNOW:       rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_FNIL );
                                        break;
            case FAMILY_DECORATIVE:     rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_FDECOR );
                                        break;
            case FAMILY_MODERN:         rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_FMODERN );
                                        break;
            case FAMILY_ROMAN:          rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_FROMAN );
                                        break;
            case FAMILY_SCRIPT:         rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_FSCRIPT );
                                        break;
            case FAMILY_SWISS:          rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_FSWISS );
                                        break;
            default:
                break;
        }
        rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_FPRQ );
        sal_uInt16 nVal = 0;
        switch( pFontItem->GetPitch() )
        {
            case PITCH_FIXED:       nVal = 1;       break;
            case PITCH_VARIABLE:    nVal = 2;       break;
            default:
                break;
        }
        rOutput.WriteUInt32AsString( nVal );

        rtl_TextEncoding eChrSet = pFontItem->GetCharSet();
        DBG_ASSERT( eChrSet != 9, "SystemCharSet?!" );
        if( RTL_TEXTENCODING_DONTKNOW == eChrSet )
            eChrSet = osl_getThreadTextEncoding();
        rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_FCHARSET );
        rOutput.WriteUInt32AsString( rtl_getBestWindowsCharsetFromTextEncoding( eChrSet ) );

        rOutput.WriteChar( ' ' );
        RTFOutFuncs::Out_String( rOutput, pFontItem->GetFamilyName(), eDestEnc );
        rOutput.WriteCharPtr( ";}" );
    }
    rOutput.WriteChar( '}' );
    rOutput << endl;

    // Write out ColorList  ...
    SvxColorList aColorList;
    // COL_AUTO should be the default color, always put it first
    aColorList.emplace_back(COL_AUTO);
    SvxColorItem const& rDefault(aEditDoc.GetItemPool().GetDefaultItem(EE_CHAR_COLOR));
    if (rDefault.GetValue() != COL_AUTO) // is the default always AUTO?
    {
        aColorList.push_back(rDefault.GetValue());
    }
    auto const nColors(aEditDoc.GetItemPool().GetItemCount2(EE_CHAR_COLOR));
    for (sal_uInt32 i = 0; i < nColors; ++i)
    {
        SvxColorItem const*const pColorItem(aEditDoc.GetItemPool().GetItem2(EE_CHAR_COLOR, i));
        if (pColorItem && pColorItem->GetValue() != COL_AUTO) // may be null!
        {
            aColorList.push_back(pColorItem->GetValue());
        }
    }

    rOutput.WriteChar( '{' ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_COLORTBL );
    for ( SvxColorList::size_type j = 0; j < aColorList.size(); j++ )
    {
        Color const color = aColorList[j];
        if (color != COL_AUTO) // auto is represented by "empty" element
        {
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_RED );
            rOutput.WriteUInt32AsString( color.GetRed() );
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_GREEN );
            rOutput.WriteUInt32AsString( color.GetGreen() );
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_BLUE );
            rOutput.WriteUInt32AsString( color.GetBlue() );
        }
        rOutput.WriteChar( ';' );
    }
    rOutput.WriteChar( '}' );
    rOutput << endl;

    std::unordered_map<SfxStyleSheetBase*, sal_uInt32> aStyleSheetToIdMap;
    // StyleSheets...
    if ( GetStyleSheetPool() )
    {
        std::shared_ptr<SfxStyleSheetIterator> aSSSIterator = std::make_shared<SfxStyleSheetIterator>(GetStyleSheetPool(),
                SfxStyleFamily::All);
        // fill aStyleSheetToIdMap
        sal_uInt32 nId = 1;
        for ( SfxStyleSheetBase* pStyle = aSSSIterator->First(); pStyle;
                                 pStyle = aSSSIterator->Next() )
        {
            aStyleSheetToIdMap[pStyle] = nId;
            nId++;
        }

        if ( aSSSIterator->Count() )
        {

            sal_uInt32 nStyle = 0;
            rOutput.WriteChar( '{' ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_STYLESHEET );

            for ( SfxStyleSheetBase* pStyle = aSSSIterator->First(); pStyle;
                                     pStyle = aSSSIterator->Next() )
            {

                rOutput << endl;
                rOutput.WriteChar( '{' ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_S );
                sal_uInt32 nNumber = nStyle + 1;
                rOutput.WriteUInt32AsString( nNumber );

                // Attribute, also from Parent!
                for ( sal_uInt16 nParAttr = EE_PARA_START; nParAttr <= EE_CHAR_END; nParAttr++ )
                {
                    if ( pStyle->GetItemSet().GetItemState( nParAttr ) == SfxItemState::SET )
                    {
                        const SfxPoolItem& rItem = pStyle->GetItemSet().Get( nParAttr );
                        WriteItemAsRTF( rItem, rOutput, 0, 0, aFontTable, aColorList );
                    }
                }

                // Parent ... (only if necessary)
                if ( !pStyle->GetParent().isEmpty() && ( pStyle->GetParent() != pStyle->GetName() ) )
                {
                    SfxStyleSheet* pParent = static_cast<SfxStyleSheet*>(GetStyleSheetPool()->Find( pStyle->GetParent(), pStyle->GetFamily() ));
                    DBG_ASSERT( pParent, "Parent not found!" );
                    rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_SBASEDON );
                    nNumber = aStyleSheetToIdMap.find(pParent)->second;
                    rOutput.WriteUInt32AsString( nNumber );
                }

                // Next Style... (more)
                // we assume that we have only SfxStyleSheet in the pool
                SfxStyleSheet* pNext = static_cast<SfxStyleSheet*>(pStyle);
                if ( !pStyle->GetFollow().isEmpty() && ( pStyle->GetFollow() != pStyle->GetName() ) )
                    pNext = static_cast<SfxStyleSheet*>(GetStyleSheetPool()->Find( pStyle->GetFollow(), pStyle->GetFamily() ));

                DBG_ASSERT( pNext, "Next not found!" );
                rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_SNEXT );
                nNumber = aStyleSheetToIdMap.find(pNext)->second;
                rOutput.WriteUInt32AsString( nNumber );

                // Name of the template...
                rOutput.WriteCharPtr( " " );
                RTFOutFuncs::Out_String( rOutput, pStyle->GetName(), eDestEnc );
                rOutput.WriteCharPtr( ";}" );
                nStyle++;
            }
            rOutput.WriteChar( '}' );
            rOutput << endl;
        }
    }

    // Write the pool defaults in advance ...
    rOutput.WriteChar( '{' ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_IGNORE ).WriteCharPtr( "\\EditEnginePoolDefaults" );
    for ( sal_uInt16 nPoolDefItem = EE_PARA_START; nPoolDefItem <= EE_CHAR_END; nPoolDefItem++)
    {
        const SfxPoolItem& rItem = aEditDoc.GetItemPool().GetDefaultItem( nPoolDefItem );
        WriteItemAsRTF( rItem, rOutput, 0, 0, aFontTable, aColorList );
    }
    rOutput.WriteChar( '}' ) << endl;

    // DefTab:
    MapMode aTwpMode( MapUnit::MapTwip );
    sal_uInt16 nDefTabTwps = static_cast<sal_uInt16>(GetRefDevice()->LogicToLogic(
                                        Point( aEditDoc.GetDefTab(), 0 ),
                                        &GetRefMapMode(), &aTwpMode ).X());
    rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_DEFTAB );
    rOutput.WriteUInt32AsString( nDefTabTwps );
    rOutput << endl;

    // iterate over the paragraphs ...
    rOutput.WriteChar( '{' ) << endl;
    for ( sal_Int32 nNode = nStartNode; nNode <= nEndNode; nNode++  )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        DBG_ASSERT( pNode, "Node not found: Search&Replace" );

        // The paragraph attributes in advance ...
        bool bAttr = false;

        // Template?
        if ( pNode->GetStyleSheet() )
        {
            // Number of template
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_S );
            sal_uInt32 nNumber = aStyleSheetToIdMap.find(pNode->GetStyleSheet())->second;
            rOutput.WriteUInt32AsString( nNumber );

            // All Attribute
            // Attribute, also from Parent!
            for ( sal_uInt16 nParAttr = EE_PARA_START; nParAttr <= EE_CHAR_END; nParAttr++ )
            {
                if ( pNode->GetStyleSheet()->GetItemSet().GetItemState( nParAttr ) == SfxItemState::SET )
                {
                    const SfxPoolItem& rItem = pNode->GetStyleSheet()->GetItemSet().Get( nParAttr );
                    WriteItemAsRTF( rItem, rOutput, nNode, 0, aFontTable, aColorList );
                    bAttr = true;
                }
            }
        }

        for ( sal_uInt16 nParAttr = EE_PARA_START; nParAttr <= EE_CHAR_END; nParAttr++ )
        {
            // Now where stylesheet processing, only hard paragraph attributes!
            if ( pNode->GetContentAttribs().GetItems().GetItemState( nParAttr ) == SfxItemState::SET )
            {
                const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItems().Get( nParAttr );
                WriteItemAsRTF( rItem, rOutput, nNode, 0, aFontTable, aColorList );
                bAttr = true;
            }
        }
        if ( bAttr )
            rOutput.WriteChar( ' ' ); // Separator

        ItemList aAttribItems;
        ParaPortion* pParaPortion = FindParaPortion( pNode );
        DBG_ASSERT( pParaPortion, "Portion not found: WriteRTF" );

        sal_Int32 nIndex = 0;
        sal_Int32 nStartPos = 0;
        sal_Int32 nEndPos = pNode->Len();
        sal_Int32 nStartPortion = 0;
        sal_Int32 nEndPortion = pParaPortion->GetTextPortions().Count() - 1;
        bool bFinishPortion = false;
        sal_Int32 nPortionStart;

        if ( nNode == nStartNode )
        {
            nStartPos = aSel.Min().GetIndex();
            nStartPortion = pParaPortion->GetTextPortions().FindPortion( nStartPos, nPortionStart );
            if ( nStartPos != 0 )
            {
                aAttribItems.Clear();
                lcl_FindValidAttribs( aAttribItems, pNode, nStartPos, GetI18NScriptType( EditPaM( pNode, 0 ) ) );
                if ( aAttribItems.Count() )
                {
                    // These attributes may not apply to the entire paragraph:
                    rOutput.WriteChar( '{' );
                    WriteItemListAsRTF( aAttribItems, rOutput, nNode, nStartPos, aFontTable, aColorList );
                    bFinishPortion = true;
                }
                aAttribItems.Clear();
            }
        }
        if ( nNode == nEndNode ) // can also be == nStart!
        {
            nEndPos = aSel.Max().GetIndex();
            nEndPortion = pParaPortion->GetTextPortions().FindPortion( nEndPos, nPortionStart );
        }

        const EditCharAttrib* pNextFeature = pNode->GetCharAttribs().FindFeature(nIndex);
        // start at 0, so the index is right ...
        for ( sal_Int32 n = 0; n <= nEndPortion; n++ )
        {
            const TextPortion& rTextPortion = pParaPortion->GetTextPortions()[n];
            if ( n < nStartPortion )
            {
                nIndex = nIndex + rTextPortion.GetLen();
                continue;
            }

            if ( pNextFeature && ( pNextFeature->GetStart() == nIndex ) && ( pNextFeature->GetItem()->Which() != EE_FEATURE_FIELD ) )
            {
                WriteItemAsRTF( *pNextFeature->GetItem(), rOutput, nNode, nIndex, aFontTable, aColorList );
                pNextFeature = pNode->GetCharAttribs().FindFeature( pNextFeature->GetStart() + 1 );
            }
            else
            {
                aAttribItems.Clear();
                sal_uInt16 nScriptTypeI18N = GetI18NScriptType( EditPaM( pNode, nIndex+1 ) );
                SvtScriptType nScriptType = SvtLanguageOptions::FromI18NToSvtScriptType(nScriptTypeI18N);
                if ( !n || IsScriptChange( EditPaM( pNode, nIndex ) ) )
                {
                    SfxItemSet aAttribs = GetAttribs( nNode, nIndex+1, nIndex+1 );
                    aAttribItems.Insert( &aAttribs.Get( GetScriptItemId( EE_CHAR_FONTINFO, nScriptType ) ) );
                    aAttribItems.Insert( &aAttribs.Get( GetScriptItemId( EE_CHAR_FONTHEIGHT, nScriptType ) ) );
                    aAttribItems.Insert( &aAttribs.Get( GetScriptItemId( EE_CHAR_WEIGHT, nScriptType ) ) );
                    aAttribItems.Insert( &aAttribs.Get( GetScriptItemId( EE_CHAR_ITALIC, nScriptType ) ) );
                    aAttribItems.Insert( &aAttribs.Get( GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType ) ) );
                }
                // Insert hard attribs AFTER CJK attribs...
                lcl_FindValidAttribs( aAttribItems, pNode, nIndex, nScriptTypeI18N );

                rOutput.WriteChar( '{' );
                if ( WriteItemListAsRTF( aAttribItems, rOutput, nNode, nIndex, aFontTable, aColorList ) )
                    rOutput.WriteChar( ' ' );

                sal_Int32 nS = nIndex;
                sal_Int32 nE = nIndex + rTextPortion.GetLen();
                if ( n == nStartPortion )
                    nS = nStartPos;
                if ( n == nEndPortion )
                    nE = nEndPos;

                OUString aRTFStr = EditDoc::GetParaAsString( pNode, nS, nE);
                RTFOutFuncs::Out_String( rOutput, aRTFStr, eDestEnc );
                rOutput.WriteChar( '}' );
            }
            if ( bFinishPortion )
            {
                rOutput.WriteChar( '}' );
                bFinishPortion = false;
            }

            nIndex = nIndex + rTextPortion.GetLen();
        }

        rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_PAR ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_PARD ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_PLAIN );
        rOutput << endl;
    }
    // RTF-trailer ...
    rOutput.WriteCharPtr( "}}" );    // 1xparentheses paragraphs, 1xparentheses RTF document
    rOutput.Flush();

    for (auto& pItem : aFontTable)
        delete pItem;

    return rOutput.GetError();
}


void ImpEditEngine::WriteItemAsRTF( const SfxPoolItem& rItem, SvStream& rOutput, sal_Int32 nPara, sal_Int32 nPos,
                            std::vector<SvxFontItem*>& rFontTable, SvxColorList& rColorList )
{
    sal_uInt16 nWhich = rItem.Which();
    switch ( nWhich )
    {
        case EE_PARA_WRITINGDIR:
        {
            const SvxFrameDirectionItem& rWritingMode = static_cast<const SvxFrameDirectionItem&>(rItem);
            if ( rWritingMode.GetValue() == SvxFrameDirection::Horizontal_RL_TB )
                rOutput.WriteCharPtr( "\\rtlpar" );
            else
                rOutput.WriteCharPtr( "\\ltrpar" );
        }
        break;
        case EE_PARA_OUTLLEVEL:
        {
            sal_Int32 nLevel = static_cast<const SfxInt16Item&>(rItem).GetValue();
            if( nLevel >= 0 )
            {
                rOutput.WriteCharPtr( "\\level" );
                rOutput.WriteInt32AsString( nLevel );
            }
        }
        break;
        case EE_PARA_OUTLLRSPACE:
        case EE_PARA_LRSPACE:
        {
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_FI );
            sal_Int32 nTxtFirst = static_cast<const SvxLRSpaceItem&>(rItem).GetTextFirstLineOfst();
            nTxtFirst = LogicToTwips( nTxtFirst );
            rOutput.WriteInt32AsString( nTxtFirst );
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_LI );
            sal_uInt32 nTxtLeft = static_cast< sal_uInt32 >(static_cast<const SvxLRSpaceItem&>(rItem).GetTextLeft());
            nTxtLeft = static_cast<sal_uInt32>(LogicToTwips( nTxtLeft ));
            rOutput.WriteInt32AsString( nTxtLeft );
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_RI );
            sal_uInt32 nTxtRight = static_cast<const SvxLRSpaceItem&>(rItem).GetRight();
            nTxtRight = LogicToTwips( nTxtRight);
            rOutput.WriteUInt32AsString( nTxtRight );
        }
        break;
        case EE_PARA_ULSPACE:
        {
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_SB );
            sal_uInt32 nUpper = static_cast<const SvxULSpaceItem&>(rItem).GetUpper();
            nUpper = static_cast<sal_uInt32>(LogicToTwips( nUpper ));
            rOutput.WriteUInt32AsString( nUpper );
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_SA );
            sal_uInt32 nLower = static_cast<const SvxULSpaceItem&>(rItem).GetLower();
            nLower = LogicToTwips( nLower );
            rOutput.WriteUInt32AsString( nLower );
        }
        break;
        case EE_PARA_SBL:
        {
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_SL );
            sal_Int32 nVal = static_cast<const SvxLineSpacingItem&>(rItem).GetLineHeight();
            char cMult = '0';
            if ( static_cast<const SvxLineSpacingItem&>(rItem).GetInterLineSpaceRule() == SvxInterLineSpaceRule::Prop )
            {
                // From where do I get the value now?
                // The SwRTF parser is based on a 240 Font!
                nVal = static_cast<const SvxLineSpacingItem&>(rItem).GetPropLineSpace();
                nVal *= 240;
                nVal /= 100;
                cMult = '1';
            }
            rOutput.WriteInt32AsString( nVal );
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_SLMULT ).WriteChar( cMult );
        }
        break;
        case EE_PARA_JUST:
        {
            SvxAdjust eJustification = static_cast<const SvxAdjustItem&>(rItem).GetAdjust();
            switch ( eJustification )
            {
                case SvxAdjust::Center: rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_QC );
                                        break;
                case SvxAdjust::Right:  rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_QR );
                                        break;
                default:                rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_QL );
                                        break;
            }
        }
        break;
        case EE_PARA_TABS:
        {
            const SvxTabStopItem& rTabs = static_cast<const SvxTabStopItem&>(rItem);
            for ( sal_uInt16 i = 0; i < rTabs.Count(); i++ )
            {
                const SvxTabStop& rTab = rTabs[i];
                rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TX );
                rOutput.WriteInt32AsString( LogicToTwips( rTab.GetTabPos() ) );
            }
        }
        break;
        case EE_CHAR_COLOR:
        {
            SvxColorList::const_iterator const iter = std::find(
                    rColorList.begin(), rColorList.end(),
                    static_cast<SvxColorItem const&>(rItem).GetValue());
            assert(iter != rColorList.end());
            sal_uInt32 const n = iter - rColorList.begin();
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_CF );
            rOutput.WriteUInt32AsString( n );
        }
        break;
        case EE_CHAR_FONTINFO:
        case EE_CHAR_FONTINFO_CJK:
        case EE_CHAR_FONTINFO_CTL:
        {
            sal_uInt32 n = 0;
            for (size_t i = 0; i < rFontTable.size(); ++i)
            {
                if (*rFontTable[i] == rItem)
                {
                    n = i;
                    break;
                }
            }

            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_F );
            rOutput.WriteUInt32AsString( n );
        }
        break;
        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_FS );
            sal_Int32 nHeight = static_cast<const SvxFontHeightItem&>(rItem).GetHeight();
            nHeight = LogicToTwips( nHeight );
            // Twips => HalfPoints
            nHeight /= 10;
            rOutput.WriteInt32AsString( nHeight );
        }
        break;
        case EE_CHAR_WEIGHT:
        case EE_CHAR_WEIGHT_CJK:
        case EE_CHAR_WEIGHT_CTL:
        {
            FontWeight e = static_cast<const SvxWeightItem&>(rItem).GetWeight();
            switch ( e )
            {
                case WEIGHT_BOLD:   rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_B );                break;
                default:            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_B ).WriteChar( '0' );     break;
            }
        }
        break;
        case EE_CHAR_UNDERLINE:
        {
            // Must underlined if in WordLineMode, but the information is
            // missing here
            FontLineStyle e = static_cast<const SvxUnderlineItem&>(rItem).GetLineStyle();
            switch ( e )
            {
                case LINESTYLE_NONE:    rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ULNONE );       break;
                case LINESTYLE_SINGLE:  rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_UL );       break;
                case LINESTYLE_DOUBLE:  rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ULDB );     break;
                case LINESTYLE_DOTTED:  rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ULD );      break;
                default:
                    break;
            }
        }
        break;
        case EE_CHAR_OVERLINE:
        {
            FontLineStyle e = static_cast<const SvxOverlineItem&>(rItem).GetLineStyle();
            switch ( e )
            {
                case LINESTYLE_NONE:    rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_OLNONE );       break;
                case LINESTYLE_SINGLE:  rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_OL );       break;
                case LINESTYLE_DOUBLE:  rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_OLDB );     break;
                case LINESTYLE_DOTTED:  rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_OLD );      break;
                default:
                    break;
            }
        }
        break;
        case EE_CHAR_STRIKEOUT:
        {
            FontStrikeout e = static_cast<const SvxCrossedOutItem&>(rItem).GetStrikeout();
            switch ( e )
            {
                case STRIKEOUT_SINGLE:
                case STRIKEOUT_DOUBLE:  rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_STRIKE );       break;
                case STRIKEOUT_NONE:    rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_STRIKE ).WriteChar( '0' );    break;
                default:
                    break;
            }
        }
        break;
        case EE_CHAR_ITALIC:
        case EE_CHAR_ITALIC_CJK:
        case EE_CHAR_ITALIC_CTL:
        {
            FontItalic e = static_cast<const SvxPostureItem&>(rItem).GetPosture();
            switch ( e )
            {
                case ITALIC_OBLIQUE:
                case ITALIC_NORMAL: rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_I );        break;
                case ITALIC_NONE:   rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_I ).WriteChar( '0' ); break;
                default:
                    break;
            }
        }
        break;
        case EE_CHAR_OUTLINE:
        {
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_OUTL );
            if ( !static_cast<const SvxContourItem&>(rItem).GetValue() )
                rOutput.WriteChar( '0' );
        }
        break;
        case EE_CHAR_RELIEF:
        {
            FontRelief nRelief = static_cast<const SvxCharReliefItem&>(rItem).GetValue();
            if ( nRelief == FontRelief::Embossed )
                rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_EMBO );
            if ( nRelief == FontRelief::Engraved )
                rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_IMPR );
        }
        break;
        case EE_CHAR_EMPHASISMARK:
        {
            FontEmphasisMark nMark = static_cast<const SvxEmphasisMarkItem&>(rItem).GetEmphasisMark();
            if ( nMark == FontEmphasisMark::NONE )
                rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ACCNONE );
            else if ( nMark == (FontEmphasisMark::Accent | FontEmphasisMark::PosAbove) )
                rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ACCCOMMA );
            else
                rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ACCDOT );
        }
        break;
        case EE_CHAR_SHADOW:
        {
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_SHAD );
            if ( !static_cast<const SvxShadowedItem&>(rItem).GetValue() )
                rOutput.WriteChar( '0' );
        }
        break;
        case EE_FEATURE_TAB:
        {
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TAB );
        }
        break;
        case EE_FEATURE_LINEBR:
        {
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_SL );
        }
        break;
        case EE_CHAR_KERNING:
        {
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_EXPNDTW );
            rOutput.WriteInt32AsString( LogicToTwips(
                static_cast<const SvxKerningItem&>(rItem).GetValue() ) );
        }
        break;
        case EE_CHAR_PAIRKERNING:
        {
            rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_KERNING );
            rOutput.WriteUInt32AsString( static_cast<const SvxAutoKernItem&>(rItem).GetValue() ? 1 : 0 );
        }
        break;
        case EE_CHAR_ESCAPEMENT:
        {
            SvxFont aFont;
            ContentNode* pNode = aEditDoc.GetObject( nPara );
            SeekCursor( pNode, nPos, aFont );
            MapMode aPntMode( MapUnit::MapPoint );
            long nFontHeight = GetRefDevice()->LogicToLogic(
                    aFont.GetFontSize(), &GetRefMapMode(), &aPntMode ).Height();
            nFontHeight *=2;    // Half Points
            sal_uInt16 const nProp = static_cast<const SvxEscapementItem&>(rItem).GetProportionalHeight();
            sal_uInt16 nProp100 = nProp*100;    // For SWG-Token Prop in 100th percent.
            short nEsc = static_cast<const SvxEscapementItem&>(rItem).GetEsc();
            if ( nEsc == DFLT_ESC_AUTO_SUPER )
            {
                nEsc = 100 - nProp;
                nProp100++; // A 1 afterwards means 'automatic'.
            }
            else if ( nEsc == DFLT_ESC_AUTO_SUB )
            {
                nEsc = sal::static_int_cast< short >( -( 100 - nProp ) );
                nProp100++;
            }
            // SWG:
            if ( nEsc )
            {
                rOutput.WriteCharPtr( "{\\*\\updnprop" ).WriteCharPtr( OString::number(
                    nProp100).getStr() ).WriteChar( '}' );
            }
            long nUpDown = nFontHeight * std::abs( nEsc ) / 100;
            OString aUpDown = OString::number(
                nUpDown);
            if ( nEsc < 0 )
                rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_DN ).WriteCharPtr( aUpDown.getStr() );
            else if ( nEsc > 0 )
                rOutput.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_UP ).WriteCharPtr( aUpDown.getStr() );
        }
        break;
    }
}

std::unique_ptr<EditTextObject> ImpEditEngine::GetEmptyTextObject()
{
    EditSelection aEmptySel;
    aEmptySel.Min() = aEditDoc.GetStartPaM();
    aEmptySel.Max() = aEditDoc.GetStartPaM();

    return CreateTextObject( aEmptySel );
}

std::unique_ptr<EditTextObject> ImpEditEngine::CreateTextObject()
{
    EditSelection aCompleteSelection;
    aCompleteSelection.Min() = aEditDoc.GetStartPaM();
    aCompleteSelection.Max() = aEditDoc.GetEndPaM();

    return CreateTextObject( aCompleteSelection );
}

std::unique_ptr<EditTextObject> ImpEditEngine::CreateTextObject(const EditSelection& rSel)
{
    return CreateTextObject(rSel, GetEditTextObjectPool(), aStatus.AllowBigObjects(), nBigTextObjectStart);
}

std::unique_ptr<EditTextObject> ImpEditEngine::CreateTextObject( EditSelection aSel, SfxItemPool* pPool, bool bAllowBigObjects, sal_Int32 nBigObjectStart )
{
    std::unique_ptr<EditTextObject> pTxtObj(new EditTextObject(pPool));
    pTxtObj->SetVertical( IsVertical(), IsTopToBottom());
    MapUnit eMapUnit = aEditDoc.GetItemPool().GetMetric( DEF_METRIC );
    pTxtObj->mpImpl->SetMetric( static_cast<sal_uInt16>(eMapUnit) );
    if ( pTxtObj->mpImpl->IsOwnerOfPool() )
        pTxtObj->mpImpl->GetPool()->SetDefaultMetric( eMapUnit );

    sal_Int32 nStartNode, nEndNode;
    sal_Int32 nTextPortions = 0;

    aSel.Adjust( aEditDoc );
    nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    bool bOnlyFullParagraphs = !( aSel.Min().GetIndex() ||
        ( aSel.Max().GetIndex() < aSel.Max().GetNode()->Len() ) );

    // Templates are not saved!
    // (Only the name and family, template itself must be in App!)
    pTxtObj->mpImpl->SetScriptType(GetItemScriptType(aSel));

    // iterate over the paragraphs ...
    sal_Int32 nNode;
    for ( nNode = nStartNode; nNode <= nEndNode; nNode++  )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        DBG_ASSERT( pNode, "Node not found: Search&Replace" );

        if ( bOnlyFullParagraphs )
        {
            const ParaPortion* pParaPortion = GetParaPortions()[nNode];
            nTextPortions += pParaPortion->GetTextPortions().Count();
        }

        sal_Int32 nStartPos = 0;
        sal_Int32 nEndPos = pNode->Len();

        bool bEmptyPara = nEndPos == 0;

        if ( ( nNode == nStartNode ) && !bOnlyFullParagraphs )
            nStartPos = aSel.Min().GetIndex();
        if ( ( nNode == nEndNode ) && !bOnlyFullParagraphs )
            nEndPos = aSel.Max().GetIndex();


        ContentInfo *pC = pTxtObj->mpImpl->CreateAndInsertContent();

        // The paragraph attributes ...
        pC->GetParaAttribs().Set( pNode->GetContentAttribs().GetItems() );

        // The StyleSheet...
        if ( pNode->GetStyleSheet() )
        {
            pC->SetStyle(pNode->GetStyleSheet()->GetName());
            pC->SetFamily(pNode->GetStyleSheet()->GetFamily());
        }

        // The Text...
        pC->SetText(pNode->Copy(nStartPos, nEndPos-nStartPos));

        // and the Attribute...
        sal_uInt16 nAttr = 0;
        EditCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
        while ( pAttr )
        {
            // In a blank paragraph keep the attributes!
            if ( bEmptyPara ||
                 ( ( pAttr->GetEnd() > nStartPos ) && ( pAttr->GetStart() < nEndPos ) ) )
            {
                std::unique_ptr<XEditAttribute> pX = pTxtObj->mpImpl->CreateAttrib(*pAttr->GetItem(), pAttr->GetStart(), pAttr->GetEnd());
                // Possibly Correct ...
                if ( ( nNode == nStartNode ) && ( nStartPos != 0 ) )
                {
                    pX->GetStart() = ( pX->GetStart() > nStartPos ) ? pX->GetStart()-nStartPos : 0;
                    pX->GetEnd() = pX->GetEnd() - nStartPos;

                }
                if ( nNode == nEndNode )
                {
                    if ( pX->GetEnd() > (nEndPos-nStartPos) )
                        pX->GetEnd() = nEndPos-nStartPos;
                }
                DBG_ASSERT( pX->GetEnd() <= (nEndPos-nStartPos), "CreateBinTextObject: Attribute too long!" );
                if ( !pX->GetLen() && !bEmptyPara )
                    pTxtObj->mpImpl->DestroyAttrib(std::move(pX));
                else
                    pC->GetCharAttribs().push_back(std::move(pX));
            }
            nAttr++;
            pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
        }

        // If possible online spelling
        if ( bAllowBigObjects && bOnlyFullParagraphs && pNode->GetWrongList() )
            pC->SetWrongList( pNode->GetWrongList()->Clone() );

    }

    // Remember the portions info in case of large text objects:
    // sleeper set up when Olli paragraphs not hacked!
    if ( bAllowBigObjects && bOnlyFullParagraphs && IsFormatted() && GetUpdateMode() && ( nTextPortions >= nBigObjectStart ) )
    {
        XParaPortionList* pXList = new XParaPortionList( GetRefDevice(), aPaperSize.Width(), nStretchX, nStretchY );
        pTxtObj->mpImpl->SetPortionInfo(std::unique_ptr<XParaPortionList>(pXList));
        for ( nNode = nStartNode; nNode <= nEndNode; nNode++  )
        {
            const ParaPortion* pParaPortion = GetParaPortions()[nNode];
            XParaPortion* pX = new XParaPortion;
            pXList->push_back(pX);

            pX->nHeight = pParaPortion->GetHeight();
            pX->nFirstLineOffset = pParaPortion->GetFirstLineOffset();

            // The TextPortions
            sal_uInt16 nCount = pParaPortion->GetTextPortions().Count();
            sal_uInt16 n;
            for ( n = 0; n < nCount; n++ )
            {
                const TextPortion& rTextPortion = pParaPortion->GetTextPortions()[n];
                TextPortion* pNew = new TextPortion( rTextPortion );
                pX->aTextPortions.Append(pNew);
            }

            // The lines
            nCount = pParaPortion->GetLines().Count();
            for ( n = 0; n < nCount; n++ )
            {
                const EditLine& rLine = pParaPortion->GetLines()[n];
                EditLine* pNew = rLine.Clone();
                pX->aLines.Append(pNew);
            }
#ifdef DBG_UTIL
            sal_uInt16 nTest;
            int nTPLen = 0, nTxtLen = 0;
            for ( nTest = pParaPortion->GetTextPortions().Count(); nTest; )
                nTPLen += pParaPortion->GetTextPortions()[--nTest].GetLen();
            for ( nTest = pParaPortion->GetLines().Count(); nTest; )
                nTxtLen += pParaPortion->GetLines()[--nTest].GetLen();
            DBG_ASSERT( ( nTPLen == pParaPortion->GetNode()->Len() ) && ( nTxtLen == pParaPortion->GetNode()->Len() ), "CreateBinTextObject: ParaPortion not completely formatted!" );
#endif
        }
    }
    return pTxtObj;
}

void ImpEditEngine::SetText( const EditTextObject& rTextObject )
{
    // Since setting a text object is not undo-able!
    ResetUndoManager();
    bool _bUpdate = GetUpdateMode();
    bool _bUndo = IsUndoEnabled();

    SetText( OUString() );
    EditPaM aPaM = aEditDoc.GetStartPaM();

    SetUpdateMode( false );
    EnableUndo( false );

    InsertText( rTextObject, EditSelection( aPaM, aPaM ) );
    SetVertical( rTextObject.IsVertical(), rTextObject.IsTopToBottom());

    DBG_ASSERT( !HasUndoManager() || !GetUndoManager().GetUndoActionCount(), "From where comes the Undo in SetText ?!" );
    SetUpdateMode( _bUpdate );
    EnableUndo( _bUndo );
}

EditSelection ImpEditEngine::InsertText( const EditTextObject& rTextObject, EditSelection aSel )
{
    aSel.Adjust( aEditDoc );
    if ( aSel.HasRange() )
        aSel = ImpDeleteSelection( aSel );
    EditSelection aNewSel = InsertTextObject( rTextObject, aSel.Max() );
    return aNewSel;
}

EditSelection ImpEditEngine::InsertTextObject( const EditTextObject& rTextObject, EditPaM aPaM )
{
    // Optimize: No getPos undFindParaportion, instead calculate index!
    EditSelection aSel( aPaM, aPaM );
    DBG_ASSERT( !aSel.DbgIsBuggy( aEditDoc ), "InsertBibTextObject: Selection broken!(1)" );

    bool bUsePortionInfo = false;
    XParaPortionList* pPortionInfo = rTextObject.mpImpl->GetPortionInfo();

    if ( pPortionInfo && ( static_cast<long>(pPortionInfo->GetPaperWidth()) == aPaperSize.Width() )
            && ( pPortionInfo->GetRefMapMode() == GetRefDevice()->GetMapMode() )
            && ( pPortionInfo->GetStretchX() == nStretchX )
            && ( pPortionInfo->GetStretchY() == nStretchY ) )
    {
        if ( (pPortionInfo->GetRefDevPtr() == GetRefDevice()) ||
             (pPortionInfo->RefDevIsVirtual() && GetRefDevice()->IsVirtual()) )
        bUsePortionInfo = true;
    }

    bool bConvertMetricOfItems = false;
    MapUnit eSourceUnit = MapUnit(), eDestUnit = MapUnit();
    if (rTextObject.mpImpl->HasMetric())
    {
        eSourceUnit = static_cast<MapUnit>(rTextObject.mpImpl->GetMetric());
        eDestUnit = aEditDoc.GetItemPool().GetMetric( DEF_METRIC );
        if ( eSourceUnit != eDestUnit )
            bConvertMetricOfItems = true;
    }

    // Before, paragraph count was of type sal_uInt16 so if nContents exceeded
    // 0xFFFF this wouldn't have worked anyway, given that nPara is used to
    // number paragraphs and is fearlessly incremented.
    sal_Int32 nContents = static_cast<sal_Int32>(rTextObject.mpImpl->GetContents().size());
    SAL_WARN_IF( nContents < 0, "editeng", "ImpEditEngine::InsertTextObject - contents overflow " << nContents);
    sal_Int32 nPara = aEditDoc.GetPos( aPaM.GetNode() );

    for (sal_Int32 n = 0; n < nContents; ++n, ++nPara)
    {
        const ContentInfo* pC = rTextObject.mpImpl->GetContents()[n].get();
        bool bNewContent = aPaM.GetNode()->Len() == 0;
        const sal_Int32 nStartPos = aPaM.GetIndex();

        aPaM = ImpFastInsertText( aPaM, pC->GetText() );

        ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
        DBG_ASSERT( pPortion, "Blind Portion in FastInsertText" );
        pPortion->MarkInvalid( nStartPos, pC->GetText().getLength() );

        // Character attributes ...
        bool bAllreadyHasAttribs = aPaM.GetNode()->GetCharAttribs().Count() != 0;
        size_t nNewAttribs = pC->GetCharAttribs().size();
        if ( nNewAttribs )
        {
            bool bUpdateFields = false;
            for (size_t nAttr = 0; nAttr < nNewAttribs; ++nAttr)
            {
                const XEditAttribute& rX = *pC->GetCharAttribs()[nAttr].get();
                // Can happen when paragraphs > 16K, it is simply wrapped.
                    //TODO! Still true, still needed?
                if ( rX.GetEnd() <= aPaM.GetNode()->Len() )
                {
                    if ( !bAllreadyHasAttribs || rX.IsFeature() )
                    {
                        // Normal attributes then go faster ...
                        // Features shall not be inserted through
                        // EditDoc:: InsertAttrib, using FastInsertText they are
                        // already in the flow
                        DBG_ASSERT( rX.GetEnd() <= aPaM.GetNode()->Len(), "InsertBinTextObject: Attribute too large!" );
                        EditCharAttrib* pAttr;
                        if ( !bConvertMetricOfItems )
                            pAttr = MakeCharAttrib( aEditDoc.GetItemPool(), *(rX.GetItem()), rX.GetStart()+nStartPos, rX.GetEnd()+nStartPos );
                        else
                        {
                            std::unique_ptr<SfxPoolItem> pNew(rX.GetItem()->Clone());
                            ConvertItem( *pNew, eSourceUnit, eDestUnit );
                            pAttr = MakeCharAttrib( aEditDoc.GetItemPool(), *pNew, rX.GetStart()+nStartPos, rX.GetEnd()+nStartPos );
                        }
                        DBG_ASSERT( pAttr->GetEnd() <= aPaM.GetNode()->Len(), "InsertBinTextObject: Attribute does not fit! (1)" );
                        aPaM.GetNode()->GetCharAttribs().InsertAttrib( pAttr );
                        if ( pAttr->Which() == EE_FEATURE_FIELD )
                            bUpdateFields = true;
                    }
                    else
                    {
                        DBG_ASSERT( rX.GetEnd()+nStartPos <= aPaM.GetNode()->Len(), "InsertBinTextObject: Attribute does not fit! (2)" );
                        // Tabs and other Features can not be inserted through InsertAttrib:
                        aEditDoc.InsertAttrib( aPaM.GetNode(), rX.GetStart()+nStartPos, rX.GetEnd()+nStartPos, *rX.GetItem() );
                    }
                }
            }
            if ( bUpdateFields )
                UpdateFields();

            // Otherwise, quick format => no attributes!
            pPortion->MarkSelectionInvalid( nStartPos );
        }

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
        CharAttribList::DbgCheckAttribs(aPaM.GetNode()->GetCharAttribs());
#endif

        bool bParaAttribs = false;
        if ( bNewContent || ( ( n > 0 ) && ( n < (nContents-1) ) ) )
        {
            // only style and ParaAttribs when new paragraph, or
            // completely internal ...
            bParaAttribs = pC->GetParaAttribs().Count() != 0;
            if ( GetStyleSheetPool() && pC->GetStyle().getLength() )
            {
                SfxStyleSheet* pStyle = static_cast<SfxStyleSheet*>(GetStyleSheetPool()->Find( pC->GetStyle(), pC->GetFamily() ));
                DBG_ASSERT( pStyle, "InsertBinTextObject - Style not found!" );
                SetStyleSheet( nPara, pStyle );
            }
            if ( !bConvertMetricOfItems )
                SetParaAttribs( aEditDoc.GetPos( aPaM.GetNode() ), pC->GetParaAttribs() );
            else
            {
                SfxItemSet aAttribs( GetEmptyItemSet() );
                ConvertAndPutItems( aAttribs, pC->GetParaAttribs(), &eSourceUnit, &eDestUnit );
                SetParaAttribs( aEditDoc.GetPos( aPaM.GetNode() ), aAttribs );
            }
            if ( bNewContent && bUsePortionInfo )
            {
                const XParaPortion& rXP = (*pPortionInfo)[n];
                ParaPortion* pParaPortion = GetParaPortions()[ nPara ];
                DBG_ASSERT( pParaPortion, "InsertBinTextObject: ParaPortion?" );
                pParaPortion->nHeight = rXP.nHeight;
                pParaPortion->nFirstLineOffset = rXP.nFirstLineOffset;
                pParaPortion->bForceRepaint = true;
                pParaPortion->SetValid();   // Do not format

                // The Text Portions
                pParaPortion->GetTextPortions().Reset();
                sal_uInt16 nCount = rXP.aTextPortions.Count();
                for ( sal_uInt16 _n = 0; _n < nCount; _n++ )
                {
                    const TextPortion& rTextPortion = rXP.aTextPortions[_n];
                    TextPortion* pNew = new TextPortion( rTextPortion );
                    pParaPortion->GetTextPortions().Insert(_n, pNew);
                }

                // The lines
                pParaPortion->GetLines().Reset();
                nCount = rXP.aLines.Count();
                for ( sal_uInt16 m = 0; m < nCount; m++ )
                {
                    const EditLine& rLine = rXP.aLines[m];
                    EditLine* pNew = rLine.Clone();
                    pNew->SetInvalid(); // Paint again!
                    pParaPortion->GetLines().Insert(m, pNew);
                }
#ifdef DBG_UTIL
                sal_uInt16 nTest;
                int nTPLen = 0, nTxtLen = 0;
                for ( nTest = pParaPortion->GetTextPortions().Count(); nTest; )
                    nTPLen += pParaPortion->GetTextPortions()[--nTest].GetLen();
                for ( nTest = pParaPortion->GetLines().Count(); nTest; )
                    nTxtLen += pParaPortion->GetLines()[--nTest].GetLen();
                DBG_ASSERT( ( nTPLen == pParaPortion->GetNode()->Len() ) && ( nTxtLen == pParaPortion->GetNode()->Len() ), "InsertBinTextObject: ParaPortion not completely formatted!" );
#endif
            }
        }
        if ( !bParaAttribs ) // DefFont is not calculated for FastInsertParagraph
        {
            aPaM.GetNode()->GetCharAttribs().GetDefFont() = aEditDoc.GetDefFont();
            if ( aStatus.UseCharAttribs() )
                aPaM.GetNode()->CreateDefFont();
        }

        if ( bNewContent && GetStatus().DoOnlineSpelling() && pC->GetWrongList() )
        {
            aPaM.GetNode()->SetWrongList( pC->GetWrongList()->Clone() );
        }

        // Wrap when followed by other ...
        if ( n < ( nContents-1) )
        {
            if ( bNewContent )
                aPaM = ImpFastInsertParagraph( nPara+1 );
            else
                aPaM = ImpInsertParaBreak( aPaM, false );
        }
    }

    aSel.Max() = aPaM;
    DBG_ASSERT( !aSel.DbgIsBuggy( aEditDoc ), "InsertBibTextObject: Selection broken!(1)" );
    return aSel;
}

void ImpEditEngine::GetAllMisspellRanges( std::vector<editeng::MisspellRanges>& rRanges ) const
{
    std::vector<editeng::MisspellRanges> aRanges;
    const EditDoc& rDoc = GetEditDoc();
    for (sal_Int32 i = 0, n = rDoc.Count(); i < n; ++i)
    {
        const ContentNode* pNode = rDoc.GetObject(i);
        const WrongList* pWrongList = pNode->GetWrongList();
        if (!pWrongList)
            continue;

        aRanges.emplace_back(i, pWrongList->GetRanges());
    }

    aRanges.swap(rRanges);
}

void ImpEditEngine::SetAllMisspellRanges( const std::vector<editeng::MisspellRanges>& rRanges )
{
    EditDoc& rDoc = GetEditDoc();
    for (auto const& rParaRanges : rRanges)
    {
        ContentNode* pNode = rDoc.GetObject(rParaRanges.mnParagraph);
        if (!pNode)
            continue;

        pNode->CreateWrongList();
        WrongList* pWrongList = pNode->GetWrongList();
        pWrongList->SetRanges(rParaRanges.maRanges);
    }
}

LanguageType ImpEditEngine::GetLanguage( const EditPaM& rPaM, sal_Int32* pEndPos ) const
{
    short nScriptTypeI18N = GetI18NScriptType( rPaM, pEndPos ); // pEndPos will be valid now, pointing to ScriptChange or NodeLen
    SvtScriptType nScriptType = SvtLanguageOptions::FromI18NToSvtScriptType(nScriptTypeI18N);
    sal_uInt16 nLangId = GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType );
    const SvxLanguageItem* pLangItem = &static_cast<const SvxLanguageItem&>(rPaM.GetNode()->GetContentAttribs().GetItem( nLangId ));
    const EditCharAttrib* pAttr = rPaM.GetNode()->GetCharAttribs().FindAttrib( nLangId, rPaM.GetIndex() );
    if ( pAttr )
        pLangItem = static_cast<const SvxLanguageItem*>(pAttr->GetItem());

    if ( pEndPos && pAttr && ( pAttr->GetEnd() < *pEndPos ) )
        *pEndPos = pAttr->GetEnd();

    return pLangItem->GetLanguage();
}

css::lang::Locale ImpEditEngine::GetLocale( const EditPaM& rPaM ) const
{
    return LanguageTag( GetLanguage( rPaM ) ).getLocale();
}

Reference< XSpellChecker1 > const & ImpEditEngine::GetSpeller()
{
    if ( !xSpeller.is() )
        xSpeller = LinguMgr::GetSpellChecker();
    return xSpeller;
}


void ImpEditEngine::CreateSpellInfo( bool bMultipleDocs )
{
    if (!pSpellInfo)
        pSpellInfo.reset( new SpellInfo );
    else
        *pSpellInfo = SpellInfo();  // reset to default values

    pSpellInfo->bMultipleDoc = bMultipleDocs;
    // always spell draw objects completely, starting at the top.
    // (spelling in only a selection or not starting with the top requires
    // further changes elsewhere to work properly)
    pSpellInfo->aSpellStart = EPaM();
    pSpellInfo->aSpellTo    = EPaM( EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND );
}


EESpellState ImpEditEngine::Spell( EditView* pEditView, bool bMultipleDoc )
{
    SAL_WARN_IF( !xSpeller.is(), "editeng", "No Spell checker set!" );

    if ( !xSpeller.is() )
        return EESpellState::NoSpeller;

    aOnlineSpellTimer.Stop();

    // In MultipleDoc always from the front / rear ...
    if ( bMultipleDoc )
    {
        pEditView->pImpEditView->SetEditSelection( aEditDoc.GetStartPaM() );
    }

    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );
    CreateSpellInfo( bMultipleDoc );

    bool bIsStart = false;
    if ( bMultipleDoc )
        bIsStart = true;    // Accessible from the front or from behind ...
    else if ( CreateEPaM( aEditDoc.GetStartPaM() ) == pSpellInfo->aSpellStart )
        bIsStart = true;

    std::unique_ptr<EditSpellWrapper> pWrp(new EditSpellWrapper( Application::GetDefDialogParent(),
            bIsStart, pEditView ));
    pWrp->SpellDocument();
    pWrp.reset();

    if ( !bMultipleDoc )
    {
        pEditView->pImpEditView->DrawSelectionXOR();
        if ( aCurSel.Max().GetIndex() > aCurSel.Max().GetNode()->Len() )
            aCurSel.Max().SetIndex( aCurSel.Max().GetNode()->Len() );
        aCurSel.Min() = aCurSel.Max();
        pEditView->pImpEditView->SetEditSelection( aCurSel );
        pEditView->pImpEditView->DrawSelectionXOR();
        pEditView->ShowCursor( true, false );
    }
    EESpellState eState = pSpellInfo->eState;
    pSpellInfo.reset();
    return eState;
}


bool ImpEditEngine::HasConvertibleTextPortion( LanguageType nSrcLang )
{
    bool    bHasConvTxt = false;

    sal_Int32 nParas = pEditEngine->GetParagraphCount();
    for (sal_Int32 k = 0;  k < nParas;  ++k)
    {
        std::vector<sal_Int32> aPortions;
        pEditEngine->GetPortions( k, aPortions );
        for ( size_t nPos = 0; nPos < aPortions.size(); ++nPos )
        {
            sal_Int32 nEnd   = aPortions[ nPos ];
            sal_Int32 nStart = nPos > 0 ? aPortions[ nPos - 1 ] : 0;

            // if the paragraph is not empty we need to increase the index
            // by one since the attribute of the character left to the
            // specified position is evaluated.
            if (nEnd > nStart)  // empty para?
                ++nStart;
            LanguageType nLangFound = pEditEngine->GetLanguage( k, nStart );
#ifdef DEBUG
            lang::Locale aLocale( LanguageTag::convertToLocale( nLangFound ) );
#endif
            bHasConvTxt =   (nSrcLang == nLangFound) ||
                            (editeng::HangulHanjaConversion::IsChinese( nLangFound ) &&
                             editeng::HangulHanjaConversion::IsChinese( nSrcLang ));
            if (bHasConvTxt)
                return bHasConvTxt;
       }
    }

    return bHasConvTxt;
}


void ImpEditEngine::Convert( EditView* pEditView,
        LanguageType nSrcLang, LanguageType nDestLang, const vcl::Font *pDestFont,
        sal_Int32 nOptions, bool bIsInteractive, bool bMultipleDoc )
{
    // modified version of ImpEditEngine::Spell

    // In MultipleDoc always from the front / rear ...
    if ( bMultipleDoc )
        pEditView->pImpEditView->SetEditSelection( aEditDoc.GetStartPaM() );


    // initialize pConvInfo
    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );
    aCurSel.Adjust( aEditDoc );
    pConvInfo.reset(new ConvInfo);
    pConvInfo->bMultipleDoc = bMultipleDoc;
    pConvInfo->aConvStart = CreateEPaM( aCurSel.Min() );

    // if it is not just a selection and we are about to begin
    // with the current conversion for the very first time
    // we need to find the start of the current (initial)
    // convertible unit in order for the text conversion to give
    // the correct result for that. Since it is easier to obtain
    // the start of the word we use that though.
    if (!aCurSel.HasRange() && ImplGetBreakIterator().is())
    {
        EditPaM aWordStartPaM(  SelectWord( aCurSel, i18n::WordType::DICTIONARY_WORD ).Min() );

        // since #118246 / #117803 still occurs if the cursor is placed
        // between the two chinese characters to be converted (because both
        // of them are words on their own!) using the word boundary here does
        // not work. Thus since chinese conversion is not interactive we start
        // at the begin of the paragraph to solve the problem, i.e. have the
        // TextConversion service get those characters together in the same call.
        pConvInfo->aConvStart.nIndex = editeng::HangulHanjaConversion::IsChinese( nSrcLang )
            ? 0 : aWordStartPaM.GetIndex();
    }

    pConvInfo->aConvContinue = pConvInfo->aConvStart;

    bool bIsStart = false;
    if ( bMultipleDoc )
        bIsStart = true;    // Accessible from the front or from behind ...
    else if ( CreateEPaM( aEditDoc.GetStartPaM() ) == pConvInfo->aConvStart )
        bIsStart = true;

    TextConvWrapper aWrp( pEditView->GetWindow()->GetFrameWeld(),
                          ::comphelper::getProcessComponentContext(),
                          LanguageTag::convertToLocale( nSrcLang ),
                          LanguageTag::convertToLocale( nDestLang ),
                          pDestFont,
                          nOptions, bIsInteractive,
                          bIsStart, pEditView );


    //!! optimization does not work since when update mode is false
    //!! the object is 'lying' about it portions, paragraphs,
    //!! EndPaM... later on.
    //!! Should not be a great problem since text boxes or cells in
    //!! Calc usually have only a rather short text.
    //
    // disallow formatting, updating the view, ... while
    // non-interactively converting the document. (saves time)
    //if (!bIsInteractive)
    //  SetUpdateMode( sal_False );

    aWrp.Convert();

    //if (!bIsInteractive)
    //SetUpdateMode( sal_True, 0, sal_True );

    if ( !bMultipleDoc )
    {
        pEditView->pImpEditView->DrawSelectionXOR();
        if ( aCurSel.Max().GetIndex() > aCurSel.Max().GetNode()->Len() )
            aCurSel.Max().SetIndex( aCurSel.Max().GetNode()->Len() );
        aCurSel.Min() = aCurSel.Max();
        pEditView->pImpEditView->SetEditSelection( aCurSel );
        pEditView->pImpEditView->DrawSelectionXOR();
        pEditView->ShowCursor( true, false );
    }
    pConvInfo.reset();
}


void ImpEditEngine::SetLanguageAndFont(
    const ESelection &rESel,
    LanguageType nLang, sal_uInt16 nLangWhichId,
    const vcl::Font *pFont,  sal_uInt16 nFontWhichId )
{
    ESelection aOldSel = pActiveView->GetSelection();
    pActiveView->SetSelection( rESel );

    // set new language attribute
    SfxItemSet aNewSet( pActiveView->GetEmptyItemSet() );
    aNewSet.Put( SvxLanguageItem( nLang, nLangWhichId ) );

    // new font to be set?
    DBG_ASSERT( pFont, "target font missing?" );
    if (pFont)
    {
        // set new font attribute
        SvxFontItem aFontItem = static_cast<const SvxFontItem&>( aNewSet.Get( nFontWhichId ) );
        aFontItem.SetFamilyName( pFont->GetFamilyName());
        aFontItem.SetFamily( pFont->GetFamilyType());
        aFontItem.SetStyleName( pFont->GetStyleName());
        aFontItem.SetPitch( pFont->GetPitch());
        aFontItem.SetCharSet( pFont->GetCharSet() );
        aNewSet.Put( aFontItem );
    }

    // apply new attributes
    pActiveView->SetAttribs( aNewSet );

    pActiveView->SetSelection( aOldSel );
}


void ImpEditEngine::ImpConvert( OUString &rConvTxt, LanguageType &rConvTxtLang,
        EditView* pEditView, LanguageType nSrcLang, const ESelection &rConvRange,
        bool bAllowImplicitChangesForNotConvertibleText,
        LanguageType nTargetLang, const vcl::Font *pTargetFont  )
{
    // modified version of ImpEditEngine::ImpSpell

    // looks for next convertible text portion to be passed on to the wrapper

    OUString aRes;
    LanguageType nResLang = LANGUAGE_NONE;

    EditPaM aPos( CreateEditPaM( pConvInfo->aConvContinue ) );
    EditSelection aCurSel = EditSelection( aPos, aPos );

    OUString aWord;

    while (aRes.isEmpty())
    {
        // empty paragraph found that needs to have language and font set?
        if (bAllowImplicitChangesForNotConvertibleText &&
            pEditEngine->GetText( pConvInfo->aConvContinue.nPara ).isEmpty())
        {
            sal_Int32 nPara = pConvInfo->aConvContinue.nPara;
            ESelection aESel( nPara, 0, nPara, 0 );
            // see comment for below same function call
            SetLanguageAndFont( aESel,
                    nTargetLang, EE_CHAR_LANGUAGE_CJK,
                    pTargetFont, EE_CHAR_FONTINFO_CJK );
        }


        if (pConvInfo->aConvContinue.nPara  == pConvInfo->aConvTo.nPara &&
            pConvInfo->aConvContinue.nIndex >= pConvInfo->aConvTo.nIndex)
            break;

        sal_Int32 nAttribStart = -1;
        sal_Int32 nAttribEnd   = -1;
        sal_Int32 nCurPos      = -1;
        EPaM aCurStart = CreateEPaM( aCurSel.Min() );
        std::vector<sal_Int32> aPortions;
        pEditEngine->GetPortions( aCurStart.nPara, aPortions );
        for ( size_t nPos = 0; nPos < aPortions.size(); ++nPos )
        {
            const sal_Int32 nEnd   = aPortions[ nPos ];
            const sal_Int32 nStart = nPos > 0 ? aPortions[ nPos - 1 ] : 0;

            // the language attribute is obtained from the left character
            // (like usually all other attributes)
            // thus we usually have to add 1 in order to get the language
            // of the text right to the cursor position
            const sal_Int32 nLangIdx = nEnd > nStart ? nStart + 1 : nStart;
            LanguageType nLangFound = pEditEngine->GetLanguage( aCurStart.nPara, nLangIdx );
#ifdef DEBUG
            lang::Locale aLocale( LanguageTag::convertToLocale( nLangFound ) );
#endif
            bool bLangOk =  (nLangFound == nSrcLang) ||
                                (editeng::HangulHanjaConversion::IsChinese( nLangFound ) &&
                                 editeng::HangulHanjaConversion::IsChinese( nSrcLang ));

            if (nAttribEnd>=0) // start already found?
            {
                DBG_ASSERT(nEnd >= aCurStart.nIndex, "error while scanning attributes (a)" );
                DBG_ASSERT(nEnd >= nAttribEnd, "error while scanning attributes (b)" );
                if (/*nEnd >= aCurStart.nIndex &&*/ nLangFound == nResLang)
                    nAttribEnd = nEnd;
                else  // language attrib has changed
                    break;
            }
            if (nAttribStart<0 && // start not yet found?
                nEnd > aCurStart.nIndex && bLangOk)
            {
                nAttribStart = nStart;
                nAttribEnd   = nEnd;
                nResLang = nLangFound;
            }
            //! the list of portions may have changed compared to the previous
            //! call to this function (because of possibly changed language
            //! attribute!)
            //! But since we don't want to start in the already processed part
            //! we clip the start accordingly.
            if (nAttribStart >= 0 && nAttribStart < aCurStart.nIndex)
            {
                nAttribStart = aCurStart.nIndex;
            }

            // check script type to the right of the start of the current portion
            EditPaM aPaM( CreateEditPaM( EPaM(aCurStart.nPara, nLangIdx) ) );
            bool bIsAsianScript = (i18n::ScriptType::ASIAN == GetI18NScriptType( aPaM ));
            // not yet processed text part with for conversion
            // not suitable language found that needs to be changed?
            if (bAllowImplicitChangesForNotConvertibleText &&
                !bLangOk && !bIsAsianScript && nEnd > aCurStart.nIndex)
            {
                ESelection aESel( aCurStart.nPara, nStart, aCurStart.nPara, nEnd );
                // set language and font to target language and font of conversion
                //! Now this especially includes all non convertible text e.g.
                //! spaces, empty paragraphs and western text.
                // This is in order for every *new* text entered at *any* position to
                // have the correct language and font attributes set.
                SetLanguageAndFont( aESel,
                        nTargetLang, EE_CHAR_LANGUAGE_CJK,
                        pTargetFont, EE_CHAR_FONTINFO_CJK );
            }

            nCurPos = nEnd;
        }

        if (nAttribStart>=0 && nAttribEnd>=0)
        {
            aCurSel.Min().SetIndex( nAttribStart );
            aCurSel.Max().SetIndex( nAttribEnd );
        }
        else if (nCurPos>=0)
        {
            // set selection to end of scanned text
            // (used to set the position where to continue from later on)
            aCurSel.Min().SetIndex( nCurPos );
            aCurSel.Max().SetIndex( nCurPos );
        }

        if ( !pConvInfo->bConvToEnd )
        {
            EPaM aEPaM( CreateEPaM( aCurSel.Min() ) );
            if ( !( aEPaM < pConvInfo->aConvTo ) )
                break;
        }

        // clip selected word to the converted area
        // (main use when conversion starts/ends **within** a word)
        EditPaM aPaM( CreateEditPaM( pConvInfo->aConvStart ) );
        if (pConvInfo->bConvToEnd &&
            aCurSel.Min().GetNode() == aPaM.GetNode() &&
            aCurSel.Min().GetIndex() < aPaM.GetIndex())
                aCurSel.Min().SetIndex( aPaM.GetIndex() );
        aPaM = CreateEditPaM( pConvInfo->aConvContinue );
        if (aCurSel.Min().GetNode() == aPaM.GetNode() &&
            aCurSel.Min().GetIndex() < aPaM.GetIndex())
                aCurSel.Min().SetIndex( aPaM.GetIndex() );
        aPaM = CreateEditPaM( pConvInfo->aConvTo );
        if ((!pConvInfo->bConvToEnd || rConvRange.HasRange())&&
            aCurSel.Max().GetNode() == aPaM.GetNode() &&
            aCurSel.Max().GetIndex() > aPaM.GetIndex())
                aCurSel.Max().SetIndex( aPaM.GetIndex() );

        aWord = GetSelected( aCurSel );

        if ( !aWord.isEmpty() /* && bLangOk */)
            aRes = aWord;

        // move to next word/paragraph if necessary
        if ( aRes.isEmpty() )
            aCurSel = WordRight( aCurSel.Min(), css::i18n::WordType::DICTIONARY_WORD );

        pConvInfo->aConvContinue = CreateEPaM( aCurSel.Max() );
    }

    pEditView->pImpEditView->DrawSelectionXOR();
    pEditView->pImpEditView->SetEditSelection( aCurSel );
    pEditView->pImpEditView->DrawSelectionXOR();
    pEditView->ShowCursor( true, false );

    rConvTxt = aRes;
    if ( !rConvTxt.isEmpty() )
        rConvTxtLang = nResLang;
}


Reference< XSpellAlternatives > ImpEditEngine::ImpSpell( EditView* pEditView )
{
    DBG_ASSERT( xSpeller.is(), "No spell checker set!" );

    ContentNode* pLastNode = aEditDoc.GetObject( aEditDoc.Count()-1 );
    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );
    aCurSel.Min() = aCurSel.Max();

    Reference< XSpellAlternatives > xSpellAlt;
    Sequence< PropertyValue > aEmptySeq;
    while (!xSpellAlt.is())
    {
        // Known (most likely) bug: If SpellToCurrent, the current has to be
        // corrected at each replacement, otherwise it may not fit exactly in
        // the end ...
        if ( pSpellInfo->bSpellToEnd || pSpellInfo->bMultipleDoc )
        {
            if ( aCurSel.Max().GetNode() == pLastNode )
            {
                if ( aCurSel.Max().GetIndex() >= pLastNode->Len() )
                    break;
            }
        }
        else if ( !pSpellInfo->bSpellToEnd )
        {
            EPaM aEPaM( CreateEPaM( aCurSel.Max() ) );
            if ( !( aEPaM < pSpellInfo->aSpellTo ) )
                break;
        }

        aCurSel = SelectWord( aCurSel, css::i18n::WordType::DICTIONARY_WORD );
        OUString aWord = GetSelected( aCurSel );

        // If afterwards a dot, this must be handed over!
        // If an abbreviation ...
        if ( !aWord.isEmpty() && ( aCurSel.Max().GetIndex() < aCurSel.Max().GetNode()->Len() ) )
        {
            sal_Unicode cNext = aCurSel.Max().GetNode()->GetChar( aCurSel.Max().GetIndex() );
            if ( cNext == '.' )
            {
                aCurSel.Max().SetIndex( aCurSel.Max().GetIndex()+1 );
                aWord += OUStringLiteral1(cNext);
            }
        }

        if ( !aWord.isEmpty() )
        {
            LanguageType eLang = GetLanguage( aCurSel.Max() );
            SvxSpellWrapper::CheckSpellLang( xSpeller, eLang );
            xSpellAlt = xSpeller->spell( aWord, static_cast<sal_uInt16>(eLang), aEmptySeq );
        }

        if ( !xSpellAlt.is() )
            aCurSel = WordRight( aCurSel.Min(), css::i18n::WordType::DICTIONARY_WORD );
        else
            pSpellInfo->eState = EESpellState::ErrorFound;
    }

    pEditView->pImpEditView->DrawSelectionXOR();
    pEditView->pImpEditView->SetEditSelection( aCurSel );
    pEditView->pImpEditView->DrawSelectionXOR();
    pEditView->ShowCursor( true, false );
    return xSpellAlt;
}

Reference< XSpellAlternatives > ImpEditEngine::ImpFindNextError(EditSelection& rSelection)
{
    EditSelection aCurSel( rSelection.Min() );

    Reference< XSpellAlternatives > xSpellAlt;
    Sequence< PropertyValue > aEmptySeq;
    while (!xSpellAlt.is())
    {
        //check if the end of the selection has been reached
        {
            EPaM aEPaM( CreateEPaM( aCurSel.Max() ) );
            if ( !( aEPaM < CreateEPaM( rSelection.Max()) ) )
                break;
        }

        aCurSel = SelectWord( aCurSel, css::i18n::WordType::DICTIONARY_WORD );
        OUString aWord = GetSelected( aCurSel );

        // If afterwards a dot, this must be handed over!
        // If an abbreviation ...
        if ( !aWord.isEmpty() && ( aCurSel.Max().GetIndex() < aCurSel.Max().GetNode()->Len() ) )
        {
            sal_Unicode cNext = aCurSel.Max().GetNode()->GetChar( aCurSel.Max().GetIndex() );
            if ( cNext == '.' )
            {
                aCurSel.Max().SetIndex( aCurSel.Max().GetIndex()+1 );
                aWord += OUStringLiteral1(cNext);
            }
        }

        if ( !aWord.isEmpty() )
            xSpellAlt = xSpeller->spell( aWord, static_cast<sal_uInt16>(GetLanguage( aCurSel.Max() )), aEmptySeq );

        if ( !xSpellAlt.is() )
            aCurSel = WordRight( aCurSel.Min(), css::i18n::WordType::DICTIONARY_WORD );
        else
        {
            pSpellInfo->eState = EESpellState::ErrorFound;
            rSelection = aCurSel;
        }
    }
    return xSpellAlt;
}

bool ImpEditEngine::SpellSentence(EditView const & rEditView,
    svx::SpellPortions& rToFill )
{
    bool bRet = false;
    EditSelection aCurSel( rEditView.pImpEditView->GetEditSelection() );
    if(!pSpellInfo)
        CreateSpellInfo( true );
    pSpellInfo->aCurSentenceStart = aCurSel.Min();
    DBG_ASSERT( xSpeller.is(), "No spell checker set!" );
    pSpellInfo->aLastSpellPortions.clear();
    pSpellInfo->aLastSpellContentSelections.clear();
    rToFill.clear();
    //if no selection previously exists the range is extended to the end of the object
    if (!aCurSel.HasRange())
    {
        ContentNode* pLastNode = aEditDoc.GetObject( aEditDoc.Count()-1);
        aCurSel.Max() = EditPaM(pLastNode, pLastNode->Len());
    }
    // check for next error in aCurSel and set aCurSel to that one if any was found
    Reference< XSpellAlternatives > xAlt = ImpFindNextError(aCurSel);
    if (xAlt.is())
    {
        bRet = true;
        //find the sentence boundaries
        EditSelection aSentencePaM = SelectSentence(aCurSel);
        //make sure that the sentence is never smaller than the error range!
        if(aSentencePaM.Max().GetIndex() < aCurSel.Max().GetIndex())
            aSentencePaM.Max() = aCurSel.Max();
        //add the portion preceding the error
        EditSelection aStartSelection(aSentencePaM.Min(), aCurSel.Min());
        if(aStartSelection.HasRange())
            AddPortionIterated(rEditView, aStartSelection, nullptr, rToFill);
        //add the error portion
        AddPortionIterated(rEditView, aCurSel, xAlt, rToFill);
        //find the end of the sentence
        //search for all errors in the rest of the sentence and add all the portions
        do
        {
            EditSelection aNextSel = EditSelection(aCurSel.Max(), aSentencePaM.Max());
            xAlt = ImpFindNextError(aNextSel);
            if(xAlt.is())
            {
                //add the part between the previous and the current error
                AddPortionIterated(rEditView, EditSelection(aCurSel.Max(), aNextSel.Min()), nullptr, rToFill);
                //add the current error
                AddPortionIterated(rEditView, aNextSel, xAlt, rToFill);
            }
            else
                AddPortionIterated(rEditView, EditSelection(aCurSel.Max(), aSentencePaM.Max()), xAlt, rToFill);
            aCurSel = aNextSel;
        }
        while( xAlt.is() );

        //set the selection to the end of the current sentence
        rEditView.pImpEditView->SetEditSelection(aSentencePaM.Max());
    }
    return bRet;
}

// Adds one portion to the SpellPortions
void ImpEditEngine::AddPortion(
                            const EditSelection& rSel,
                            const uno::Reference< XSpellAlternatives >& xAlt,
                            svx::SpellPortions& rToFill,
                            bool bIsField)
{
    if(rSel.HasRange())
    {
        svx::SpellPortion aPortion;
        aPortion.sText = GetSelected( rSel );
        aPortion.eLanguage = GetLanguage( rSel.Min() );
        aPortion.xAlternatives = xAlt;
        aPortion.bIsField = bIsField;
        rToFill.push_back(aPortion);

        //save the spelled portions for later use
        pSpellInfo->aLastSpellPortions.push_back(aPortion);
        pSpellInfo->aLastSpellContentSelections.push_back(rSel);

    }
}

// Adds one or more portions of text to the SpellPortions depending on language changes
void ImpEditEngine::AddPortionIterated(
                            EditView const & rEditView,
                            const EditSelection& rSel,
                            const Reference< XSpellAlternatives >& xAlt,
                            svx::SpellPortions& rToFill)
{
    if (rSel.HasRange())
    {
        if(xAlt.is())
        {
            AddPortion(rSel, xAlt, rToFill, false);
        }
        else
        {
            //iterate and search for language attribute changes
            //save the start and end positions
            bool bTest = rSel.Min().GetIndex() <= rSel.Max().GetIndex();
            EditPaM aStart(bTest ? rSel.Min() : rSel.Max());
            EditPaM aEnd(bTest ? rSel.Max() : rSel.Min());
            //iterate over the text to find changes in language
            //set the mark equal to the point
            EditPaM aCursor(aStart);
            rEditView.pImpEditView->SetEditSelection( aCursor );
            LanguageType eStartLanguage = GetLanguage( aCursor );
            //search for a field attribute at the beginning - only the end position
            //of this field is kept to end a portion at that position
            const EditCharAttrib* pFieldAttr = aCursor.GetNode()->GetCharAttribs().
                                                    FindFeature( aCursor.GetIndex() );
            bool bIsField = pFieldAttr &&
                    pFieldAttr->GetStart() == aCursor.GetIndex() &&
                    pFieldAttr->GetStart() != pFieldAttr->GetEnd() &&
                    pFieldAttr->Which() == EE_FEATURE_FIELD;
            sal_Int32 nEndField = bIsField ? pFieldAttr->GetEnd() : -1;
            do
            {
                aCursor = CursorRight( aCursor);
                //determine whether a field and has been reached
                bool bIsEndField = nEndField == aCursor.GetIndex();
                //search for a new field attribute
                const EditCharAttrib* _pFieldAttr = aCursor.GetNode()->GetCharAttribs().
                                                        FindFeature( aCursor.GetIndex() );
                bIsField = _pFieldAttr &&
                        _pFieldAttr->GetStart() == aCursor.GetIndex() &&
                        _pFieldAttr->GetStart() != _pFieldAttr->GetEnd() &&
                        _pFieldAttr->Which() == EE_FEATURE_FIELD;
                //on every new field move the end position
                if (bIsField)
                    nEndField = _pFieldAttr->GetEnd();

                LanguageType eCurLanguage = GetLanguage( aCursor );
                if(eCurLanguage != eStartLanguage || bIsField || bIsEndField)
                {
                    eStartLanguage = eCurLanguage;
                    //go one step back - the cursor currently selects the first character
                    //with a different language
                    //create a selection from start to the current Cursor
                    EditSelection aSelection(aStart, aCursor);
                    AddPortion(aSelection, xAlt, rToFill, bIsEndField);
                    aStart = aCursor;
                }
            }
            while(aCursor.GetIndex() < aEnd.GetIndex());
            EditSelection aSelection(aStart, aCursor);
            AddPortion(aSelection, xAlt, rToFill, bIsField);
        }
    }
}

void ImpEditEngine::ApplyChangedSentence(EditView const & rEditView,
    const svx::SpellPortions& rNewPortions,
    bool bRecheck )
{
    // Note: rNewPortions.size() == 0 is valid and happens when the whole
    // sentence got removed in the dialog

    DBG_ASSERT(pSpellInfo, "pSpellInfo not initialized");
    if (pSpellInfo &&
        !pSpellInfo->aLastSpellPortions.empty())  // no portions -> no text to be changed
    {
        // get current paragraph length to calculate later on how the sentence length changed,
        // in order to place the cursor at the end of the sentence again
        EditSelection aOldSel( rEditView.pImpEditView->GetEditSelection() );
        sal_Int32 nOldLen = aOldSel.Max().GetNode()->Len();

        UndoActionStart( EDITUNDO_INSERT );
        if(pSpellInfo->aLastSpellPortions.size() == rNewPortions.size())
        {
            DBG_ASSERT( !rNewPortions.empty(), "rNewPortions should not be empty here" );
            DBG_ASSERT( pSpellInfo->aLastSpellPortions.size() == pSpellInfo->aLastSpellContentSelections.size(),
                    "aLastSpellPortions and aLastSpellContentSelections size mismatch" );

            //the simple case: the same number of elements on both sides
            //each changed element has to be applied to the corresponding source element
            svx::SpellPortions::const_iterator aCurrentNewPortion = rNewPortions.end();
            svx::SpellPortions::const_iterator aCurrentOldPortion = pSpellInfo->aLastSpellPortions.end();
            SpellContentSelections::const_iterator aCurrentOldPosition = pSpellInfo->aLastSpellContentSelections.end();
            bool bSetToEnd = false;
            do
            {
                --aCurrentNewPortion;
                --aCurrentOldPortion;
                --aCurrentOldPosition;
                //set the cursor to the end of the sentence - necessary to
                //resume there at the next step
                if(!bSetToEnd)
                {
                    bSetToEnd = true;
                    rEditView.pImpEditView->SetEditSelection( aCurrentOldPosition->Max() );
                }

                SvtScriptType nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( aCurrentNewPortion->eLanguage );
                sal_uInt16 nLangWhichId = EE_CHAR_LANGUAGE;
                switch(nScriptType)
                {
                    case SvtScriptType::ASIAN : nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
                    case SvtScriptType::COMPLEX : nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
                    default: break;
                }
                if(aCurrentNewPortion->sText != aCurrentOldPortion->sText)
                {
                    //change text and apply language
                    SfxItemSet aSet( aEditDoc.GetItemPool(), {{nLangWhichId, nLangWhichId}});
                    aSet.Put(SvxLanguageItem(aCurrentNewPortion->eLanguage, nLangWhichId));
                    SetAttribs( *aCurrentOldPosition, aSet );
                    ImpInsertText( *aCurrentOldPosition, aCurrentNewPortion->sText );
                }
                else if(aCurrentNewPortion->eLanguage != aCurrentOldPortion->eLanguage)
                {
                    //apply language
                    SfxItemSet aSet( aEditDoc.GetItemPool(), {{nLangWhichId, nLangWhichId}});
                    aSet.Put(SvxLanguageItem(aCurrentNewPortion->eLanguage, nLangWhichId));
                    SetAttribs( *aCurrentOldPosition, aSet );
                }
            }
            while(aCurrentNewPortion != rNewPortions.begin());
        }
        else
        {
            DBG_ASSERT( !pSpellInfo->aLastSpellContentSelections.empty(), "aLastSpellContentSelections should not be empty here" );

            //select the complete sentence
            SpellContentSelections::const_iterator aCurrentEndPosition = pSpellInfo->aLastSpellContentSelections.end();
            --aCurrentEndPosition;
            SpellContentSelections::const_iterator aCurrentStartPosition = pSpellInfo->aLastSpellContentSelections.begin();
            EditSelection aAllSentence(aCurrentStartPosition->Min(), aCurrentEndPosition->Max());

            //delete the sentence completely
            ImpDeleteSelection( aAllSentence );
            EditPaM aCurrentPaM = aAllSentence.Min();
            for(const auto& rCurrentNewPortion : rNewPortions)
            {
                //set the language attribute
                LanguageType eCurLanguage = GetLanguage( aCurrentPaM );
                if(eCurLanguage != rCurrentNewPortion.eLanguage)
                {
                    SvtScriptType nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( rCurrentNewPortion.eLanguage );
                    sal_uInt16 nLangWhichId = EE_CHAR_LANGUAGE;
                    switch(nScriptType)
                    {
                        case SvtScriptType::ASIAN : nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
                        case SvtScriptType::COMPLEX : nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
                        default: break;
                    }
                    SfxItemSet aSet( aEditDoc.GetItemPool(), {{nLangWhichId, nLangWhichId}});
                    aSet.Put(SvxLanguageItem(rCurrentNewPortion.eLanguage, nLangWhichId));
                    SetAttribs( aCurrentPaM, aSet );
                }
                //insert the new string and set the cursor to the end of the inserted string
                aCurrentPaM = ImpInsertText( aCurrentPaM , rCurrentNewPortion.sText );
            }
        }
        UndoActionEnd();

        EditPaM aNext;
        if (bRecheck)
            aNext = pSpellInfo->aCurSentenceStart;
        else
        {
            // restore cursor position to the end of the modified sentence.
            // (This will define the continuation position for spell/grammar checking)
            // First: check if the sentence/para length changed
            const sal_Int32 nDelta = rEditView.pImpEditView->GetEditSelection().Max().GetNode()->Len() - nOldLen;
            const sal_Int32 nEndOfSentence = aOldSel.Max().GetIndex() + nDelta;
            aNext = EditPaM( aOldSel.Max().GetNode(), nEndOfSentence );
        }
        rEditView.pImpEditView->SetEditSelection( aNext );

        FormatAndUpdate();
        aEditDoc.SetModified(true);
    }
}

void ImpEditEngine::PutSpellingToSentenceStart( EditView const & rEditView )
{
    if( pSpellInfo && !pSpellInfo->aLastSpellContentSelections.empty() )
    {
        rEditView.pImpEditView->SetEditSelection( pSpellInfo->aLastSpellContentSelections.begin()->Min() );
    }
}


void ImpEditEngine::DoOnlineSpelling( ContentNode* pThisNodeOnly, bool bSpellAtCursorPos, bool bInterruptible )
{
    /*
     It will iterate over all the paragraphs, paragraphs with only
     invalidated wrong list will be checked ...

     All the words are checked in the invalidated region. Is a word wrong,
     but not in the wrong list, or vice versa, the range of the word will be
     invalidated
     (no Invalidate, but if only transitions wrong from right =>, simple Paint,
      even out properly with VDev on transitions from wrong => right)
    */

    if ( !xSpeller.is() )
        return;

    EditPaM aCursorPos;
    if( pActiveView && !bSpellAtCursorPos )
    {
        aCursorPos = pActiveView->pImpEditView->GetEditSelection().Max();
    }

    bool bRestartTimer = false;

    ContentNode* pLastNode = aEditDoc.GetObject( aEditDoc.Count() - 1 );
    sal_Int32 nNodes = GetEditDoc().Count();
    sal_Int32 nInvalids = 0;
    Sequence< PropertyValue > aEmptySeq;
    for ( sal_Int32 n = 0; n < nNodes; n++ )
    {
        ContentNode* pNode = GetEditDoc().GetObject( n );
        if ( pThisNodeOnly )
            pNode = pThisNodeOnly;

        pNode->EnsureWrongList();
        if (!pNode->GetWrongList()->IsValid())
        {
            WrongList* pWrongList = pNode->GetWrongList();
            const size_t nInvStart = pWrongList->GetInvalidStart();
            const size_t nInvEnd = pWrongList->GetInvalidEnd();

            sal_Int32 nPaintFrom = -1;
            sal_Int32 nPaintTo = 0;
            bool bSimpleRepaint = true;

            pWrongList->SetValid();

            EditPaM aPaM( pNode, nInvStart );
            EditSelection aSel( aPaM, aPaM );
            while ( aSel.Max().GetNode() == pNode )
            {
                if ( ( static_cast<size_t>(aSel.Min().GetIndex()) > nInvEnd )
                        || ( ( aSel.Max().GetNode() == pLastNode ) && ( aSel.Max().GetIndex() >= pLastNode->Len() ) ) )
                    break;  // Document end or end of invalid region

                aSel = SelectWord( aSel, i18n::WordType::DICTIONARY_WORD );
                // If afterwards a dot, this must be handed over!
                // If an abbreviation ...
                bool bDottAdded = false;
                if ( aSel.Max().GetIndex() < aSel.Max().GetNode()->Len() )
                {
                    sal_Unicode cNext = aSel.Max().GetNode()->GetChar( aSel.Max().GetIndex() );
                    if ( cNext == '.' )
                    {
                        aSel.Max().SetIndex( aSel.Max().GetIndex()+1 );
                        bDottAdded = true;
                    }
                }
                OUString aWord = GetSelected(aSel);

                bool bChanged = false;
                if (!aWord.isEmpty())
                {
                    const sal_Int32 nWStart = aSel.Min().GetIndex();
                    const sal_Int32 nWEnd = aSel.Max().GetIndex();
                    if ( !xSpeller->isValid( aWord, static_cast<sal_uInt16>(GetLanguage( EditPaM( aSel.Min().GetNode(), nWStart+1 ) )), aEmptySeq ) )
                    {
                        // Check if already marked correctly...
                        const sal_Int32 nXEnd = bDottAdded ? nWEnd -1 : nWEnd;
                        if ( !pWrongList->HasWrong( nWStart, nXEnd ) )
                        {
                            // Mark Word as wrong...
                            // But only when not at Cursor-Position...
                            bool bCursorPos = false;
                            if ( aCursorPos.GetNode() == pNode )
                            {
                                if ( ( nWStart <= aCursorPos.GetIndex() ) && nWEnd >= aCursorPos.GetIndex() )
                                    bCursorPos = true;
                            }
                            if ( bCursorPos )
                            {
                                // Then continue to mark as invalid ...
                                pWrongList->ResetInvalidRange(nWStart, nWEnd);
                                bRestartTimer = true;
                            }
                            else
                            {
                                // It may be that the Wrongs in the list ar not
                                // spanning exactly over words because the
                                // WordDelimiters during expansion are not
                                // evaluated.
                                pWrongList->InsertWrong(nWStart, nXEnd);
                                bChanged = true;
                            }
                        }
                    }
                    else
                    {
                        // Check if not marked as wrong
                        if ( pWrongList->HasAnyWrong( nWStart, nWEnd ) )
                        {
                            pWrongList->ClearWrongs( nWStart, nWEnd, pNode );
                            bSimpleRepaint = false;
                            bChanged = true;
                        }
                    }
                    if ( bChanged  )
                    {
                        if ( nPaintFrom<0 )
                            nPaintFrom = nWStart;
                        nPaintTo = nWEnd;
                    }
                }

                EditPaM aLastEnd( aSel.Max() );
                aSel = WordRight( aSel.Max(), i18n::WordType::DICTIONARY_WORD );
                if ( bChanged && ( aSel.Min().GetNode() == pNode ) &&
                        ( aSel.Min().GetIndex()-aLastEnd.GetIndex() > 1 ) )
                {
                    // If two words are separated by more than one blank, it
                    // can happen that when splitting a Wrongs the start of
                    // the second word is before the actually word
                    pWrongList->ClearWrongs( aLastEnd.GetIndex(), aSel.Min().GetIndex(), pNode );
                }
            }

            // Invalidate?
            if ( nPaintFrom>=0 )
            {
                aStatus.GetStatusWord() |= EditStatusFlags::WRONGWORDCHANGED;
                CallStatusHdl();

                if (!aEditViews.empty())
                {
                    // For SimpleRepaint one was painted over a range without
                    // reaching VDEV, but then one would have to intersect, c
                    // clipping, ... over all views. Probably not worthwhile.
                    EditPaM aStartPaM( pNode, nPaintFrom );
                    EditPaM aEndPaM( pNode, nPaintTo );
                    tools::Rectangle aStartCursor( PaMtoEditCursor( aStartPaM ) );
                    tools::Rectangle aEndCursor( PaMtoEditCursor( aEndPaM ) );
                    DBG_ASSERT( aInvalidRect.IsEmpty(), "InvalidRect set!" );
                    aInvalidRect.SetLeft( 0 );
                    aInvalidRect.SetRight( GetPaperSize().Width() );
                    aInvalidRect.SetTop( aStartCursor.Top() );
                    aInvalidRect.SetBottom( aEndCursor.Bottom() );
                    if ( pActiveView && pActiveView->HasSelection() )
                    {
                        // Then no output through VDev.
                        UpdateViews();
                    }
                    else if ( bSimpleRepaint )
                    {
                        for (EditView* pView : aEditViews)
                        {
                            tools::Rectangle aClipRect( aInvalidRect );
                            aClipRect.Intersection( pView->GetVisArea() );
                            if ( !aClipRect.IsEmpty() )
                            {
                                // convert to window coordinates ....
                                aClipRect.SetPos( pView->pImpEditView->GetWindowPos( aClipRect.TopLeft() ) );
                                pView->pImpEditView->GetWindow()->Invalidate(aClipRect);
                            }
                        }
                    }
                    else
                    {
                        UpdateViews( pActiveView );
                    }
                    aInvalidRect = tools::Rectangle();
                }
            }
            // After two corrected nodes give up the control ...
            nInvalids++;
            if ( bInterruptible && ( nInvalids >= 2 ) )
            {
                bRestartTimer = true;
                break;
            }
        }

        if ( pThisNodeOnly )
            break;
    }
    if ( bRestartTimer )
        aOnlineSpellTimer.Start();
}


EESpellState ImpEditEngine::HasSpellErrors()
{
    DBG_ASSERT( xSpeller.is(), "No spell checker set!" );

    ContentNode* pLastNode = aEditDoc.GetObject( aEditDoc.Count() - 1 );
    EditSelection aCurSel( aEditDoc.GetStartPaM() );

    OUString aWord;
    Reference< XSpellAlternatives > xSpellAlt;
    Sequence< PropertyValue > aEmptySeq;
    while ( !xSpellAlt.is() )
    {
        if ( ( aCurSel.Max().GetNode() == pLastNode ) &&
             ( aCurSel.Max().GetIndex() >= pLastNode->Len() ) )
        {
            return EESpellState::Ok;
        }

        aCurSel = SelectWord( aCurSel, css::i18n::WordType::DICTIONARY_WORD );
        aWord = GetSelected( aCurSel );
        if ( !aWord.isEmpty() )
        {
            LanguageType eLang = GetLanguage( aCurSel.Max() );
            SvxSpellWrapper::CheckSpellLang( xSpeller, eLang );
            xSpellAlt = xSpeller->spell( aWord, static_cast<sal_uInt16>(eLang), aEmptySeq );
        }
        aCurSel = WordRight( aCurSel.Max(), css::i18n::WordType::DICTIONARY_WORD );
    }

    return EESpellState::ErrorFound;
}

void ImpEditEngine::ClearSpellErrors()
{
    aEditDoc.ClearSpellErrors();
}

EESpellState ImpEditEngine::StartThesaurus( EditView* pEditView )
{
    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );
    if ( !aCurSel.HasRange() )
        aCurSel = SelectWord( aCurSel, css::i18n::WordType::DICTIONARY_WORD );
    OUString aWord( GetSelected( aCurSel ) );

    Reference< XThesaurus > xThes( LinguMgr::GetThesaurus() );
    if (!xThes.is())
        return EESpellState::ErrorFound;

    EditAbstractDialogFactory* pFact = EditAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractThesaurusDialog> xDlg(pFact->CreateThesaurusDialog( pEditView->GetWindow(), xThes, aWord, GetLanguage( aCurSel.Max() ) ));
    if (xDlg->Execute() == RET_OK)
    {
        // Replace Word...
        pEditView->pImpEditView->DrawSelectionXOR();
        pEditView->pImpEditView->SetEditSelection( aCurSel );
        pEditView->pImpEditView->DrawSelectionXOR();
        pEditView->InsertText(xDlg->GetWord());
        pEditView->ShowCursor(true, false);
    }

    return EESpellState::Ok;
}

sal_Int32 ImpEditEngine::StartSearchAndReplace( EditView* pEditView, const SvxSearchItem& rSearchItem )
{
    sal_Int32 nFound = 0;

    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );

    // FIND_ALL is not possible without multiple selection.
    if ( ( rSearchItem.GetCommand() == SvxSearchCmd::FIND ) ||
         ( rSearchItem.GetCommand() == SvxSearchCmd::FIND_ALL ) )
    {
        if ( Search( rSearchItem, pEditView ) )
            nFound++;
    }
    else if ( rSearchItem.GetCommand() == SvxSearchCmd::REPLACE )
    {
        // The word is selected if the user not altered the selection
        // in between:
        if ( aCurSel.HasRange() )
        {
            pEditView->InsertText( rSearchItem.GetReplaceString() );
            nFound = 1;
        }
        else
            if( Search( rSearchItem, pEditView ) )
                nFound = 1;
    }
    else if ( rSearchItem.GetCommand() == SvxSearchCmd::REPLACE_ALL )
    {
        // The Writer replaces all front beginning to end ...
        SvxSearchItem aTmpItem( rSearchItem );
        aTmpItem.SetBackward( false );

        pEditView->pImpEditView->DrawSelectionXOR();

        aCurSel.Adjust( aEditDoc );
        EditPaM aStartPaM = aTmpItem.GetSelection() ? aCurSel.Min() : aEditDoc.GetStartPaM();
        EditSelection aFoundSel( aCurSel.Max() );
        bool bFound = ImpSearch( aTmpItem, aCurSel, aStartPaM, aFoundSel );
        if ( bFound )
            UndoActionStart( EDITUNDO_REPLACEALL );
        while ( bFound )
        {
            nFound++;
            aStartPaM = ImpInsertText( aFoundSel, rSearchItem.GetReplaceString() );
            bFound = ImpSearch( aTmpItem, aCurSel, aStartPaM, aFoundSel );
        }
        if ( nFound )
        {
            EditPaM aNewPaM( aFoundSel.Max() );
            if ( aNewPaM.GetIndex() > aNewPaM.GetNode()->Len() )
                aNewPaM.SetIndex( aNewPaM.GetNode()->Len() );
            pEditView->pImpEditView->SetEditSelection( aNewPaM );
            FormatAndUpdate( pEditView );
            UndoActionEnd();
        }
        else
        {
            pEditView->pImpEditView->DrawSelectionXOR();
            pEditView->ShowCursor( true, false );
        }
    }
    return nFound;
}

bool ImpEditEngine::Search( const SvxSearchItem& rSearchItem, EditView* pEditView )
{
    EditSelection aSel( pEditView->pImpEditView->GetEditSelection() );
    aSel.Adjust( aEditDoc );
    EditPaM aStartPaM( aSel.Max() );
    if ( rSearchItem.GetSelection() && !rSearchItem.GetBackward() )
        aStartPaM = aSel.Min();

    EditSelection aFoundSel;
    bool bFound = ImpSearch( rSearchItem, aSel, aStartPaM, aFoundSel );
    if ( bFound && ( aFoundSel == aSel ) )  // For backwards-search
    {
        aStartPaM = aSel.Min();
        bFound = ImpSearch( rSearchItem, aSel, aStartPaM, aFoundSel );
    }

    pEditView->pImpEditView->DrawSelectionXOR();
    if ( bFound )
    {
        // First, set the minimum, so the whole word is in the visible range.
        pEditView->pImpEditView->SetEditSelection( aFoundSel.Min() );
        pEditView->ShowCursor( true, false );
        pEditView->pImpEditView->SetEditSelection( aFoundSel );
    }
    else
        pEditView->pImpEditView->SetEditSelection( aSel.Max() );

    pEditView->pImpEditView->DrawSelectionXOR();
    pEditView->ShowCursor( true, false );
    return bFound;
}

bool ImpEditEngine::ImpSearch( const SvxSearchItem& rSearchItem,
    const EditSelection& rSearchSelection, const EditPaM& rStartPos, EditSelection& rFoundSel )
{
    i18nutil::SearchOptions2 aSearchOptions( rSearchItem.GetSearchOptions() );
    aSearchOptions.Locale = GetLocale( rStartPos );

    bool bBack = rSearchItem.GetBackward();
    bool bSearchInSelection = rSearchItem.GetSelection();
    sal_Int32 nStartNode = aEditDoc.GetPos( rStartPos.GetNode() );
    sal_Int32 nEndNode;
    if ( bSearchInSelection )
    {
        nEndNode = aEditDoc.GetPos( bBack ? rSearchSelection.Min().GetNode() : rSearchSelection.Max().GetNode() );
    }
    else
    {
        nEndNode = bBack ? 0 : aEditDoc.Count()-1;
    }

    utl::TextSearch aSearcher( aSearchOptions );

    // iterate over the paragraphs ...
    for ( sal_Int32 nNode = nStartNode;
            bBack ? ( nNode >= nEndNode ) : ( nNode <= nEndNode) ;
            bBack ? nNode-- : nNode++ )
    {
        // For backwards-search if nEndNode = 0:
        if ( nNode < 0 )
            return false;

        ContentNode* pNode = aEditDoc.GetObject( nNode );

        sal_Int32 nStartPos = 0;
        sal_Int32 nEndPos = pNode->GetExpandedLen();
        if ( nNode == nStartNode )
        {
            if ( bBack )
                nEndPos = rStartPos.GetIndex();
            else
                nStartPos = rStartPos.GetIndex();
        }
        if ( ( nNode == nEndNode ) && bSearchInSelection )
        {
            if ( bBack )
                nStartPos = rSearchSelection.Min().GetIndex();
            else
                nEndPos = rSearchSelection.Max().GetIndex();
        }

        // Searching ...
        OUString aParaStr( pNode->GetExpandedText() );
        bool bFound = false;
        if ( bBack )
        {
            sal_Int32 nTemp;
            nTemp = nStartPos;
            nStartPos = nEndPos;
            nEndPos = nTemp;

            bFound = aSearcher.SearchBackward( aParaStr, &nStartPos, &nEndPos);
        }
        else
        {
            bFound = aSearcher.SearchForward( aParaStr, &nStartPos, &nEndPos);
        }
        if ( bFound )
        {
            pNode->UnExpandPositions( nStartPos, nEndPos );

            rFoundSel.Min().SetNode( pNode );
            rFoundSel.Min().SetIndex( nStartPos );
            rFoundSel.Max().SetNode( pNode );
            rFoundSel.Max().SetIndex( nEndPos );
            return true;
        }
    }
    return false;
}

bool ImpEditEngine::HasText( const SvxSearchItem& rSearchItem )
{
    SvxSearchItem aTmpItem( rSearchItem );
    aTmpItem.SetBackward( false );
    aTmpItem.SetSelection( false );

    EditPaM aStartPaM( aEditDoc.GetStartPaM() );
    EditSelection aDummySel( aStartPaM );
    EditSelection aFoundSel;
    return ImpSearch( aTmpItem, aDummySel, aStartPaM, aFoundSel );
}

void ImpEditEngine::SetAutoCompleteText(const OUString& rStr, bool bClearTipWindow)
{
    aAutoCompleteText = rStr;
    if ( bClearTipWindow && pActiveView )
        Help::ShowQuickHelp( pActiveView->GetWindow(), tools::Rectangle(), OUString() );
}

namespace
{
    struct eeTransliterationChgData
    {
        sal_Int32                   nStart;
        sal_Int32                   nLen;
        EditSelection               aSelection;
        OUString                    aNewText;
        uno::Sequence< sal_Int32 >  aOffsets;
    };
}

EditSelection ImpEditEngine::TransliterateText( const EditSelection& rSelection, TransliterationFlags nTransliterationMode )
{
    uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
    if (!_xBI.is())
        return rSelection;

    EditSelection aSel( rSelection );
    aSel.Adjust( aEditDoc );

    if ( !aSel.HasRange() )
        aSel = SelectWord( aSel );

    // tdf#107176: if there's still no range, just return aSel
    if ( !aSel.HasRange() )
        return aSel;

    EditSelection aNewSel( aSel );

    const sal_Int32 nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    const sal_Int32 nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    bool bChanges = false;
    bool bLenChanged = false;
    std::unique_ptr<EditUndoTransliteration> pUndo;

    utl::TransliterationWrapper aTransliterationWrapper( ::comphelper::getProcessComponentContext(), nTransliterationMode );
    bool bConsiderLanguage = aTransliterationWrapper.needLanguageForTheMode();

    for ( sal_Int32 nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        const OUString& aNodeStr = pNode->GetString();
        const sal_Int32 nStartPos = nNode==nStartNode ? aSel.Min().GetIndex() : 0;
        const sal_Int32 nEndPos = nNode==nEndNode ? aSel.Max().GetIndex() : aNodeStr.getLength(); // can also be == nStart!

        sal_Int32 nCurrentStart = nStartPos;
        sal_Int32 nCurrentEnd = nEndPos;
        LanguageType nLanguage = LANGUAGE_SYSTEM;

        // since we don't use Hiragana/Katakana or half-width/full-width transliterations here
        // it is fine to use ANYWORD_IGNOREWHITESPACES. (ANY_WORD btw is broken and will
        // occasionally miss words in consecutive sentences). Also with ANYWORD_IGNOREWHITESPACES
        // text like 'just-in-time' will be converted to 'Just-In-Time' which seems to be the
        // proper thing to do.
        const sal_Int16 nWordType = i18n::WordType::ANYWORD_IGNOREWHITESPACES;

        //! In order to have less trouble with changing text size, e.g. because
        //! of ligatures or German small sz being resolved, we need to process
        //! the text replacements from end to start.
        //! This way the offsets for the yet to be changed words will be
        //! left unchanged by the already replaced text.
        //! For this we temporarily save the changes to be done in this vector
        std::vector< eeTransliterationChgData >   aChanges;
        eeTransliterationChgData                  aChgData;

        if (nTransliterationMode == TransliterationFlags::TITLE_CASE)
        {
            // for 'capitalize every word' we need to iterate over each word

            i18n::Boundary aSttBndry;
            i18n::Boundary aEndBndry;
            aSttBndry = _xBI->getWordBoundary(
                        aNodeStr, nStartPos,
                        GetLocale( EditPaM( pNode, nStartPos + 1 ) ),
                        nWordType, true /*prefer forward direction*/);
            aEndBndry = _xBI->getWordBoundary(
                        aNodeStr, nEndPos,
                        GetLocale( EditPaM( pNode, nEndPos + 1 ) ),
                        nWordType, false /*prefer backward direction*/);

            // prevent backtracking to the previous word if selection is at word boundary
            if (aSttBndry.endPos <= nStartPos)
            {
                aSttBndry = _xBI->nextWord(
                        aNodeStr, aSttBndry.endPos,
                        GetLocale( EditPaM( pNode, aSttBndry.endPos + 1 ) ),
                        nWordType);
            }
            // prevent advancing to the next word if selection is at word boundary
            if (aEndBndry.startPos >= nEndPos)
            {
                aEndBndry = _xBI->previousWord(
                        aNodeStr, aEndBndry.startPos,
                        GetLocale( EditPaM( pNode, aEndBndry.startPos + 1 ) ),
                        nWordType);
            }

            i18n::Boundary aCurWordBndry( aSttBndry );
            while (aCurWordBndry.endPos && aCurWordBndry.startPos <= aEndBndry.startPos)
            {
                nCurrentStart = aCurWordBndry.startPos;
                nCurrentEnd   = aCurWordBndry.endPos;
                sal_Int32 nLen = nCurrentEnd - nCurrentStart;
                DBG_ASSERT( nLen > 0, "invalid word length of 0" );

                Sequence< sal_Int32 > aOffsets;
                OUString aNewText( aTransliterationWrapper.transliterate(aNodeStr,
                        GetLanguage( EditPaM( pNode, nCurrentStart + 1 ) ),
                        nCurrentStart, nLen, &aOffsets ));

                if (aNodeStr != aNewText)
                {
                    aChgData.nStart     = nCurrentStart;
                    aChgData.nLen       = nLen;
                    aChgData.aSelection = EditSelection( EditPaM( pNode, nCurrentStart ), EditPaM( pNode, nCurrentEnd ) );
                    aChgData.aNewText   = aNewText;
                    aChgData.aOffsets   = aOffsets;
                    aChanges.push_back( aChgData );
                }
#if OSL_DEBUG_LEVEL > 1
                OUString aSelTxt ( GetSelected( aChgData.aSelection ) );
                (void) aSelTxt;
#endif

                aCurWordBndry = _xBI->nextWord(aNodeStr, nCurrentStart,
                        GetLocale( EditPaM( pNode, nCurrentStart + 1 ) ),
                        nWordType);
            }
            DBG_ASSERT( nCurrentEnd >= aEndBndry.endPos, "failed to reach end of transliteration" );
        }
        else if (nTransliterationMode == TransliterationFlags::SENTENCE_CASE)
        {
            // for 'sentence case' we need to iterate sentence by sentence

            sal_Int32 nLastStart = _xBI->beginOfSentence(
                    aNodeStr, nEndPos,
                    GetLocale( EditPaM( pNode, nEndPos + 1 ) ) );
            sal_Int32 nLastEnd = _xBI->endOfSentence(
                    aNodeStr, nLastStart,
                    GetLocale( EditPaM( pNode, nLastStart + 1 ) ) );

            // extend nCurrentStart, nCurrentEnd to the current sentence boundaries
            nCurrentStart = _xBI->beginOfSentence(
                    aNodeStr, nStartPos,
                    GetLocale( EditPaM( pNode, nStartPos + 1 ) ) );
            nCurrentEnd = _xBI->endOfSentence(
                    aNodeStr, nCurrentStart,
                    GetLocale( EditPaM( pNode, nCurrentStart + 1 ) ) );

            // prevent backtracking to the previous sentence if selection starts at end of a sentence
            if (nCurrentEnd <= nStartPos)
            {
                // now nCurrentStart is probably located on a non-letter word. (unless we
                // are in Asian text with no spaces...)
                // Thus to get the real sentence start we should locate the next real word,
                // that is one found by DICTIONARY_WORD
                i18n::Boundary aBndry = _xBI->nextWord( aNodeStr, nCurrentEnd,
                        GetLocale( EditPaM( pNode, nCurrentEnd + 1 ) ),
                        i18n::WordType::DICTIONARY_WORD);

                // now get new current sentence boundaries
                nCurrentStart = _xBI->beginOfSentence(
                        aNodeStr, aBndry.startPos,
                        GetLocale( EditPaM( pNode, aBndry.startPos + 1 ) ) );
                nCurrentEnd = _xBI->endOfSentence(
                        aNodeStr, nCurrentStart,
                        GetLocale( EditPaM( pNode, nCurrentStart + 1 ) ) );
            }
            // prevent advancing to the next sentence if selection ends at start of a sentence
            if (nLastStart >= nEndPos)
            {
                // now nCurrentStart is probably located on a non-letter word. (unless we
                // are in Asian text with no spaces...)
                // Thus to get the real sentence start we should locate the previous real word,
                // that is one found by DICTIONARY_WORD
                i18n::Boundary aBndry = _xBI->previousWord( aNodeStr, nLastStart,
                        GetLocale( EditPaM( pNode, nLastStart + 1 ) ),
                        i18n::WordType::DICTIONARY_WORD);
                nLastEnd = _xBI->endOfSentence(
                        aNodeStr, aBndry.startPos,
                        GetLocale( EditPaM( pNode, aBndry.startPos + 1 ) ) );
                if (nCurrentEnd > nLastEnd)
                    nCurrentEnd = nLastEnd;
            }

            while (nCurrentStart < nLastEnd)
            {
                const sal_Int32 nLen = nCurrentEnd - nCurrentStart;
                DBG_ASSERT( nLen > 0, "invalid word length of 0" );

                Sequence< sal_Int32 > aOffsets;
                OUString aNewText( aTransliterationWrapper.transliterate( aNodeStr,
                        GetLanguage( EditPaM( pNode, nCurrentStart + 1 ) ),
                        nCurrentStart, nLen, &aOffsets ));

                if (aNodeStr != aNewText)
                {
                    aChgData.nStart     = nCurrentStart;
                    aChgData.nLen       = nLen;
                    aChgData.aSelection = EditSelection( EditPaM( pNode, nCurrentStart ), EditPaM( pNode, nCurrentEnd ) );
                    aChgData.aNewText   = aNewText;
                    aChgData.aOffsets   = aOffsets;
                    aChanges.push_back( aChgData );
                }

                i18n::Boundary aFirstWordBndry;
                aFirstWordBndry = _xBI->nextWord(
                        aNodeStr, nCurrentEnd,
                        GetLocale( EditPaM( pNode, nCurrentEnd + 1 ) ),
                        nWordType);
                nCurrentStart = aFirstWordBndry.startPos;
                nCurrentEnd = _xBI->endOfSentence(
                        aNodeStr, nCurrentStart,
                        GetLocale( EditPaM( pNode, nCurrentStart + 1 ) ) );
            }
            DBG_ASSERT( nCurrentEnd >= nLastEnd, "failed to reach end of transliteration" );
        }
        else
        {
            do
            {
                if ( bConsiderLanguage )
                {
                    nLanguage = GetLanguage( EditPaM( pNode, nCurrentStart+1 ), &nCurrentEnd );
                    if ( nCurrentEnd > nEndPos )
                        nCurrentEnd = nEndPos;
                }

                const sal_Int32 nLen = nCurrentEnd - nCurrentStart;

                Sequence< sal_Int32 > aOffsets;
                OUString aNewText( aTransliterationWrapper.transliterate( aNodeStr, nLanguage, nCurrentStart, nLen, &aOffsets ) );

                if (aNodeStr != aNewText)
                {
                    aChgData.nStart     = nCurrentStart;
                    aChgData.nLen       = nLen;
                    aChgData.aSelection = EditSelection( EditPaM( pNode, nCurrentStart ), EditPaM( pNode, nCurrentEnd ) );
                    aChgData.aNewText   = aNewText;
                    aChgData.aOffsets   = aOffsets;
                    aChanges.push_back( aChgData );
                }

                nCurrentStart = nCurrentEnd;
            } while( nCurrentEnd < nEndPos );
        }

        if (!aChanges.empty())
        {
            // Create a single UndoAction on Demand for all the changes ...
            if ( !pUndo && IsUndoEnabled() && !IsInUndo() )
            {
                // adjust selection to include all changes
                for (eeTransliterationChgData & aChange : aChanges)
                {
                    const EditSelection &rSel = aChange.aSelection;
                    if (aSel.Min().GetNode() == rSel.Min().GetNode() &&
                        aSel.Min().GetIndex() > rSel.Min().GetIndex())
                        aSel.Min().SetIndex( rSel.Min().GetIndex() );
                    if (aSel.Max().GetNode() == rSel.Max().GetNode() &&
                        aSel.Max().GetIndex() < rSel.Max().GetIndex())
                        aSel.Max().SetIndex( rSel.Max().GetIndex() );
                }
                aNewSel = aSel;

                ESelection aESel( CreateESel( aSel ) );
                pUndo.reset(new EditUndoTransliteration(pEditEngine, aESel, nTransliterationMode));

                const bool bSingleNode = aSel.Min().GetNode()== aSel.Max().GetNode();
                const bool bHasAttribs = aSel.Min().GetNode()->GetCharAttribs().HasAttrib( aSel.Min().GetIndex(), aSel.Max().GetIndex() );
                if (bSingleNode && !bHasAttribs)
                    pUndo->SetText( aSel.Min().GetNode()->Copy( aSel.Min().GetIndex(), aSel.Max().GetIndex()-aSel.Min().GetIndex() ) );
                else
                    pUndo->SetText( CreateTextObject( aSel, nullptr ) );
            }

            // now apply the changes from end to start to leave the offsets of the
            // yet unchanged text parts remain the same.
            for (size_t i = 0; i < aChanges.size(); ++i)
            {
                eeTransliterationChgData& rData = aChanges[ aChanges.size() - 1 - i ];

                bChanges = true;
                if (rData.nLen != rData.aNewText.getLength())
                    bLenChanged = true;

                // Change text without losing the attributes
                const sal_Int32 nDiffs =
                    ReplaceTextOnly( rData.aSelection.Min().GetNode(),
                        rData.nStart, rData.aNewText, rData.aOffsets );

                // adjust selection in end node to possibly changed size
                if (aSel.Max().GetNode() == rData.aSelection.Max().GetNode())
                    aNewSel.Max().SetIndex( aNewSel.Max().GetIndex() + nDiffs );

                sal_Int32 nSelNode = aEditDoc.GetPos( rData.aSelection.Min().GetNode() );
                ParaPortion* pParaPortion = GetParaPortions()[nSelNode];
                pParaPortion->MarkSelectionInvalid( rData.nStart );
            }
        }
    }

    if ( pUndo )
    {
        ESelection aESel( CreateESel( aNewSel ) );
        pUndo->SetNewSelection( aESel );
        InsertUndo( std::move(pUndo) );
    }

    if ( bChanges )
    {
        TextModified();
        SetModifyFlag( true );
        if ( bLenChanged )
            UpdateSelections();
        FormatAndUpdate();
    }

    return aNewSel;
}


short ImpEditEngine::ReplaceTextOnly(
    ContentNode* pNode,
    sal_Int32 nCurrentStart,
    const OUString& rNewText,
    const uno::Sequence< sal_Int32 >& rOffsets )
{
    // Change text without losing the attributes
    sal_Int32 nCharsAfterTransliteration = rOffsets.getLength();
    const sal_Int32* pOffsets = rOffsets.getConstArray();
    short nDiffs = 0;
    for ( sal_Int32 n = 0; n < nCharsAfterTransliteration; n++ )
    {
        sal_Int32 nCurrentPos = nCurrentStart+n;
        sal_Int32 nDiff = (nCurrentPos-nDiffs) - pOffsets[n];

        if ( !nDiff )
        {
            DBG_ASSERT( nCurrentPos < pNode->Len(), "TransliterateText - String smaller than expected!" );
            pNode->SetChar( nCurrentPos, rNewText[n] );
        }
        else if ( nDiff < 0 )
        {
            // Replace first char, delete the rest...
            DBG_ASSERT( nCurrentPos < pNode->Len(), "TransliterateText - String smaller than expected!" );
            pNode->SetChar( nCurrentPos, rNewText[n] );

            DBG_ASSERT( (nCurrentPos+1) < pNode->Len(), "TransliterateText - String smaller than expected!" );
            GetEditDoc().RemoveChars( EditPaM( pNode, nCurrentPos+1 ), -nDiff);
        }
        else
        {
            DBG_ASSERT( nDiff == 1, "TransliterateText - Diff other than expected! But should work..." );
            GetEditDoc().InsertText( EditPaM( pNode, nCurrentPos ), OUString(rNewText[n]) );

        }
        nDiffs = sal::static_int_cast< short >(nDiffs + nDiff);
    }

    return nDiffs;
}


void ImpEditEngine::SetAsianCompressionMode( CharCompressType n )
{
    if ( n != nAsianCompressionMode )
    {
        nAsianCompressionMode = n;
        if ( ImplHasText() )
        {
            FormatFullDoc();
            UpdateViews();
        }
    }
}

void ImpEditEngine::SetKernAsianPunctuation( bool b )
{
    if ( b != bKernAsianPunctuation )
    {
        bKernAsianPunctuation = b;
        if ( ImplHasText() )
        {
            FormatFullDoc();
            UpdateViews();
        }
    }
}

void ImpEditEngine::SetAddExtLeading( bool bExtLeading )
{
    if ( IsAddExtLeading() != bExtLeading )
    {
        bAddExtLeading = bExtLeading;
        if ( ImplHasText() )
        {
            FormatFullDoc();
            UpdateViews();
        }
    }
};


bool ImpEditEngine::ImplHasText() const
{
    return ( ( GetEditDoc().Count() > 1 ) || GetEditDoc().GetObject(0)->Len() );
}

sal_Int32 ImpEditEngine::LogicToTwips(sal_Int32 n)
{
    Size aSz(n, 0);
    MapMode aTwipsMode( MapUnit::MapTwip );
    aSz = pRefDev->LogicToLogic( aSz, nullptr, &aTwipsMode );
    return aSz.Width();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
