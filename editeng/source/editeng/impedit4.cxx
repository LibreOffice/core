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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <svl/srchitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/tstpitem.hxx>

#include <eertfpar.hxx>
#include <editeng/editeng.hxx>
#include <impedit.hxx>
#include <editeng/editview.hxx>
#include <eehtml.hxx>
#include <editobj2.hxx>
#include <i18npool/lang.h>

#include "editxml.hxx"

#include <editeng/akrnitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/escpitem.hxx>
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
#include <editeng/emphitem.hxx>
#include <textconv.hxx>
#include <rtl/tencinfo.h>
#include <svtools/rtfout.hxx>
#include <edtspell.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/unolingu.hxx>
#include <linguistic/lngprops.hxx>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/textsearch.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/help.hxx>
#include <svtools/rtfkeywd.hxx>
#include <editeng/edtdlg.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

void Swapsal_uIt16s( sal_uInt16& rX, sal_uInt16& rY )
{
    sal_uInt16 n = rX;
    rX = rY;
    rY = n;
}

EditPaM ImpEditEngine::Read( SvStream& rInput, const String& rBaseURL, EETextFormat eFormat, EditSelection aSel, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    sal_Bool _bUpdate = GetUpdateMode();
    SetUpdateMode( sal_False );
    EditPaM aPaM;
    if ( eFormat == EE_FORMAT_TEXT )
        aPaM = ReadText( rInput, aSel );
    else if ( eFormat == EE_FORMAT_RTF )
        aPaM = ReadRTF( rInput, aSel );
    else if ( eFormat == EE_FORMAT_XML )
        aPaM = ReadXML( rInput, aSel );
    else if ( eFormat == EE_FORMAT_HTML )
        aPaM = ReadHTML( rInput, rBaseURL, aSel, pHTTPHeaderAttrs );
    else if ( eFormat == EE_FORMAT_BIN)
        aPaM = ReadBin( rInput, aSel );
    else
    {
        DBG_ERROR( "Read: Unbekanntes Format" );
    }

    FormatFullDoc();        // reicht vielleicht auch ein einfaches Format?
    SetUpdateMode( _bUpdate );

    return aPaM;
}

EditPaM ImpEditEngine::ReadText( SvStream& rInput, EditSelection aSel )
{
    if ( aSel.HasRange() )
        aSel = ImpDeleteSelection( aSel );
    EditPaM aPaM = aSel.Max();

    XubString aTmpStr, aStr;
    sal_Bool bDone = rInput.ReadByteStringLine( aTmpStr );
    while ( bDone )
    {
        aTmpStr.Erase( MAXCHARSINPARA );
        aPaM = ImpInsertText( EditSelection( aPaM, aPaM ), aTmpStr );
        aPaM = ImpInsertParaBreak( aPaM );
        bDone = rInput.ReadByteStringLine( aTmpStr );
    }
    return aPaM;
}

EditPaM ImpEditEngine::ReadXML( SvStream& rInput, EditSelection aSel )
{
#ifndef SVX_LIGHT
    if ( aSel.HasRange() )
        aSel = ImpDeleteSelection( aSel );

    ESelection aESel = CreateESel( aSel );

    ::SvxReadXML( *GetEditEnginePtr(), rInput, aESel );

    return aSel.Max();
#else
    return EditPaM();
#endif
}

EditPaM ImpEditEngine::ReadRTF( SvStream& rInput, EditSelection aSel )
{
#ifndef SVX_LIGHT

#if defined (EDITDEBUG) && !defined( UNX )
    SvFileStream aRTFOut( String( RTL_CONSTASCII_USTRINGPARAM ( "d:\\rtf_in.rtf" ) ), STREAM_WRITE );
    aRTFOut << rInput;
    aRTFOut.Close();
    rInput.Seek( 0 );
#endif
    if ( aSel.HasRange() )
        aSel = ImpDeleteSelection( aSel );

//  sal_Bool bCharsBeforeInsertPos = ( aSel.Min().GetIndex() ) ? sal_True : sal_False;
//  sal_Bool bCharsBehindInsertPos = ( aSel.Min().GetIndex() < aSel.Min().GetNode()->Len() ) ? sal_True : sal_False;

    // Der SvRTF-Parser erwartet, dass das Which-Mapping am uebergebenen Pool,
    // nicht an einem Secondary haengt.
    SfxItemPool* pPool = &aEditDoc.GetItemPool();
    while ( pPool->GetSecondaryPool() && !pPool->GetName().EqualsAscii( "EditEngineItemPool" ) )
    {
        pPool = pPool->GetSecondaryPool();

    }
    DBG_ASSERT( pPool && pPool->GetName().EqualsAscii( "EditEngineItemPool" ), "ReadRTF: Kein EditEnginePool!" );

    EditRTFParserRef xPrsr = new EditRTFParser( rInput, aSel, *pPool, this );
    SvParserState eState = xPrsr->CallParser();
    if ( ( eState != SVPAR_ACCEPTED ) && ( !rInput.GetError() ) )
    {
        rInput.SetError( EE_READWRITE_WRONGFORMAT );
        return aSel.Min();
    }
    return xPrsr->GetCurPaM();
#else
    return EditPaM();
#endif
}

EditPaM ImpEditEngine::ReadHTML( SvStream& rInput, const String& rBaseURL, EditSelection aSel, SvKeyValueIterator* pHTTPHeaderAttrs )
{
#ifndef SVX_LIGHT

    if ( aSel.HasRange() )
        aSel = ImpDeleteSelection( aSel );

//  sal_Bool bCharsBeforeInsertPos = ( aSel.Min().GetIndex() ) ? sal_True : sal_False;
//  sal_Bool bCharsBehindInsertPos = ( aSel.Min().GetIndex() < aSel.Min().GetNode()->Len() ) ? sal_True : sal_False;

    EditHTMLParserRef xPrsr = new EditHTMLParser( rInput, rBaseURL, pHTTPHeaderAttrs );
    SvParserState eState = xPrsr->CallParser( this, aSel.Max() );
    if ( ( eState != SVPAR_ACCEPTED ) && ( !rInput.GetError() ) )
    {
        rInput.SetError( EE_READWRITE_WRONGFORMAT );
        return aSel.Min();
    }
    return xPrsr->GetCurSelection().Max();
#else
    return EditPaM();
#endif
}

EditPaM ImpEditEngine::ReadBin( SvStream& rInput, EditSelection aSel )
{
    // Einfach ein temporaeres TextObject missbrauchen...
    EditTextObject* pObj = EditTextObject::Create( rInput, NULL );

    EditPaM aLastPaM = aSel.Max();
    if ( pObj )
        aLastPaM = InsertText( *pObj, aSel ).Max();

    delete pObj;
    return aLastPaM;
}

#ifndef SVX_LIGHT
void ImpEditEngine::Write( SvStream& rOutput, EETextFormat eFormat, EditSelection aSel )
{
    if ( !rOutput.IsWritable() )
        rOutput.SetError( SVSTREAM_WRITE_ERROR );

    if ( !rOutput.GetError() )
    {
        if ( eFormat == EE_FORMAT_TEXT )
            WriteText( rOutput, aSel );
        else if ( eFormat == EE_FORMAT_RTF )
            WriteRTF( rOutput, aSel );
        else if ( eFormat == EE_FORMAT_XML )
            WriteXML( rOutput, aSel );
        else if ( eFormat == EE_FORMAT_HTML )
            WriteHTML( rOutput, aSel );
        else if ( eFormat == EE_FORMAT_BIN)
            WriteBin( rOutput, aSel );
        else
        {
            DBG_ERROR( "Write: Unbekanntes Format" );
        }
    }
}
#endif

sal_uInt32 ImpEditEngine::WriteText( SvStream& rOutput, EditSelection aSel )
{
    sal_uInt16 nStartNode, nEndNode;
    sal_Bool bRange = aSel.HasRange();
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

    // ueber die Absaetze iterieren...
    for ( sal_uInt16 nNode = nStartNode; nNode <= nEndNode; nNode++  )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        DBG_ASSERT( pNode, "Node nicht gefunden: Search&Replace" );

        sal_uInt16 nStartPos = 0;
        sal_uInt16 nEndPos = pNode->Len();
        if ( bRange )
        {
            if ( nNode == nStartNode )
                nStartPos = aSel.Min().GetIndex();
            if ( nNode == nEndNode ) // kann auch == nStart sein!
                nEndPos = aSel.Max().GetIndex();
        }
        XubString aTmpStr = aEditDoc.GetParaAsString( pNode, nStartPos, nEndPos );
        rOutput.WriteByteStringLine( aTmpStr );
    }

    return rOutput.GetError();
}

sal_Bool ImpEditEngine::WriteItemListAsRTF( ItemList& rLst, SvStream& rOutput, sal_uInt16 nPara, sal_uInt16 nPos,
                        SvxFontTable& rFontTable, SvxColorList& rColorList )
{
    const SfxPoolItem* pAttrItem = rLst.First();
    while ( pAttrItem )
    {
        WriteItemAsRTF( *pAttrItem, rOutput, nPara, nPos,rFontTable, rColorList );
        pAttrItem = rLst.Next();
    }
    return ( rLst.Count() ? sal_True : sal_False );
}

void lcl_FindValidAttribs( ItemList& rLst, ContentNode* pNode, sal_uInt16 nIndex, sal_uInt16 nScriptType )
{
    sal_uInt16 nAttr = 0;
    EditCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    while ( pAttr && ( pAttr->GetStart() <= nIndex ) )
    {
        // Start wird in While ueberprueft...
        if ( pAttr->GetEnd() > nIndex )
        {
            if ( IsScriptItemValid( pAttr->GetItem()->Which(), nScriptType ) )
                rLst.Insert( pAttr->GetItem(), LIST_APPEND );
        }
        nAttr++;
        pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    }
}

sal_uInt32 ImpEditEngine::WriteBin( SvStream& rOutput, EditSelection aSel, sal_Bool bStoreUnicodeStrings ) const
{
    BinTextObject* pObj = (BinTextObject*)CreateBinTextObject( aSel, NULL );
    pObj->StoreUnicodeStrings( bStoreUnicodeStrings );
    pObj->Store( rOutput );
    delete pObj;
    return 0;
}

#ifndef SVX_LIGHT
sal_uInt32 ImpEditEngine::WriteXML( SvStream& rOutput, EditSelection aSel )
{
    ESelection aESel = CreateESel( aSel );

    SvxWriteXML( *GetEditEnginePtr(), rOutput, aESel );

    return 0;
}
#endif

static sal_uInt16 getStylePos( const SfxStyles& rStyles, SfxStyleSheet* pSheet )
{
    sal_uInt16 nNumber = 0;
    SfxStyles::const_iterator iter( rStyles.begin() );
    while( iter != rStyles.end() )
    {
        if( (*iter++).get() == pSheet )
            return nNumber;
        ++nNumber;
    }
    return 0;
}

