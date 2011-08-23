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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#define _SVSTDARR_STRINGS

#include <string.h>
#include <stdio.h>			// sscanf

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SFX_FCONTNR_HXX //autogen
#include <bf_sfx2/fcontnr.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <bf_sfx2/docfile.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX //autogen
#include <bf_svx/tstpitem.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _SWGPAR_HXX
#include <swgpar.hxx>           // fuer den SW/G Parser
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>
#endif
#ifndef _W4WFLT_HXX
#include <w4wflt.hxx>           // AutoDetect
#endif
// enable implementation and members
#define IS_SW_DLL 1
#ifndef _IODETECT_HXX
#include <iodetect.hxx>
#endif
#ifndef _HINTS_HXX //autogen
#include <hints.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SWFLTOPT_HXX
#include <swfltopt.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif

#include <bf_svtools/moduleoptions.hxx>

namespace binfilter {

using namespace utl;
using namespace rtl;
using namespace ::com::sun::star::uno;

SwRead ReadRtf = 0, ReadAscii = 0, ReadSwg = 0, ReadSw3 = 0,
        ReadHTML = 0, ReadXML = 0;

/*N*/ inline BOOL IsDocShellRegistered() { return SvtModuleOptions().IsWriter(); }


/*N*/ IO_DETECT_IMPL1
/*N*/ IO_DETECT_IMPL2
/*N*/ IO_DETECT_IMPL3
/*N*/ IO_DETECT_IMPL4


/*N*/ inline void _SetFltPtr( USHORT& rPos, SwRead pReader
/*N*/ 						, const sal_Char* pNm
/* pNm optimiert der Compiler weg, wird nur in der nicht PRODUCT benoetigt! */
/*N*/ 						)
/*N*/ {
/*N*/ 	ASSERT( !strcmp( aReaderWriter[ rPos ].pName, pNm ), "falscher Filter" );
/*N*/ 	aReaderWriter[ rPos++ ].pReader = pReader;
/*N*/ }

/*N*/ void _InitFilter()
/*N*/ {
/*N*/    SwRead pRd;
/*N*/
/*N*/ 	USHORT nCnt = 0;
/*N*/ 	_SetFltPtr( nCnt, (ReadSw3 = new Sw3Reader), FILTER_SW5 );
/*N*/ 	_SetFltPtr( nCnt, ReadSw3, FILTER_SW4 );
/*N*/ 	_SetFltPtr( nCnt, ReadSw3, FILTER_SW3 );
/*N*/ 	_SetFltPtr( nCnt, (ReadSwg = new SwgReader), FILTER_SWG );
/*N*/ 	_SetFltPtr( nCnt, ReadSwg, FILTER_SWGV );
/*?*/   _SetFltPtr( nCnt, new Sw6Reader, sSwDos );
/*N*/ 	_SetFltPtr( nCnt, (ReadAscii = new AsciiReader), FILTER_BAS );
/*N*/  _SetFltPtr( nCnt, new W4WReader, FILTER_W4W );
/*N*/   _SetFltPtr( nCnt, ( pRd = new ExcelReader ), sCExcel );
/*N*/   _SetFltPtr( nCnt, pRd, sExcel );
/*N*/   _SetFltPtr( nCnt, new LotusReader, sLotusD );
/*N*/ 	_SetFltPtr( nCnt, ReadSwg, sSwg1 );
/*?*/ 	_SetFltPtr( nCnt, (ReadXML = new XMLReader), FILTER_XML );
/*N*/
/*N*/
/*N*/     _SetFltPtr( nCnt, ReadAscii, FILTER_TEXT );
/*N*/
/*N*/ 	ASSERT( MAXFILTER == nCnt, "Anzahl Filter ungleich der Definierten" );
/*N*/ }




/*N*/ void _FinitFilter()
/*N*/ {
/*N*/ 	// die Reader vernichten
/*N*/ 	for( USHORT n = 0; n < MAXFILTER; ++n )
/*N*/ 	{
/*N*/ 		SwIoDetect& rIo = aReaderWriter[n];
/*N*/ 		if( rIo.bDelReader && rIo.pReader )
/*N*/ 			delete rIo.pReader;
/*N*/ 	}
/*N*/ }


/*  */

/*N*/ void SwIoSystem::GetWriter( const String& rFltName, WriterRef& xRet )
/*N*/ {
/*N*/ 	for( USHORT n = 0; n < MAXFILTER; ++n )
/*N*/ 		if( aReaderWriter[n].IsFilter( rFltName ) )
/*N*/ 		{
/*N*/ 			aReaderWriter[n].GetWriter( rFltName, xRet );
/*N*/ 			break;
/*N*/ 		}
/*N*/ }


 SwRead SwIoSystem::GetReader( const String& rFltName )
 {
    SwRead pRead = 0;
    for( USHORT n = 0; n < MAXFILTER; ++n )
        if( aReaderWriter[n].IsFilter( rFltName ) )
        {
            pRead = aReaderWriter[n].GetReader();
            // fuer einige Reader noch eine Sonderbehandlung:
            pRead->SetFltName( rFltName );
            break;
        }
    return pRead;
 }

        // suche ueber den Filtertext den Filtereintrag
/*N*/ const SfxFilter* SwIoSystem::GetFilterOfFilterTxt( const String& rFilterNm,
/*N*/ 								const SfxFactoryFilterContainer* pCnt )
/*N*/ {
/*N*/ 	const SfxFactoryFilterContainer* pFltCnt = pCnt ? pCnt :
/*N*/ 		( IsDocShellRegistered()
/*N*/ 			? SwDocShell::Factory().GetFilterContainer()
/*N*/ 			: SwWebDocShell::Factory().GetFilterContainer() );
/*N*/
/*N*/ 	do {
/*N*/ 		if( pFltCnt )
/*N*/ 		{
/*N*/ 			const SfxFilter* pFilter;
/*N*/ 			USHORT nCount = pFltCnt->GetFilterCount();
/*N*/ 			for( USHORT i = 0; i < nCount; ++i )
/*N*/ 				if( ( pFilter = pFltCnt->GetFilter( i ))->GetFilterName() == rFilterNm )
/*N*/ 					return pFilter;
/*N*/ 		}
/*?*/ 		if( pCnt || pFltCnt == SwWebDocShell::Factory().GetFilterContainer())
/*?*/ 			break;
/*?*/ 		pFltCnt = SwWebDocShell::Factory().GetFilterContainer();
/*?*/ 	} while( TRUE );
/*?*/
/*?*/ 	return 0;
/*N*/ }


/*  */

/////////////// die Storage Reader/Writer ////////////////////////////////


/*N*/ void GetSw3Writer( const String&, WriterRef& xRet )
/*N*/ {
/*N*/ 	xRet = new Sw3Writer;
/*N*/ }


