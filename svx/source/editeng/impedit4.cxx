/*************************************************************************
 *
 *  $RCSfile: impedit4.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-07 17:49:39 $
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

#include <eeng_pch.hxx>

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <vcl/svapp.hxx>
#pragma hdrstop

#include <vcl/system.hxx>
#include <srchitem.hxx>
#include <lspcitem.hxx>
#include <adjitem.hxx>
#include <tstpitem.hxx>

#include <eertfpar.hxx>
#include <impedit.hxx>
#include <editview.hxx>
#include <eehtml.hxx>
#include <editobj2.hxx>
#include <tools/isolang.hxx>


#include "akrnitem.hxx"
#include "cntritem.hxx"
#include "colritem.hxx"
#include "crsditem.hxx"
#include "escpitem.hxx"
#include "fhgtitem.hxx"
#include "fontitem.hxx"
#include "kernitem.hxx"
#include "lrspitem.hxx"
#include "postitem.hxx"
#include "shdditem.hxx"
#include "udlnitem.hxx"
#include "ulspitem.hxx"
#include "wghtitem.hxx"
#include "langitem.hxx"

#include <rtl/tencinfo.h>

#include <svtools/rtfout.hxx>

#ifndef SVX_LIGHT
#include <edtspell.hxx>
#endif

#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XTHESAURUS_HPP_
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XMEANING_HPP_
#include <com/sun/star/linguistic2/XMeaning.hpp>
#endif

#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif

#ifndef _TXTCMP_HXX //autogen
#include <unotools/textsearch.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif

#include <svtools/rtfkeywd.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

void SwapUSHORTs( sal_uInt16& rX, sal_uInt16& rY )
{
    sal_uInt16 n = rX;
    rX = rY;
    rY = n;
}

EditPaM ImpEditEngine::Read( SvStream& rInput, EETextFormat eFormat, EditSelection aSel, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    sal_Bool bUpdate = GetUpdateMode();
    SetUpdateMode( sal_False );
    EditPaM aPaM;
    if ( eFormat == EE_FORMAT_TEXT )
        aPaM = ReadText( rInput, aSel );
    else if ( eFormat == EE_FORMAT_RTF )
        aPaM = ReadRTF( rInput, aSel );
    else if ( eFormat == EE_FORMAT_HTML )
        aPaM = ReadHTML( rInput, aSel, pHTTPHeaderAttrs );
    else if ( eFormat == EE_FORMAT_BIN)
        aPaM = ReadBin( rInput, aSel );
    else
        DBG_ERROR( "Read: Unbekanntes Format" );

    FormatFullDoc();        // reicht vielleicht auch ein einfaches Format?
    SetUpdateMode( bUpdate );

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

EditPaM ImpEditEngine::ReadRTF( SvStream& rInput, EditSelection aSel )
{
#ifndef SVX_LIGHT

#if defined (EDITDEBUG) && !defined(MAC) && !defined( UNX )
    SvFileStream aRTFOut( "d:\\rtfout.rtf", STREAM_WRITE );
    aRTFOut << rInput;
    aRTFOut.Close();
    rInput.Seek( 0 );
#endif
    if ( aSel.HasRange() )
        aSel = ImpDeleteSelection( aSel );

    sal_Bool bCharsBeforeInsertPos = ( aSel.Min().GetIndex() ) ? sal_True : sal_False;
    sal_Bool bCharsBehindInsertPos = ( aSel.Min().GetIndex() < aSel.Min().GetNode()->Len() ) ? sal_True : sal_False;

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

EditPaM ImpEditEngine::ReadHTML( SvStream& rInput, EditSelection aSel, SvKeyValueIterator* pHTTPHeaderAttrs )
{
#ifndef SVX_LIGHT

    if ( aSel.HasRange() )
        aSel = ImpDeleteSelection( aSel );

    sal_Bool bCharsBeforeInsertPos = ( aSel.Min().GetIndex() ) ? sal_True : sal_False;
    sal_Bool bCharsBehindInsertPos = ( aSel.Min().GetIndex() < aSel.Min().GetNode()->Len() ) ? sal_True : sal_False;

    EditHTMLParserRef xPrsr = new EditHTMLParser( rInput, pHTTPHeaderAttrs );
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
        else if ( eFormat == EE_FORMAT_HTML )
            WriteHTML( rOutput, aSel );
        else if ( eFormat == EE_FORMAT_BIN)
            WriteBin( rOutput, aSel );
        else
            DBG_ERROR( "Write: Unbekanntes Format" );
    }
}

sal_uInt32 ImpEditEngine::WriteText( SvStream& rOutput, EditSelection aSel )
{
    sal_uInt16 nStartNode, nEndNode;
    EditSelection   aTmpSel;

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

void lcl_FindValidAttribs( ItemList& rLst, ContentNode* pNode, sal_uInt16 nIndex, sal_Bool bStartAtPos )
{
    sal_uInt16 nAttr = 0;
    EditCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    while ( pAttr && ( pAttr->GetStart() <= nIndex ) )
    {
        // Start wird in While ueberprueft...
        if ( pAttr->GetEnd() > nIndex )
        {
            if ( !bStartAtPos || ( pAttr->GetStart() == nIndex) )
                rLst.Insert( pAttr->GetItem(), LIST_APPEND );
        }
        nAttr++;
        pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    }
}

sal_uInt32 ImpEditEngine::WriteBin( SvStream& rOutput, EditSelection aSel, BOOL bStoreUnicodeStrings ) const
{
    BinTextObject* pObj = (BinTextObject*)CreateBinTextObject( aSel, NULL );
    pObj->StoreUnicodeStrings( bStoreUnicodeStrings );
    pObj->Store( rOutput );
    delete pObj;
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
    EditSelection   aTmpSel;

    aSel.Adjust( aEditDoc );

    nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    // RTF-Vorspann...
    rOutput << '{' ;

    rOutput << sRTF_RTF;

#ifndef MAC
    rOutput << sRTF_ANSI;
    rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252;
#else
    rOutput << sRTF_MAC;
    rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_APPLE_ROMAN;
#endif

    // Fonttabelle erzeugen und rausschreiben...
    SvxFontTable aFontTable;
    // DefaultFont muss ganz vorne stehen, damit DEF-Font im RTF
    aFontTable.Insert( 0, new SvxFontItem( (const SvxFontItem&)aEditDoc.GetItemPool().GetDefaultItem( EE_CHAR_FONTINFO ) ) );
    sal_uInt16 i = 0;
    SvxFontItem* pFontItem = (SvxFontItem*)aEditDoc.GetItemPool().GetItem( EE_CHAR_FONTINFO, i );
    while ( pFontItem )
    {
        // i+1, da Default-Font an Stelle 0.
        aFontTable.Insert( (sal_uInt32)i+1, new SvxFontItem( *pFontItem ) );
        pFontItem = (SvxFontItem*)aEditDoc.GetItemPool().GetItem( EE_CHAR_FONTINFO, ++i );
    }

    rOutput << endl << '{' << sRTF_FONTTBL;
    sal_uInt16 j;
    for ( j = 0; j < aFontTable.Count(); j++ )
    {
        pFontItem = aFontTable.Get( j );
        rOutput << '{';
        rOutput << sRTF_F;
        rOutput.WriteNumber( j );
        switch ( pFontItem->GetFamily()  )
        {
            case FAMILY_DONTKNOW:       rOutput << sRTF_FNIL;
                                        break;
            case FAMILY_DECORATIVE:     rOutput << sRTF_FDECOR;
                                        break;
            case FAMILY_MODERN:         rOutput << sRTF_FMODERN;
                                        break;
            case FAMILY_ROMAN:          rOutput << sRTF_FROMAN;
                                        break;
            case FAMILY_SCRIPT:         rOutput << sRTF_FSCRIPT;
                                        break;
            case FAMILY_SWISS:          rOutput << sRTF_FSWISS;
                                        break;
        }
        rOutput << sRTF_FPRQ;
        sal_uInt16 nVal = 0;
        switch( pFontItem->GetPitch() )
        {
            case PITCH_FIXED:       nVal = 1;       break;
            case PITCH_VARIABLE:    nVal = 2;       break;
        }
        rOutput.WriteNumber( nVal );

        CharSet eChrSet = pFontItem->GetCharSet();
        DBG_ASSERT( eChrSet != 9, "SystemCharSet?!" );
        if( RTL_TEXTENCODING_DONTKNOW == eChrSet )
            eChrSet = gsl_getSystemTextEncoding();
        rOutput << sRTF_FCHARSET;
        rOutput.WriteNumber( rtl_getBestWindowsCharsetFromTextEncoding( eChrSet ) );

        rOutput << ' ';
        rOutput << ByteString( pFontItem->GetFamilyName(), eChrSet ).GetBuffer();
        rOutput << ";}";
    }
    rOutput << '}';
    rOutput << endl;

    // ColorList rausschreiben...
    SvxColorList aColorList;
    i = 0;
    SvxColorItem* pColorItem = (SvxColorItem*)aEditDoc.GetItemPool().GetItem( EE_CHAR_COLOR, i );
    while ( pColorItem )
    {
        aColorList.Insert( new SvxColorItem( *pColorItem ), (sal_uInt32)i );
        pColorItem = (SvxColorItem*)aEditDoc.GetItemPool().GetItem( EE_CHAR_COLOR, ++i );
    }
    aColorList.Insert( new SvxColorItem( (const SvxColorItem&)aEditDoc.GetItemPool().GetDefaultItem( EE_CHAR_COLOR) ), (sal_uInt32)i );

    rOutput << '{' << sRTF_COLORTBL;
    for ( j = 0; j < aColorList.Count(); j++ )
    {
        pColorItem = aColorList.GetObject( j );
        rOutput << sRTF_RED;
        rOutput.WriteNumber( pColorItem->GetValue().GetRed() );
        rOutput << sRTF_GREEN;
        rOutput.WriteNumber( pColorItem->GetValue().GetGreen() );
        rOutput << sRTF_BLUE;
        rOutput.WriteNumber( pColorItem->GetValue().GetBlue() );
        rOutput << ';';
    }
    rOutput << '}';
    rOutput << endl;

    // StyleSheets...
    if ( GetStyleSheetPool() )
    {
        sal_uInt16 nStyles = (sal_uInt16)GetStyleSheetPool()->GetStyles().Count();
        if ( nStyles )
        {
            rOutput << '{' << sRTF_STYLESHEET;

            for ( sal_uInt16 nStyle = 0; nStyle < nStyles; nStyle++ )
            {

                SfxStyleSheet* pStyle = (SfxStyleSheet*)GetStyleSheetPool()->GetStyles().GetObject( nStyle );

                rOutput << endl << '{' << sRTF_S;
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
                    rOutput << sRTF_SBASEDON;
                    nNumber = (sal_uInt16) GetStyleSheetPool()->GetStyles().GetPos( pParent ) + 1;
                    rOutput.WriteNumber( nNumber );
                }

                // Folgevorlage...(immer)
                SfxStyleSheet* pNext = pStyle;
                if ( pStyle->GetFollow().Len() && ( pStyle->GetFollow() != pStyle->GetName() ) )
                    pNext = (SfxStyleSheet*)GetStyleSheetPool()->Find( pStyle->GetFollow(), pStyle->GetFamily() );

                DBG_ASSERT( pNext, "Naechsten nicht gefunden!" );
                rOutput << sRTF_SNEXT;
                nNumber = (sal_uInt16) GetStyleSheetPool()->GetStyles().GetPos( pNext ) + 1;
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
    rOutput << '{' << sRTF_IGNORE << "\\EditEnginePoolDefaults";
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
    // rOutput << '{' << sRTF_IGNORE << "\\EditEnginePoolDefaultHeight}" << endl;

    // DefTab:
    MapMode aTwpMode( MAP_TWIP );
    sal_uInt16 nDefTabTwps = (sal_uInt16) GetRefDevice()->LogicToLogic(
                                        Point( aEditDoc.GetDefTab(), 0 ),
                                        &GetRefMapMode(), &aTwpMode ).X();
    rOutput << sRTF_DEFTAB;
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
            rOutput << sRTF_S;
            sal_uInt16 nNumber = (sal_uInt16) GetStyleSheetPool()->GetStyles().GetPos( pNode->GetStyleSheet() ) + 1;
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
                lcl_FindValidAttribs( aAttribItems, pNode, nStartPos, sal_False );
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
                nIndex += pTextPortion->GetLen();
                continue;
            }

            if ( pNextFeature && ( pNextFeature->GetStart() == nIndex ) )
            {
                WriteItemAsRTF( *pNextFeature->GetItem(), rOutput, nNode, nIndex, aFontTable, aColorList );
                pNextFeature = pNode->GetCharAttribs().FindFeature( pNextFeature->GetStart() + 1 );
            }
            else
            {
                aAttribItems.Clear();
                lcl_FindValidAttribs( aAttribItems, pNode, nIndex, sal_False );

                rOutput << '{';
                if ( WriteItemListAsRTF( aAttribItems, rOutput, nNode, nIndex, aFontTable, aColorList ) )
                    rOutput << ' ';

                USHORT nS = nIndex;
                USHORT nE = nIndex + pTextPortion->GetLen();
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

            nIndex += pTextPortion->GetLen();
        }

        rOutput << sRTF_PAR << sRTF_PARD << sRTF_PLAIN;;
        rOutput << endl;
    }
    // RTF-Nachspann...
    rOutput << "}}";    // 1xKlammerung Absaetze, 1x Klammerung RTF-Dokument
    rOutput.Flush();


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
        case EE_PARA_OUTLLEVEL:
        {
            rOutput << "\\level";
            sal_uInt16 nLevel = ((const SfxUInt16Item&)rItem).GetValue();
            rOutput.WriteNumber( nLevel );
        }
        break;
        case EE_PARA_OUTLLRSPACE:
        case EE_PARA_LRSPACE:
        {
            rOutput << sRTF_FI;
            short nTxtFirst = ((const SvxLRSpaceItem&)rItem).GetTxtFirstLineOfst();
            nTxtFirst = (short)LogicToTwips( nTxtFirst );
            rOutput.WriteNumber( nTxtFirst );
            rOutput << sRTF_LI;
            sal_uInt16 nTxtLeft = (USHORT)((const SvxLRSpaceItem&)rItem).GetTxtLeft();
            nTxtLeft = (sal_uInt16)LogicToTwips( nTxtLeft );
            rOutput.WriteNumber( nTxtLeft );
            rOutput << sRTF_RI;
            sal_uInt32 nTxtRight = ((const SvxLRSpaceItem&)rItem).GetRight();
            nTxtRight = LogicToTwips( nTxtRight);
            rOutput.WriteNumber( nTxtRight );
        }
        break;
        case EE_PARA_ULSPACE:
        {
            rOutput << sRTF_SB;
            sal_uInt16 nUpper = ((const SvxULSpaceItem&)rItem).GetUpper();
            nUpper = (sal_uInt16)LogicToTwips( nUpper );
            rOutput.WriteNumber( nUpper );
            rOutput << sRTF_SA;
            sal_uInt16 nLower = ((const SvxULSpaceItem&)rItem).GetLower();
            nLower = (sal_uInt16)LogicToTwips( nLower );
            rOutput.WriteNumber( nLower );
        }
        break;
        case EE_PARA_SBL:
        {
            rOutput << sRTF_SL;
            long nVal = ((const SvxLineSpacingItem&)rItem).GetLineHeight();
            if ( ((const SvxLineSpacingItem&)rItem).GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
            {
                // Woher kriege ich jetzt den Wert?
                // Der SwRTF-Parser geht von einem 240er Font aus!
                nVal = ((const SvxLineSpacingItem&)rItem).GetPropLineSpace();
                nVal *= 240;
                nVal /= 100;
            }
            rOutput.WriteNumber( nVal );
        }
        break;
        case EE_PARA_JUST:
        {
            SvxAdjust eJustification = ((const SvxAdjustItem&)rItem).GetAdjust();
            switch ( eJustification )
            {
                case SVX_ADJUST_CENTER: rOutput << sRTF_QC;
                                        break;
                case SVX_ADJUST_RIGHT:  rOutput << sRTF_QR;
                                        break;
                default:                rOutput << sRTF_QL;
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
                rOutput << sRTF_TX;
                rOutput.WriteNumber( LogicToTwips( rTab.GetTabPos() ) );
            }
        }
        break;
        case EE_CHAR_COLOR:
        {
            sal_uInt32 n = rColorList.GetId( (const SvxColorItem&)rItem );
            rOutput << sRTF_CF;
            rOutput.WriteNumber( n );
        }
        break;
        case EE_CHAR_FONTINFO:
        {
            sal_uInt32 n = rFontTable.GetId( (const SvxFontItem&)rItem );
            rOutput << sRTF_F;
            rOutput.WriteNumber( n );
        }
        break;
        case EE_CHAR_FONTHEIGHT:
        {
            rOutput << sRTF_FS;
            long nHeight = ((const SvxFontHeightItem&)rItem).GetHeight();
            nHeight = LogicToTwips( nHeight );
            // Twips => HalfPoints
            nHeight /= 10;
            rOutput.WriteNumber( nHeight );
        }
        break;
        case EE_CHAR_WEIGHT:
        {
            FontWeight e = ((const SvxWeightItem&)rItem).GetWeight();
            switch ( e )
            {
                case WEIGHT_BOLD:   rOutput << sRTF_B;              break;
                default:            rOutput << sRTF_B << '0';       break;
            }
        }
        break;
        case EE_CHAR_UNDERLINE:
        {
            // muesste bei WordLineMode ggf. ulw werden,
            // aber die Information fehlt hier
            FontUnderline e = ((const SvxUnderlineItem&)rItem).GetUnderline();
            switch ( e )
            {
                case UNDERLINE_NONE:    rOutput << sRTF_ULNONE;     break;
                case UNDERLINE_SINGLE:  rOutput << sRTF_UL;         break;
                case UNDERLINE_DOUBLE:  rOutput << sRTF_ULDB;       break;
                case UNDERLINE_DOTTED:  rOutput << sRTF_ULD;        break;
            }
        }
        break;
        case EE_CHAR_STRIKEOUT:
        {
            FontStrikeout e = ((const SvxCrossedOutItem&)rItem).GetStrikeout();
            switch ( e )
            {
                case STRIKEOUT_SINGLE:
                case STRIKEOUT_DOUBLE:  rOutput << sRTF_STRIKE;         break;
                case STRIKEOUT_NONE:    rOutput << sRTF_STRIKE << '0';  break;
            }
        }
        break;
        case EE_CHAR_ITALIC:
        {
            FontItalic e = ((const SvxPostureItem&)rItem).GetPosture();
            switch ( e )
            {
                case ITALIC_OBLIQUE:
                case ITALIC_NORMAL: rOutput << sRTF_I;      break;
                case ITALIC_NONE:   rOutput << sRTF_I << '0';   break;
            }
        }
        break;
        case EE_CHAR_OUTLINE:
        {
            rOutput << sRTF_OUTL;
            if ( ((const SvxContourItem&)rItem).GetValue() == 0 )
                rOutput << '0';
        }
        break;
        case EE_CHAR_SHADOW:
        {
            rOutput << sRTF_SHAD;
            if ( ((const SvxShadowedItem&)rItem).GetValue() == 0 )
                rOutput << '0';
        }
        break;
        case EE_FEATURE_TAB:
        {
            rOutput << sRTF_TAB;
        }
        break;
        case EE_FEATURE_LINEBR:
        {
            rOutput << sRTF_SL;
        }
        break;
        case EE_CHAR_KERNING:
        {
            rOutput << sRTF_EXPNDTW;
            rOutput.WriteNumber( LogicToTwips(
                ((const SvxKerningItem&)rItem).GetValue() ) );
        }
        break;
        case EE_CHAR_PAIRKERNING:
        {
            rOutput << sRTF_KERNING;
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
                nEsc = -( 100 - nProp );
                nProp100++;
            }
            // SWG:
            if ( nEsc )
                rOutput << "{\\*\\updnprop" << ByteString::CreateFromInt32( nProp100 ).GetBuffer() << '}';
            long nUpDown = nFontHeight * Abs( nEsc ) / 100;
            ByteString aUpDown = ByteString::CreateFromInt32( nUpDown );
            if ( nEsc < 0 )
                rOutput << sRTF_DN << aUpDown.GetBuffer();
            else if ( nEsc > 0 )
                rOutput << sRTF_UP << aUpDown.GetBuffer();
        }
        break;
    }
}

sal_uInt32 ImpEditEngine::WriteHTML( SvStream& rOutput, EditSelection aSel )
{
    return 0;
/*
    // Das hier ist erstmal das grobe Geruest vom RTF...
    // Noch klaeren:
    // - Wann Absaetze aufmachen?
    // - Attribute beenden

    DBG_ASSERT( GetUpdateMode(), "WriteHTML bei UpdateMode = sal_False!" );
    CheckIdleFormatter();
    if ( !IsFormatted() )
        FormatDoc();

    sal_uInt16 nStartNode, nEndNode;
    EditSelection aTmpSel;

    aSel.Adjust( aEditDoc );

    nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    // Vorspann...
    // ..............

    // ueber die Absaetze iterieren...
    for ( sal_uInt16 nNode = nStartNode; nNode <= nEndNode; nNode++  )
    {
        ContentNode* pNode = aEditDoc.SaveGetObject( nNode );
        DBG_ASSERT( pNode, "Node nicht gefunden: Search&Replace" );

        // Die Absatzattribute vorweg...
        // .....

        // Vorlage ?
        // ....
        if ( pNode->GetStyleSheet() )
        {
            // Hx, PRE oder anderes StyleSheet
            // .............
        }
        for ( sal_uInt16 nParAttr = EE_ITEMS_START; nParAttr <= EE_CHAR_END; nParAttr++ )
        {
            // ...
        }

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
                lcl_FindValidAttribs( aAttribItems, pNode, nStartPos, sal_False );
                if ( aAttribItems.Count() )
                {
                    // Diese Attribute duerfen nicht fuer den gesamten
                    // Absatz gelten:
                    // .....
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
                nIndex += pTextPortion->GetLen();
                continue;
            }

            if ( pNextFeature && ( pNextFeature->GetStart() == nIndex ) )
            {
                // .. Feature rausschreiben => TAB, ...
                pNextFeature = pNode->GetCharAttribs().FindFeature( pNextFeature->GetStart() + 1 );
            }
            else
            {
                // Attribute !!!
                // Eine Portion kann auch durch einen Zeilenumbruch
                // entstanden sein, dann brauch ich keine neuen Attribute schreiben...
                aAttribItems.Clear();
                lcl_FindValidAttribs( aAttribItems, pNode, nIndex, sal_False );
                // Items schreiben...
                XubString aTmpStr( *pNode, nIndex, pTextPortion->GetLen() );
                if ( ( n == nStartPortion ) && nStartPos )
                {
                    aTmpStr.Cut( 0, nStartPos - nIndex );
                }
                if ( ( n == nEndPortion ) && ( nEndPos != pNode->Len() ) )
                {
                    sal_uInt16 nLen = pTextPortion->GetLen();
                    sal_uInt16 nDelChars = nIndex+nLen-nEndPos;
                    aTmpStr.Cut( aTmpStr.Len()-nDelChars, nDelChars );
                }
                // Auf bestimmte Zeichen pruefen ?!
                // .....

                rOutput << aTmpStr.GetStr();
            }

            nIndex += pTextPortion->GetLen();
        }
    }
    // Nachspann
    // ....

    rOutput.Flush();
    return rOutput.GetError();
*/
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

        // Seit der 5.1 werden im Outliner die LRSpaceItems anders interpretiert.
        // Damit sie in einer 5.0 und aelter richtig sind, muss das Flag
        // richtig gesetzt sein...
        sal_Bool bOutliner = aStatus.IsAnyOutliner();
        for ( sal_uInt16 n = 0; n <=1; n++ )
        {
            sal_uInt16 nItemId = n ? EE_PARA_LRSPACE : EE_PARA_OUTLLRSPACE;
            if ( pC->GetParaAttribs().GetItemState( nItemId ) == SFX_ITEM_ON )
            {
                const SvxLRSpaceItem& rItem = (const SvxLRSpaceItem&) pC->GetParaAttribs().Get( nItemId );
                if ( rItem.IsBulletFI() != bOutliner )
                {
                    SvxLRSpaceItem aNewItem( rItem );
                    aNewItem.SetBulletFI( bOutliner );
                    pC->GetParaAttribs().Put( aNewItem );
                }
            }
        }


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
                    pX->GetEnd() -= nStartPos;

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
            USHORT nTest, nTPLen = 0, nTxtLen = 0;
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
    sal_Bool bUpdate = GetUpdateMode();
    sal_Bool bUndo = IsUndoEnabled();

    SetText( XubString() );
    EditPaM aPaM = aEditDoc.GetStartPaM();

    SetUpdateMode( sal_False );
    EnableUndo( sal_False );

    InsertText( rTextObject, EditSelection( aPaM, aPaM ) );
    SetVertical( rTextObject.IsVertical() );

    DBG_ASSERT( !HasUndoManager() || !GetUndoManager().GetUndoActionCount(), "Woher kommt das Undo in SetText ?!" );
    SetUpdateMode( bUpdate );
    EnableUndo( bUndo );
}