sal_uInt32 ImpEditEngine::WriteRTF( SvStream& rOutput, EditSelection aSel )
{
#ifndef SVX_LIGHT
    DBG_ASSERT( GetUpdateMode(), "WriteRTF bei UpdateMode = sal_False!" );
    CheckIdleFormatter();
    if ( !IsFormatted() )
        FormatDoc();

    sal_uInt16 nStartNode, nEndNode;
    aSel.Adjust( aEditDoc );

    nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    // RTF-Vorspann...
    rOutput << '{' ;

    rOutput << OOO_STRING_SVTOOLS_RTF_RTF;

    rOutput << OOO_STRING_SVTOOLS_RTF_ANSI;
    rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252;

    // Fonttabelle erzeugen und rausschreiben...
    SvxFontTable aFontTable;
    // DefaultFont muss ganz vorne stehen, damit DEF-Font im RTF
    aFontTable.Insert( 0, new SvxFontItem( (const SvxFontItem&)aEditDoc.GetItemPool().GetDefaultItem( EE_CHAR_FONTINFO ) ) );
    aFontTable.Insert( 1, new SvxFontItem( (const SvxFontItem&)aEditDoc.GetItemPool().GetDefaultItem( EE_CHAR_FONTINFO_CJK ) ) );
    aFontTable.Insert( 2, new SvxFontItem( (const SvxFontItem&)aEditDoc.GetItemPool().GetDefaultItem( EE_CHAR_FONTINFO_CTL ) ) );
    for ( sal_uInt16 nScriptType = 0; nScriptType < 3; nScriptType++ )
    {
        sal_uInt16 nWhich = EE_CHAR_FONTINFO;
        if ( nScriptType == 1 )
            nWhich = EE_CHAR_FONTINFO_CJK;
        else if ( nScriptType == 2 )
            nWhich = EE_CHAR_FONTINFO_CTL;

        sal_uInt32 i = 0;
        SvxFontItem* pFontItem = (SvxFontItem*)aEditDoc.GetItemPool().GetItem2( nWhich, i );
        while ( pFontItem )
        {
            bool bAlreadyExist = false;
            sal_uLong nTestMax = nScriptType ? aFontTable.Count() : 1;
            for ( sal_uLong nTest = 0; !bAlreadyExist && ( nTest < nTestMax ); nTest++ )
            {
                bAlreadyExist = *aFontTable.Get( nTest ) == *pFontItem;
            }

            if ( !bAlreadyExist )
                aFontTable.Insert( aFontTable.Count(), new SvxFontItem( *pFontItem ) );

            pFontItem = (SvxFontItem*)aEditDoc.GetItemPool().GetItem2( nWhich, ++i );
        }
    }

    rOutput << endl << '{' << OOO_STRING_SVTOOLS_RTF_FONTTBL;
    sal_uInt16 j;
    for ( j = 0; j < aFontTable.Count(); j++ )
    {
        SvxFontItem* pFontItem = aFontTable.Get( j );
        rOutput << '{';
        rOutput << OOO_STRING_SVTOOLS_RTF_F;
        rOutput.WriteNumber( j );
        switch ( pFontItem->GetFamily()  )
        {
            case FAMILY_DONTKNOW:       rOutput << OOO_STRING_SVTOOLS_RTF_FNIL;
                                        break;
            case FAMILY_DECORATIVE:     rOutput << OOO_STRING_SVTOOLS_RTF_FDECOR;
                                        break;
            case FAMILY_MODERN:         rOutput << OOO_STRING_SVTOOLS_RTF_FMODERN;
                                        break;
            case FAMILY_ROMAN:          rOutput << OOO_STRING_SVTOOLS_RTF_FROMAN;
                                        break;
            case FAMILY_SCRIPT:         rOutput << OOO_STRING_SVTOOLS_RTF_FSCRIPT;
                                        break;
            case FAMILY_SWISS:          rOutput << OOO_STRING_SVTOOLS_RTF_FSWISS;
                                        break;
            default:
                break;
        }
        rOutput << OOO_STRING_SVTOOLS_RTF_FPRQ;
        sal_uInt16 nVal = 0;
        switch( pFontItem->GetPitch() )
        {
            case PITCH_FIXED:       nVal = 1;       break;
            case PITCH_VARIABLE:    nVal = 2;       break;
            default:
                break;
        }
        rOutput.WriteNumber( nVal );

        CharSet eChrSet = pFontItem->GetCharSet();
        DBG_ASSERT( eChrSet != 9, "SystemCharSet?!" );
        if( RTL_TEXTENCODING_DONTKNOW == eChrSet )
            eChrSet = gsl_getSystemTextEncoding();
        rOutput << OOO_STRING_SVTOOLS_RTF_FCHARSET;
        rOutput.WriteNumber( rtl_getBestWindowsCharsetFromTextEncoding( eChrSet ) );

        rOutput << ' ';
        RTFOutFuncs::Out_String( rOutput, pFontItem->GetFamilyName(), eDestEnc );
        rOutput << ";}";
    }
    rOutput << '}';
    rOutput << endl;

    // ColorList rausschreiben...
    SvxColorList aColorList;
    sal_uInt32 i = 0;
    SvxColorItem* pColorItem = (SvxColorItem*)aEditDoc.GetItemPool().GetItem2( EE_CHAR_COLOR, i );
    while ( pColorItem )
    {
        sal_uInt32 nPos = i;
        if ( pColorItem->GetValue() == COL_AUTO )
            nPos = 0;
        aColorList.Insert( new SvxColorItem( *pColorItem ), nPos );
        pColorItem = (SvxColorItem*)aEditDoc.GetItemPool().GetItem2( EE_CHAR_COLOR, ++i );
    }
    aColorList.Insert( new SvxColorItem( (const SvxColorItem&)aEditDoc.GetItemPool().GetDefaultItem( EE_CHAR_COLOR) ), i );

    rOutput << '{' << OOO_STRING_SVTOOLS_RTF_COLORTBL;
    for ( j = 0; j < aColorList.Count(); j++ )
    {
        pColorItem = aColorList.GetObject( j );
        if ( !j || ( pColorItem->GetValue() != COL_AUTO ) )
        {
            rOutput << OOO_STRING_SVTOOLS_RTF_RED;
            rOutput.WriteNumber( pColorItem->GetValue().GetRed() );
            rOutput << OOO_STRING_SVTOOLS_RTF_GREEN;
            rOutput.WriteNumber( pColorItem->GetValue().GetGreen() );
            rOutput << OOO_STRING_SVTOOLS_RTF_BLUE;
            rOutput.WriteNumber( pColorItem->GetValue().GetBlue() );
        }
        rOutput << ';';
    }
    rOutput << '}';
    rOutput << endl;

    // StyleSheets...
    if ( GetStyleSheetPool() )
    {
        sal_uInt16 nStyles = (sal_uInt16)GetStyleSheetPool()->GetStyles().size();
        if ( nStyles )
        {
            rOutput << '{' << OOO_STRING_SVTOOLS_RTF_STYLESHEET;

            for ( sal_uInt16 nStyle = 0; nStyle < nStyles; nStyle++ )
            {

                SfxStyleSheet* pStyle = (SfxStyleSheet*)GetStyleSheetPool()->GetStyles()[ nStyle ].get();

                rOutput << endl << '{' << OOO_STRING_SVTOOLS_RTF_S;
                sal_uInt16 nNumber = (sal_uInt16) (nStyle + 1);
                rOutput.WriteNumber( nNumber );

                // Attribute, auch aus Parent!
                for ( sal_uInt16 nParAttr = EE_PARA_START; nParAttr <= EE_CHAR_END; nParAttr++ )
                {
                    if ( pStyle->GetItemSet().GetItemState( nParAttr ) == SFX_ITEM_ON )
                    {
                        const SfxPoolItem& rItem = pStyle->GetItemSet().Get( nParAttr );
                        WriteItemAsRTF( rItem, rOutput, 0, 0, aFontTable, aColorList );
                    }
                }

                // Parent...(nur wenn noetig)
                if ( pStyle->GetParent().Len() && ( pStyle->GetParent() != pStyle->GetName() ) )
                {
                    SfxStyleSheet* pParent = (SfxStyleSheet*)GetStyleSheetPool()->Find( pStyle->GetParent(), pStyle->GetFamily() );
                    DBG_ASSERT( pParent, "Parent nicht gefunden!" );
                    rOutput << OOO_STRING_SVTOOLS_RTF_SBASEDON;
                    nNumber = (sal_uInt16) getStylePos( GetStyleSheetPool()->GetStyles(), pParent ) + 1;
                    rOutput.WriteNumber( nNumber );
                }

                // Folgevorlage...(immer)
                SfxStyleSheet* pNext = pStyle;
                if ( pStyle->GetFollow().Len() && ( pStyle->GetFollow() != pStyle->GetName() ) )
                    pNext = (SfxStyleSheet*)GetStyleSheetPool()->Find( pStyle->GetFollow(), pStyle->GetFamily() );

                DBG_ASSERT( pNext, "Naechsten nicht gefunden!" );
                rOutput << OOO_STRING_SVTOOLS_RTF_SNEXT;
                nNumber = (sal_uInt16) getStylePos( GetStyleSheetPool()->GetStyles(), pNext ) + 1;
                rOutput.WriteNumber( nNumber );

                // Namen der Vorlage...
                rOutput << " " << ByteString( pStyle->GetName(), eDestEnc ).GetBuffer();
                rOutput << ";}";
            }
            rOutput << '}';
            rOutput << endl;
        }
    }

    // Die Pool-Defaults vorweg schreiben...
    rOutput << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE << "\\EditEnginePoolDefaults";
    for ( sal_uInt16 nPoolDefItem = EE_PARA_START; nPoolDefItem <= EE_CHAR_END; nPoolDefItem++)
    {
        const SfxPoolItem& rItem = aEditDoc.GetItemPool().GetDefaultItem( nPoolDefItem );
        WriteItemAsRTF( rItem, rOutput, 0, 0, aFontTable, aColorList );
    }
    rOutput << '}' << endl;

    // Def-Hoehe vorweg, da sonst 12Pt
    // Doch nicht, onst in jedem Absatz hart!
    // SfxItemSet aTmpSet( GetEmptyItemSet() );
    // const SvxFontHeightItem& rDefFontHeight = (const SvxFontHeightItem&)aTmpSet.Get( EE_CHAR_FONTHEIGHT );
    // WriteItemAsRTF( rDefFontHeight, rOutput, aFontTable, aColorList );
    // rOutput << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE << "\\EditEnginePoolDefaultHeight}" << endl;

    // DefTab:
    MapMode aTwpMode( MAP_TWIP );
    sal_uInt16 nDefTabTwps = (sal_uInt16) GetRefDevice()->LogicToLogic(
                                        Point( aEditDoc.GetDefTab(), 0 ),
                                        &GetRefMapMode(), &aTwpMode ).X();
    rOutput << OOO_STRING_SVTOOLS_RTF_DEFTAB;
    rOutput.WriteNumber( nDefTabTwps );
    rOutput << endl;

    // ueber die Absaetze iterieren...
    rOutput << '{' << endl;
    for ( sal_uInt16 nNode = nStartNode; nNode <= nEndNode; nNode++  )
    {
        ContentNode* pNode = aEditDoc.SaveGetObject( nNode );
        DBG_ASSERT( pNode, "Node nicht gefunden: Search&Replace" );

        // Die Absatzattribute vorweg...
        sal_Bool bAttr = sal_False;

        // Vorlage ?
        if ( pNode->GetStyleSheet() )
        {
            // Nummer der Vorlage
            rOutput << OOO_STRING_SVTOOLS_RTF_S;
            sal_uInt16 nNumber = (sal_uInt16) getStylePos( GetStyleSheetPool()->GetStyles(), pNode->GetStyleSheet() ) + 1;
            rOutput.WriteNumber( nNumber );

            // Alle Attribute
            // Attribute, auch aus Parent!
            for ( sal_uInt16 nParAttr = EE_PARA_START; nParAttr <= EE_CHAR_END; nParAttr++ )
            {
                if ( pNode->GetStyleSheet()->GetItemSet().GetItemState( nParAttr ) == SFX_ITEM_ON )
                {
                    const SfxPoolItem& rItem = pNode->GetStyleSheet()->GetItemSet().Get( nParAttr );
                    WriteItemAsRTF( rItem, rOutput, nNode, 0, aFontTable, aColorList );
                    bAttr = sal_True;
                }
            }
        }

        for ( sal_uInt16 nParAttr = EE_PARA_START; nParAttr <= EE_CHAR_END; nParAttr++ )
        {
//          const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItem( nParAttr );
            // Jetzt, wo StyleSheet-Verarbeitung, nur noch harte Absatzattribute!
            if ( pNode->GetContentAttribs().GetItems().GetItemState( nParAttr ) == SFX_ITEM_ON )
            {
                const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItems().Get( nParAttr );
                WriteItemAsRTF( rItem, rOutput, nNode, 0, aFontTable, aColorList );
                bAttr = sal_True;
            }
        }
        if ( bAttr )
            rOutput << ' '; // Separator

        ItemList aAttribItems;
        ParaPortion* pParaPortion = FindParaPortion( pNode );
        DBG_ASSERT( pParaPortion, "Portion nicht gefunden: WriteRTF" );

        sal_uInt16 nIndex = 0;
        sal_uInt16 nStartPos = 0;
        sal_uInt16 nEndPos = pNode->Len();
        sal_uInt16 nStartPortion = 0;
        sal_uInt16 nEndPortion = (sal_uInt16)pParaPortion->GetTextPortions().Count() - 1;
        sal_Bool bFinishPortion = sal_False;
        sal_uInt16 nPortionStart;

        if ( nNode == nStartNode )
        {
            nStartPos = aSel.Min().GetIndex();
            nStartPortion = pParaPortion->GetTextPortions().FindPortion( nStartPos, nPortionStart );
            if ( nStartPos != 0 )
            {
                aAttribItems.Clear();
                lcl_FindValidAttribs( aAttribItems, pNode, nStartPos, GetScriptType( EditPaM( pNode, 0 ) ) );
                if ( aAttribItems.Count() )
                {
                    // Diese Attribute duerfen nicht fuer den gesamten
                    // Absatz gelten:
                    rOutput << '{';
                    WriteItemListAsRTF( aAttribItems, rOutput, nNode, nStartPos, aFontTable, aColorList );
                    bFinishPortion = sal_True;
                }
                aAttribItems.Clear();
            }
        }
        if ( nNode == nEndNode ) // kann auch == nStart sein!
        {
            nEndPos = aSel.Max().GetIndex();
            nEndPortion = pParaPortion->GetTextPortions().FindPortion( nEndPos, nPortionStart );
        }

        EditCharAttrib* pNextFeature = pNode->GetCharAttribs().FindFeature( nIndex );
        // Bei 0 anfangen, damit der Index richtig ist...

        for ( sal_uInt16 n = 0; n <= nEndPortion; n++ )
        {
            TextPortion* pTextPortion = pParaPortion->GetTextPortions().GetObject(n);
            if ( n < nStartPortion )
            {
                nIndex = nIndex + pTextPortion->GetLen();
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
                sal_uInt16 nScriptType = GetScriptType( EditPaM( pNode, nIndex+1 ) );
                if ( !n || IsScriptChange( EditPaM( pNode, nIndex ) ) )
                {
                    SfxItemSet aAttribs = GetAttribs( nNode, nIndex+1, nIndex+1 );
                    aAttribItems.Insert( &aAttribs.Get( GetScriptItemId( EE_CHAR_FONTINFO, nScriptType ) ), LIST_APPEND );
                    aAttribItems.Insert( &aAttribs.Get( GetScriptItemId( EE_CHAR_FONTHEIGHT, nScriptType ) ), LIST_APPEND );
                    aAttribItems.Insert( &aAttribs.Get( GetScriptItemId( EE_CHAR_WEIGHT, nScriptType ) ), LIST_APPEND );
                    aAttribItems.Insert( &aAttribs.Get( GetScriptItemId( EE_CHAR_ITALIC, nScriptType ) ), LIST_APPEND );
                    aAttribItems.Insert( &aAttribs.Get( GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType ) ), LIST_APPEND );
                }
                // #96298# Insert hard attribs AFTER CJK attribs...
                lcl_FindValidAttribs( aAttribItems, pNode, nIndex, nScriptType );

                rOutput << '{';
                if ( WriteItemListAsRTF( aAttribItems, rOutput, nNode, nIndex, aFontTable, aColorList ) )
                    rOutput << ' ';

                sal_uInt16 nS = nIndex;
                sal_uInt16 nE = nIndex + pTextPortion->GetLen();
                if ( n == nStartPortion )
                    nS = nStartPos;
                if ( n == nEndPortion )
                    nE = nEndPos;

                XubString aRTFStr = aEditDoc.GetParaAsString( pNode, nS, nE);
                RTFOutFuncs::Out_String( rOutput, aRTFStr, eDestEnc );
                rOutput << '}';
            }
            if ( bFinishPortion )
            {
                rOutput << '}';
                bFinishPortion = sal_False;
            }

            nIndex = nIndex + pTextPortion->GetLen();
        }

        rOutput << OOO_STRING_SVTOOLS_RTF_PAR << OOO_STRING_SVTOOLS_RTF_PARD << OOO_STRING_SVTOOLS_RTF_PLAIN;;
        rOutput << endl;
    }
    // RTF-Nachspann...
    rOutput << "}}";    // 1xKlammerung Absaetze, 1x Klammerung RTF-Dokument
    rOutput.Flush();

#if defined (EDITDEBUG) && !defined( UNX )
    {
        SvFileStream aStream( String( RTL_CONSTASCII_USTRINGPARAM ( "d:\\rtf_out.rtf" ) ), STREAM_WRITE|STREAM_TRUNC );
        sal_uLong nP = rOutput.Tell();
        rOutput.Seek( 0 );
        aStream << rOutput;
        rOutput.Seek( nP );
    }
#endif

    return rOutput.GetError();
#else
    return 0;
#endif
}