 ULONG StgReader::OpenMainStream( SvStorageStreamRef& rRef, USHORT& rBuffSize )
 {
    ULONG nRet = ERR_SWG_READ_ERROR;
    ASSERT( pStg, "wo ist mein Storage?" );
    const SfxFilter* pFltr = SwIoSystem::GetFilterOfFormat( aFltName );
    if( pFltr )
    {
        rRef = pStg->OpenStream( SwIoSystem::GetSubStorageName( *pFltr ),
                                    STREAM_READ | STREAM_SHARE_DENYALL );

        if( rRef.Is() )
        {
            if( SVSTREAM_OK == rRef->GetError() )
            {
                USHORT nOld = rRef->GetBufferSize();
                rRef->SetBufferSize( rBuffSize );
                rBuffSize = nOld;
                nRet = 0;
            }
            else
                nRet = rRef->GetError();
        }
    }
    return nRet;
 }

/*  */


/*N*/ ULONG Sw3Reader::Read( SwDoc &rDoc, SwPaM &rPam, const String & )
/*N*/ {
/*N*/ 	ULONG nRet;
/*N*/ 	if( pStg && pIO )
/*N*/ 	{
/*N*/ 		// TRUE: Vorlagen ueberschreiben
/*N*/ 		pIO->SetReadOptions( aOpt,TRUE );
/*N*/ 		if( !bInsertMode )
/*N*/ 		{
/*N*/ 			// Im Laden-Modus darf der PaM-Content-Teil nicht
/*N*/ 			// in den Textbereich zeigen (Nodes koennen geloescht werden)
/*N*/ 			rPam.GetBound( TRUE ).nContent.Assign( 0, 0 );
/*N*/ 			rPam.GetBound( FALSE ).nContent.Assign( 0, 0 );
/*N*/ 		}
/*N*/ 		nRet = pIO->Load( pStg, bInsertMode ? &rPam : 0 );
/*N*/ 		aOpt.ResetAllFmtsOnly();
/*N*/ 		pIO->SetReadOptions( aOpt, TRUE );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		ASSERT( !this, "Sw3-Read ohne Storage und/oder IO-System" );
/*?*/ 		nRet = ERR_SWG_READ_ERROR;
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

    // read the sections of the document, which is equal to the medium.
    // returns the count of it


/*N*/ ULONG Sw3Writer::WriteStorage()
/*N*/ {
/*N*/ 	ULONG nRet;
/*N*/ 	if( pIO )
/*N*/ 	{
/*N*/ 		// der gleiche Storage -> Save, sonst SaveAs aufrufen
/*N*/ 		if( !bSaveAs )
/*?*/ 			nRet = pIO->Save( pOrigPam, bWriteAll );
/*N*/ 		else
/*N*/ 			nRet = pIO->SaveAs( pStg, pOrigPam, bWriteAll );
/*N*/
/*N*/ 		pIO = 0;		// nach dem Schreiben ist der Pointer ungueltig !!
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		ASSERT( !this, "Sw3-Writer ohne IO-System" )
/*?*/ 		nRet = ERR_SWG_WRITE_ERROR;
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }







 BOOL Writer::IsStgWriter() const { return FALSE; }
 BOOL Writer::IsSw3Writer() const { return FALSE; }


/*  */


ULONG SwgReader::Read( SwDoc &rDoc, SwPaM &rPam, const String& rFileName )
{
    if( !pStrm )
    {
        ASSERT( !this, "SWG-Read ohne Stream" );
        return ERR_SWG_READ_ERROR;
    }
    SwSwgParser *pSwgParser = new SwSwgParser( &rDoc, &rPam, pStrm,
                                                rFileName, !bInsertMode );
    USHORT nBits = SWGRD_NORMAL;
    SwgReader* pRdr = (SwgReader*) ReadSwg;
    if( pRdr->aOpt.IsFmtsOnly() )
    {
        nBits = 0;
        if( pRdr->aOpt.IsFrmFmts()   ) nBits |= SWGRD_FRAMEFMTS;
        if( pRdr->aOpt.IsTxtFmts()   ) nBits |= SWGRD_CHARFMTS | SWGRD_PARAFMTS;
        if( pRdr->aOpt.IsPageDescs() ) nBits |= SWGRD_PAGEFMTS;
        if( !pRdr->aOpt.IsMerge() )
            nBits |= SWGRD_FORCE;
    }
    ULONG nRet = pSwgParser->CallParser( nBits );
    delete pSwgParser;

    // die Flags muessen natuerlich wieder geloescht werden!
    pRdr->aOpt.ResetAllFmtsOnly();

    return nRet;
}


 BOOL SwReader::NeedsPasswd( const Reader& rOptions )
 {
    BOOL bRes = FALSE;
    if( &rOptions == ReadSwg )
    {
        if( !pStrm && pMedium && !pMedium->IsStorage() )
            pStrm = pMedium->GetInStream();

        ASSERT( pStrm, "Passwort-Test ohne Stream" );
        if( pStrm )
        {
            SwSwgParser *pSwgParser = new SwSwgParser( pStrm );
            bRes = pSwgParser->NeedsPasswd();
            delete pSwgParser;
        }
    }
    return bRes;
 }


 BOOL SwReader::CheckPasswd( const String& rPasswd, const Reader& rOptions )
 {
    BOOL bRes = TRUE;
    if( &rOptions == ReadSwg )
    {
        if( !pStrm && pMedium && !pMedium->IsStorage() )
            pStrm = pMedium->GetInStream();

        ASSERT( pStrm, "Passwort-Check ohne Stream" );
        if( pStrm )
        {
            SwSwgParser *pSwgParser = new SwSwgParser( pStrm );
            bRes = pSwgParser->CheckPasswd( rPasswd );
            delete pSwgParser;
        }
    }
    return bRes;
 }

/*  */

//-----------------------------------------------------------------------
// Filter Flags lesen, wird von WW8 / W4W / EXCEL / LOTUS benutzt.
//-----------------------------------------------------------------------

/*
<FilterFlags>
        <Excel_Lotus>
                <MinRow cfg:type="long">0</MinRow>
                <MaxRow cfg:type="long">0</MaxRow>
                <MinCol cfg:type="long">0</MinCol>
                <MaxCol cfg:type="long">0</MaxCol>
        </Excel_Lotus>
        <W4W>
                <W4WHD cfg:type="long">0</W4WHD>
                <W4WFT cfg:type="long">0</W4WFT>
                <W4W000 cfg:type="long">0</W4W000>
        </W4W>
        <WinWord>
                <WW1F cfg:type="long">0</WW1F>
                <WW cfg:type="long">0</WW>
                <WW8 cfg:type="long">0</WW8>
                <WWF cfg:type="long">0</WWF>
                <WWFA0 cfg:type="long">0</WWFA0>
                <WWFA1 cfg:type="long">0</WWFA1>
                <WWFA2 cfg:type="long">0</WWFA2>
                <WWFB0 cfg:type="long">0</WWFB0>
                <WWFB1 cfg:type="long">0</WWFB1>
                <WWFB2 cfg:type="long">0</WWFB2>
                <WWFLX cfg:type="long">0</WWFLX>
                <WWFLY cfg:type="long">0</WWFLY>
                <WWFT cfg:type="long">0</WWFT>
                <WWWR cfg:type="long">0</WWWR>
        </WinWord>
        <Writer>
                <SW3Imp cfg:type="long">0</SW3Imp>
        </Writer>
</FilterFlags>
*/

/*N*/ #define FILTER_OPTION_ROOT 		String::CreateFromAscii( \
/*N*/ 				RTL_CONSTASCII_STRINGPARAM( "Office.Writer/FilterFlags" ) )

 SwFilterOptions::SwFilterOptions()
    : ConfigItem( FILTER_OPTION_ROOT )
 {
 }

/*N*/ SwFilterOptions::SwFilterOptions( sal_uInt16 nCnt, const sal_Char** ppNames,
/*N*/ 			  					sal_uInt32* pValues )
/*N*/ 	: ConfigItem( FILTER_OPTION_ROOT )
/*N*/ {
/*N*/ 	GetValues( nCnt, ppNames, pValues );
/*N*/ }

/*N*/ void SwFilterOptions::GetValues( sal_uInt16 nCnt, const sal_Char** ppNames,
/*N*/ 			  						sal_uInt32* pValues )
/*N*/ {
/*N*/ 	Sequence<OUString> aNames( nCnt );
/*N*/ 	OUString* pNames = aNames.getArray();
        USHORT n=0;
/*N*/ 	for( n = 0; n < nCnt; ++n )
/*N*/ 		pNames[ n ] = OUString::createFromAscii( ppNames[ n ] );
/*N*/ 	Sequence<Any> aValues = GetProperties( aNames );
/*N*/
/*N*/ 	if( nCnt == aValues.getLength() )
/*N*/ 	{
/*N*/ 		const Any* pAnyValues = aValues.getConstArray();
/*N*/ 		for( n = 0; n < nCnt; ++n )
/*N*/ 			pValues[ n ] = pAnyValues[ n ].hasValue()
/*N*/ 							? *(sal_uInt32*)pAnyValues[ n ].getValue()
/*N*/ 							: 0;
/*N*/ 	}
/*N*/ 	else
/*?*/ 		for( n = 0; n < nCnt; ++n )
/*?*/ 			pValues[ n ] = 0;
/*N*/ }

 sal_Bool SwFilterOptions::CheckNodeContentExist( const sal_Char* pNode,
                                                 const sal_Char* pCntnt )
 {
    Sequence<OUString> aNames( GetNodeNames(
                                        OUString::createFromAscii( pNode )));
    sal_Bool bExist = sal_False;
    const OUString* pNames = aNames.getConstArray();
    for( long n = 0, nEnd = aNames.getLength(); n < nEnd; ++n, ++pNames )
        if( !pNames->compareToAscii( pCntnt ))
        {
            bExist = sal_True;
            break;
        }
    return bExist;
 }

/*  */






/*  */

SwRelNumRuleSpaces::SwRelNumRuleSpaces( SwDoc& rDoc, BOOL bNDoc )
    : bNewDoc( bNDoc )
{
    pNumRuleTbl = new SwNumRuleTbl( 8, 8 );
    if( !bNDoc )
        pNumRuleTbl->Insert( &rDoc.GetNumRuleTbl(), 0 );
}

SwRelNumRuleSpaces::~SwRelNumRuleSpaces()
{
    if( pNumRuleTbl )
    {
        pNumRuleTbl->Remove( 0, pNumRuleTbl->Count() );
        delete pNumRuleTbl;
    }
}

void SwRelNumRuleSpaces::SetNumRelSpaces( SwDoc& rDoc )
{
    SwNumRuleTbl* pRuleTbl = bNewDoc ? &rDoc.GetNumRuleTbl() : pNumRuleTbl;
    if( !bNewDoc )
    {
        // jetzt alle schon vorhanden NumRules aus dem Array entfernen,
        // damit nur die neuen angepasst werden
        SwNumRuleTbl aNumRuleTbl;
        aNumRuleTbl.Insert( pRuleTbl, 0 );
        pRuleTbl->Remove( 0, pRuleTbl->Count() );
        const SwNumRuleTbl& rRuleTbl = rDoc.GetNumRuleTbl();
        SwNumRule* pRule;

        for( USHORT n = 0; n < rRuleTbl.Count(); ++n )
            if( USHRT_MAX == aNumRuleTbl.GetPos( ( pRule = rRuleTbl[ n ] )))
                // war noch nicht vorhanden, also neu
                pRuleTbl->Insert( pRule, pRuleTbl->Count() );

        aNumRuleTbl.Remove( 0, aNumRuleTbl.Count() );
    }

    if( pRuleTbl )
    {
        for( USHORT n = pRuleTbl->Count(); n; )
        {
            SwNumRule* pRule = (*pRuleTbl)[ --n ];
            // Rule noch gueltig und am Doc vorhanden?
            if( USHRT_MAX != rDoc.GetNumRuleTbl().GetPos( pRule ))
            {
                SwNumRuleInfo aUpd( pRule->GetName() );
                aUpd.MakeList( rDoc );

                // bei allen nmumerierten Absaetzen vom linken Rand
                // den absoluten Wert des NumFormates abziehen
                for( ULONG nUpdPos = 0; nUpdPos < aUpd.GetList().Count();
                    ++nUpdPos )
                {
                    SwTxtNode* pNd = aUpd.GetList().GetObject( nUpdPos );
                    SetNumLSpace( *pNd, *pRule );
                }
            }
        }
    }

    if( pNumRuleTbl )
    {
        pNumRuleTbl->Remove( 0, pNumRuleTbl->Count() );
        delete pNumRuleTbl, pNumRuleTbl = 0;
    }

    if( bNewDoc )
    {
        SetOultineRelSpaces( SwNodeIndex( rDoc.GetNodes() ),
                            SwNodeIndex( rDoc.GetNodes().GetEndOfContent()));
    }
}

void SwRelNumRuleSpaces::SetOultineRelSpaces( const SwNodeIndex& rStt,
                                            const SwNodeIndex& rEnd )
{
    SwDoc* pDoc = rStt.GetNode().GetDoc();
    const SwOutlineNodes& rOutlNds = pDoc->GetNodes().GetOutLineNds();
    if( rOutlNds.Count() )
    {
        USHORT nPos;
        rOutlNds.Seek_Entry( &rStt.GetNode(), &nPos );
        for( ; nPos < rOutlNds.Count() &&
                rOutlNds[ nPos ]->GetIndex() < rEnd.GetIndex(); ++nPos )
        {
            SwTxtNode* pNd = rOutlNds[ nPos ]->GetTxtNode();
            if( pNd->GetOutlineNum() && !pNd->GetNumRule() )
                SetNumLSpace( *pNd, *pDoc->GetOutlineNumRule() );
        }
    }
}

void SwRelNumRuleSpaces::SetNumLSpace( SwTxtNode& rNd, const SwNumRule& rRule )
{
    BOOL bOutlineRule = OUTLINE_RULE == rRule.GetRuleType();
    BYTE nLvl;
    {
        SwNodeNum aNdNum( 0 );
        const SwNodeNum* pNum;
        if( bOutlineRule )
        {
            if( 0 == ( pNum = rNd.GetOutlineNum() ))
                pNum = rNd.UpdateOutlineNum( aNdNum );
        }
        else if( 0 == ( pNum = rNd.GetNum() ))
            pNum = rNd.UpdateNum( aNdNum );
        nLvl = GetRealLevel( pNum->GetLevel() );
    }
    const SwNumFmt& rFmt = rRule.Get( nLvl );
    const SvxLRSpaceItem& rLR = rNd.GetSwAttrSet().GetLRSpace();

    SvxLRSpaceItem aLR( rLR );
    aLR.SetTxtFirstLineOfst( 0 );

    // sagt der Node, das die Numerierung den Wert vorgibt?
    if( !bOutlineRule && rNd.IsSetNumLSpace() )
        aLR.SetTxtLeft( 0 );
    else
    {
        long nLeft = rFmt.GetAbsLSpace(), nParaLeft = rLR.GetTxtLeft();
        if( 0 < rLR.GetTxtFirstLineOfst() )
            nParaLeft += rLR.GetTxtFirstLineOfst();
        else if( nParaLeft >= nLeft )
            // #82963#/#82962#: set correct paragraph indent
            nParaLeft -= nLeft;
        else
            //#83154#, Don't think any of the older #80856# bugfix code is
            //relevent anymore.
            nParaLeft = rLR.GetTxtLeft()+rLR.GetTxtFirstLineOfst();
        aLR.SetTxtLeft( nParaLeft );
    }

    if( aLR.GetTxtLeft() != rLR.GetTxtLeft() )
    {
        //bevor rLR geloescht wird!
        long nOffset = rLR.GetTxtLeft() - aLR.GetTxtLeft();
        rNd.SwCntntNode::SetAttr( aLR );

        // Tabs anpassen !!
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rNd.GetSwAttrSet().GetItemState(
                RES_PARATR_TABSTOP, TRUE, &pItem ))
        {
            SvxTabStopItem aTStop( *(SvxTabStopItem*)pItem );
            for( USHORT n = 0; n < aTStop.Count(); ++n )
            {
                SvxTabStop& rTab = (SvxTabStop&)aTStop[ n ];
                if( SVX_TAB_ADJUST_DEFAULT != rTab.GetAdjustment() )
                {
                    if( !rTab.GetTabPos() )
                    {
                        aTStop.Remove( n );
                        --n;
                    }
                    else
                        rTab.GetTabPos() += nOffset;
                }
            }
            rNd.SwCntntNode::SetAttr( aTStop );
        }
    }
}
/*  */
 struct CharSetNameMap
 {
     rtl_TextEncoding eCode;
     const sal_Char* pName;
 };

 const CharSetNameMap *GetCharSetNameMap()
 {
     static const CharSetNameMap aMapArr[] =
     {
 #   define IMPLENTRY(X) { RTL_TEXTENCODING_##X, "" #X "" }
         IMPLENTRY(DONTKNOW),
         IMPLENTRY(MS_1252),
         IMPLENTRY(APPLE_ROMAN),
         IMPLENTRY(IBM_437),
         IMPLENTRY(IBM_850),
         IMPLENTRY(IBM_860),
         IMPLENTRY(IBM_861),
         IMPLENTRY(IBM_863),
         IMPLENTRY(IBM_865),
         IMPLENTRY(SYMBOL),
         IMPLENTRY(ASCII_US),
         IMPLENTRY(ISO_8859_1),
         IMPLENTRY(ISO_8859_2),
         IMPLENTRY(ISO_8859_3),
         IMPLENTRY(ISO_8859_4),
         IMPLENTRY(ISO_8859_5),
         IMPLENTRY(ISO_8859_6),
         IMPLENTRY(ISO_8859_7),
         IMPLENTRY(ISO_8859_8),
         IMPLENTRY(ISO_8859_9),
         IMPLENTRY(ISO_8859_14),
         IMPLENTRY(ISO_8859_15),
         IMPLENTRY(IBM_737),
         IMPLENTRY(IBM_775),
         IMPLENTRY(IBM_852),
         IMPLENTRY(IBM_855),
         IMPLENTRY(IBM_857),
         IMPLENTRY(IBM_862),
         IMPLENTRY(IBM_864),
         IMPLENTRY(IBM_866),
         IMPLENTRY(IBM_869),
         IMPLENTRY(MS_874),
         IMPLENTRY(MS_1250),
         IMPLENTRY(MS_1251),
         IMPLENTRY(MS_1253),
         IMPLENTRY(MS_1254),
         IMPLENTRY(MS_1255),
         IMPLENTRY(MS_1256),
         IMPLENTRY(MS_1257),
         IMPLENTRY(MS_1258),
         IMPLENTRY(APPLE_ARABIC),
         IMPLENTRY(APPLE_CENTEURO),
         IMPLENTRY(APPLE_CROATIAN),
         IMPLENTRY(APPLE_CYRILLIC),
         IMPLENTRY(APPLE_DEVANAGARI),
         IMPLENTRY(APPLE_FARSI),
         IMPLENTRY(APPLE_GREEK),
         IMPLENTRY(APPLE_GUJARATI),
         IMPLENTRY(APPLE_GURMUKHI),
         IMPLENTRY(APPLE_HEBREW),
         IMPLENTRY(APPLE_ICELAND),
         IMPLENTRY(APPLE_ROMANIAN),
         IMPLENTRY(APPLE_THAI),
         IMPLENTRY(APPLE_TURKISH),
         IMPLENTRY(APPLE_UKRAINIAN),
         IMPLENTRY(APPLE_CHINSIMP),
         IMPLENTRY(APPLE_CHINTRAD),
         IMPLENTRY(APPLE_JAPANESE),
         IMPLENTRY(APPLE_KOREAN),
         IMPLENTRY(MS_932),
         IMPLENTRY(MS_936),
         IMPLENTRY(MS_949),
         IMPLENTRY(MS_950),
         IMPLENTRY(SHIFT_JIS),
         IMPLENTRY(GB_2312),
         IMPLENTRY(GBT_12345),
         IMPLENTRY(GBK),
         IMPLENTRY(BIG5),
         IMPLENTRY(EUC_JP),
         IMPLENTRY(EUC_CN),
         IMPLENTRY(EUC_TW),
         IMPLENTRY(ISO_2022_JP),
         IMPLENTRY(ISO_2022_CN),
         IMPLENTRY(KOI8_R),
         IMPLENTRY(KOI8_U),
         IMPLENTRY(UTF7),
         IMPLENTRY(UTF8),
         IMPLENTRY(ISO_8859_10),
         IMPLENTRY(ISO_8859_13),
         IMPLENTRY(EUC_KR),
         IMPLENTRY(ISO_2022_KR),
         IMPLENTRY(JIS_X_0201),
         IMPLENTRY(JIS_X_0208),
         IMPLENTRY(JIS_X_0212),
         IMPLENTRY(MS_1361),
         IMPLENTRY(GB_18030),
         IMPLENTRY(BIG5_HKSCS),
         IMPLENTRY(TIS_620),
         IMPLENTRY(UCS4),
         IMPLENTRY(UCS2),
         IMPLENTRY(UNICODE),
         {0,0}       //Last
     };
     return &aMapArr[0];
 }
/*
 Get a rtl_TextEncoding from its name
 */
 rtl_TextEncoding CharSetFromName(const String& rChrSetStr)
 {
     const CharSetNameMap *pStart = GetCharSetNameMap();
     rtl_TextEncoding nRet = pStart->eCode;

     for(const CharSetNameMap *pMap = pStart; pMap->pName; ++pMap)
     {
         if(rChrSetStr.EqualsIgnoreCaseAscii(pMap->pName))
         {
             nRet = pMap->eCode;
             break;
         }
     }

     ASSERT(nRet != pStart->eCode, "TXT: That was an unknown language!");

    return nRet;
 }


/*
 Get the String name of an rtl_TextEncoding
 */

 struct LangNameMap
 {
     LanguageType nId;
     const sal_Char* pLanguageNm;
 };

 const LangNameMap *GetLangNameMap()
 {
     static const LangNameMap aMapArr[] =
     {
         {LANGUAGE_DONTKNOW,                    "DONTKNOW"              },
         {LANGUAGE_NONE,                        "NONE"                  },
         {LANGUAGE_SYSTEM,                  "SYSTEM"                },
         {LANGUAGE_AFRIKAANS,               "AFRIKAANS"             },
         {LANGUAGE_ALBANIAN,                    "ALBANIAN"              },
         {LANGUAGE_ARABIC_PRIMARY_ONLY,     "ARABIC"                },
         {LANGUAGE_ARABIC_SAUDI_ARABIA,     "ARABIC (SAUDI ARABIA)" },
         {LANGUAGE_ARABIC_IRAQ,             "ARABIC (IRAQ)"         },
         {LANGUAGE_ARABIC_EGYPT,                "ARABIC (EGYPT)"        },
         {LANGUAGE_ARABIC_LIBYA,                "ARABIC (LIBYA)"        },
         {LANGUAGE_ARABIC_ALGERIA,          "ARABIC (ALGERIA)"      },
         {LANGUAGE_ARABIC_MOROCCO,          "ARABIC (MOROCCO)"      },
         {LANGUAGE_ARABIC_TUNISIA,          "ARABIC (TUNISIA)"      },
         {LANGUAGE_ARABIC_OMAN,             "ARABIC (OMAN)"         },
         {LANGUAGE_ARABIC_YEMEN,                "ARABIC (YEMEN)"        },
         {LANGUAGE_ARABIC_SYRIA,                "ARABIC (SYRIA)"        },
         {LANGUAGE_ARABIC_JORDAN,           "ARABIC (JORDAN)"       },
         {LANGUAGE_ARABIC_LEBANON,          "ARABIC (LEBANON)"      },
         {LANGUAGE_ARABIC_KUWAIT,           "ARABIC (KUWAIT)"       },
         {LANGUAGE_ARABIC_UAE,              "ARABIC (UAE)"          },
         {LANGUAGE_ARABIC_BAHRAIN,          "ARABIC (BAHRAIN)"      },
         {LANGUAGE_ARABIC_QATAR,                "ARABIC (QATAR)"        },
         {LANGUAGE_ARMENIAN,                    "ARMENIAN"              },
         {LANGUAGE_ASSAMESE,                    "ASSAMESE"              },
         {LANGUAGE_AZERI,                   "AZERI"                 },
         {LANGUAGE_AZERI_LATIN,             "AZERI (LATIN)"         },
         {LANGUAGE_AZERI_CYRILLIC,          "AZERI (CYRILLIC)"      },
         {LANGUAGE_BASQUE,                  "BASQUE"                },
         {LANGUAGE_BELARUSIAN,              "BELARUSIAN"            },
         {LANGUAGE_BENGALI,                 "BENGALI"               },
         {LANGUAGE_BULGARIAN,               "BULGARIAN"             },
         {LANGUAGE_BURMESE,                  "BURMESE"               },
         {LANGUAGE_CATALAN,                 "CATALAN"               },
         {LANGUAGE_CHINESE,                 "CHINESE"               },
         {LANGUAGE_CHINESE_TRADITIONAL,     "CHINESE (TRADITIONAL)" },
         {LANGUAGE_CHINESE_SIMPLIFIED,      "CHINESE (SIMPLIFIED)"  },
         {LANGUAGE_CHINESE_HONGKONG,            "CHINESE (HONGKONG)"    },
         {LANGUAGE_CHINESE_SINGAPORE,       "CHINESE (SINGAPORE)"   },
         {LANGUAGE_CHINESE_MACAU,           "CHINESE (MACAU)"       },
         {LANGUAGE_CZECH,                   "CZECH"                 },
         {LANGUAGE_DANISH,                  "DANISH"                },
         {LANGUAGE_DUTCH,                   "DUTCH"                 },
         {LANGUAGE_DUTCH_BELGIAN,           "DUTCH_BELGIAN"         },
         {LANGUAGE_ENGLISH,                 "ENGLISH"               },
         {LANGUAGE_ENGLISH_US,              "ENGLISH (US)"          },
         {LANGUAGE_ENGLISH_UK,              "ENGLISH (UK)"          },
         {LANGUAGE_ENGLISH_AUS,             "ENGLISH (AUS)"         },
         {LANGUAGE_ENGLISH_CAN,             "ENGLISH (CAN)"         },
         {LANGUAGE_ENGLISH_NZ,              "ENGLISH (NZ)"          },
         {LANGUAGE_ENGLISH_EIRE,                "ENGLISH (EIRE)"        },
         {LANGUAGE_ENGLISH_SAFRICA,         "ENGLISH (SAFRICA)"     },
         {LANGUAGE_ENGLISH_JAMAICA,         "ENGLISH (JAMAICA)"     },
         {LANGUAGE_ENGLISH_CARRIBEAN,       "ENGLISH (CARRIBEAN)"   },
         {LANGUAGE_ENGLISH_BELIZE,          "ENGLISH (BELIZE)"      },
         {LANGUAGE_ENGLISH_TRINIDAD,            "ENGLISH (TRINIDAD)"    },
         {LANGUAGE_ENGLISH_ZIMBABWE,            "ENGLISH (ZIMBABWE)"    },
         {LANGUAGE_ENGLISH_PHILIPPINES,     "ENGLISH (PHILIPPINES)" },
         {LANGUAGE_ESTONIAN,                    "ESTONIAN"              },
         {LANGUAGE_FAEROESE,                    "FAEROESE"              },
         {LANGUAGE_FARSI,                   "FARSI"                 },
         {LANGUAGE_FINNISH,                 "FINNISH"               },
         {LANGUAGE_FRENCH,                  "FRENCH"                },
         {LANGUAGE_FRENCH_BELGIAN,          "FRENCH (BELGIAN)"      },
         {LANGUAGE_FRENCH_CANADIAN,         "FRENCH (CANADIAN)"     },
         {LANGUAGE_FRENCH_SWISS,                "FRENCH (SWISS)"        },
         {LANGUAGE_FRENCH_LUXEMBOURG,       "FRENCH (LUXEMBOURG)"   },
         {LANGUAGE_FRENCH_MONACO,           "FRENCH (MONACO)"       },
         {LANGUAGE_FRENCH_WEST_INDIES,      "FRENCH (WEST INDIES)"  },
         {LANGUAGE_FRENCH_REUNION,          "FRENCH (REUNION)"      },
         {LANGUAGE_FRENCH_ZAIRE,                "FRENCH (ZAIRE)"        },
         {LANGUAGE_FRENCH_SENEGAL,          "FRENCH (SENEGAL)"      },
         {LANGUAGE_FRENCH_CAMEROON,         "FRENCH (CAMEROON)"     },
         {LANGUAGE_FRENCH_COTE_D_IVOIRE,    "FRENCH (COTE D IVOIRE)"},
         {LANGUAGE_FRENCH_MALI,             "FRENCH (MALI)"         },
         {LANGUAGE_FRISIAN_NETHERLANDS,     "FRISIAN (NETHERLANDS)" },
         {LANGUAGE_GAELIC_SCOTLAND,         "GAELIC (SCOTLAND)"     },
         {LANGUAGE_GAELIC_IRELAND,          "GAELIC (IRELAND)"      },
         {LANGUAGE_GALICIAN,                    "GALICIAN"              },
         {LANGUAGE_GEORGIAN,                "GEORGIAN"              },
         {LANGUAGE_GERMAN,                  "GERMAN"                },
         {LANGUAGE_GERMAN_SWISS,                "GERMAN (SWISS)"        },
         {LANGUAGE_GERMAN_AUSTRIAN,         "GERMAN (AUSTRIAN)"     },
         {LANGUAGE_GERMAN_LUXEMBOURG,       "GERMAN (LUXEMBOURG)"   },
         {LANGUAGE_GERMAN_LIECHTENSTEIN,        "GERMAN (LIECHTENSTEIN)"},
         {LANGUAGE_GREEK,                   "GREEK"                 },
         {LANGUAGE_GUJARATI,                    "GUJARATI"              },
         {LANGUAGE_HEBREW,                  "HEBREW"                },
         {LANGUAGE_HINDI,                   "HINDI"                 },
         {LANGUAGE_HUNGARIAN,               "HUNGARIAN"             },
         {LANGUAGE_ICELANDIC,               "ICELANDIC"             },
         {LANGUAGE_INDONESIAN,              "INDONESIAN"            },
         {LANGUAGE_ITALIAN,                 "ITALIAN"               },
         {LANGUAGE_ITALIAN_SWISS,           "ITALIAN (SWISS)"       },
         {LANGUAGE_JAPANESE,                    "JAPANESE"              },
         {LANGUAGE_KANNADA,                 "KANNADA"               },
         {LANGUAGE_KASHMIRI,                    "KASHMIRI"              },
         {LANGUAGE_KASHMIRI_INDIA,          "KASHMIRI (INDIA)"      },
         {LANGUAGE_KAZAK,                   "KAZAK"                 },
         {LANGUAGE_KHMER,                   "KHMER"                 },
         {LANGUAGE_KIRGHIZ,                 "KIRGHIZ"               },
         {LANGUAGE_KONKANI,                 "KONKANI"               },
         {LANGUAGE_KOREAN,                  "KOREAN"                },
         {LANGUAGE_KOREAN_JOHAB,                "KOREAN (JOHAB)"        },
         {LANGUAGE_LAO,                     "LAO"                   },
         {LANGUAGE_LATVIAN,                 "LATVIAN"               },
         {LANGUAGE_LITHUANIAN,              "LITHUANIAN"            },
         {LANGUAGE_LITHUANIAN_CLASSIC,      "LITHUANIAN (CLASSIC)"  },
         {LANGUAGE_MACEDONIAN,              "MACEDONIAN"            },
         {LANGUAGE_MALAY,                   "MALAY"                 },
         {LANGUAGE_MALAY_MALAYSIA,          "MALAY (MALAYSIA)"      },
         {LANGUAGE_MALAY_BRUNEI_DARUSSALAM,  "MALAY (BRUNEI DARUSSALAM)"},
         {LANGUAGE_MALAYALAM,               "MALAYALAM"             },
         {LANGUAGE_MALTESE,                 "MALTESE"               },
         {LANGUAGE_MANIPURI,                    "MANIPURI"              },
         {LANGUAGE_MARATHI,                 "MARATHI"               },
         {LANGUAGE_MONGOLIAN,               "MONGOLIAN"             },
         {LANGUAGE_NEPALI,                  "NEPALI"                },
         {LANGUAGE_NEPALI_INDIA,                "NEPALI (INDIA)"        },
         {LANGUAGE_NORWEGIAN,               "NORWEGIAN"             },
         {LANGUAGE_NORWEGIAN_BOKMAL,            "NORWEGIAN (BOKMAL)"    },
         {LANGUAGE_NORWEGIAN_NYNORSK,       "NORWEGIAN (NYNORSK)"   },
         {LANGUAGE_ORIYA,                   "ORIYA"                 },
         {LANGUAGE_POLISH,                  "POLISH"                },
         {LANGUAGE_PORTUGUESE,              "PORTUGUESE"            },
         {LANGUAGE_PORTUGUESE_BRAZILIAN,        "PORTUGUESE (BRAZILIAN)"},
         {LANGUAGE_PUNJABI,                 "PUNJABI"               },
         {LANGUAGE_RHAETO_ROMAN,                "RHAETO (ROMAN)"        },
         {LANGUAGE_ROMANIAN,                    "ROMANIAN"              },
         {LANGUAGE_ROMANIAN_MOLDOVA,         "ROMANIAN (MOLDOVA)"    },
         {LANGUAGE_RUSSIAN,                 "RUSSIAN"               },
         {LANGUAGE_RUSSIAN_MOLDOVA,         "RUSSIAN (MOLDOVA)"     },
         {LANGUAGE_SAMI_LAPPISH,                "SAMI (LAPPISH)"        },
         {LANGUAGE_SANSKRIT,                    "SANSKRIT"              },
         {LANGUAGE_SERBIAN,                 "SERBIAN"               },
         {LANGUAGE_CROATIAN,                    "CROATIAN"              },
         {LANGUAGE_SERBIAN_LATIN,           "SERBIAN (LATIN)"       },
         {LANGUAGE_SERBIAN_CYRILLIC,            "SERBIAN (CYRILLIC)"    },
         {LANGUAGE_SESOTHO,                 "SESOTHO"               },
         {LANGUAGE_SINDHI,                  "SINDHI"                },
         {LANGUAGE_SLOVAK,                  "SLOVAK"                },
         {LANGUAGE_SLOVENIAN,               "SLOVENIAN"             },
         {LANGUAGE_SORBIAN,                 "SORBIAN"               },
         {LANGUAGE_SPANISH_DATED,           "SPANISH"               },
         {LANGUAGE_SPANISH_MEXICAN,         "SPANISH (MEXICAN)"     },
         {LANGUAGE_SPANISH_MODERN,          "SPANISH (MODERN)"      },
         {LANGUAGE_SPANISH_GUATEMALA,       "SPANISH (GUATEMALA)"   },
         {LANGUAGE_SPANISH_COSTARICA,       "SPANISH (COSTARICA)"   },
         {LANGUAGE_SPANISH_PANAMA,          "SPANISH (PANAMA)"      },
         {LANGUAGE_SPANISH_DOMINICAN_REPUBLIC,"SPANISH (DOMINICAN REPUBLIC)"},
         {LANGUAGE_SPANISH_VENEZUELA,        "SPANISH (VENEZUELA)"  },
         {LANGUAGE_SPANISH_COLOMBIA,         "SPANISH (COLOMBIA)"   },
         {LANGUAGE_SPANISH_PERU,             "SPANISH (PERU)"       },
         {LANGUAGE_SPANISH_ARGENTINA,        "SPANISH (ARGENTINA)"  },
         {LANGUAGE_SPANISH_ECUADOR,          "SPANISH (ECUADOR)"        },
         {LANGUAGE_SPANISH_CHILE,            "SPANISH (CHILE)"      },
         {LANGUAGE_SPANISH_URUGUAY,          "SPANISH (URUGUAY)"        },
         {LANGUAGE_SPANISH_PARAGUAY,         "SPANISH (PARAGUAY)"   },
         {LANGUAGE_SPANISH_BOLIVIA,          "SPANISH (BOLIVIA)"        },
         {LANGUAGE_SPANISH_EL_SALVADOR,      "SPANISH (EL SALVADOR)"    },
         {LANGUAGE_SPANISH_HONDURAS,         "SPANISH (HONDURAS)"   },
         {LANGUAGE_SPANISH_NICARAGUA,        "SPANISH (NICARAGUA)"  },
         {LANGUAGE_SPANISH_PUERTO_RICO,      "SPANISH (PUERTO RICO)"    },
         {LANGUAGE_SWAHILI,                  "SWAHILI"              },
         {LANGUAGE_SWEDISH,                  "SWEDISH"              },
         {LANGUAGE_SWEDISH_FINLAND,          "SWEDISH (FINLAND)"        },
         {LANGUAGE_TAJIK,                    "TAJIK"                    },
         {LANGUAGE_TAMIL,                    "TAMIL"                    },
         {LANGUAGE_TATAR,                    "TATAR"                    },
         {LANGUAGE_TELUGU,                   "TELUGU"               },
         {LANGUAGE_THAI,                     "THAI"                 },
         {LANGUAGE_TIBETAN,                  "TIBETAN"              },
         {LANGUAGE_TSONGA,                   "TSONGA"               },
         {LANGUAGE_TSWANA,                   "TSWANA"               },
         {LANGUAGE_TURKISH,                  "TURKISH"              },
         {LANGUAGE_TURKMEN,                  "TURKMEN"              },
         {LANGUAGE_UKRAINIAN,                "UKRAINIAN"                },
         {LANGUAGE_URDU,                     "URDU"                 },
         {LANGUAGE_URDU_PAKISTAN,            "URDU (PAKISTAN)"      },
         {LANGUAGE_URDU_INDIA,               "URDU (INDIA)"             },
         {LANGUAGE_UZBEK_LATIN,              "UZBEK (LATIN)"            },
         {LANGUAGE_UZBEK_CYRILLIC,           "UZBEK (CYRILLIC)"         },
         {LANGUAGE_VENDA,                    "VENDA"                    },
         {LANGUAGE_VIETNAMESE,               "VIETNAMESE"           },
         {LANGUAGE_WELSH,                    "WELSH"                    },
         {LANGUAGE_XHOSA,                    "XHOSA"                    },
         {LANGUAGE_ZULU,                     "ZULU"                 },
         {0,0}       //Last
     };
     return &aMapArr[0];
 }

 static LanguageType LanguageFromName(const String& rLngStr)
 {
     const LangNameMap *pStart = GetLangNameMap();
     LanguageType nRet = pStart->nId;

     for (const LangNameMap *pMap = pStart; pMap->pLanguageNm; ++pMap)
     {
         if (rLngStr.EqualsIgnoreCaseAscii(pMap->pLanguageNm))
         {
             nRet = pMap->nId;
             break;
         }
     }

     ASSERT(nRet != pStart->nId, "TXT: That was an unknown language!");

    return nRet;
 }


// for the automatic conversion (mail/news/...)
// The user data contains the options for the ascii import/export filter.
// The format is:
//	1. CharSet - as ascii chars
//	2. LineEnd - as CR/LR/CRLF
//	3. Fontname
//	4. Language
// the delimetercharacter is ","
//

 void SwAsciiOptions::ReadUserData( const String& rStr )
 {
    xub_StrLen nToken = 0;
    USHORT nCnt = 0;
    String sToken;
    do {
        if( 0 != (sToken = rStr.GetToken( 0, ',', nToken )).Len() )
        {
            switch( nCnt )
            {
            case 0:     // CharSet
                 eCharSet = CharSetFromName(sToken);
                break;
            case 1:     // LineEnd
                if( sToken.EqualsIgnoreCaseAscii( "CRLF" ))
                    eCRLF_Flag = LINEEND_CRLF;
                else if( sToken.EqualsIgnoreCaseAscii( "LF" ))
                    eCRLF_Flag = LINEEND_LF;
                else
                    eCRLF_Flag = LINEEND_CR;
                break;
            case 2:     // fontname
                sFont = sToken;
                break;
            case 3:     // Language
                 nLanguage = LanguageFromName(sToken);
                break;
            }
        }
        ++nCnt;
    } while( STRING_NOTFOUND != nToken );
 }

/* -----------------------------02.03.00 17:33--------------------------------

 ---------------------------------------------------------------------------*/
Color ConvertBrushStyle(const Color& rCol, const Color& rFillCol, BYTE nStyle)
{
    Color aColor = rCol;
    switch ( nStyle )
    {
    case SW_SV_BRUSH_25:
        {
            ULONG   nRed    = aColor.GetRed();
            ULONG   nGreen  = aColor.GetGreen();
            ULONG   nBlue   = aColor.GetBlue();
            nRed   += (ULONG)(rFillCol.GetRed())*2;
            nGreen += (ULONG)(rFillCol.GetGreen())*2;
            nBlue  += (ULONG)(rFillCol.GetBlue())*2;
            aColor = Color( (BYTE)(nRed/3), (BYTE)(nGreen/3), (BYTE)(nBlue/3) );
        }
        break;

    case SW_SV_BRUSH_50:
        {
            ULONG   nRed    = aColor.GetRed();
            ULONG   nGreen  = aColor.GetGreen();
            ULONG   nBlue   = aColor.GetBlue();
            nRed   += (ULONG)(rFillCol.GetRed());
            nGreen += (ULONG)(rFillCol.GetGreen());
            nBlue  += (ULONG)(rFillCol.GetBlue());
            aColor = Color( (BYTE)(nRed/2), (BYTE)(nGreen/2), (BYTE)(nBlue/2) );
        }
        break;

    case SW_SV_BRUSH_75:
        {
            ULONG   nRed    = aColor.GetRed()*2;
            ULONG   nGreen  = aColor.GetGreen()*2;
            ULONG   nBlue   = aColor.GetBlue()*2;
            nRed   += (ULONG)(rFillCol.GetRed());
            nGreen += (ULONG)(rFillCol.GetGreen());
            nBlue  += (ULONG)(rFillCol.GetBlue());
            aColor = Color( (BYTE)(nRed/3), (BYTE)(nGreen/3), (BYTE)(nBlue/3) );
        }
        break;

        case SW_SV_BRUSH_NULL:
            aColor = Color( COL_TRANSPARENT );
    }

    return aColor;
}

    void SwFilterOptions::Commit() {}
    void SwFilterOptions::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames ) {}

}