EditSelection ImpEditEngine::InsertText( const EditTextObject& rTextObject, EditSelection aSel )
{
    aSel.Adjust( aEditDoc );
    if ( aSel.HasRange() )
        aSel = ImpDeleteSelection( aSel );
    return InsertBinTextObject( (BinTextObject&)rTextObject, aSel.Max() );

    // MT 05/00: InsertBinTextObject direkt hier machen...
}

EditSelection ImpEditEngine::InsertBinTextObject( BinTextObject& rTextObject, EditPaM aPaM )
{
    // Optimieren:
    // Kein GetPos undFindParaportion, sondern Index berechnen!
    EditSelection aSel( aPaM, aPaM );
    DBG_ASSERT( !aSel.DbgIsBuggy( aEditDoc ), "InsertBibTextObject: Selektion kaput!(1)" );

    sal_Bool bUsePortionInfo = sal_False;
    sal_Bool bFields = sal_False;
    XParaPortionList* pPortionInfo = rTextObject.GetPortionInfo();

    if ( pPortionInfo && ( (long)pPortionInfo->GetPaperWidth() == aPaperSize.Width() )
            && ( pPortionInfo->GetRefMapMode() == GetRefDevice()->GetMapMode() ) )
    {
        if ( ( pPortionInfo->GetRefDevPtr() == (sal_uInt32)GetRefDevice() ) ||
             ( ( pPortionInfo->GetRefDevType() == OUTDEV_VIRDEV ) &&
               ( GetRefDevice()->GetOutDevType() == OUTDEV_VIRDEV ) ) )
        bUsePortionInfo = sal_True;
    }

    sal_Bool bConvertItems = sal_False;
    MapUnit eSourceUnit, eDestUnit;
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
                            UpdateFields();
                    }
                    else
                    {
                        DBG_ASSERT( pX->GetEnd()+nStartPos <= aPaM.GetNode()->Len(), "InsertBinTextObject: Attribut passt nicht! (2)" );
                        // Tabs und andere Features koennen nicht ueber InsertAttrib eingefuegt werden:
                        aEditDoc.InsertAttrib( aPaM.GetNode(), pX->GetStart()+nStartPos, pX->GetEnd()+nStartPos, *pX->GetItem() );
                    }
                }
            }
            // Sonst QuickFormat => Keine Attribute!
            pPortion->MarkSelectionInvalid( nStartPos, pC->GetText().Len() );
        }

        DBG_ASSERT( CheckOrderedList( aPaM.GetNode()->GetCharAttribs().GetAttribs(), sal_True ), "InsertBinTextObject: Start-Liste verdreht" );

        sal_Bool bParaAttribs = sal_False;
        if ( bNewContent || ( ( n > 0 ) && ( n < (nContents-1) ) ) )
        {
            // nur dann Style und ParaAttribs, wenn neuer Absatz, oder
            // komplett inneliegender...
            bParaAttribs = pC->GetParaAttribs().Count() ? sal_True : sal_False;
            if ( GetStyleSheetPool() && pC->GetStyle().Len() )
                SetStyleSheet( nPara, (SfxStyleSheet*)GetStyleSheetPool()->Find( pC->GetStyle(), pC->GetFamily() ) );
            if ( !bConvertItems )
                SetParaAttribs( aEditDoc.GetPos( aPaM.GetNode() ), pC->GetParaAttribs() );
            else
            {
                SfxItemSet aAttribs( GetEmptyItemSet() );
                ConvertAndPutItems( aAttribs, pC->GetParaAttribs(), &eSourceUnit, &eDestUnit );
                SetParaAttribs( aEditDoc.GetPos( aPaM.GetNode() ), aAttribs );
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
                for ( sal_uInt16 n = 0; n < nCount; n++ )
                {
                    TextPortion* pTextPortion = pXP->aTextPortions[n];
                    TextPortion* pNew = new TextPortion( *pTextPortion );
                    pParaPortion->GetTextPortions().Insert( pNew, n );
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
                USHORT nTest, nTPLen = 0, nTxtLen = 0;
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
            aPaM.GetNode()->SetWrongList( pC->GetWrongList()->Clone() );
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

LanguageType ImpEditEngine::GetLanguage( const EditPaM& rPaM, USHORT* pEndPos ) const
{
    short nScriptType = GetScriptType( rPaM, pEndPos ); // pEndPos will be valid now, pointing to ScriptChange or NodeLen
    USHORT nLangId = GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType );
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

EESpellState ImpEditEngine::Spell( EditView* pEditView, sal_Bool bMultipleDoc )
{
#ifdef SVX_LIGHT
    return EE_SPELL_NOSPELLER;
#else

    DBG_ASSERTWARNING( xSpeller.is(), "Kein Speller gesetzt!" );

    if ( !xSpeller.is() )
        return EE_SPELL_NOSPELLER;

    aOnlineSpellTimer.Stop();

    sal_Bool bForward = !EditSpellWrapper::IsSpellReverse();

    // Bei MultipleDoc immer von vorne/hinten...
    if ( bMultipleDoc )
    {
        if ( bForward )
            pEditView->pImpEditView->SetEditSelection( aEditDoc.GetStartPaM() );
        else
            pEditView->pImpEditView->SetEditSelection( aEditDoc.GetEndPaM() );
    }

    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );
    pSpellInfo = new SpellInfo;
    pSpellInfo->bMultipleDoc = bMultipleDoc;
    if ( bForward )
        pSpellInfo->aSpellStart = CreateEPaM( SelectWord( aCurSel, ::com::sun::star::i18n::WordType::DICTIONARY_WORD ).Min() );
    else
        pSpellInfo->aSpellStart = CreateEPaM( SelectWord( aCurSel, ::com::sun::star::i18n::WordType::DICTIONARY_WORD ).Max() );

    sal_Bool bIsStart = sal_False;
    if ( bMultipleDoc )
        bIsStart = sal_True;    // Immer von Vorne bzw. von hinten...
    else if ( bForward && ( CreateEPaM( aEditDoc.GetStartPaM() ) == pSpellInfo->aSpellStart ) )
        bIsStart = sal_True;
    else if ( !bForward && ( CreateEPaM( aEditDoc.GetEndPaM() ) == pSpellInfo->aSpellStart ) )
        bIsStart = sal_True;

    EditSpellWrapper* pWrp = new EditSpellWrapper( Application::GetDefModalDialogParent(),
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

Reference< XSpellAlternatives > ImpEditEngine::ImpSpell( EditView* pEditView )
{
#ifdef SVX_LIGHT
    return Reference< XSpellAlternatives >();
#else

    DBG_ASSERT( xSpeller.is(), "Kein Speller gesetzt!" );

    sal_Bool bForward = !EditSpellWrapper::IsSpellReverse();
    ContentNode* pLastNode = aEditDoc.SaveGetObject( bForward ? (aEditDoc.Count()-1) : 0 );
    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );
    if ( bForward )
        aCurSel.Min() = aCurSel.Max();
    else
        aCurSel.Max() = aCurSel.Min();

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
                if ( bForward && ( aCurSel.Max().GetIndex() >= pLastNode->Len() ) )
                    break;
                else if ( !bForward && ( !aCurSel.Min().GetIndex() ) )
                    break;
            }
        }
        else if ( !pSpellInfo->bSpellToEnd )
        {
            EPaM aEPaM( CreateEPaM( aCurSel.Max() ) );
            if ( bForward && !( aEPaM < pSpellInfo->aSpellTo ) )
                break;
            else if ( !bForward && ( aEPaM < pSpellInfo->aSpellTo ) )
                break;
        }

        // Bei Rueckwaerts-Suche erst zurueck, sonst wird das gleiche Wort wieder gespellt.
        if ( !bForward )
            aCurSel = WordLeft( aCurSel.Min(), ::com::sun::star::i18n::WordType::DICTIONARY_WORD );

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

        if ( aWord.Len() > 1 )
            xSpellAlt = xSpeller->spell( aWord, GetLanguage( aCurSel.Max() ), aEmptySeq );

        if ( bForward && !xSpellAlt.is() )
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

            sal_uInt16 nPaintFrom = 0xFFFF, nPaintTo;
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
                if ( aWord.Len() > 1 )
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
            if ( ( nPaintFrom != 0xFFFF ) && ( GetStatus().DoDrawRedLines() ) )
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
                                aInvalidRec = Rectangle();
                            }
                        }
                    }
                    else
                        UpdateViews( pActiveView );
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
        if ( aWord.Len() > 1 )
            xSpellAlt = xSpeller->spell( aWord, GetLanguage( aCurSel.Max() ), aEmptySeq );
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

    SvxThesaurusDialog aDialog( pEditView->GetWindow(), xThes, aWord, GetLanguage( aCurSel.Max() ) );

    if ( aDialog.Execute() == RET_OK )
    {
        // Wort ersetzen...
        pEditView->pImpEditView->DrawSelection();
        pEditView->pImpEditView->SetEditSelection( aCurSel );
        pEditView->pImpEditView->DrawSelection();
        pEditView->InsertText( aDialog.GetWord() );
        pEditView->ShowCursor( sal_True, sal_False );
    }

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