void ImpEditEngine::WriteItemAsRTF( const SfxPoolItem& rItem, SvStream& rOutput, sal_uInt16 nPara, sal_uInt16 nPos,
                            SvxFontTable& rFontTable, SvxColorList& rColorList )
{
    sal_uInt16 nWhich = rItem.Which();
    switch ( nWhich )
    {
        case EE_PARA_WRITINGDIR:
        {
            const SvxFrameDirectionItem& rWritingMode = (const SvxFrameDirectionItem&)rItem;
            if ( rWritingMode.GetValue() == FRMDIR_HORI_RIGHT_TOP )
                rOutput << "\\rtlpar";
            else
                rOutput << "\\ltrpar";
        }
        break;
        case EE_PARA_OUTLLEVEL:
        {
            sal_Int16 nLevel = ((const SfxInt16Item&)rItem).GetValue();
            if( nLevel >= 0 )
            {
                rOutput << "\\level";
                rOutput.WriteNumber( nLevel );
            }
        }
        break;
        case EE_PARA_OUTLLRSPACE:
        case EE_PARA_LRSPACE:
        {
//            const ContentNode *pNode = aEditDoc.GetObject( nPara );

            rOutput << OOO_STRING_SVTOOLS_RTF_FI;
            short nTxtFirst = ((const SvxLRSpaceItem&)rItem).GetTxtFirstLineOfst();
            nTxtFirst = (short)LogicToTwips( nTxtFirst );
            rOutput.WriteNumber( nTxtFirst );
            rOutput << OOO_STRING_SVTOOLS_RTF_LI;
            sal_uInt16 nTxtLeft = static_cast< sal_uInt16 >(((const SvxLRSpaceItem&)rItem).GetTxtLeft());
            nTxtLeft = (sal_uInt16)LogicToTwips( nTxtLeft );
            rOutput.WriteNumber( nTxtLeft );
            rOutput << OOO_STRING_SVTOOLS_RTF_RI;
            sal_uInt32 nTxtRight = ((const SvxLRSpaceItem&)rItem).GetRight();
            nTxtRight = LogicToTwips( nTxtRight);
            rOutput.WriteNumber( nTxtRight );
        }
        break;
        case EE_PARA_ULSPACE:
        {
            rOutput << OOO_STRING_SVTOOLS_RTF_SB;
            sal_uInt16 nUpper = ((const SvxULSpaceItem&)rItem).GetUpper();
            nUpper = (sal_uInt16)LogicToTwips( nUpper );
            rOutput.WriteNumber( nUpper );
            rOutput << OOO_STRING_SVTOOLS_RTF_SA;
            sal_uInt16 nLower = ((const SvxULSpaceItem&)rItem).GetLower();
            nLower = (sal_uInt16)LogicToTwips( nLower );
            rOutput.WriteNumber( nLower );
        }
        break;
        case EE_PARA_SBL:
        {
            rOutput << OOO_STRING_SVTOOLS_RTF_SL;
            long nVal = ((const SvxLineSpacingItem&)rItem).GetLineHeight();
            char cMult = '0';
            if ( ((const SvxLineSpacingItem&)rItem).GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
            {
                // Woher kriege ich jetzt den Wert?
                // Der SwRTF-Parser geht von einem 240er Font aus!
                nVal = ((const SvxLineSpacingItem&)rItem).GetPropLineSpace();
                nVal *= 240;
                nVal /= 100;
                cMult = '1';
            }
            rOutput.WriteNumber( nVal );
            rOutput << OOO_STRING_SVTOOLS_RTF_SLMULT << cMult;
        }
        break;
        case EE_PARA_JUST:
        {
            SvxAdjust eJustification = ((const SvxAdjustItem&)rItem).GetAdjust();
            switch ( eJustification )
            {
                case SVX_ADJUST_CENTER: rOutput << OOO_STRING_SVTOOLS_RTF_QC;
                                        break;
                case SVX_ADJUST_RIGHT:  rOutput << OOO_STRING_SVTOOLS_RTF_QR;
                                        break;
                default:                rOutput << OOO_STRING_SVTOOLS_RTF_QL;
                                        break;
            }
        }
        break;
        case EE_PARA_TABS:
        {
            const SvxTabStopItem& rTabs = (const SvxTabStopItem&) rItem;
            for ( sal_uInt16 i = 0; i < rTabs.Count(); i++ )
            {
                const SvxTabStop& rTab = rTabs[i];
                rOutput << OOO_STRING_SVTOOLS_RTF_TX;
                rOutput.WriteNumber( LogicToTwips( rTab.GetTabPos() ) );
            }
        }
        break;
        case EE_CHAR_COLOR:
        {
            sal_uInt32 n = rColorList.GetId( (const SvxColorItem&)rItem );
            rOutput << OOO_STRING_SVTOOLS_RTF_CF;
            rOutput.WriteNumber( n );
        }
        break;
        case EE_CHAR_FONTINFO:
        case EE_CHAR_FONTINFO_CJK:
        case EE_CHAR_FONTINFO_CTL:
        {
            sal_uInt32 n = rFontTable.GetId( (const SvxFontItem&)rItem );
            rOutput << OOO_STRING_SVTOOLS_RTF_F;
            rOutput.WriteNumber( n );
        }
        break;
        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            rOutput << OOO_STRING_SVTOOLS_RTF_FS;
            long nHeight = ((const SvxFontHeightItem&)rItem).GetHeight();
            nHeight = LogicToTwips( nHeight );
            // Twips => HalfPoints
            nHeight /= 10;
            rOutput.WriteNumber( nHeight );
        }
        break;
        case EE_CHAR_WEIGHT:
        case EE_CHAR_WEIGHT_CJK:
        case EE_CHAR_WEIGHT_CTL:
        {
            FontWeight e = ((const SvxWeightItem&)rItem).GetWeight();
            switch ( e )
            {
                case WEIGHT_BOLD:   rOutput << OOO_STRING_SVTOOLS_RTF_B;                break;
                default:            rOutput << OOO_STRING_SVTOOLS_RTF_B << '0';     break;
            }
        }
        break;
        case EE_CHAR_UNDERLINE:
        {
            // muesste bei WordLineMode ggf. ulw werden,
            // aber die Information fehlt hier
            FontUnderline e = ((const SvxUnderlineItem&)rItem).GetLineStyle();
            switch ( e )
            {
                case UNDERLINE_NONE:    rOutput << OOO_STRING_SVTOOLS_RTF_ULNONE;       break;
                case UNDERLINE_SINGLE:  rOutput << OOO_STRING_SVTOOLS_RTF_UL;       break;
                case UNDERLINE_DOUBLE:  rOutput << OOO_STRING_SVTOOLS_RTF_ULDB;     break;
                case UNDERLINE_DOTTED:  rOutput << OOO_STRING_SVTOOLS_RTF_ULD;      break;
                default:
                    break;
            }
        }
        break;
        case EE_CHAR_OVERLINE:
        {
            FontUnderline e = ((const SvxOverlineItem&)rItem).GetLineStyle();
            switch ( e )
            {
                case UNDERLINE_NONE:    rOutput << OOO_STRING_SVTOOLS_RTF_OLNONE;       break;
                case UNDERLINE_SINGLE:  rOutput << OOO_STRING_SVTOOLS_RTF_OL;       break;
                case UNDERLINE_DOUBLE:  rOutput << OOO_STRING_SVTOOLS_RTF_OLDB;     break;
                case UNDERLINE_DOTTED:  rOutput << OOO_STRING_SVTOOLS_RTF_OLD;      break;
                default:
                    break;
            }
        }
        break;
        case EE_CHAR_STRIKEOUT:
        {
            FontStrikeout e = ((const SvxCrossedOutItem&)rItem).GetStrikeout();
            switch ( e )
            {
                case STRIKEOUT_SINGLE:
                case STRIKEOUT_DOUBLE:  rOutput << OOO_STRING_SVTOOLS_RTF_STRIKE;       break;
                case STRIKEOUT_NONE:    rOutput << OOO_STRING_SVTOOLS_RTF_STRIKE << '0';    break;
                default:
                    break;
            }
        }
        break;
        case EE_CHAR_ITALIC:
        case EE_CHAR_ITALIC_CJK:
        case EE_CHAR_ITALIC_CTL:
        {
            FontItalic e = ((const SvxPostureItem&)rItem).GetPosture();
            switch ( e )
            {
                case ITALIC_OBLIQUE:
                case ITALIC_NORMAL: rOutput << OOO_STRING_SVTOOLS_RTF_I;        break;
                case ITALIC_NONE:   rOutput << OOO_STRING_SVTOOLS_RTF_I << '0'; break;
                default:
                    break;
            }
        }
        break;
        case EE_CHAR_OUTLINE:
        {
            rOutput << OOO_STRING_SVTOOLS_RTF_OUTL;
            if ( ((const SvxContourItem&)rItem).GetValue() == 0 )
                rOutput << '0';
        }
        break;
        case EE_CHAR_RELIEF:
        {
            sal_uInt16 nRelief = ((const SvxCharReliefItem&)rItem).GetValue();
            if ( nRelief == RELIEF_EMBOSSED )
                rOutput << OOO_STRING_SVTOOLS_RTF_EMBO;
            if ( nRelief == RELIEF_ENGRAVED )
                rOutput << OOO_STRING_SVTOOLS_RTF_IMPR;
        }
        break;
        case EE_CHAR_EMPHASISMARK:
        {
            sal_uInt16 nMark = ((const SvxEmphasisMarkItem&)rItem).GetValue();
            if ( nMark == EMPHASISMARK_NONE )
                rOutput << OOO_STRING_SVTOOLS_RTF_ACCNONE;
            else if ( nMark == EMPHASISMARK_SIDE_DOTS )
                rOutput << OOO_STRING_SVTOOLS_RTF_ACCCOMMA;
            else
                rOutput << OOO_STRING_SVTOOLS_RTF_ACCDOT;
        }
        break;
        case EE_CHAR_SHADOW:
        {
            rOutput << OOO_STRING_SVTOOLS_RTF_SHAD;
            if ( ((const SvxShadowedItem&)rItem).GetValue() == 0 )
                rOutput << '0';
        }
        break;
        case EE_FEATURE_TAB:
        {
            rOutput << OOO_STRING_SVTOOLS_RTF_TAB;
        }
        break;
        case EE_FEATURE_LINEBR:
        {
            rOutput << OOO_STRING_SVTOOLS_RTF_SL;
        }
        break;
        case EE_CHAR_KERNING:
        {
            rOutput << OOO_STRING_SVTOOLS_RTF_EXPNDTW;
            rOutput.WriteNumber( LogicToTwips(
                ((const SvxKerningItem&)rItem).GetValue() ) );
        }
        break;
        case EE_CHAR_PAIRKERNING:
        {
            rOutput << OOO_STRING_SVTOOLS_RTF_KERNING;
            rOutput.WriteNumber( ((const SvxAutoKernItem&)rItem).GetValue() ? 1 : 0 );
        }
        break;
        case EE_CHAR_ESCAPEMENT:
        {
            SvxFont aFont;
            ContentNode* pNode = aEditDoc.GetObject( nPara );
            SeekCursor( pNode, nPos, aFont );
            MapMode aPntMode( MAP_POINT );
            long nFontHeight = GetRefDevice()->LogicToLogic(
                    aFont.GetSize(), &GetRefMapMode(), &aPntMode ).Height();
            nFontHeight *=2;    // HalfPoints
            sal_uInt16 nProp = ((const SvxEscapementItem&)rItem).GetProp();
            sal_uInt16 nProp100 = nProp*100;    // Fuer SWG-Token Prop in 100tel Prozent.
            short nEsc = ((const SvxEscapementItem&)rItem).GetEsc();
            if ( nEsc == DFLT_ESC_AUTO_SUPER )
            {
                nEsc = 100 - nProp;
                nProp100++; // Eine 1 hinten bedeutet 'automatisch'.
            }
            else if ( nEsc == DFLT_ESC_AUTO_SUB )
            {
                nEsc = sal::static_int_cast< short >( -( 100 - nProp ) );
                nProp100++;
            }
            // SWG:
            if ( nEsc )
                rOutput << "{\\*\\updnprop" << ByteString::CreateFromInt32( nProp100 ).GetBuffer() << '}';
            long nUpDown = nFontHeight * Abs( nEsc ) / 100;
            ByteString aUpDown = ByteString::CreateFromInt32( nUpDown );
            if ( nEsc < 0 )
                rOutput << OOO_STRING_SVTOOLS_RTF_DN << aUpDown.GetBuffer();
            else if ( nEsc > 0 )
                rOutput << OOO_STRING_SVTOOLS_RTF_UP << aUpDown.GetBuffer();
        }
        break;
    }
}

sal_uInt32 ImpEditEngine::WriteHTML( SvStream&, EditSelection )
{
    return 0;
}


EditTextObject* ImpEditEngine::CreateTextObject()
{
    EditSelection aCompleteSelection;
    aCompleteSelection.Min() = aEditDoc.GetStartPaM();
    aCompleteSelection.Max() = aEditDoc.GetEndPaM();

    return CreateTextObject( aCompleteSelection );
}

EditTextObject* ImpEditEngine::CreateTextObject( EditSelection aSel )
{
    return CreateBinTextObject( aSel, GetEditTextObjectPool(), aStatus.AllowBigObjects(), nBigTextObjectStart );
}

EditTextObject* ImpEditEngine::CreateBinTextObject( EditSelection aSel, SfxItemPool* pPool, sal_Bool bAllowBigObjects, sal_uInt16 nBigObjectStart ) const
{
    BinTextObject* pTxtObj = new BinTextObject( pPool );
    pTxtObj->SetVertical( IsVertical() );
    MapUnit eMapUnit = (MapUnit)aEditDoc.GetItemPool().GetMetric( DEF_METRIC );
    pTxtObj->SetMetric( (sal_uInt16) eMapUnit );
    if ( pTxtObj->IsOwnerOfPool() )
        pTxtObj->GetPool()->SetDefaultMetric( (SfxMapUnit) eMapUnit );

    sal_uInt16 nStartNode, nEndNode;
    sal_uInt32 nTextPortions = 0;

    aSel.Adjust( aEditDoc );
    nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    sal_Bool bOnlyFullParagraphs = ( aSel.Min().GetIndex() ||
        ( aSel.Max().GetIndex() < aSel.Max().GetNode()->Len() ) ) ?
            sal_False : sal_True;

    // Vorlagen werden nicht gespeichert!
    // ( Nur Name und Familie, Vorlage selbst muss in App stehen! )

    pTxtObj->SetScriptType( GetScriptType( aSel ) );

    // ueber die Absaetze iterieren...
    sal_uInt16 nNode;
    for ( nNode = nStartNode; nNode <= nEndNode; nNode++  )
    {
        ContentNode* pNode = aEditDoc.SaveGetObject( nNode );
        DBG_ASSERT( pNode, "Node nicht gefunden: Search&Replace" );

        if ( bOnlyFullParagraphs )
        {
            ParaPortion* pParaPortion = GetParaPortions()[nNode];
            nTextPortions += pParaPortion->GetTextPortions().Count();
        }

        sal_uInt16 nStartPos = 0;
        sal_uInt16 nEndPos = pNode->Len();

        sal_Bool bEmptyPara = nEndPos ? sal_False : sal_True;

        if ( ( nNode == nStartNode ) && !bOnlyFullParagraphs )
            nStartPos = aSel.Min().GetIndex();
        if ( ( nNode == nEndNode ) && !bOnlyFullParagraphs )
            nEndPos = aSel.Max().GetIndex();


        ContentInfo* pC = pTxtObj->CreateAndInsertContent();

        // Die Absatzattribute...
        pC->GetParaAttribs().Set( pNode->GetContentAttribs().GetItems() );

        // Das StyleSheet...
        if ( pNode->GetStyleSheet() )
        {
            pC->GetStyle() = pNode->GetStyleSheet()->GetName();
            pC->GetFamily() = pNode->GetStyleSheet()->GetFamily();
        }

        // Der Text...
        pC->GetText() = pNode->Copy( nStartPos, nEndPos-nStartPos );

        // und die Attribute...
        sal_uInt16 nAttr = 0;
        EditCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
        while ( pAttr )
        {
            // In einem leeren Absatz die Attribute behalten!
            if ( bEmptyPara ||
                 ( ( pAttr->GetEnd() > nStartPos ) && ( pAttr->GetStart() < nEndPos ) ) )
            {
                XEditAttribute* pX = pTxtObj->CreateAttrib( *pAttr->GetItem(), pAttr->GetStart(), pAttr->GetEnd() );
                // Evtl. korrigieren...
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
                DBG_ASSERT( pX->GetEnd() <= (nEndPos-nStartPos), "CreateBinTextObject: Attribut zu lang!" );
                if ( !pX->GetLen() && !bEmptyPara )
                    pTxtObj->DestroyAttrib( pX );
                else
                    pC->GetAttribs().Insert( pX, pC->GetAttribs().Count() );
            }
            nAttr++;
            pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
        }

#ifndef SVX_LIGHT
        // ggf. Online-Spelling
        if ( bAllowBigObjects && bOnlyFullParagraphs && pNode->GetWrongList() )
            pC->SetWrongList( pNode->GetWrongList()->Clone() );
#endif // !SVX_LIGHT

    }

    // Bei grossen Textobjekten die PortionInfos merken:
    // Schwelle rauf setzen, wenn Olli die Absaetze nicht mehr zerhackt!
    if ( bAllowBigObjects && bOnlyFullParagraphs && IsFormatted() && GetUpdateMode() && ( nTextPortions >= nBigObjectStart ) )
    {
        XParaPortionList* pXList = new XParaPortionList( GetRefDevice(), aPaperSize.Width() );
        pTxtObj->SetPortionInfo( pXList );
        for ( nNode = nStartNode; nNode <= nEndNode; nNode++  )
        {
            ParaPortion* pParaPortion = GetParaPortions()[nNode];
            XParaPortion* pX = new XParaPortion;
            pXList->Insert( pX, pXList->Count() );

            pX->nHeight = pParaPortion->GetHeight();
            pX->nFirstLineOffset = pParaPortion->GetFirstLineOffset();

            // Die TextPortions
            sal_uInt16 nCount = pParaPortion->GetTextPortions().Count();
            sal_uInt16 n;
            for ( n = 0; n < nCount; n++ )
            {
                TextPortion* pTextPortion = pParaPortion->GetTextPortions()[n];
                TextPortion* pNew = new TextPortion( *pTextPortion );
                pX->aTextPortions.Insert( pNew, pX->aTextPortions.Count() );
            }

            // Die Zeilen
            nCount = pParaPortion->GetLines().Count();
            for ( n = 0; n < nCount; n++ )
            {
                EditLine* pLine = pParaPortion->GetLines()[n];
                EditLine* pNew = pLine->Clone();
                pX->aLines.Insert( pNew, pX->aLines.Count() );
            }
#ifdef DBG_UTIL
            sal_uInt16 nTest;
            int nTPLen = 0, nTxtLen = 0;
            for ( nTest = pParaPortion->GetTextPortions().Count(); nTest; )
                nTPLen += pParaPortion->GetTextPortions().GetObject( --nTest )->GetLen();
            for ( nTest = pParaPortion->GetLines().Count(); nTest; )
                nTxtLen += pParaPortion->GetLines().GetObject( --nTest )->GetLen();
            DBG_ASSERT( ( nTPLen == pParaPortion->GetNode()->Len() ) && ( nTxtLen == pParaPortion->GetNode()->Len() ), "CreateBinTextObject: ParaPortion not completely formatted!" );
#endif
        }
    }
    return pTxtObj;
}

void ImpEditEngine::SetText( const EditTextObject& rTextObject )
{
    // Da Setzen eines TextObject ist nicht Undo-faehig!
    ResetUndoManager();
    sal_Bool _bUpdate = GetUpdateMode();
    sal_Bool _bUndo = IsUndoEnabled();

    SetText( XubString() );
    EditPaM aPaM = aEditDoc.GetStartPaM();

    SetUpdateMode( sal_False );
    EnableUndo( sal_False );

    InsertText( rTextObject, EditSelection( aPaM, aPaM ) );
    SetVertical( rTextObject.IsVertical() );

#ifndef SVX_LIGHT
    DBG_ASSERT( !HasUndoManager() || !GetUndoManager().GetUndoActionCount(), "Woher kommt das Undo in SetText ?!" );
#endif
    SetUpdateMode( _bUpdate );
    EnableUndo( _bUndo );
}

EditSelection ImpEditEngine::InsertText( const EditTextObject& rTextObject, EditSelection aSel )
{
    EnterBlockNotifications();
    aSel.Adjust( aEditDoc );
    if ( aSel.HasRange() )
        aSel = ImpDeleteSelection( aSel );
    EditSelection aNewSel = InsertBinTextObject( (BinTextObject&)rTextObject, aSel.Max() );
    LeaveBlockNotifications();
    return aNewSel;

    // MT 05/00: InsertBinTextObject direkt hier machen...
}

EditSelection ImpEditEngine::InsertBinTextObject( BinTextObject& rTextObject, EditPaM aPaM )
{
    // Optimieren:
    // Kein GetPos undFindParaportion, sondern Index berechnen!
    EditSelection aSel( aPaM, aPaM );
    DBG_ASSERT( !aSel.DbgIsBuggy( aEditDoc ), "InsertBibTextObject: Selektion kaput!(1)" );

    sal_Bool bUsePortionInfo = sal_False;
//  sal_Bool bFields = sal_False;
    XParaPortionList* pPortionInfo = rTextObject.GetPortionInfo();

    if ( pPortionInfo && ( (long)pPortionInfo->GetPaperWidth() == aPaperSize.Width() )
            && ( pPortionInfo->GetRefMapMode() == GetRefDevice()->GetMapMode() ) )
    {
        if ( ( pPortionInfo->GetRefDevPtr() == (sal_uIntPtr)GetRefDevice() ) ||
             ( ( pPortionInfo->GetRefDevType() == OUTDEV_VIRDEV ) &&
               ( GetRefDevice()->GetOutDevType() == OUTDEV_VIRDEV ) ) )
        bUsePortionInfo = sal_True;
    }

    sal_Bool bConvertItems = sal_False;
    MapUnit eSourceUnit = MapUnit(), eDestUnit = MapUnit();
    if ( rTextObject.HasMetric() )
    {
        eSourceUnit = (MapUnit)rTextObject.GetMetric();
        eDestUnit = (MapUnit)aEditDoc.GetItemPool().GetMetric( DEF_METRIC );
        if ( eSourceUnit != eDestUnit )
            bConvertItems = sal_True;
    }

    sal_uInt16 nContents = rTextObject.GetContents().Count();
    sal_uInt16 nPara = aEditDoc.GetPos( aPaM.GetNode() );

    for ( sal_uInt16 n = 0; n < nContents; n++, nPara++ )
    {
        ContentInfo* pC = rTextObject.GetContents().GetObject( n );
        sal_Bool bNewContent = aPaM.GetNode()->Len() ? sal_False: sal_True;
        sal_uInt16 nStartPos = aPaM.GetIndex();

        aPaM = ImpFastInsertText( aPaM, pC->GetText() );

        ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
        DBG_ASSERT( pPortion, "Blinde Portion in FastInsertText" );
        pPortion->MarkInvalid( nStartPos, pC->GetText().Len() );

        // Zeicheattribute...
        sal_Bool bAllreadyHasAttribs = aPaM.GetNode()->GetCharAttribs().Count() ? sal_True : sal_False;
        sal_uInt16 nNewAttribs = pC->GetAttribs().Count();
        if ( nNewAttribs )
        {
            sal_Bool bUpdateFields = sal_False;
            for ( sal_uInt16 nAttr = 0; nAttr < nNewAttribs; nAttr++ )
            {
                XEditAttribute* pX = pC->GetAttribs().GetObject( nAttr );
                // Kann passieren wenn Absaetze >16K entstehen, dann wird einfach umgebrochen.
                if ( pX->GetEnd() <= aPaM.GetNode()->Len() )
                {
                    if ( !bAllreadyHasAttribs || pX->IsFeature() )
                    {
                        // Normale Attribute gehen dann schneller...
                        // Features duerfen nicht ueber EditDoc::InsertAttrib
                        // eingefuegt werden, sie sind bei FastInsertText schon im TextFluss
                        DBG_ASSERT( pX->GetEnd() <= aPaM.GetNode()->Len(), "InsertBinTextObject: Attribut zu gross!" );
                        EditCharAttrib* pAttr;
                        if ( !bConvertItems )
                            pAttr = MakeCharAttrib( aEditDoc.GetItemPool(), *(pX->GetItem()), pX->GetStart()+nStartPos, pX->GetEnd()+nStartPos );
                        else
                        {
                            SfxPoolItem* pNew = pX->GetItem()->Clone();
                            ConvertItem( *pNew, eSourceUnit, eDestUnit );
                            pAttr = MakeCharAttrib( aEditDoc.GetItemPool(), *pNew, pX->GetStart()+nStartPos, pX->GetEnd()+nStartPos );
                            delete pNew;
                        }
                        DBG_ASSERT( pAttr->GetEnd() <= aPaM.GetNode()->Len(), "InsertBinTextObject: Attribut passt nicht! (1)" );
                        aPaM.GetNode()->GetCharAttribs().InsertAttrib( pAttr );
                        if ( pAttr->Which() == EE_FEATURE_FIELD )
                            bUpdateFields = sal_True;
                    }
                    else
                    {
                        DBG_ASSERT( pX->GetEnd()+nStartPos <= aPaM.GetNode()->Len(), "InsertBinTextObject: Attribut passt nicht! (2)" );
                        // Tabs und andere Features koennen nicht ueber InsertAttrib eingefuegt werden:
                        aEditDoc.InsertAttrib( aPaM.GetNode(), pX->GetStart()+nStartPos, pX->GetEnd()+nStartPos, *pX->GetItem() );
                    }
                }
            }
            if ( bUpdateFields )
                UpdateFields();

            // Sonst QuickFormat => Keine Attribute!
            pPortion->MarkSelectionInvalid( nStartPos, pC->GetText().Len() );
        }

        DBG_ASSERT( CheckOrderedList( aPaM.GetNode()->GetCharAttribs().GetAttribs(), sal_True ), "InsertBinTextObject: Start-Liste verdreht" );

        sal_Bool bParaAttribs = sal_False;
        if ( bNewContent || ( ( n > 0 ) && ( n < (nContents-1) ) ) )
        {
            bParaAttribs = sal_False;
            // #101512# Don't overwrite level/style from existing paragraph in OutlineView
            // MT 10/2002: Removed because of #103874#, handled in Outliner::EndPasteOrDropHdl now.
//            if ( !aStatus.IsOutliner() || n )
            {
                // nur dann Style und ParaAttribs, wenn neuer Absatz, oder
                // komplett inneliegender...
                bParaAttribs = pC->GetParaAttribs().Count() ? sal_True : sal_False;
                if ( GetStyleSheetPool() && pC->GetStyle().Len() )
                {
                    SfxStyleSheet* pStyle = (SfxStyleSheet*)GetStyleSheetPool()->Find( pC->GetStyle(), pC->GetFamily() );
                    DBG_ASSERT( pStyle, "InsertBinTextObject - Style not found!" );
                    SetStyleSheet( nPara, pStyle );
                }
                if ( !bConvertItems )
                    SetParaAttribs( aEditDoc.GetPos( aPaM.GetNode() ), pC->GetParaAttribs() );
                else
                {
                    SfxItemSet aAttribs( GetEmptyItemSet() );
                    ConvertAndPutItems( aAttribs, pC->GetParaAttribs(), &eSourceUnit, &eDestUnit );
                    SetParaAttribs( aEditDoc.GetPos( aPaM.GetNode() ), aAttribs );
                }
            }
            if ( bNewContent && bUsePortionInfo )
            {
                XParaPortion* pXP = pPortionInfo->GetObject( n );
                DBG_ASSERT( pXP, "InsertBinTextObject: PortionInfo?" );
                ParaPortion* pParaPortion = GetParaPortions()[ nPara ];
                DBG_ASSERT( pParaPortion, "InsertBinTextObject: ParaPortion?" );
                pParaPortion->nHeight = pXP->nHeight;
                pParaPortion->nFirstLineOffset = pXP->nFirstLineOffset;
                pParaPortion->bForceRepaint = sal_True;
                pParaPortion->SetValid();   // Nicht formatieren

                // Die TextPortions
                pParaPortion->GetTextPortions().Reset();
                sal_uInt16 nCount = pXP->aTextPortions.Count();
                for ( sal_uInt16 _n = 0; _n < nCount; _n++ )
                {
                    TextPortion* pTextPortion = pXP->aTextPortions[_n];
                    TextPortion* pNew = new TextPortion( *pTextPortion );
                    pParaPortion->GetTextPortions().Insert( pNew, _n );
                }

                // Die Zeilen
                pParaPortion->GetLines().Reset();
                nCount = pXP->aLines.Count();
                for ( sal_uInt16 m = 0; m < nCount; m++ )
                {
                    EditLine* pLine = pXP->aLines[m];
                    EditLine* pNew = pLine->Clone();
                    pNew->SetInvalid(); // neu Painten!
                    pParaPortion->GetLines().Insert( pNew, m );
                }
#ifdef DBG_UTIL
                sal_uInt16 nTest;
                int nTPLen = 0, nTxtLen = 0;
                for ( nTest = pParaPortion->GetTextPortions().Count(); nTest; )
                    nTPLen += pParaPortion->GetTextPortions().GetObject( --nTest )->GetLen();
                for ( nTest = pParaPortion->GetLines().Count(); nTest; )
                    nTxtLen += pParaPortion->GetLines().GetObject( --nTest )->GetLen();
                DBG_ASSERT( ( nTPLen == pParaPortion->GetNode()->Len() ) && ( nTxtLen == pParaPortion->GetNode()->Len() ), "InsertBinTextObject: ParaPortion not completely formatted!" );
#endif
            }
        }
        if ( !bParaAttribs ) // DefFont wird bei FastInsertParagraph nicht berechnet
        {
            aPaM.GetNode()->GetCharAttribs().GetDefFont() = aEditDoc.GetDefFont();
            if ( aStatus.UseCharAttribs() )
                aPaM.GetNode()->CreateDefFont();
        }

#ifndef SVX_LIGHT
        if ( bNewContent && GetStatus().DoOnlineSpelling() && pC->GetWrongList() )
        {
            aPaM.GetNode()->DestroyWrongList(); // otherwise MLK, if list exists...
            aPaM.GetNode()->SetWrongList( pC->GetWrongList()->Clone() );
        }
#endif // !SVX_LIGHT

        // Zeilenumbruch, wenn weitere folgen...
        if ( n < ( nContents-1) )
        {
            if ( bNewContent )
                aPaM = ImpFastInsertParagraph( nPara+1 );
            else
                aPaM = ImpInsertParaBreak( aPaM, sal_False );
        }
    }

    aSel.Max() = aPaM;
    DBG_ASSERT( !aSel.DbgIsBuggy( aEditDoc ), "InsertBibTextObject: Selektion kaput!(1)" );
    return aSel;
}

LanguageType ImpEditEngine::GetLanguage( const EditPaM& rPaM, sal_uInt16* pEndPos ) const
{
    short nScriptType = GetScriptType( rPaM, pEndPos ); // pEndPos will be valid now, pointing to ScriptChange or NodeLen
    sal_uInt16 nLangId = GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType );
    const SvxLanguageItem* pLangItem = &(const SvxLanguageItem&)rPaM.GetNode()->GetContentAttribs().GetItem( nLangId );
    EditCharAttrib* pAttr = rPaM.GetNode()->GetCharAttribs().FindAttrib( nLangId, rPaM.GetIndex() );
    if ( pAttr )
        pLangItem = (const SvxLanguageItem*)pAttr->GetItem();

    if ( pEndPos && pAttr && ( pAttr->GetEnd() < *pEndPos ) )
        *pEndPos = pAttr->GetEnd();

    return pLangItem->GetLanguage();
}

::com::sun::star::lang::Locale ImpEditEngine::GetLocale( const EditPaM& rPaM ) const
{
    return SvxCreateLocale( GetLanguage( rPaM ) );
}

Reference< XSpellChecker1 > ImpEditEngine::GetSpeller()
{
#ifndef SVX_LIGHT
    if ( !xSpeller.is() )
        xSpeller = SvxGetSpellChecker();
#endif
    return xSpeller;
}