sal_Bool ImpEditEngine::ImpSearch( const SvxSearchItem& rSearchItem,
    const EditSelection& rSearchSelection, const EditPaM& rStartPos, EditSelection& rFoundSel )
{
#ifndef SVX_LIGHT
    sal_uInt16 nStartNode, nEndNode;

    EditSelection   aTmpSel;
    utl::SearchParam    aSearchParam( rSearchItem.GetSearchString(),
                        utl::SearchParam::SRCH_NORMAL, rSearchItem.GetExact(),
                        rSearchItem.GetWordOnly(), rSearchItem.GetSelection() );

    if ( rSearchItem.GetRegExp() )
        aSearchParam.SetSrchType( utl::SearchParam::SRCH_REGEXP );
    else if ( rSearchItem.IsLevenshtein() )
    {
        aSearchParam.SetSrchType( utl::SearchParam::SRCH_LEVDIST );
        aSearchParam.SetSrchRelaxed( rSearchItem.IsLEVRelaxed() ? sal_True : sal_False );
        aSearchParam.SetLEVOther( rSearchItem.GetLEVOther() );
        aSearchParam.SetLEVShorter( rSearchItem.GetLEVShorter() );
        aSearchParam.SetLEVLonger( rSearchItem.GetLEVLonger() );
    }

    sal_Bool bBack = rSearchItem.GetBackward();
    if ( aSearchParam.IsSrchInSelection() )
    {
        nEndNode = bBack
                    ? aEditDoc.GetPos( rSearchSelection.Min().GetNode() )
                    : aEditDoc.GetPos( rSearchSelection.Max().GetNode() );
        aSearchParam.SetSrchInSelection( sal_True );
    }
    else
    {
        nEndNode = bBack ? 0 : aEditDoc.Count()-1;
    }

    nStartNode = aEditDoc.GetPos( rStartPos.GetNode() );
    utl::TextSearch aSearcher( aSearchParam, Application::GetAppInternational().GetLanguage() );

    // ueber die Absaetze iterieren...
    for ( sal_uInt16 nNode = nStartNode;
            bBack ? ( nNode >= nEndNode ) : ( nNode <= nEndNode) ;
            bBack ? nNode-- : nNode++ )
    {
        // Bei rueckwaertsuche, wenn nEndNode = 0:
        if ( nNode >= 0xFFFF )
            return sal_False;

        ContentNode* pNode = aEditDoc.SaveGetObject( nNode );
        DBG_ASSERT( pNode, "Node nicht gefunden !" );

        sal_uInt16 nStartPos = 0;
        sal_uInt16 nEndPos = pNode->Len();
        if ( nNode == nStartNode )
        {
            if ( bBack )
                nEndPos = rStartPos.GetIndex();
            else
                nStartPos = rStartPos.GetIndex();
        }
        if ( ( nNode == nEndNode ) && aSearchParam.IsSrchInSelection() )
        {
            if ( bBack )
                nStartPos = rSearchSelection.Min().GetIndex();
            else
                nEndPos = rSearchSelection.Max().GetIndex();
        }

        // Suchen...
        XubString aParaStr( GetEditDoc().GetParaAsString( pNode ) );
        sal_Bool bFound = sal_False;
        if ( bBack )
        {
            SwapUSHORTs( nStartPos, nEndPos );
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

void ImpEditEngine::TransliterateText( const EditSelection& rSelection, sal_Int32 nTransliterationMode )
{
    EditSelection aSel( rSelection );
    aSel.Adjust( aEditDoc );

    if ( !aSel.HasRange() )
        aSel = SelectWord( aSel );

    USHORT nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    USHORT nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    BOOL bChanges = FALSE;
    EditUndoTransliteration* pUndo = NULL;
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        ESelection aESel( CreateESel( aSel ) );
        pUndo = new EditUndoTransliteration( this, aESel, nTransliterationMode );

        if ( ( nStartNode == nEndNode ) && !aSel.Min().GetNode()->GetCharAttribs().HasAttrib( aSel.Min().GetIndex(), aSel.Max().GetIndex() ) )
            pUndo->SetText( aSel.Min().GetNode()->Copy( aSel.Min().GetIndex(), aSel.Max().GetIndex()-aSel.Min().GetIndex() ) );
        else
            pUndo->SetText( CreateBinTextObject( aSel, NULL ) );
    }

    EditPaM aLastPaM = aSel.Max();

    utl::TransliterationWrapper aTranslitarationWrapper( ::comphelper::getProcessServiceFactory(), nTransliterationMode );
    BOOL bConsiderLanguage = aTranslitarationWrapper.needLanguageForTheMode();

    for ( USHORT nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        xub_StrLen nStartPos = 0;
        xub_StrLen nEndPos = pNode->Len();
        if ( nNode == nStartNode )
            nStartPos = aSel.Min().GetIndex();
        if ( nNode == nEndNode ) // kann auch == nStart sein!
            nEndPos = aSel.Max().GetIndex();

        USHORT nCurrentStart = nStartPos;
        USHORT nCurrentEnd = nEndPos;
        sal_uInt16 nLanguage = LANGUAGE_SYSTEM;

        do
        {
            if ( bConsiderLanguage )
            {
                nLanguage = GetLanguage( EditPaM( pNode, nCurrentStart ), &nCurrentEnd );
                if ( nCurrentEnd > nEndPos )
                    nCurrentEnd = nEndPos;
            }

            xub_StrLen nLen = nCurrentEnd - nCurrentStart;

            Sequence <long> aOffsets;
            String aNewText( aTranslitarationWrapper.transliterate( *pNode, nLanguage, nCurrentStart, nLen, &aOffsets ) );

            // Transliteration and Undo are prepared for 1:x transliteration, but attribs are not corrected...
            DBG_ASSERT( nLen == aNewText.Len(), "Transliteration: Change of TextLen - Attributes not corrected!" );
            if( ( nLen != aNewText.Len() ) || !pNode->Equals( aNewText, nCurrentStart, nLen ) )
            {
                bChanges = TRUE;
                pNode->Erase( nCurrentStart, nLen );
                pNode->Insert( aNewText, nCurrentStart );

                if ( nLen != aNewText.Len()  )
                {
                    short nDiff = aNewText.Len() - nLen;
                    nCurrentEnd += nDiff;
                    nEndPos += nDiff;

                    USHORT nMaxEnd = pNode->Len();
                    for( USHORT nAttr = pNode->GetCharAttribs().Count(); nAttr; )
                    {
                        EditCharAttrib* pAttr = pNode->GetCharAttribs().GetAttribs()[--nAttr];
                        if ( pAttr->GetEnd() > nMaxEnd )
                            pAttr->GetEnd() = nMaxEnd;
                    }
                }

                ParaPortion* pParaPortion = GetParaPortions()[nNode];
                pParaPortion->MarkSelectionInvalid( nCurrentStart, Max( nCurrentStart+nLen, nCurrentStart+aNewText.Len() ) );
            }
            if ( nNode == nEndNode )
                aLastPaM.GetIndex() = nCurrentStart+aNewText.Len();

            nCurrentStart = nCurrentEnd;
        } while( nCurrentEnd < nEndPos );
    }

    if ( pUndo )
    {
        if ( bChanges )
        {
            EditSelection aNewSel( aSel );
            aNewSel.Max() = aLastPaM;
            ESelection aESel( CreateESel( aSel ) );
            pUndo->SetNewSelection( aESel );
            InsertUndo( pUndo );
        }
        else
        {
            delete pUndo;
        }
    }

    if ( bChanges )
        FormatAndUpdate();
}