SpellInfo * ImpEditEngine::CreateSpellInfo( const EditSelection &rSel, bool bMultipleDocs )
{
    if (!pSpellInfo)
        pSpellInfo = new SpellInfo;
    else
        *pSpellInfo = SpellInfo();  // reset to default values

    pSpellInfo->bMultipleDoc = bMultipleDocs;
    EditSelection aSentenceSel( SelectSentence( rSel ) );
//    pSpellInfo->aSpellStart = CreateEPaM( aSentenceSel.Min() );
//    pSpellInfo->aSpellTo    = CreateEPaM( rSel.HasRange()? aSentenceSel.Max() : aSentenceSel.Min() );
    // always spell draw objects completely, startting at the top.
    // (spelling in only a selection or not starting with the top requires
    // further changes elsewehe to work properly)
    pSpellInfo->aSpellStart = EPaM();
    pSpellInfo->aSpellTo    = EPaM( EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND );
    return pSpellInfo;
}


EESpellState ImpEditEngine::Spell( EditView* pEditView, sal_Bool bMultipleDoc )
{
#ifdef SVX_LIGHT
    return EE_SPELL_NOSPELLER;
#else

    DBG_ASSERTWARNING( xSpeller.is(), "Kein Speller gesetzt!" );

    if ( !xSpeller.is() )
        return EE_SPELL_NOSPELLER;

    aOnlineSpellTimer.Stop();

    // Bei MultipleDoc immer von vorne/hinten...
    if ( bMultipleDoc )
    {
        pEditView->pImpEditView->SetEditSelection( aEditDoc.GetStartPaM() );
    }

    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );
    pSpellInfo = CreateSpellInfo( aCurSel, bMultipleDoc );

    sal_Bool bIsStart = sal_False;
    if ( bMultipleDoc )
        bIsStart = sal_True;    // Immer von Vorne bzw. von hinten...
    else if ( ( CreateEPaM( aEditDoc.GetStartPaM() ) == pSpellInfo->aSpellStart ) )
        bIsStart = sal_True;

    EditSpellWrapper* pWrp = new EditSpellWrapper( Application::GetDefDialogParent(),
            xSpeller, bIsStart, sal_False, pEditView );
    pWrp->SpellDocument();
    delete pWrp;

    if ( !bMultipleDoc )
    {
        pEditView->pImpEditView->DrawSelection();
        if ( aCurSel.Max().GetIndex() > aCurSel.Max().GetNode()->Len() )
            aCurSel.Max().GetIndex() = aCurSel.Max().GetNode()->Len();
        aCurSel.Min() = aCurSel.Max();
        pEditView->pImpEditView->SetEditSelection( aCurSel );
        pEditView->pImpEditView->DrawSelection();
        pEditView->ShowCursor( sal_True, sal_False );
    }
    EESpellState eState = pSpellInfo->eState;
    delete pSpellInfo;
    pSpellInfo = 0;
    return eState;
#endif
}


sal_Bool ImpEditEngine::HasConvertibleTextPortion( LanguageType nSrcLang )
{
#ifdef SVX_LIGHT
    return sal_False;
#else
    sal_Bool    bHasConvTxt = sal_False;

    sal_uInt16 nParas = pEditEngine->GetParagraphCount();
    for (sal_uInt16 k = 0;  k < nParas;  ++k)
    {
        SvUShorts aPortions;
        pEditEngine->GetPortions( k, aPortions );
        for ( sal_uInt16 nPos = 0; nPos < aPortions.Count(); ++nPos )
        {
            sal_uInt16 nEnd   = aPortions.GetObject( nPos );
            sal_uInt16 nStart = nPos > 0 ? aPortions.GetObject( nPos - 1 ) : 0;

            // if the paragraph is not empty we need to increase the index
            // by one since the attribute of the character left to the
            // specified position is evaluated.
            if (nEnd > nStart)  // empty para?
                ++nStart;
            LanguageType nLangFound = pEditEngine->GetLanguage( k, nStart );
#ifdef DEBUG
            lang::Locale aLocale( SvxCreateLocale( nLangFound ) );
#endif
            bHasConvTxt =   (nSrcLang == nLangFound) ||
                            (editeng::HangulHanjaConversion::IsChinese( nLangFound ) &&
                             editeng::HangulHanjaConversion::IsChinese( nSrcLang ));
            if (bHasConvTxt)
                return bHasConvTxt;
       }
    }

#endif
    return bHasConvTxt;
}


void ImpEditEngine::Convert( EditView* pEditView,
        LanguageType nSrcLang, LanguageType nDestLang, const Font *pDestFont,
        sal_Int32 nOptions, sal_Bool bIsInteractive, sal_Bool bMultipleDoc )
{
    // modified version of ImpEditEngine::Spell

#ifdef SVX_LIGHT
#else

    // Bei MultipleDoc immer von vorne/hinten...
    if ( bMultipleDoc )
        pEditView->pImpEditView->SetEditSelection( aEditDoc.GetStartPaM() );

    //
    // initialize pConvInfo
    //
    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );
    aCurSel.Adjust( aEditDoc );
    pConvInfo = new ConvInfo;
    pConvInfo->bMultipleDoc = bMultipleDoc;
    pConvInfo->aConvStart = CreateEPaM( aCurSel.Min() );
    //
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
        sal_uInt16 nStartIdx = ( editeng::HangulHanjaConversion::IsChinese( nSrcLang ) ) ?
                                0 : aWordStartPaM.GetIndex();
        pConvInfo->aConvStart.nIndex = nStartIdx;
    }
    //
    pConvInfo->aConvContinue = pConvInfo->aConvStart;

    sal_Bool bIsStart = sal_False;
    if ( bMultipleDoc )
        bIsStart = sal_True;    // Immer von Vorne bzw. von hinten...
    else if ( CreateEPaM( aEditDoc.GetStartPaM() ) == pConvInfo->aConvStart )
        bIsStart = sal_True;

    bImpConvertFirstCall = sal_True;    // next ImpConvert call is the very first in this conversion turn

    Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    TextConvWrapper aWrp( Application::GetDefDialogParent(), xMSF,
                          SvxCreateLocale( nSrcLang ), SvxCreateLocale( nDestLang ),
                          pDestFont,
                          nOptions, bIsInteractive,
                          bIsStart, pEditView );

    //
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
        pEditView->pImpEditView->DrawSelection();
        if ( aCurSel.Max().GetIndex() > aCurSel.Max().GetNode()->Len() )
            aCurSel.Max().GetIndex() = aCurSel.Max().GetNode()->Len();
        aCurSel.Min() = aCurSel.Max();
        pEditView->pImpEditView->SetEditSelection( aCurSel );
        pEditView->pImpEditView->DrawSelection();
        pEditView->ShowCursor( sal_True, sal_False );
    }
    delete pConvInfo;
    pConvInfo = 0;
#endif
}


void ImpEditEngine::SetLanguageAndFont(
    const ESelection &rESel,
    LanguageType nLang, sal_uInt16 nLangWhichId,
    const Font *pFont,  sal_uInt16 nFontWhichId )
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
        SvxFontItem aFontItem = (SvxFontItem&) aNewSet.Get( nFontWhichId );
        aFontItem.GetFamilyName()   = pFont->GetName();
        aFontItem.GetFamily()       = pFont->GetFamily();
        aFontItem.GetStyleName()    = pFont->GetStyleName();
        aFontItem.GetPitch()        = pFont->GetPitch();
        aFontItem.GetCharSet()      = pFont->GetCharSet();
        aNewSet.Put( aFontItem );
    }

    // apply new attributes
    pActiveView->SetAttribs( aNewSet );

    pActiveView->SetSelection( aOldSel );
}


void ImpEditEngine::ImpConvert( rtl::OUString &rConvTxt, LanguageType &rConvTxtLang,
        EditView* pEditView, LanguageType nSrcLang, const ESelection &rConvRange,
        sal_Bool bAllowImplicitChangesForNotConvertibleText,
        LanguageType nTargetLang, const Font *pTargetFont  )
{
    // modified version of ImpEditEngine::ImpSpell

    // looks for next convertible text portion to be passed on to the wrapper

    String aRes;
    LanguageType nResLang = LANGUAGE_NONE;

#ifdef SVX_LIGHT
    rConvTxt = rtl::OUString();
    rConvTxtLang = LANGUAGE_NONE;
#else

    /* ContentNode* pLastNode = */ aEditDoc.SaveGetObject( aEditDoc.Count()-1 );

    EditPaM aPos( CreateEditPaM( pConvInfo->aConvContinue ) );
    EditSelection aCurSel = EditSelection( aPos, aPos );

    String aWord;

    while (!aRes.Len())
    {
        // empty paragraph found that needs to have language and font set?
        if (bAllowImplicitChangesForNotConvertibleText &&
            !pEditEngine->GetText( pConvInfo->aConvContinue.nPara ).Len())
        {
            sal_uInt16 nPara = pConvInfo->aConvContinue.nPara;
            ESelection aESel( nPara, 0, nPara, 0 );
            // see comment for below same function call
            SetLanguageAndFont( aESel,
                    nTargetLang, EE_CHAR_LANGUAGE_CJK,
                    pTargetFont, EE_CHAR_FONTINFO_CJK );
        }


        if (pConvInfo->aConvContinue.nPara  == pConvInfo->aConvTo.nPara &&
            pConvInfo->aConvContinue.nIndex >= pConvInfo->aConvTo.nIndex)
            break;

/*
        // Bekannter (wahrscheinlicher) Bug: Wenn SpellToCurrent, muss
        // Current bei jeder Ersetzung korrigiert werden, sonst passt
        // das Ende evtl. nicht mehr genau...
        if ( pConvInfo->bConvToEnd || pConvInfo->bMultipleDoc )
        {
            if ( aCurSel.Max().GetNode() == pLastNode &&
                 aCurSel.Max().GetIndex() >= pLastNode->Len() )
                break;
        }
*/

        sal_uInt16 nAttribStart = USHRT_MAX;
        sal_uInt16 nAttribEnd   = USHRT_MAX;
        sal_uInt16 nCurPos      = USHRT_MAX;
        EPaM aCurStart = CreateEPaM( aCurSel.Min() );
        SvUShorts aPortions;
        pEditEngine->GetPortions( (sal_uInt16)aCurStart.nPara, aPortions );
        for ( sal_uInt16 nPos = 0; nPos < aPortions.Count(); ++nPos )
        {
            sal_uInt16 nEnd   = aPortions.GetObject( nPos );
            sal_uInt16 nStart = nPos > 0 ? aPortions.GetObject( nPos - 1 ) : 0;

            // the language attribute is obtained from the left character
            // (like usually all other attributes)
            // thus we usually have to add 1 in order to get the language
            // of the text right to the cursor position
            sal_uInt16 nLangIdx = nEnd > nStart ? nStart + 1 : nStart;
            LanguageType nLangFound = pEditEngine->GetLanguage( aCurStart.nPara, nLangIdx );
#ifdef DEBUG
            lang::Locale aLocale( SvxCreateLocale( nLangFound ) );
#endif
            sal_Bool bLangOk =  (nLangFound == nSrcLang) ||
                                (editeng::HangulHanjaConversion::IsChinese( nLangFound ) &&
                                 editeng::HangulHanjaConversion::IsChinese( nSrcLang ));

            if (nAttribEnd != USHRT_MAX) // start already found?
            {
                DBG_ASSERT(nEnd >= aCurStart.nIndex, "error while scanning attributes (a)" );
                DBG_ASSERT(nEnd >= nAttribEnd, "error while scanning attributes (b)" );
                if (/*nEnd >= aCurStart.nIndex &&*/ nLangFound == nResLang)
                    nAttribEnd = nEnd;
                else  // language attrib has changed
                    break;
            }
            if (nAttribStart == USHRT_MAX && // start not yet found?
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
            if (nAttribStart < aCurStart.nIndex)
            {
                nAttribStart = aCurStart.nIndex;
            }

            // check script type to the right of the start of the current portion
            EditPaM aPaM( CreateEditPaM( EPaM(aCurStart.nPara, nLangIdx) ) );
            sal_Bool bIsAsianScript = (i18n::ScriptType::ASIAN == GetScriptType( aPaM ));
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

        if (nAttribStart != USHRT_MAX  &&  nAttribEnd != USHRT_MAX)
        {
            aCurSel.Min().SetIndex( nAttribStart );
            aCurSel.Max().SetIndex( nAttribEnd );
        }
        else if (nCurPos != USHRT_MAX)
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

        if ( aWord.Len() > 0 /* && bLangOk */)
            aRes = aWord;

        // move to next word/paragraph if necessary
        if ( !aRes.Len() )
            aCurSel = WordRight( aCurSel.Min(), ::com::sun::star::i18n::WordType::DICTIONARY_WORD );

        pConvInfo->aConvContinue = CreateEPaM( aCurSel.Max() );
    }

    pEditView->pImpEditView->DrawSelection();
    pEditView->pImpEditView->SetEditSelection( aCurSel );
    pEditView->pImpEditView->DrawSelection();
    pEditView->ShowCursor( sal_True, sal_False );

    rConvTxt = aRes;
    if (rConvTxt.getLength())
        rConvTxtLang = nResLang;
#endif
}


Reference< XSpellAlternatives > ImpEditEngine::ImpSpell( EditView* pEditView )
{
#ifdef SVX_LIGHT
    return Reference< XSpellAlternatives >();
#else

    DBG_ASSERT( xSpeller.is(), "Kein Speller gesetzt!" );

    ContentNode* pLastNode = aEditDoc.SaveGetObject( (aEditDoc.Count()-1) );
    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );
    aCurSel.Min() = aCurSel.Max();

    String aWord;
    Reference< XSpellAlternatives > xSpellAlt;
    Sequence< PropertyValue > aEmptySeq;
    while (!xSpellAlt.is())
    {

        // Bekannter (wahrscheinlicher) Bug: Wenn SpellToCurrent, muss
        // Current bei jeder Ersetzung korrigiert werden, sonst passt
        // das Ende evtl. nicht mehr genau...
        if ( pSpellInfo->bSpellToEnd || pSpellInfo->bMultipleDoc )
        {
            if ( aCurSel.Max().GetNode() == pLastNode )
            {
                if ( ( aCurSel.Max().GetIndex() >= pLastNode->Len() ) )
                    break;
            }
        }
        else if ( !pSpellInfo->bSpellToEnd )
        {
            EPaM aEPaM( CreateEPaM( aCurSel.Max() ) );
            if ( !( aEPaM < pSpellInfo->aSpellTo ) )
                break;
        }

        aCurSel = SelectWord( aCurSel, ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
        aWord = GetSelected( aCurSel );

        // Wenn Punkt dahinter, muss dieser mit uebergeben werden !
        // Falls Abkuerzung...
        if ( aWord.Len() && ( aCurSel.Max().GetIndex() < aCurSel.Max().GetNode()->Len() ) )
        {
            sal_Unicode cNext = aCurSel.Max().GetNode()->GetChar( aCurSel.Max().GetIndex() );
            if ( cNext == '.' )
            {
                aCurSel.Max().GetIndex()++;
                aWord += cNext;
            }
        }

        if ( aWord.Len() > 0 )
        {
            LanguageType eLang = GetLanguage( aCurSel.Max() );
            SvxSpellWrapper::CheckSpellLang( xSpeller, eLang );
            xSpellAlt = xSpeller->spell( aWord, eLang, aEmptySeq );
        }

        if ( !xSpellAlt.is() )
            aCurSel = WordRight( aCurSel.Min(), ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
        else
            pSpellInfo->eState = EE_SPELL_ERRORFOUND;
    }

    pEditView->pImpEditView->DrawSelection();
    pEditView->pImpEditView->SetEditSelection( aCurSel );
    pEditView->pImpEditView->DrawSelection();
    pEditView->ShowCursor( sal_True, sal_False );
    return xSpellAlt;
#endif
}
/*-- 13.10.2003 16:43:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void ImpEditEngine::EndSpelling()
{
    DELETEZ(pSpellInfo);
}
/*-- 13.10.2003 16:43:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void ImpEditEngine::StartSpelling(EditView& rEditView, sal_Bool bMultipleDoc)
{
    DBG_ASSERT(!pSpellInfo, "pSpellInfo already set?");
    rEditView.pImpEditView->SetEditSelection( aEditDoc.GetStartPaM() );
    EditSelection aCurSel( rEditView.pImpEditView->GetEditSelection() );
    pSpellInfo = CreateSpellInfo( aCurSel, bMultipleDoc );
}
/*-- 13.10.2003 16:43:27---------------------------------------------------
    Search for the next wrong word within the given selection
  -----------------------------------------------------------------------*/
Reference< XSpellAlternatives > ImpEditEngine::ImpFindNextError(EditSelection& rSelection)
{
    /* ContentNode* pLastNode = */ aEditDoc.SaveGetObject( (aEditDoc.Count()-1) );
    EditSelection aCurSel( rSelection.Min() );

    String aWord;
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

        aCurSel = SelectWord( aCurSel, ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
        aWord = GetSelected( aCurSel );

        // Wenn Punkt dahinter, muss dieser mit uebergeben werden !
        // Falls Abkuerzung...
        if ( aWord.Len() && ( aCurSel.Max().GetIndex() < aCurSel.Max().GetNode()->Len() ) )
        {
            sal_Unicode cNext = aCurSel.Max().GetNode()->GetChar( aCurSel.Max().GetIndex() );
            if ( cNext == '.' )
            {
                aCurSel.Max().GetIndex()++;
                aWord += cNext;
            }
        }

        if ( aWord.Len() > 0 )
            xSpellAlt = xSpeller->spell( aWord, GetLanguage( aCurSel.Max() ), aEmptySeq );

        if ( !xSpellAlt.is() )
            aCurSel = WordRight( aCurSel.Min(), ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
        else
        {
            pSpellInfo->eState = EE_SPELL_ERRORFOUND;
            rSelection = aCurSel;
        }
    }
    return xSpellAlt;
}
/*-- 13.10.2003 16:43:27---------------------------------------------------

  -----------------------------------------------------------------------*/
bool ImpEditEngine::SpellSentence(EditView& rEditView,
    ::svx::SpellPortions& rToFill,
    bool /*bIsGrammarChecking*/ )
{
#ifdef SVX_LIGHT
#else
    bool bRet = false;
    EditSelection aCurSel( rEditView.pImpEditView->GetEditSelection() );
    if(!pSpellInfo)
        pSpellInfo = CreateSpellInfo( aCurSel, true );
    pSpellInfo->aCurSentenceStart = aCurSel.Min();
    DBG_ASSERT( xSpeller.is(), "Kein Speller gesetzt!" );
    pSpellInfo->aLastSpellPortions.clear();
    pSpellInfo->aLastSpellContentSelections.clear();
    rToFill.clear();
    //if no selection previously exists the range is extended to the end of the object
    if(aCurSel.Min() == aCurSel.Max())
    {
        ContentNode* pLastNode = aEditDoc.SaveGetObject( aEditDoc.Count()-1);
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
        //add the portion preceeding the error
        EditSelection aStartSelection(aSentencePaM.Min(), aCurSel.Min());
        if(aStartSelection.HasRange())
            AddPortionIterated(rEditView, aStartSelection, 0, rToFill);
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
                AddPortionIterated(rEditView, EditSelection(aCurSel.Max(), aNextSel.Min()), 0, rToFill);
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
#endif
    return bRet;
}

/*-- 15.10.2003 16:09:12---------------------------------------------------
    adds one portion to the SpellPortions
  -----------------------------------------------------------------------*/
void ImpEditEngine::AddPortion(
                            const EditSelection rSel,
                            uno::Reference< XSpellAlternatives > xAlt,
                                ::svx::SpellPortions& rToFill,
                                bool bIsField)
{
#ifdef SVX_LIGHT
#else
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
#endif
}

/*-- 15.10.2003 16:07:47---------------------------------------------------
    adds one or more portions of text to the SpellPortions depending on language changes
  -----------------------------------------------------------------------*/
void ImpEditEngine::AddPortionIterated(
                            EditView& rEditView,
                            const EditSelection rSel,
                            Reference< XSpellAlternatives > xAlt,
                                ::svx::SpellPortions& rToFill)
{
#ifdef SVX_LIGHT
#else
    if(rSel.Min() != rSel.Max())
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
            sal_uInt16 nEndField = bIsField ? pFieldAttr->GetEnd() : USHRT_MAX;
            bool bIsEndField = false;
            do
            {
                aCursor = CursorRight( aCursor);
                //determine whether a field and has been reached
                bIsEndField = nEndField == aCursor.GetIndex();
                //search for a new field attribute
                EditCharAttrib* _pFieldAttr = aCursor.GetNode()->GetCharAttribs().
                                                        FindFeature( aCursor.GetIndex() );
                bIsField = _pFieldAttr &&
                        _pFieldAttr->GetStart() == aCursor.GetIndex() &&
                        _pFieldAttr->GetStart() != _pFieldAttr->GetEnd() &&
                        _pFieldAttr->Which() == EE_FEATURE_FIELD;
                //on every new field move the end position
                if(bIsField)
                    nEndField = bIsField ? _pFieldAttr->GetEnd() : USHRT_MAX;

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
#endif
}

/*-- 13.10.2003 16:43:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void ImpEditEngine::ApplyChangedSentence(EditView& rEditView,
    const ::svx::SpellPortions& rNewPortions,
    bool bRecheck )
{
#ifdef SVX_LIGHT
#else
    // Note: rNewPortions.size() == 0 is valid and happens when the whole
    // sentence got removed in the dialog

    DBG_ASSERT(pSpellInfo, "pSpellInfo not initialized");
    if (pSpellInfo &&
        pSpellInfo->aLastSpellPortions.size() > 0)  // no portions -> no text to be changed
    {
        // get current paragraph length to calculate later on how the sentence length changed,
        // in order to place the cursor at the end of the sentence again
        EditSelection aOldSel( rEditView.pImpEditView->GetEditSelection() );
        xub_StrLen nOldLen = aOldSel.Max().GetNode()->Len();

        UndoActionStart( EDITUNDO_INSERT );
        if(pSpellInfo->aLastSpellPortions.size() == rNewPortions.size())
        {
            DBG_ASSERT( rNewPortions.size() > 0, "rNewPortions should not be empty here" );
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

                sal_uInt16 nScriptType = GetI18NScriptTypeOfLanguage( aCurrentNewPortion->eLanguage );
//                LanguageType eTextLanguage = GetLanguage( aCurrentOldPosition->Min() );

                sal_uInt16 nLangWhichId = EE_CHAR_LANGUAGE;
                switch(nScriptType)
                {
                    case SCRIPTTYPE_ASIAN : nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
                    case SCRIPTTYPE_COMPLEX : nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
                }
                if(aCurrentNewPortion->sText != aCurrentOldPortion->sText)
                {
                    //change text and apply language
                    SfxItemSet aSet( aEditDoc.GetItemPool(), nLangWhichId, nLangWhichId);
                    aSet.Put(SvxLanguageItem(aCurrentNewPortion->eLanguage, nLangWhichId));
                    SetAttribs( *aCurrentOldPosition, aSet );
                    ImpInsertText( *aCurrentOldPosition, aCurrentNewPortion->sText );
                }
                else if(aCurrentNewPortion->eLanguage != aCurrentOldPortion->eLanguage)
                {
                    //apply language
                    SfxItemSet aSet( aEditDoc.GetItemPool(), nLangWhichId, nLangWhichId);
                    aSet.Put(SvxLanguageItem(aCurrentNewPortion->eLanguage, nLangWhichId));
                    SetAttribs( *aCurrentOldPosition, aSet );
                }
                if(aCurrentNewPortion == rNewPortions.begin())
                    break;
            }
            while(aCurrentNewPortion != rNewPortions.begin());
        }
        else
        {
            DBG_ASSERT( pSpellInfo->aLastSpellContentSelections.size() > 0, "aLastSpellContentSelections should not be empty here" );

            //select the complete sentence
            SpellContentSelections::const_iterator aCurrentEndPosition = pSpellInfo->aLastSpellContentSelections.end();
            --aCurrentEndPosition;
            SpellContentSelections::const_iterator aCurrentStartPosition = pSpellInfo->aLastSpellContentSelections.begin();
            EditSelection aAllSentence(aCurrentStartPosition->Min(), aCurrentEndPosition->Max());

            //delete the sentence completely
            ImpDeleteSelection( aAllSentence );
            svx::SpellPortions::const_iterator aCurrentNewPortion = rNewPortions.begin();
            EditPaM aCurrentPaM = aAllSentence.Min();
            while(aCurrentNewPortion != rNewPortions.end())
            {
                //set the language attribute
                LanguageType eCurLanguage = GetLanguage( aCurrentPaM );
                if(eCurLanguage != aCurrentNewPortion->eLanguage)
                {
                    sal_uInt16 nScriptType = GetI18NScriptTypeOfLanguage( aCurrentNewPortion->eLanguage );
                    sal_uInt16 nLangWhichId = EE_CHAR_LANGUAGE;
                    switch(nScriptType)
                    {
                        case SCRIPTTYPE_ASIAN : nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
                        case SCRIPTTYPE_COMPLEX : nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
                    }
                    SfxItemSet aSet( aEditDoc.GetItemPool(), nLangWhichId, nLangWhichId);
                    aSet.Put(SvxLanguageItem(aCurrentNewPortion->eLanguage, nLangWhichId));
                    SetAttribs( aCurrentPaM, aSet );
                }
                //insert the new string and set the cursor to the end of the inserted string
                aCurrentPaM = ImpInsertText( aCurrentPaM , aCurrentNewPortion->sText );
                ++aCurrentNewPortion;
            }
        }
        UndoActionEnd( EDITUNDO_INSERT );

        EditPaM aNext;
        if (bRecheck)
            aNext = pSpellInfo->aCurSentenceStart;
        else
        {
            // restore cursor position to the end of the modified sentence.
            // (This will define the continuation position for spell/grammar checking)
            // First: check if the sentence/para length changed
            sal_Int32 nDelta = rEditView.pImpEditView->GetEditSelection().Max().GetNode()->Len() - nOldLen;
            xub_StrLen nEndOfSentence = aOldSel.Max().GetIndex() + nDelta;
            aNext = EditPaM( aOldSel.Max().GetNode(), nEndOfSentence );
        }
        rEditView.pImpEditView->SetEditSelection( aNext );

        FormatAndUpdate();
        aEditDoc.SetModified(sal_True);
    }
#endif
}
/*-- 08.09.2008 11:33:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void ImpEditEngine::PutSpellingToSentenceStart( EditView& rEditView )
{
#ifdef SVX_LIGHT
#else
    if( pSpellInfo && pSpellInfo->aLastSpellContentSelections.size() )
    {
        rEditView.pImpEditView->SetEditSelection( pSpellInfo->aLastSpellContentSelections.begin()->Min() );
    }

#endif
}


void ImpEditEngine::DoOnlineSpelling( ContentNode* pThisNodeOnly, sal_Bool bSpellAtCursorPos, sal_Bool bInteruptable )
{
#ifndef SVX_LIGHT
    /*
     Er wird ueber alle Absaetze iteriert, nur Absaetze mit invalidierter
     WrongList werden geprueft...

     Es werden alle Woerter im invalidierten Bereich geprueft.
     Ist ein Wort falsch, aber noch nicht in der WrongList, oder umgekehrt,
     wird der Bereich des Wortes invalidiert
      ( kein Invalidate, sondern wenn nur Uebergaenge von richtig=>falsch,
        einfaches Paint, bei Uebergaengen von falsch=>richtig mit VDev
        ueberplaetten )
    */

     if ( !xSpeller.is() )
        return;

    EditPaM aCursorPos;
    if( pActiveView && !bSpellAtCursorPos )
    {
        DBG_CHKOBJ( pActiveView, EditView, 0 );
        aCursorPos = pActiveView->pImpEditView->GetEditSelection().Max();
    }
    sal_Bool bRestartTimer = sal_False;

    ContentNode* pLastNode = aEditDoc.SaveGetObject( aEditDoc.Count() - 1 );
    sal_uInt16 nNodes = GetEditDoc().Count();
    sal_uInt16 nInvalids = 0;
    Sequence< PropertyValue > aEmptySeq;
    for ( sal_uInt16 n = 0; n < nNodes; n++ )
    {
        ContentNode* pNode = GetEditDoc().GetObject( n );
        if ( pThisNodeOnly )
            pNode = pThisNodeOnly;

        if ( pNode->GetWrongList()->IsInvalid() )
        {
            WrongList* pWrongList = pNode->GetWrongList();
            sal_uInt16 nInvStart = pWrongList->GetInvalidStart();
            sal_uInt16 nInvEnd = pWrongList->GetInvalidEnd();

            sal_uInt16 nWrongs = 0; // Auch im Absatz mal die Kontrolle abgeben...
//          sal_Bool bStop = sal_False;

            sal_uInt16 nPaintFrom = 0xFFFF, nPaintTo = 0;
            sal_Bool bSimpleRepaint = sal_True;

            pWrongList->SetValid();

            EditPaM aPaM( pNode, nInvStart );
            EditSelection aSel( aPaM, aPaM );
            while ( ( aSel.Max().GetNode() == pNode ) /* && !bStop */ )
            {
                if ( ( aSel.Min().GetIndex() > nInvEnd )
                        || ( ( aSel.Max().GetNode() == pLastNode ) && ( aSel.Max().GetIndex() >= pLastNode->Len() ) ) )
                    break;  // Dokument- oder Ungueltigkeitsbereich-Ende

                aSel = SelectWord( aSel, ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
                String aWord( GetSelected( aSel ) );
                // Wenn Punkt dahinter, muss dieser mit uebergeben werden !
                // Falls Abkuerzung...
                sal_Bool bDottAdded = sal_False;
                if ( aSel.Max().GetIndex() < aSel.Max().GetNode()->Len() )
                {
                    sal_Unicode cNext = aSel.Max().GetNode()->GetChar( aSel.Max().GetIndex() );
                    if ( cNext == '.' )
                    {
                        aSel.Max().GetIndex()++;
                        aWord += cNext;
                        bDottAdded = sal_True;
                    }
                }


                sal_Bool bChanged = sal_False;
                if ( aWord.Len() > 0 )
                {
                    sal_uInt16 nWStart = aSel.Min().GetIndex();
                    sal_uInt16 nWEnd= aSel.Max().GetIndex();
                    if ( !xSpeller->isValid( aWord, GetLanguage( EditPaM( aSel.Min().GetNode(), nWStart+1 ) ), aEmptySeq ) )
                    {
                        // Pruefen, ob schon richtig markiert...
                        nWrongs++;
                        // Nur bei SimpleRepaint stoppen, sonst zu oft VDev
    //                      if ( ( nWrongs > 8 ) && bSimpleRepaint )
    //                      {
    //                          bStop = sal_True;
    //                          pWrongList->MarkInvalid( aSel.Max().GetIndex(), nInvEnd );
    //                      }
                        sal_uInt16 nXEnd = bDottAdded ? nWEnd -1 : nWEnd;
                        if ( !pWrongList->HasWrong( nWStart, nXEnd ) )
                        {
                            // Wort als falsch markieren...
                            // Aber nur, wenn nicht an Cursor-Position...
                            sal_Bool bCursorPos = sal_False;
                            if ( aCursorPos.GetNode() == pNode )
                            {
                                if ( ( nWStart <= aCursorPos.GetIndex() ) && nWEnd >= aCursorPos.GetIndex() )
                                    bCursorPos = sal_True;
                            }
                            if ( bCursorPos )
                            {
                                // Dann weiter als ungueltig markieren...
                                pWrongList->GetInvalidStart() = nWStart;
                                pWrongList->GetInvalidEnd() = nWEnd;
                                bRestartTimer = sal_True;
                            }
                            else
                            {
                                // Es kann sein, dass die Wrongs in der Liste nicht
                                // genau ueber Woerter aufgespannt sind, weil die
                                // WordDelimiters beim Expandieren nicht ausgewrtet werden.
                                pWrongList->InsertWrong( nWStart, nXEnd, sal_True );
                                bChanged = sal_True;
                            }
                        }
                    }
                    else
                    {
                        // Pruefen, ob nicht als als falsch markiert....
                        if ( pWrongList->HasAnyWrong( nWStart, nWEnd ) )
                        {
                            pWrongList->ClearWrongs( nWStart, nWEnd, pNode );
                            bSimpleRepaint = sal_False;
                            bChanged = sal_True;
                        }
                    }
                    if ( bChanged  )
                    {
                        if ( nPaintFrom == 0xFFFF )
                            nPaintFrom = nWStart;
                        nPaintTo = nWEnd;
                    }
                }

                EditPaM aLastEnd( aSel.Max() );
                aSel = WordRight( aSel.Max(), ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
                if ( bChanged && ( aSel.Min().GetNode() == pNode ) &&
                        ( ( aSel.Min().GetIndex()-aLastEnd.GetIndex() > 1 ) ) )
                {
                    // Wenn zwei Worte durch mehr Zeichen als ein Blank getrennt
                    // sind, kann es passieren, dass beim Aufsplitten eines Wrongs
                    // der Start den zweiten Wortes vor dem tatsaechlich Wort liegt
                    pWrongList->ClearWrongs( aLastEnd.GetIndex(), aSel.Min().GetIndex(), pNode );
                }
            }

            // Invalidieren?
            if ( ( nPaintFrom != 0xFFFF ) )
            {
                aStatus.GetStatusWord() |= EE_STAT_WRONGWORDCHANGED;
                CallStatusHdl();

                if ( aEditViews.Count() )
                {
                    // Bei SimpleRepaint wuerde ein uebermalen ohne VDev reichen,
                    // aber dann muesste ich ueber alle Views, Intersecten,
                    // Clippen, ...
                    // Lohnt wahrscheinlich nicht.
                    EditPaM aStartPaM( pNode, nPaintFrom );
                    EditPaM aEndPaM( pNode, nPaintTo );
                    Rectangle aStartCursor( PaMtoEditCursor( aStartPaM ) );
                    Rectangle aEndCursor( PaMtoEditCursor( aEndPaM ) );
                    DBG_ASSERT( aInvalidRec.IsEmpty(), "InvalidRect gesetzt!" );
                    aInvalidRec.Left() = 0;
                    aInvalidRec.Right() = GetPaperSize().Width();
                    aInvalidRec.Top() = aStartCursor.Top();
                    aInvalidRec.Bottom() = aEndCursor.Bottom();
                    if ( pActiveView && pActiveView->HasSelection() )
                    {
                        // Dann darf nicht ueber VDev ausgegeben werden
                        UpdateViews( NULL );
                    }
                    else if ( bSimpleRepaint )
                    {
                        for ( sal_uInt16 nView = 0; nView < aEditViews.Count(); nView++ )
                        {
                            EditView* pView = aEditViews[nView];
                            Rectangle aClipRec( aInvalidRec );
                            aClipRec.Intersection( pView->GetVisArea() );
                            if ( !aClipRec.IsEmpty() )
                            {
                                // in Fensterkoordinaten umwandeln....
                                aClipRec.SetPos( pView->pImpEditView->GetWindowPos( aClipRec.TopLeft() ) );
                                // Wenn Selektion, dann VDev...
                                Paint( pView->pImpEditView, aClipRec, pView->HasSelection() );
                            }
                        }
                    }
                    else
                    {
                        UpdateViews( pActiveView );
                    }
                    aInvalidRec = Rectangle();
                }
            }
            // Nach zwei korrigierten Nodes die Kontrolle abgeben...
            nInvalids++;
            if ( bInteruptable && ( nInvalids >= 2 ) )
            {
                bRestartTimer = sal_True;
                break;
            }
        }

        if ( pThisNodeOnly )
            break;
    }
    if ( bRestartTimer )
        aOnlineSpellTimer.Start();
#endif // !SVX_LIGHT
}


EESpellState ImpEditEngine::HasSpellErrors()
{
    DBG_ASSERT( xSpeller.is(), "Kein Speller gesetzt!" );

#ifndef SVX_LIGHT
    ContentNode* pLastNode = aEditDoc.SaveGetObject( aEditDoc.Count() - 1 );
    EditSelection aCurSel( aEditDoc.GetStartPaM() );

    String aWord;
    Reference< XSpellAlternatives > xSpellAlt;
    Sequence< PropertyValue > aEmptySeq;
    while ( !xSpellAlt.is() )
    {
        if ( ( aCurSel.Max().GetNode() == pLastNode ) &&
             ( aCurSel.Max().GetIndex() >= pLastNode->Len() ) )
        {
            return EE_SPELL_OK;
        }

        aCurSel = SelectWord( aCurSel, ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
        aWord = GetSelected( aCurSel );
        if ( aWord.Len() > 0 )
        {
            LanguageType eLang = GetLanguage( aCurSel.Max() );
            SvxSpellWrapper::CheckSpellLang( xSpeller, eLang );
            xSpellAlt = xSpeller->spell( aWord, eLang, aEmptySeq );
        }
        aCurSel = WordRight( aCurSel.Max(), ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
    }
#endif

    return EE_SPELL_ERRORFOUND;
}

EESpellState ImpEditEngine::StartThesaurus( EditView* pEditView )
{
#ifndef SVX_LIGHT
    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );
    if ( !aCurSel.HasRange() )
        aCurSel = SelectWord( aCurSel, ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
    String aWord( GetSelected( aCurSel ) );

    Reference< XThesaurus > xThes( SvxGetThesaurus() );
    if (!xThes.is())
        return EE_SPELL_ERRORFOUND;

    EditAbstractDialogFactory* pFact = EditAbstractDialogFactory::Create();
    AbstractThesaurusDialog* pDlg = pFact->CreateThesaurusDialog( pEditView->GetWindow(), xThes, aWord, GetLanguage( aCurSel.Max() ) );
    if ( pDlg->Execute() == RET_OK )
    {
        // Wort ersetzen...
        pEditView->pImpEditView->DrawSelection();
        pEditView->pImpEditView->SetEditSelection( aCurSel );
        pEditView->pImpEditView->DrawSelection();
        pEditView->InsertText( pDlg->GetWord() );
        pEditView->ShowCursor( sal_True, sal_False );
    }

    delete pDlg;
    return EE_SPELL_OK;
#else
    return EE_SPELL_NOSPELLER;
#endif
}

sal_uInt16 ImpEditEngine::StartSearchAndReplace( EditView* pEditView, const SvxSearchItem& rSearchItem )
{
    sal_uInt16 nFound = 0;

#ifndef SVX_LIGHT
    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );

    // FIND_ALL ohne Mehrfachselektion nicht moeglich.
    if ( ( rSearchItem.GetCommand() == SVX_SEARCHCMD_FIND ) ||
         ( rSearchItem.GetCommand() == SVX_SEARCHCMD_FIND_ALL ) )
    {
        if ( Search( rSearchItem, pEditView ) )
            nFound++;
    }
    else if ( rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE )
    {
        // Das Wort ist selektiert, wenn der Anwender die Selektion
        // nicht zwischendurch manipuliert:
        if ( aCurSel.HasRange() )
        {
            pEditView->InsertText( rSearchItem.GetReplaceString() );
            nFound = 1;
        }
        else
            if( Search( rSearchItem, pEditView ) )
                nFound = 1;
    }
    else if ( rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE_ALL )
    {
        // Der Writer ersetzt alle, vorn Anfang bis Ende...
        SvxSearchItem aTmpItem( rSearchItem );
        aTmpItem.SetBackward( sal_False );

        pEditView->pImpEditView->DrawSelection();

        aCurSel.Adjust( aEditDoc );
        EditPaM aStartPaM = aTmpItem.GetSelection() ? aCurSel.Min() : aEditDoc.GetStartPaM();
        EditSelection aFoundSel( aCurSel.Max() );
        sal_Bool bFound = ImpSearch( aTmpItem, aCurSel, aStartPaM, aFoundSel );
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
                aNewPaM.GetIndex() =  aNewPaM.GetNode()->Len();
            pEditView->pImpEditView->SetEditSelection( aNewPaM );
            FormatAndUpdate( pEditView );
            UndoActionEnd( EDITUNDO_REPLACEALL );
        }
        else
        {
            pEditView->pImpEditView->DrawSelection();
            pEditView->ShowCursor( sal_True, sal_False );
        }
    }
#endif // !SVX_LIGHT
    return nFound;
}

sal_Bool ImpEditEngine::Search( const SvxSearchItem& rSearchItem, EditView* pEditView )
{
    EditSelection aSel( pEditView->pImpEditView->GetEditSelection() );
    aSel.Adjust( aEditDoc );
    EditPaM aStartPaM( aSel.Max() );
    if ( rSearchItem.GetSelection() && !rSearchItem.GetBackward() )
        aStartPaM = aSel.Min();

    EditSelection aFoundSel;
    sal_Bool bFound = ImpSearch( rSearchItem, aSel, aStartPaM, aFoundSel );
    if ( bFound && ( aFoundSel == aSel ) )  // Bei Rueckwaetssuche
    {
        aStartPaM = aSel.Min();
        bFound = ImpSearch( rSearchItem, aSel, aStartPaM, aFoundSel );
    }

    pEditView->pImpEditView->DrawSelection();
    if ( bFound )
    {
        // Erstmal das Min einstellen, damit das ganze Wort in den sichtbaren Bereich kommt.
        pEditView->pImpEditView->SetEditSelection( aFoundSel.Min() );
        pEditView->ShowCursor( sal_True, sal_False );
        pEditView->pImpEditView->SetEditSelection( aFoundSel );
    }
    else
        pEditView->pImpEditView->SetEditSelection( aSel.Max() );

    pEditView->pImpEditView->DrawSelection();
    pEditView->ShowCursor( sal_True, sal_False );
    return bFound;
}

sal_Bool ImpEditEngine::ImpSearch( const SvxSearchItem& rSearchItem,
    const EditSelection& rSearchSelection, const EditPaM& rStartPos, EditSelection& rFoundSel )
{
#ifndef SVX_LIGHT
    util::SearchOptions aSearchOptions( rSearchItem.GetSearchOptions() );
    aSearchOptions.Locale = GetLocale( rStartPos );

    sal_Bool bBack = rSearchItem.GetBackward();
    sal_Bool bSearchInSelection = rSearchItem.GetSelection();
    sal_uInt16 nStartNode = aEditDoc.GetPos( rStartPos.GetNode() );
    sal_uInt16 nEndNode;
    if ( bSearchInSelection )
    {
        nEndNode = aEditDoc.GetPos( bBack ? rSearchSelection.Min().GetNode() : rSearchSelection.Max().GetNode() );
    }
    else
    {
        nEndNode = bBack ? 0 : aEditDoc.Count()-1;
    }

    utl::TextSearch aSearcher( aSearchOptions );

    // ueber die Absaetze iterieren...
    for ( sal_uInt16 nNode = nStartNode;
            bBack ? ( nNode >= nEndNode ) : ( nNode <= nEndNode) ;
            bBack ? nNode-- : nNode++ )
    {
        // Bei rueckwaertsuche, wenn nEndNode = 0:
        if ( nNode >= 0xFFFF )
            return sal_False;

        ContentNode* pNode = aEditDoc.GetObject( nNode );

        sal_uInt16 nStartPos = 0;
        sal_uInt16 nEndPos = pNode->Len();
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

        // Suchen...
        XubString aParaStr( GetEditDoc().GetParaAsString( pNode ) );
        bool bFound = false;
        if ( bBack )
        {
            Swapsal_uIt16s( nStartPos, nEndPos );
            bFound = aSearcher.SearchBkwrd( aParaStr, &nStartPos, &nEndPos);
        }
        else
            bFound = aSearcher.SearchFrwrd( aParaStr, &nStartPos, &nEndPos);

        if ( bFound )
        {
            rFoundSel.Min().SetNode( pNode );
            rFoundSel.Min().SetIndex( nStartPos );
            rFoundSel.Max().SetNode( pNode );
            rFoundSel.Max().SetIndex( nEndPos );
            return sal_True;
        }
    }
#endif // !SVX_LIGHT
    return sal_False;
}

sal_Bool ImpEditEngine::HasText( const SvxSearchItem& rSearchItem )
{
#ifndef SVX_LIGHT
    SvxSearchItem aTmpItem( rSearchItem );
    aTmpItem.SetBackward( sal_False );
    aTmpItem.SetSelection( sal_False );

    EditPaM aStartPaM( aEditDoc.GetStartPaM() );
    EditSelection aDummySel( aStartPaM );
    EditSelection aFoundSel;
    return ImpSearch( aTmpItem, aDummySel, aStartPaM, aFoundSel );
#else
    return sal_False;
#endif
}

void ImpEditEngine::SetAutoCompleteText( const String& rStr, sal_Bool bClearTipWindow )
{
#ifndef SVX_LIGHT
    aAutoCompleteText = rStr;
    if ( bClearTipWindow && pActiveView )
        Help::ShowQuickHelp( pActiveView->GetWindow(), Rectangle(), String(), 0 );
#endif // !SVX_LIGHT
}


struct TransliterationChgData
{
    sal_uInt16                      nStart;
    xub_StrLen                  nLen;
    EditSelection               aSelection;
    String                      aNewText;
    uno::Sequence< sal_Int32 >  aOffsets;
};


EditSelection ImpEditEngine::TransliterateText( const EditSelection& rSelection, sal_Int32 nTransliterationMode )
{
    uno::Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
    if (!_xBI.is())
        return rSelection;

    EditSelection aSel( rSelection );
    aSel.Adjust( aEditDoc );

    if ( !aSel.HasRange() )
        aSel = SelectWord( aSel );

    EditSelection aNewSel( aSel );

    const sal_uInt16 nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    const sal_uInt16 nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    sal_Bool bChanges = sal_False;
    sal_Bool bLenChanged = sal_False;
    EditUndoTransliteration* pUndo = NULL;

    utl::TransliterationWrapper aTranslitarationWrapper( ::comphelper::getProcessServiceFactory(), nTransliterationMode );
    sal_Bool bConsiderLanguage = aTranslitarationWrapper.needLanguageForTheMode();

    for ( sal_uInt16 nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        xub_StrLen nStartPos = 0;
        xub_StrLen nEndPos = pNode->Len();
        if ( nNode == nStartNode )
            nStartPos = aSel.Min().GetIndex();
        if ( nNode == nEndNode ) // kann auch == nStart sein!
            nEndPos = aSel.Max().GetIndex();

        sal_uInt16 nCurrentStart = nStartPos;
        sal_uInt16 nCurrentEnd = nEndPos;
        sal_uInt16 nLanguage = LANGUAGE_SYSTEM;

        // since we don't use Hiragana/Katakana or half-width/full-width transliterations here
        // it is fine to use ANYWORD_IGNOREWHITESPACES. (ANY_WORD btw is broken and will
        // occasionaly miss words in consecutive sentences). Also with ANYWORD_IGNOREWHITESPACES
        // text like 'just-in-time' will be converted to 'Just-In-Time' which seems to be the
        // proper thing to do.
        const sal_Int16 nWordType = i18n::WordType::ANYWORD_IGNOREWHITESPACES;

        //! In order to have less trouble with changing text size, e.g. because
        //! of ligatures or � (German small sz) being resolved, we need to process
        //! the text replacements from end to start.
        //! This way the offsets for the yet to be changed words will be
        //! left unchanged by the already replaced text.
        //! For this we temporarily save the changes to be done in this vector
        std::vector< TransliterationChgData >   aChanges;
        TransliterationChgData                  aChgData;

        if (nTransliterationMode == i18n::TransliterationModulesExtra::TITLE_CASE)
        {
            // for 'capitalize every word' we need to iterate over each word

            i18n::Boundary aSttBndry;
            i18n::Boundary aEndBndry;
            aSttBndry = _xBI->getWordBoundary(
                        *pNode, nStartPos,
                        SvxCreateLocale( GetLanguage( EditPaM( pNode, nStartPos + 1 ) ) ),
                        nWordType, sal_True /*prefer forward direction*/);
            aEndBndry = _xBI->getWordBoundary(
                        *pNode, nEndPos,
                        SvxCreateLocale( GetLanguage( EditPaM( pNode, nEndPos + 1 ) ) ),
                        nWordType, sal_False /*prefer backward direction*/);

            // prevent backtracking to the previous word if selection is at word boundary
            if (aSttBndry.endPos <= nStartPos)
            {
                aSttBndry = _xBI->nextWord(
                        *pNode, aSttBndry.endPos,
                        SvxCreateLocale( GetLanguage( EditPaM( pNode, aSttBndry.endPos + 1 ) ) ),
                        nWordType);
            }
            // prevent advancing to the next word if selection is at word boundary
            if (aEndBndry.startPos >= nEndPos)
            {
                aEndBndry = _xBI->previousWord(
                        *pNode, aEndBndry.startPos,
                        SvxCreateLocale( GetLanguage( EditPaM( pNode, aEndBndry.startPos + 1 ) ) ),
                        nWordType);
            }

            i18n::Boundary aCurWordBndry( aSttBndry );
            while (aCurWordBndry.startPos <= aEndBndry.startPos)
            {
                nCurrentStart = (xub_StrLen)aCurWordBndry.startPos;
                nCurrentEnd   = (xub_StrLen)aCurWordBndry.endPos;
                sal_Int32 nLen = nCurrentEnd - nCurrentStart;
                DBG_ASSERT( nLen > 0, "invalid word length of 0" );
#if OSL_DEBUG_LEVEL > 1
                String aText( pNode->Copy( nCurrentStart, nLen ) );
#endif

                Sequence< sal_Int32 > aOffsets;
                String aNewText( aTranslitarationWrapper.transliterate( *pNode,
                        GetLanguage( EditPaM( pNode, nCurrentStart + 1 ) ),
                        nCurrentStart, nLen, &aOffsets ));

                if (!pNode->Equals( aNewText, nCurrentStart, nLen ))
                {
                    aChgData.nStart     = nCurrentStart;
                    aChgData.nLen       = nLen;
                    aChgData.aSelection = EditSelection( EditPaM( pNode, nCurrentStart ), EditPaM( pNode, nCurrentEnd ) );
                    aChgData.aNewText   = aNewText;
                    aChgData.aOffsets   = aOffsets;
                    aChanges.push_back( aChgData );
                }
#if OSL_DEBUG_LEVEL > 1
                String aSelTxt ( GetSelected( aChgData.aSelection ) );
                (void) aSelTxt;
#endif

                aCurWordBndry = _xBI->nextWord( *pNode, nCurrentEnd,
                        SvxCreateLocale( GetLanguage( EditPaM( pNode, nCurrentEnd + 1 ) ) ),
                        nWordType);
            }
            DBG_ASSERT( nCurrentEnd >= aEndBndry.endPos, "failed to reach end of transliteration" );
        }
        else if (nTransliterationMode == i18n::TransliterationModulesExtra::SENTENCE_CASE)
        {
            // for 'sentence case' we need to iterate sentence by sentence

            sal_Int32 nLastStart = _xBI->beginOfSentence(
                    *pNode, nEndPos,
                    SvxCreateLocale( GetLanguage( EditPaM( pNode, nEndPos + 1 ) ) ) );
            sal_Int32 nLastEnd = _xBI->endOfSentence(
                    *pNode, nLastStart,
                    SvxCreateLocale( GetLanguage( EditPaM( pNode, nLastStart + 1 ) ) ) );

            // extend nCurrentStart, nCurrentEnd to the current sentence boundaries
            nCurrentStart = _xBI->beginOfSentence(
                    *pNode, nStartPos,
                    SvxCreateLocale( GetLanguage( EditPaM( pNode, nStartPos + 1 ) ) ) );
            nCurrentEnd = _xBI->endOfSentence(
                    *pNode, nCurrentStart,
                    SvxCreateLocale( GetLanguage( EditPaM( pNode, nCurrentStart + 1 ) ) ) );

            // prevent backtracking to the previous sentence if selection starts at end of a sentence
            if (nCurrentEnd <= nStartPos)
            {
                // now nCurrentStart is probably located on a non-letter word. (unless we
                // are in Asian text with no spaces...)
                // Thus to get the real sentence start we should locate the next real word,
                // that is one found by DICTIONARY_WORD
                i18n::Boundary aBndry = _xBI->nextWord( *pNode, nCurrentEnd,
                        SvxCreateLocale( GetLanguage( EditPaM( pNode, nCurrentEnd + 1 ) ) ),
                        i18n::WordType::DICTIONARY_WORD);

                // now get new current sentence boundaries
                nCurrentStart = _xBI->beginOfSentence(
                        *pNode, aBndry.startPos,
                        SvxCreateLocale( GetLanguage( EditPaM( pNode, aBndry.startPos + 1 ) ) ) );
                nCurrentEnd = _xBI->endOfSentence(
                        *pNode, nCurrentStart,
                        SvxCreateLocale( GetLanguage( EditPaM( pNode, nCurrentStart + 1 ) ) ) );
            }
            // prevent advancing to the next sentence if selection ends at start of a sentence
            if (nLastStart >= nEndPos)
            {
                // now nCurrentStart is probably located on a non-letter word. (unless we
                // are in Asian text with no spaces...)
                // Thus to get the real sentence start we should locate the previous real word,
                // that is one found by DICTIONARY_WORD
                i18n::Boundary aBndry = _xBI->previousWord( *pNode, nLastStart,
                        SvxCreateLocale( GetLanguage( EditPaM( pNode, nLastStart + 1 ) ) ),
                        i18n::WordType::DICTIONARY_WORD);
                nLastEnd = _xBI->endOfSentence(
                        *pNode, aBndry.startPos,
                        SvxCreateLocale( GetLanguage( EditPaM( pNode, aBndry.startPos + 1 ) ) ) );
                if (nCurrentEnd > nLastEnd)
                    nCurrentEnd = nLastEnd;
            }

            while (nCurrentStart < nLastEnd)
            {
                sal_Int32 nLen = nCurrentEnd - nCurrentStart;
                DBG_ASSERT( nLen > 0, "invalid word length of 0" );
#if OSL_DEBUG_LEVEL > 1
                String aText( pNode->Copy( nCurrentStart, nLen ) );
#endif

                Sequence< sal_Int32 > aOffsets;
                String aNewText( aTranslitarationWrapper.transliterate( *pNode,
                        GetLanguage( EditPaM( pNode, nCurrentStart + 1 ) ),
                        nCurrentStart, nLen, &aOffsets ));

                if (!pNode->Equals( aNewText, nCurrentStart, nLen ))
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
                        *pNode, nCurrentEnd,
                        SvxCreateLocale( GetLanguage( EditPaM( pNode, nCurrentEnd + 1 ) ) ),
                        nWordType);
                nCurrentStart = aFirstWordBndry.startPos;
                nCurrentEnd = _xBI->endOfSentence(
                        *pNode, nCurrentStart,
                        SvxCreateLocale( GetLanguage( EditPaM( pNode, nCurrentStart + 1 ) ) ) );
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

                xub_StrLen nLen = nCurrentEnd - nCurrentStart;

                Sequence< sal_Int32 > aOffsets;
                String aNewText( aTranslitarationWrapper.transliterate( *pNode, nLanguage, nCurrentStart, nLen, &aOffsets ) );

                if (!pNode->Equals( aNewText, nCurrentStart, nLen ))
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

        if (aChanges.size() > 0)
        {
#ifndef SVX_LIGHT
            // Create a single UndoAction on Demand for all the changes ...
            if ( !pUndo && IsUndoEnabled() && !IsInUndo() )
            {
                // adjust selection to include all changes
                for (size_t i = 0; i < aChanges.size(); ++i)
                {
                    const EditSelection &rSel = aChanges[i].aSelection;
                    if (aSel.Min().GetNode() == rSel.Min().GetNode() &&
                        aSel.Min().GetIndex() > rSel.Min().GetIndex())
                        aSel.Min().SetIndex( rSel.Min().GetIndex() );
                    if (aSel.Max().GetNode() == rSel.Max().GetNode() &&
                        aSel.Max().GetIndex() < rSel.Max().GetIndex())
                        aSel.Max().SetIndex( rSel.Max().GetIndex() );
                }
                aNewSel = aSel;

                ESelection aESel( CreateESel( aSel ) );
                pUndo = new EditUndoTransliteration( this, aESel, nTransliterationMode );

                const bool bSingleNode = aSel.Min().GetNode()== aSel.Max().GetNode();
                const bool bHasAttribs = aSel.Min().GetNode()->GetCharAttribs().HasAttrib( aSel.Min().GetIndex(), aSel.Max().GetIndex() );
                if (bSingleNode && !bHasAttribs)
                    pUndo->SetText( aSel.Min().GetNode()->Copy( aSel.Min().GetIndex(), aSel.Max().GetIndex()-aSel.Min().GetIndex() ) );
                else
                    pUndo->SetText( CreateBinTextObject( aSel, NULL ) );
            }
#endif

            // now apply the changes from end to start to leave the offsets of the
            // yet unchanged text parts remain the same.
            for (size_t i = 0; i < aChanges.size(); ++i)
            {
                const TransliterationChgData &rData = aChanges[ aChanges.size() - 1 - i ];

                bChanges = sal_True;
                if (rData.nLen != rData.aNewText.Len())
                    bLenChanged = sal_True;

                // Change text without loosing the attributes
                sal_uInt16 nDiffs = ReplaceTextOnly( rData.aSelection.Min().GetNode(),
                        rData.nStart, rData.nLen, rData.aNewText, rData.aOffsets );

                // adjust selection in end node to possibly changed size
                if (aSel.Max().GetNode() == rData.aSelection.Max().GetNode())
                    aNewSel.Max().GetIndex() = aNewSel.Max().GetIndex() + nDiffs;

                sal_uInt16 nSelNode = aEditDoc.GetPos( rData.aSelection.Min().GetNode() );
                ParaPortion* pParaPortion = GetParaPortions()[nSelNode];
                pParaPortion->MarkSelectionInvalid( rData.nStart,
                        std::max< sal_uInt16 >( rData.nStart + rData.nLen,
                                            rData.nStart + rData.aNewText.Len() ) );
            }
        } // if (aChanges.size() > 0)
    }

#ifndef SVX_LIGHT
    if ( pUndo )
    {
        ESelection aESel( CreateESel( aNewSel ) );
        pUndo->SetNewSelection( aESel );
        InsertUndo( pUndo );
    }
#endif

    if ( bChanges )
    {
        TextModified();
        SetModifyFlag( sal_True );
        if ( bLenChanged )
            UpdateSelections();
        FormatAndUpdate();
    }

    return aNewSel;
}


short ImpEditEngine::ReplaceTextOnly(
    ContentNode* pNode,
    sal_uInt16 nCurrentStart, xub_StrLen nLen,
    const String& rNewText,
    const uno::Sequence< sal_Int32 >& rOffsets )
{
    (void)  nLen;

    // Change text without loosing the attributes
    sal_uInt16 nCharsAfterTransliteration =
        sal::static_int_cast< sal_uInt16 >(rOffsets.getLength());
    const sal_Int32* pOffsets = rOffsets.getConstArray();
    short nDiffs = 0;
    for ( sal_uInt16 n = 0; n < nCharsAfterTransliteration; n++ )
    {
        sal_uInt16 nCurrentPos = nCurrentStart+n;
        sal_Int32 nDiff = (nCurrentPos-nDiffs) - pOffsets[n];

        if ( !nDiff )
        {
            DBG_ASSERT( nCurrentPos < pNode->Len(), "TransliterateText - String smaller than expected!" );
            pNode->SetChar( nCurrentPos, rNewText.GetChar(n) );
        }
        else if ( nDiff < 0 )
        {
            // Replace first char, delete the rest...
            DBG_ASSERT( nCurrentPos < pNode->Len(), "TransliterateText - String smaller than expected!" );
            pNode->SetChar( nCurrentPos, rNewText.GetChar(n) );

            DBG_ASSERT( (nCurrentPos+1) < pNode->Len(), "TransliterateText - String smaller than expected!" );
            GetEditDoc().RemoveChars( EditPaM( pNode, nCurrentPos+1 ), sal::static_int_cast< sal_uInt16 >(-nDiff) );
        }
        else
        {
            DBG_ASSERT( nDiff == 1, "TransliterateText - Diff other than expected! But should work..." );
            GetEditDoc().InsertText( EditPaM( pNode, nCurrentPos ), rNewText.GetChar(n) );

        }
        nDiffs = sal::static_int_cast< short >(nDiffs + nDiff);
    }

    return nDiffs;
}


void ImpEditEngine::SetAsianCompressionMode( sal_uInt16 n )
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

void ImpEditEngine::SetKernAsianPunctuation( sal_Bool b )
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

void ImpEditEngine::SetAddExtLeading( sal_Bool bExtLeading )
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



sal_Bool ImpEditEngine::ImplHasText() const
{
    return ( ( GetEditDoc().Count() > 1 ) || GetEditDoc().GetObject(0)->Len() );
}

long ImpEditEngine::LogicToTwips( long n )
{
    Size aSz( n, 0 );
    MapMode aTwipsMode( MAP_TWIP );
    aSz = pRefDev->LogicToLogic( aSz, NULL, &aTwipsMode );
    return aSz.Width();
}


