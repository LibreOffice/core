/*************************************************************************
 *
 *  $RCSfile: fltini.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 14:17:21 $
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
#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include <string.h>
#include <stdio.h>          // sscanf

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif
#ifndef _PARHTML_HXX //autogen
#include <svtools/parhtml.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
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
#ifndef _IODETECT_HXX
#include <iodetect.hxx>
#endif
#ifndef _HINTS_HXX //autogen
#include <hints.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif

#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen wg. SvxBoxItem
#include <svx/boxitem.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
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

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

SwRead ReadRtf = 0, ReadAscii = 0, ReadSwg = 0, ReadSw3 = 0,
        ReadHTML = 0, ReadXML = 0;

inline BOOL IsDocShellRegistered() { return 0 != SwDocShell::_GetInterface(); }

IO_DETECT_IMPL1
IO_DETECT_IMPL2
IO_DETECT_IMPL3
IO_DETECT_IMPL4


inline void _SetFltPtr( USHORT& rPos, SwRead pReader
                        , const sal_Char* pNm
/* pNm optimiert der Compiler weg, wird nur in der nicht PRODUCT benoetigt! */
                        )
{
    ASSERT( !strcmp( aReaderWriter[ rPos ].pName, pNm ), "falscher Filter" );
    aReaderWriter[ rPos++ ].pReader = pReader;
}

void _InitFilter()
{
    SwRead pRd, pWW8Rd = new WW8Reader;

    USHORT nCnt = 0;
    _SetFltPtr( nCnt, (ReadSw3 = new Sw3Reader), FILTER_SW5 );
    _SetFltPtr( nCnt, ReadSw3, FILTER_SW4 );
    _SetFltPtr( nCnt, ReadSw3, FILTER_SW3 );
    _SetFltPtr( nCnt, (ReadSwg = new SwgReader), FILTER_SWG );
    _SetFltPtr( nCnt, ReadSwg, FILTER_SWGV );
    _SetFltPtr( nCnt, (ReadRtf = new RtfReader), FILTER_RTF );
    _SetFltPtr( nCnt, new Sw6Reader, sSwDos );
    _SetFltPtr( nCnt, (ReadAscii = new AsciiReader), STEXT );
    _SetFltPtr( nCnt, ReadAscii, FILTER_BAS );
    _SetFltPtr( nCnt, pWW8Rd, sWW6 );
    _SetFltPtr( nCnt, pWW8Rd, FILTER_WW8 );
    _SetFltPtr( nCnt, new W4WReader, FILTER_W4W );
    _SetFltPtr( nCnt, ReadRtf, sRtfWH );
    _SetFltPtr( nCnt, ( pRd = new ExcelReader ), sCExcel );
    _SetFltPtr( nCnt, pRd, sExcel );
    _SetFltPtr( nCnt, new LotusReader, sLotusD );
    _SetFltPtr( nCnt, (ReadHTML = new HTMLReader), sHTML);
    _SetFltPtr( nCnt, new WW1Reader, sWW1 );
    _SetFltPtr( nCnt, pWW8Rd, sWW5 );
    _SetFltPtr( nCnt, ReadSwg, sSwg1 );
    _SetFltPtr( nCnt, (ReadXML = new XMLReader), FILTER_XML );

#ifdef NEW_WW97_EXPORT
    aReaderWriter[ 9 ].fnGetWriter =  &::GetWW8Writer;
    aReaderWriter[ 10 ].fnGetWriter = &::GetWW8Writer;
#endif

#ifdef DEBUG_SH
    _SetFltPtr( nCnt, new Internal_W4WReader, sW4W_Int);
#endif // DEBUG_SH

#if !( defined(PRODUCT) || defined(MAC) || defined(PM2))
    nCnt += 2;      // haben keine Reader sind nur EXPORT!
#endif

    ASSERT( MAXFILTER == nCnt, "Anzahl Filter ungleich der Definierten" );
}




void _FinitFilter()
{
    // die Reader vernichten
    for( USHORT n = 0; n < MAXFILTER; ++n )
    {
        SwIoDetect& rIo = aReaderWriter[n];
        if( rIo.bDelReader && rIo.pReader )
            delete rIo.pReader;
    }
}


/*  */

void SwIoSystem::GetWriter( const String& rFltName, WriterRef& xRet )
{
    for( USHORT n = 0; n < MAXFILTER; ++n )
        if( aReaderWriter[n].IsFilter( rFltName ) )
        {
            aReaderWriter[n].GetWriter( rFltName, xRet );
            break;
        }
}


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
const SfxFilter* SwIoSystem::GetFilterOfFilterTxt( const String& rFilterNm,
                                const SfxFactoryFilterContainer* pCnt )
{
    const SfxFactoryFilterContainer* pFltCnt = pCnt ? pCnt :
        ( IsDocShellRegistered()
            ? SwDocShell::Factory().GetFilterContainer()
            : SwWebDocShell::Factory().GetFilterContainer() );

    do {
        if( pFltCnt )
        {
            const SfxFilter* pFilter;
            USHORT nCount = pFltCnt->GetFilterCount();
            for( USHORT i = 0; i < nCount; ++i )
                if( ( pFilter = pFltCnt->GetFilter( i ))->GetFilterName() == rFilterNm )
                    return pFilter;
        }
        if( pCnt || pFltCnt == SwWebDocShell::Factory().GetFilterContainer())
            break;
        pFltCnt = SwWebDocShell::Factory().GetFilterContainer();
    } while( TRUE );

    return 0;
}


/*  */

/////////////// die Storage Reader/Writer ////////////////////////////////

#if 0   // SH: WW8-Writer als Fake ueber WW6-Writer
Writer* GetWW8Writer( const String& )
{
    ASSERT( FALSE, "WinWord 97 - Writer ist noch nicht implementiert" );
    return 0;
}
#endif

void GetSw3Writer( const String&, WriterRef& xRet )
{
    xRet = new Sw3Writer;
}


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


ULONG Sw3Reader::Read( SwDoc &rDoc, SwPaM &rPam, const String & )
{
    ULONG nRet;
    if( pStg && pIO )
    {
        // TRUE: Vorlagen ueberschreiben
        pIO->SetReadOptions( aOpt,TRUE );
        if( !bInsertMode )
        {
            // Im Laden-Modus darf der PaM-Content-Teil nicht
            // in den Textbereich zeigen (Nodes koennen geloescht werden)
            rPam.GetBound( TRUE ).nContent.Assign( 0, 0 );
            rPam.GetBound( FALSE ).nContent.Assign( 0, 0 );
        }
        nRet = pIO->Load( pStg, bInsertMode ? &rPam : 0 );
        aOpt.ResetAllFmtsOnly();
        pIO->SetReadOptions( aOpt, TRUE );
    }
    else
    {
        ASSERT( !this, "Sw3-Read ohne Storage und/oder IO-System" );
        nRet = ERR_SWG_READ_ERROR;
    }
    return nRet;
}


ULONG Sw3Writer::WriteStorage()
{
    ULONG nRet;
    if( pIO )
    {
        // der gleiche Storage -> Save, sonst SaveAs aufrufen
        if( !bSaveAs )
            nRet = pIO->Save( pOrigPam, bWriteAll );
        else
            nRet = pIO->SaveAs( pStg, pOrigPam, bWriteAll );

        pIO = 0;        // nach dem Schreiben ist der Pointer ungueltig !!
    }
    else
    {
        ASSERT( !this, "Sw3-Writer ohne IO-System" )
        nRet = ERR_SWG_WRITE_ERROR;
    }
    return nRet;
}

BOOL Sw3Writer::IsSw3Writer() const { return TRUE; }


void Writer::SetPasswd( const String& ) {}


void Writer::SetVersion( const String&, long ) {}


BOOL Writer::IsStgWriter() const { return FALSE; }
BOOL Writer::IsSw3Writer() const { return FALSE; }

BOOL StgWriter::IsStgWriter() const { return TRUE; }

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

SwFilterOptions::SwFilterOptions( sal_uInt16 nCnt, const sal_Char** ppNames,
                                                      sal_uInt32* pValues )
    : ConfigItem( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM(
                                "Office.Writer/FilterFlags" ) ))
{
    Sequence<OUString> aNames( nCnt );
    OUString* pNames = aNames.getArray();
    for( USHORT n = 0; n < nCnt; ++n )
        pNames[ n ] = OUString::createFromAscii( ppNames[ n ] );
    Sequence<Any> aValues = GetProperties( aNames );

    if( nCnt == aValues.getLength() )
    {
        const Any* pAnyValues = aValues.getConstArray();
        for( n = 0; n < nCnt; ++n )
            pValues[ n ] = pAnyValues[ n ].hasValue()
                            ? *(sal_uInt32*)pAnyValues[ n ].getValue()
                            : 0;
    }
    else
        for( n = 0; n < nCnt; ++n )
            pValues[ n ] = 0;
}

/*  */


void LotusReader::SetFltName( const String& rFltNm )
{
    eCodeSet = rFltNm.EqualsAscii( sLotusD ) ? RTL_TEXTENCODING_IBM_850
                                             : RTL_TEXTENCODING_MS_1252;
}


void StgReader::SetFltName( const String& rFltNm )
{
    if( SW_STORAGE_READER & GetReaderType() )
        aFltName = rFltNm;
}


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
        USHORT nLeft = rFmt.GetAbsLSpace(), nParaLeft = rLR.GetTxtLeft();
        if( 0 < rLR.GetTxtFirstLineOfst() )
            nParaLeft += rLR.GetTxtFirstLineOfst();
        else if( nLeft < nParaLeft )
            nParaLeft -= nLeft;
        else
            nParaLeft = 0;
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


void CalculateFlySize( SfxItemSet& rFlySet, SwNodeIndex& rAnchor,
                          SwTwips nPageWidth )
{
    const SfxPoolItem* pItem = 0;
    if( SFX_ITEM_SET != rFlySet.GetItemState( RES_FRM_SIZE, TRUE, &pItem ) ||
        MINFLY > ((SwFmtFrmSize*)pItem)->GetWidth() )
    {
        SwFmtFrmSize aSz( (SwFmtFrmSize&) (pItem ? *pItem
                                    : rFlySet.Get( RES_FRM_SIZE, TRUE )) );

        SwTwips nWidth;
        // dann die Breite des Flys selbst bestimmen. Ist eine Tabelle
        // defininiert, dann benutze deren Breite, sonst die Breite der
        // Seite
        const SwTableNode* pTblNd = rAnchor.GetNode().FindTableNode();
        if( pTblNd )
            nWidth = pTblNd->GetTable().GetFrmFmt()->GetFrmSize().GetWidth();
        else
            nWidth = nPageWidth;

        const SwNodeIndex* pSttNd = ((SwFmtCntnt&)rFlySet.Get( RES_CNTNT )).
                                    GetCntntIdx();
        if( pSttNd )
        {
            BOOL bOnlyOneNode = TRUE;
            ULONG nMinFrm = 0;
            ULONG nMaxFrm = 0;
            SwTxtNode* pFirstTxtNd = 0;
            SwNodeIndex aIdx( *pSttNd, 1 );
            SwNodeIndex aEnd( *pSttNd->GetNode().EndOfSectionNode() );
            while( aIdx < aEnd )
            {
                SwTxtNode *pTxtNd = aIdx.GetNode().GetTxtNode();
                if( pTxtNd )
                {
                    if( !pFirstTxtNd )
                        pFirstTxtNd = pTxtNd;
                    else if( pFirstTxtNd != pTxtNd )
                    {
                        // forget it
                        bOnlyOneNode = FALSE;
                        break;
                    }

                    ULONG nAbsMinCnts;
                    pTxtNd->GetMinMaxSize( aIdx.GetIndex(), nMinFrm,
                                            nMaxFrm, nAbsMinCnts );
                }
                aIdx++;
            }

            if( bOnlyOneNode )
            {
                if( nMinFrm < MINLAY && pFirstTxtNd )
                {
                    // if the first node dont contained any content, then
                    // insert one char in it calc again and delete once again
                    SwIndex aNdIdx( pFirstTxtNd );
                    pFirstTxtNd->Insert( String::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM( "MM" )), aNdIdx );
                    ULONG nAbsMinCnts;
                    pFirstTxtNd->GetMinMaxSize( pFirstTxtNd->GetIndex(),
                                            nMinFrm, nMaxFrm, nAbsMinCnts );
                    aNdIdx -= 2;
                    pFirstTxtNd->Erase( aNdIdx, 2 );
                }

                // Umrandung und Abstand zum Inhalt beachten
                const SvxBoxItem& rBoxItem = (SvxBoxItem&)rFlySet.Get( RES_BOX );
                USHORT nLine = BOX_LINE_LEFT;
                for( int i = 0; i < 2; ++i )
                {
                    const SvxBorderLine* pLn = rBoxItem.GetLine( nLine );
                    if( pLn )
                    {
                        USHORT nWidth = pLn->GetOutWidth() + pLn->GetInWidth();
                        nWidth += rBoxItem.GetDistance( nLine );
                        nMinFrm += nWidth;
                        nMaxFrm += nWidth;
                    }
                    nLine = BOX_LINE_RIGHT;
                }

                // Mindestbreite fuer Inhalt einhalten
                if( nMinFrm < MINLAY )
                    nMinFrm = MINLAY;
                if( nMaxFrm < MINLAY )
                    nMaxFrm = MINLAY;

                if( nWidth > (USHORT)nMaxFrm )
                    nWidth = nMaxFrm;
                else if( nWidth > (USHORT)nMinFrm )
                    nWidth = nMinFrm;
            }
        }

        if( MINFLY > nWidth )
            nWidth = MINFLY;

        aSz.SetWidth( nWidth );
        if( MINFLY > aSz.GetHeight() )
            aSz.SetHeight( MINFLY );
        rFlySet.Put( aSz );
    }
    else if( MINFLY > ((SwFmtFrmSize*)pItem)->GetHeight() )
    {
        SwFmtFrmSize aSz( *(SwFmtFrmSize*)pItem );
        aSz.SetHeight( MINFLY );
        rFlySet.Put( aSz );
    }
}

/*  */


static BOOL lcl_FindCharSet( BOOL bSearchId, String& rChrSetStr,
                                            rtl_TextEncoding& rChrSet )
{
    static const sal_Char
        sToken001[] = "ANSI",
        sToken002[] = "MAC",
        sToken003[] = "DOS",
        sToken004[] = "IBM_437",
        sToken005[] = "IBM_860",
        sToken006[] = "IBM_861",
        sToken007[] = "IBM_863",
        sToken008[] = "IBM_865",
        sToken009[] = "ASCII_US",
        sToken010[] = "ISO_8859_1",
        sToken011[] = "ISO_8859_2",
        sToken012[] = "ISO_8859_3",
        sToken013[] = "ISO_8859_4",
        sToken014[] = "ISO_8859_5",
        sToken015[] = "ISO_8859_6",
        sToken016[] = "ISO_8859_7",
        sToken017[] = "ISO_8859_8",
        sToken018[] = "ISO_8859_9",
        sToken019[] = "ISO_8859_14",
        sToken020[] = "ISO_8859_15",
        sToken021[] = "IBM_737",
        sToken022[] = "IBM_775",
        sToken023[] = "IBM_852",
        sToken024[] = "IBM_855",
        sToken025[] = "IBM_857",
        sToken026[] = "IBM_862",
        sToken027[] = "IBM_864",
        sToken028[] = "IBM_866",
        sToken029[] = "IBM_869",
        sToken030[] = "MS_874",
        sToken031[] = "MS_1250",
        sToken032[] = "MS_1251",
        sToken033[] = "MS_1253",
        sToken034[] = "MS_1254",
        sToken035[] = "MS_1255",
        sToken036[] = "MS_1256",
        sToken037[] = "MS_1257",
        sToken038[] = "MS_1258",
        sToken039[] = "APPLE_ARABIC",
        sToken040[] = "APPLE_CENTEURO",
        sToken041[] = "APPLE_CROATIAN",
        sToken042[] = "APPLE_CYRILLIC",
        sToken043[] = "APPLE_DEVANAGARI",
        sToken044[] = "APPLE_FARSI",
        sToken045[] = "APPLE_GREEK",
        sToken046[] = "APPLE_GUJARATI",
        sToken047[] = "APPLE_GURMUKHI",
        sToken048[] = "APPLE_HEBREW",
        sToken049[] = "APPLE_ICELAND",
        sToken050[] = "APPLE_ROMANIAN",
        sToken051[] = "APPLE_THAI",
        sToken052[] = "APPLE_TURKISH",
        sToken053[] = "APPLE_UKRAINIAN",
        sToken054[] = "APPLE_CHINSIMP",
        sToken055[] = "APPLE_CHINTRAD",
        sToken056[] = "APPLE_JAPANESE",
        sToken057[] = "APPLE_KOREAN",
        sToken058[] = "MS_932",
        sToken059[] = "MS_936",
        sToken060[] = "MS_949",
        sToken061[] = "MS_950",
        sToken062[] = "SHIFT_JIS",
        sToken063[] = "GB_2312",
        sToken064[] = "GBT_12345",
        sToken065[] = "GBK",
        sToken066[] = "BIG5",
        sToken067[] = "EUC_JP",
        sToken068[] = "EUC_CN",
        sToken069[] = "EUC_TW",
        sToken070[] = "ISO_2022_JP",
        sToken071[] = "ISO_2022_CN",
        sToken072[] = "KOI8_R",
        sToken073[] = "UTF7",
        sToken074[] = "UTF8",
        sToken075[] = "ISO_8859_10",
        sToken076[] = "ISO_8859_13",
        sToken077[] = "EUC_KR",
        sToken078[] = "ISO_2022_KR",
        sToken079[] = "UNICODE_2"
            ;
    struct _Dummy_MAP
    {
        rtl_TextEncoding eCode;
        const sal_Char* pChrSetNm;
    };
    static const _Dummy_MAP aMapArr[] = {

        RTL_TEXTENCODING_MS_1252,               sToken001,
        RTL_TEXTENCODING_APPLE_ROMAN,           sToken002,
        RTL_TEXTENCODING_IBM_850,               sToken003,
        RTL_TEXTENCODING_IBM_437,               sToken004,
        RTL_TEXTENCODING_IBM_860,               sToken005,
        RTL_TEXTENCODING_IBM_861,               sToken006,
        RTL_TEXTENCODING_IBM_863,               sToken007,
        RTL_TEXTENCODING_IBM_865,               sToken008,
        RTL_TEXTENCODING_ASCII_US,              sToken009,
        RTL_TEXTENCODING_ISO_8859_1,            sToken010,
        RTL_TEXTENCODING_ISO_8859_2,            sToken011,
        RTL_TEXTENCODING_ISO_8859_3,            sToken012,
        RTL_TEXTENCODING_ISO_8859_4,            sToken013,
        RTL_TEXTENCODING_ISO_8859_5,            sToken014,
        RTL_TEXTENCODING_ISO_8859_6,            sToken015,
        RTL_TEXTENCODING_ISO_8859_7,            sToken016,
        RTL_TEXTENCODING_ISO_8859_8,            sToken017,
        RTL_TEXTENCODING_ISO_8859_9,            sToken018,
        RTL_TEXTENCODING_ISO_8859_14,           sToken019,
        RTL_TEXTENCODING_ISO_8859_15,           sToken020,
        RTL_TEXTENCODING_IBM_737,               sToken021,
        RTL_TEXTENCODING_IBM_775,               sToken022,
        RTL_TEXTENCODING_IBM_852,               sToken023,
        RTL_TEXTENCODING_IBM_855,               sToken024,
        RTL_TEXTENCODING_IBM_857,               sToken025,
        RTL_TEXTENCODING_IBM_862,               sToken026,
        RTL_TEXTENCODING_IBM_864,               sToken027,
        RTL_TEXTENCODING_IBM_866,               sToken028,
        RTL_TEXTENCODING_IBM_869,               sToken029,
        RTL_TEXTENCODING_MS_874,                sToken030,
        RTL_TEXTENCODING_MS_1250,               sToken031,
        RTL_TEXTENCODING_MS_1251,               sToken032,
        RTL_TEXTENCODING_MS_1253,               sToken033,
        RTL_TEXTENCODING_MS_1254,               sToken034,
        RTL_TEXTENCODING_MS_1255,               sToken035,
        RTL_TEXTENCODING_MS_1256,               sToken036,
        RTL_TEXTENCODING_MS_1257,               sToken037,
        RTL_TEXTENCODING_MS_1258,               sToken038,
        RTL_TEXTENCODING_APPLE_ARABIC,          sToken039,
        RTL_TEXTENCODING_APPLE_CENTEURO,        sToken040,
        RTL_TEXTENCODING_APPLE_CROATIAN,        sToken041,
        RTL_TEXTENCODING_APPLE_CYRILLIC,        sToken042,
        RTL_TEXTENCODING_APPLE_DEVANAGARI,      sToken043,
        RTL_TEXTENCODING_APPLE_FARSI,           sToken044,
        RTL_TEXTENCODING_APPLE_GREEK,           sToken045,
        RTL_TEXTENCODING_APPLE_GUJARATI,        sToken046,
        RTL_TEXTENCODING_APPLE_GURMUKHI,        sToken047,
        RTL_TEXTENCODING_APPLE_HEBREW,          sToken048,
        RTL_TEXTENCODING_APPLE_ICELAND,         sToken049,
        RTL_TEXTENCODING_APPLE_ROMANIAN,        sToken050,
        RTL_TEXTENCODING_APPLE_THAI,            sToken051,
        RTL_TEXTENCODING_APPLE_TURKISH,         sToken052,
        RTL_TEXTENCODING_APPLE_UKRAINIAN,       sToken053,
        RTL_TEXTENCODING_APPLE_CHINSIMP,        sToken054,
        RTL_TEXTENCODING_APPLE_CHINTRAD,        sToken055,
        RTL_TEXTENCODING_APPLE_JAPANESE,        sToken056,
        RTL_TEXTENCODING_APPLE_KOREAN,          sToken057,
        RTL_TEXTENCODING_MS_932,                sToken058,
        RTL_TEXTENCODING_MS_936,                sToken059,
        RTL_TEXTENCODING_MS_949,                sToken060,
        RTL_TEXTENCODING_MS_950,                sToken061,
        RTL_TEXTENCODING_SHIFT_JIS,             sToken062,
        RTL_TEXTENCODING_GB_2312,               sToken063,
        RTL_TEXTENCODING_GBT_12345,             sToken064,
        RTL_TEXTENCODING_GBK,                   sToken065,
        RTL_TEXTENCODING_BIG5,                  sToken066,
        RTL_TEXTENCODING_EUC_JP,                sToken067,
        RTL_TEXTENCODING_EUC_CN,                sToken068,
        RTL_TEXTENCODING_EUC_TW,                sToken069,
        RTL_TEXTENCODING_ISO_2022_JP,           sToken070,
        RTL_TEXTENCODING_ISO_2022_CN,           sToken071,
        RTL_TEXTENCODING_KOI8_R,                sToken072,
        RTL_TEXTENCODING_UTF7,                  sToken073,
        RTL_TEXTENCODING_UTF8,                  sToken074,
        RTL_TEXTENCODING_ISO_8859_10,           sToken075,
        RTL_TEXTENCODING_ISO_8859_13,           sToken076,
        RTL_TEXTENCODING_EUC_KR,                sToken077,
        RTL_TEXTENCODING_ISO_2022_KR,           sToken078,
        RTL_TEXTENCODING_UCS2,                  sToken079
    };

    BOOL bFnd = FALSE;
    USHORT nLen = sizeof( aMapArr ) / sizeof( aMapArr[0] );
    if( bSearchId )
    {
        for( USHORT n = 0; n < nLen; ++n )
            if( rChrSetStr.EqualsIgnoreCaseAscii( aMapArr[ n ].pChrSetNm ))
            {
                rChrSet = aMapArr[ n ].eCode;
                bFnd = TRUE;
                break;
            }
    }
    else
    {
        for( USHORT n = 0; n < nLen; ++n )
            if( rChrSet == aMapArr[ n ].eCode )
            {
                rChrSetStr.AssignAscii( aMapArr[ n ].pChrSetNm  );
                bFnd = TRUE;
                break;
            }
    }
    return bFnd;
}

static BOOL lcl_FindLanguage( BOOL bSearchId, String& rLngStr, USHORT& rId )
{
    static const sal_Char
        sToken001[] = "UNKNOWN",
        sToken002[] = "SYSTEM",
        sToken003[] = "AFRIKAANS",
        sToken004[] = "ALBANIAN",
        sToken005[] = "ARABIC",
        sToken006[] = "ARABIC (IRAQ)",
        sToken007[] = "ARABIC (EGYT)",
        sToken008[] = "ARABIC (LIBYA)",
        sToken009[] = "ARABIC (ALGERIA)",
        sToken010[] = "ARABIC (MOROCCO)",
        sToken011[] = "ARABIC (TUNESIA)",
        sToken012[] = "ARABIC (OMAN)",
        sToken013[] = "ARABIC (YEMEN)",
        sToken014[] = "ARABIC (SYRIA)",
        sToken015[] = "ARABIC (JORDAN)",
        sToken016[] = "ARABIC (LEBANON)",
        sToken017[] = "ARABIC (KUWAIT)",
        sToken018[] = "ARABIC (VAE)",
        sToken019[] = "ARABIC (BAHREIN)",
        sToken020[] = "ARABIC (QATAR)",
        sToken021[] = "BASQUE",
        sToken022[] = "BULGARIAN",
        sToken023[] = "BELORUSSIAN",
        sToken024[] = "CATALAN",
        sToken025[] = "CHINESE",
        sToken026[] = "CHINESE (TRAD.)",
        sToken027[] = "CHINESE (SIMPLE)",
        sToken028[] = "CHINESE (HONGKONG)",
        sToken029[] = "CHINESE (SINGAPORE)",
        sToken030[] = "CROATIAN",
        sToken031[] = "CHECH",
        sToken032[] = "DANISH",
        sToken033[] = "DUTCH",
        sToken034[] = "DUTCH (BELGIUM)",
        sToken035[] = "ENGLISH",
        sToken036[] = "ENGLISH (US)",
        sToken037[] = "ENGLISH (UK)",
        sToken038[] = "ENGLISH (AUS)",
        sToken039[] = "ENGLISH (CAN)",
        sToken040[] = "ENGLISH (NZ)",
        sToken041[] = "ENGLISH (EIRE)",
        sToken042[] = "ENGLISH (SA)",
        sToken043[] = "ENGLISH (JAMAICA)",
        sToken044[] = "ENGLISH (CARIBBEAN)",
        sToken045[] = "ENGLISH (BELIZE)",
        sToken046[] = "ENGLISH (TRINIDAD)",
        sToken047[] = "ENGLISH (ZIMBABWE)",
        sToken048[] = "ENGLISH (PHILIPPINES)",
        sToken049[] = "ESTONIAN",
        sToken050[] = "FINNISH",
        sToken051[] = "FAROAN",
        sToken052[] = "FARSI",
        sToken053[] = "FRENCH",
        sToken054[] = "FRENCH (BELGIUM)",
        sToken055[] = "FRENCH (CAN)",
        sToken056[] = "FRENCH (CH)",
        sToken057[] = "FRENCH (LUX)",
        sToken058[] = "FRENCH (MONACO)",
        sToken059[] = "GERMAN",
        sToken060[] = "GERMAN (CH)",
        sToken061[] = "GERMAN (A)",
        sToken062[] = "GERMAN (LUX)",
        sToken063[] = "GERMAN (LIE)",
        sToken064[] = "GREEK",
        sToken065[] = "HEBREW",
        sToken066[] = "HUNGARIAN",
        sToken067[] = "ICELANDIC",
        sToken068[] = "INDONESIAN",
        sToken069[] = "ITALIAN",
        sToken070[] = "ITALIAN (CH)",
        sToken071[] = "JAPANESE",
        sToken072[] = "KOREAN",
        sToken073[] = "KOREAN (JOHAB)",
        sToken074[] = "LATVIAN",
        sToken075[] = "LITHUANIAN",
        sToken076[] = "MACEDONIAN",
        sToken077[] = "MALAYSIAN",
        sToken078[] = "NORWEGIAN",
        sToken079[] = "NORWEGIAN BOKMAL",
        sToken080[] = "NORWEGIAN NYNORSK",
        sToken081[] = "POLISH",
        sToken082[] = "PORTUGUESE",
        sToken083[] = "PORTUGUESE (BRAZIL)",
        sToken084[] = "ROMANSCH",
        sToken085[] = "RUMANIAN",
        sToken086[] = "RUSSIAN",
        sToken087[] = "SLOVAK",
        sToken088[] = "SLOVENIAN",
        sToken089[] = "SERBIAN",
        sToken090[] = "SPANISH",
        sToken091[] = "SPANISH (MEXICO)",
        sToken092[] = "SPANISH (MODERN)",
        sToken093[] = "SPANISH (GUATEMALA)",
        sToken094[] = "SPANISH (COSTA RICA)",
        sToken095[] = "SPANISH (PANAMA)",
//  sToken096[] = "SPANISH (DOMINICAN REP.)",
        sToken097[] = "SPANISH (VENEZUELA)",
        sToken098[] = "SPANISH (COLUMBIA)",
        sToken099[] = "SPANISH (PERU)",
        sToken100[] = "SPANISH (ARGENTINA)",
        sToken101[] = "SPANISH (EQUADOR)",
        sToken102[] = "SPANISH (CHILE)",
        sToken103[] = "SPANISH (URUGUAY)",
        sToken104[] = "SPANISH (PARAGUAY)",
        sToken105[] = "SPANISH (BOLIVIA)",
        sToken106[] = "SPANISH (EL SALVADOR)",
        sToken107[] = "SPANISH (HONDURAS)",
        sToken108[] = "SPANISH (NICARAGUA)",
        sToken109[] = "SPANISH (PUERTO RICO)",
        sToken110[] = "SWEDISH",
        sToken111[] = "SWEDISH (FINLAND)",
        sToken112[] = "THAI",
        sToken113[] = "TURKISH",
        sToken114[] = "URDU",
        sToken115[] = "UKRAINIAN",

        sToken116[] = "ARABIC (SAUDI ARABIA)",
        sToken117[] = "ARMENIAN",
        sToken118[] = "ASSAMESE",
        sToken119[] = "AZERI",
        sToken120[] = "AZERI (LATIN)",
        sToken121[] = "AZERI (CYRILLIC)",
        sToken122[] = "BENGALI",
        sToken123[] = "CHINESE (MACAU)",
        sToken124[] = "GUJARATI",
        sToken125[] = "HINDI",
        sToken126[] = "KANNADA",
        sToken127[] = "KASHMIRI",
        sToken128[] = "KASHMIRI (INDIA)",
        sToken129[] = "KAZAK",
        sToken130[] = "KONKANI",
        sToken131[] = "LITHUANIAN (CLASSIC)",
        sToken132[] = "MALAY (MALAYSIA)",
        sToken133[] = "MALAY (BRUNEI DARUSSALAM)",
        sToken134[] = "MALAYALAM",
        sToken135[] = "MANIPURI",
        sToken136[] = "MARATHI",
        sToken137[] = "NEPALI",
        sToken138[] = "NEPALI (INDIA)",
        sToken139[] = "ORIYA",
        sToken140[] = "PUNJABI",
        sToken141[] = "SANSKRIT",
        sToken142[] = "SERBIAN",
        sToken143[] = "SERBIAN (LATIN)",
        sToken144[] = "SERBIAN (CYRILLIC)",
        sToken145[] = "SINDHI",
        sToken146[] = "SWAHILI",
        sToken147[] = "TAMIL",
        sToken148[] = "TATAR",
        sToken149[] = "TELUGU",
        sToken150[] = "URDU (PAKISTAN)",
        sToken151[] = "URDU (INDIA)",
        sToken152[] = "UZBEK",
        sToken153[] = "UZBEK (LATIN)",
        sToken154[] = "UZBEK (CYRILLIC)"
            ;
    struct _Dummy_MAP
    {
        USHORT nId;
        const sal_Char* pLanguageNm;
    };
    static const _Dummy_MAP aMapArr[] = {

         LANGUAGE_DONTKNOW,                 sToken001,
         LANGUAGE_SYSTEM,                   sToken002,
         LANGUAGE_AFRIKAANS,                sToken003,
         LANGUAGE_ALBANIAN,                 sToken004,
         LANGUAGE_ARABIC,                   sToken005,
         LANGUAGE_ARABIC_IRAQ,              sToken006,
         LANGUAGE_ARABIC_EGYPT,             sToken007,
         LANGUAGE_ARABIC_LIBYA,             sToken008,
         LANGUAGE_ARABIC_ALGERIA,           sToken009,
         LANGUAGE_ARABIC_MOROCCO,           sToken010,
         LANGUAGE_ARABIC_TUNISIA,           sToken011,
         LANGUAGE_ARABIC_OMAN,              sToken012,
         LANGUAGE_ARABIC_YEMEN,             sToken013,
         LANGUAGE_ARABIC_SYRIA,             sToken014,
         LANGUAGE_ARABIC_JORDAN,            sToken015,
         LANGUAGE_ARABIC_LEBANON,           sToken016,
         LANGUAGE_ARABIC_KUWAIT,            sToken017,
         LANGUAGE_ARABIC_UAE,               sToken018,
         LANGUAGE_ARABIC_BAHRAIN,           sToken019,
         LANGUAGE_ARABIC_QATAR,             sToken020,
         LANGUAGE_BASQUE,                   sToken021,
         LANGUAGE_BULGARIAN,                sToken022,
         LANGUAGE_BELARUSIAN,               sToken023,
         LANGUAGE_CATALAN,                  sToken024,
         LANGUAGE_CHINESE,                  sToken025,
         LANGUAGE_CHINESE_TRADITIONAL,      sToken026,
         LANGUAGE_CHINESE_SIMPLIFIED,       sToken027,
         LANGUAGE_CHINESE_HONGKONG,         sToken028,
         LANGUAGE_CHINESE_SINGAPORE,        sToken029,
         LANGUAGE_CROATIAN,                 sToken030,
         LANGUAGE_CZECH,                    sToken031,
         LANGUAGE_DANISH,                   sToken032,
         LANGUAGE_DUTCH,                    sToken033,
         LANGUAGE_DUTCH_BELGIAN,            sToken034,
         LANGUAGE_ENGLISH,                  sToken035,
         LANGUAGE_ENGLISH_US,               sToken036,
         LANGUAGE_ENGLISH_UK,               sToken037,
         LANGUAGE_ENGLISH_AUS,              sToken038,
         LANGUAGE_ENGLISH_CAN,              sToken039,
         LANGUAGE_ENGLISH_NZ,               sToken040,
         LANGUAGE_ENGLISH_EIRE,             sToken041,
         LANGUAGE_ENGLISH_SAFRICA,          sToken042,
         LANGUAGE_ENGLISH_JAMAICA,          sToken043,
         LANGUAGE_ENGLISH_CARRIBEAN,        sToken044,
         LANGUAGE_ENGLISH_BELIZE,           sToken045,
         LANGUAGE_ENGLISH_TRINIDAD,         sToken046,
         LANGUAGE_ENGLISH_ZIMBABWE,         sToken047,
         LANGUAGE_ENGLISH_PHILIPPINES,      sToken048,
         LANGUAGE_ESTONIAN,                 sToken049,
         LANGUAGE_FINNISH,                  sToken050,
         LANGUAGE_FAEROESE,                 sToken051,
         LANGUAGE_FARSI,                    sToken052,
         LANGUAGE_FRENCH,                   sToken053,
         LANGUAGE_FRENCH_BELGIAN,           sToken054,
         LANGUAGE_FRENCH_CANADIAN,          sToken055,
         LANGUAGE_FRENCH_SWISS,             sToken056,
         LANGUAGE_FRENCH_LUXEMBOURG,        sToken057,
         LANGUAGE_FRENCH_MONACO,            sToken058,
         LANGUAGE_GERMAN,                   sToken059,
         LANGUAGE_GERMAN_SWISS,             sToken060,
         LANGUAGE_GERMAN_AUSTRIAN,          sToken061,
         LANGUAGE_GERMAN_LUXEMBOURG,        sToken062,
         LANGUAGE_GERMAN_LIECHTENSTEIN,     sToken063,
         LANGUAGE_GREEK,                    sToken064,
         LANGUAGE_HEBREW,                   sToken065,
         LANGUAGE_HUNGARIAN,                sToken066,
         LANGUAGE_ICELANDIC,                sToken067,
         LANGUAGE_INDONESIAN,               sToken068,
         LANGUAGE_ITALIAN,                  sToken069,
         LANGUAGE_ITALIAN_SWISS,            sToken070,
         LANGUAGE_JAPANESE,                 sToken071,
         LANGUAGE_KOREAN,                   sToken072,
         LANGUAGE_KOREAN_JOHAB,             sToken073,
         LANGUAGE_LATVIAN,                  sToken074,
         LANGUAGE_LITHUANIAN,               sToken075,
         LANGUAGE_MACEDONIAN,               sToken076,
         LANGUAGE_MALAY,                    sToken077,
         LANGUAGE_NORWEGIAN,                sToken078,
         LANGUAGE_NORWEGIAN_BOKMAL,         sToken079,
         LANGUAGE_NORWEGIAN_NYNORSK,        sToken080,
         LANGUAGE_POLISH,                   sToken081,
         LANGUAGE_PORTUGUESE,               sToken082,
         LANGUAGE_PORTUGUESE_BRAZILIAN,     sToken083,
         LANGUAGE_RHAETO_ROMAN,             sToken084,
         LANGUAGE_ROMANIAN,                 sToken085,
         LANGUAGE_RUSSIAN,                  sToken086,
         LANGUAGE_SLOVAK,                   sToken087,
         LANGUAGE_SLOVENIAN,                sToken088,
         LANGUAGE_SORBIAN,                  sToken089,
         LANGUAGE_SPANISH,                  sToken090,
         LANGUAGE_SPANISH_MEXICAN,          sToken091,
         LANGUAGE_SPANISH_MODERN,           sToken092,
         LANGUAGE_SPANISH_GUATEMALA,        sToken093,
         LANGUAGE_SPANISH_COSTARICA,        sToken094,
         LANGUAGE_SPANISH_PANAMA,           sToken095,
//       LANGUAGE_SPANISH_DOMINICAN,        sToken096,
         LANGUAGE_SPANISH_VENEZUELA,        sToken097,
         LANGUAGE_SPANISH_COLOMBIA,         sToken098,
         LANGUAGE_SPANISH_PERU,             sToken099,
         LANGUAGE_SPANISH_ARGENTINA,        sToken100,
         LANGUAGE_SPANISH_ECUADOR,          sToken101,
         LANGUAGE_SPANISH_CHILE,            sToken102,
         LANGUAGE_SPANISH_URUGUAY,          sToken103,
         LANGUAGE_SPANISH_PARAGUAY,         sToken104,
         LANGUAGE_SPANISH_BOLIVIA,          sToken105,
         LANGUAGE_SPANISH_EL_SALVADOR,      sToken106,
         LANGUAGE_SPANISH_HONDURAS,         sToken107,
         LANGUAGE_SPANISH_NICARAGUA,        sToken108,
         LANGUAGE_SPANISH_PUERTO_RICO,      sToken109,
         LANGUAGE_SWEDISH,                  sToken110,
         LANGUAGE_SWEDISH_FINLAND,          sToken111,
         LANGUAGE_THAI,                     sToken112,
         LANGUAGE_TURKISH,                  sToken113,
         LANGUAGE_URDU,                     sToken114,
         LANGUAGE_UKRAINIAN,                sToken115,
         LANGUAGE_ARABIC_SAUDI_ARABIA,      sToken116,
         LANGUAGE_ARMENIAN,                 sToken117,
         LANGUAGE_ASSAMESE,                 sToken118,
         LANGUAGE_AZERI,                    sToken119,
         LANGUAGE_AZERI_LATIN,              sToken120,
         LANGUAGE_AZERI_CYRILLIC,           sToken121,
         LANGUAGE_BENGALI,                  sToken122,
         LANGUAGE_CHINESE_MACAU,            sToken123,
         LANGUAGE_GUJARATI,                 sToken124,
         LANGUAGE_HINDI,                    sToken125,
         LANGUAGE_KANNADA,                  sToken126,
         LANGUAGE_KASHMIRI,                 sToken127,
         LANGUAGE_KASHMIRI_INDIA,           sToken128,
         LANGUAGE_KAZAK,                    sToken129,
         LANGUAGE_KONKANI,                  sToken130,
         LANGUAGE_LITHUANIAN_CLASSIC,       sToken131,
         LANGUAGE_MALAY_MALAYSIA,           sToken132,
         LANGUAGE_MALAY_BRUNEI_DARUSSALAM,  sToken133,
         LANGUAGE_MALAYALAM,                sToken134,
         LANGUAGE_MANIPURI,                 sToken135,
         LANGUAGE_MARATHI,                  sToken136,
         LANGUAGE_NEPALI,                   sToken137,
         LANGUAGE_NEPALI_INDIA,             sToken138,
         LANGUAGE_ORIYA,                    sToken139,
         LANGUAGE_PUNJABI,                  sToken140,
         LANGUAGE_SANSKRIT,                 sToken141,
         LANGUAGE_SERBIAN,                  sToken142,
         LANGUAGE_SERBIAN_LATIN,            sToken143,
         LANGUAGE_SERBIAN_CYRILLIC,         sToken144,
         LANGUAGE_SINDHI,                   sToken145,
         LANGUAGE_SWAHILI,                  sToken146,
         LANGUAGE_TAMIL,                    sToken147,
         LANGUAGE_TATAR,                    sToken148,
         LANGUAGE_TELUGU,                   sToken149,
         LANGUAGE_URDU_PAKISTAN,            sToken150,
         LANGUAGE_URDU_INDIA,               sToken151,
         LANGUAGE_UZBEK,                    sToken152,
         LANGUAGE_UZBEK_LATIN,              sToken153,
         LANGUAGE_UZBEK_CYRILLIC,           sToken154
    };

    BOOL bFnd = FALSE;
    USHORT nLen = sizeof( aMapArr ) / sizeof( aMapArr[0] );
    if( bSearchId )
    {
        for( USHORT n = 0; n < nLen; ++n )
            if( rLngStr.EqualsIgnoreCaseAscii( aMapArr[ n ].pLanguageNm ))
            {
                rId = aMapArr[ n ].nId;
                bFnd = TRUE;
                break;
            }
    }
    else
    {
        for( USHORT n = 0; n < nLen; ++n )
            if( rId == aMapArr[ n ].nId )
            {
                rLngStr.AssignAscii( aMapArr[ n ].pLanguageNm );
                bFnd = TRUE;
                break;
            }
    }
    return bFnd;
}


// for the automatic conversion (mail/news/...)
// The user data contains the options for the ascii import/export filter.
// The format is:
//  1. CharSet - as ascii chars
//  2. LineEnd - as CR/LR/CRLF
//  3. Fontname
//  4. Language
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
                ::lcl_FindCharSet( TRUE, sToken, eCharSet );
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
                ::lcl_FindLanguage( TRUE, sToken, nLanguage );
                break;
            }
        }
        ++nCnt;
    } while( STRING_NOTFOUND != nToken );
}

void SwAsciiOptions::WriteUserData( String& rStr )
{
    rStr.Erase();

    // 1. charset
    ::lcl_FindCharSet( FALSE, rStr, eCharSet );
    rStr += ',';

    // 2. LineEnd
    switch( eCRLF_Flag )
    {
    case LINEEND_CRLF:  rStr.AppendAscii( "CRLF" );     break;
    case LINEEND_CR:    rStr.AppendAscii(  "CR" );      break;
    case LINEEND_LF:    rStr.AppendAscii(  "LF" );      break;
    }
    rStr += ',';

    // 3. Fontname
    rStr += sFont;
    rStr += ',';

    // 4. Language
    if( nLanguage )
    {
        String sTmp;
        ::lcl_FindLanguage( FALSE, sTmp, nLanguage );
        rStr += sTmp;
    }
    rStr += ',';
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

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.3  2000/11/13 10:46:09  jp
      remove IniManager

      Revision 1.2  2000/10/06 13:08:02  jp
      should changes: don't use IniManager

      Revision 1.1.1.1  2000/09/19 10:59:15  hr
      initial import

      Revision 1.175  2000/09/18 16:04:39  willem.vandorp
      OpenOffice header added.

      Revision 1.174  2000/08/04 16:26:25  jp
      read/write unicode ascii files

      Revision 1.173  2000/06/30 13:44:04  tl
      old lingu header removed

      Revision 1.172  2000/05/16 16:12:44  jp
      Changes for Unicode

      Revision 1.171  2000/05/15 16:35:27  jp
      Changes for Unicode

      Revision 1.170  2000/05/08 16:42:32  jp
      Changes for Unicode

      Revision 1.169  2000/04/17 09:09:10  khz
      Task #70451# use CalculateFlySize() for WW frames with 'auto'width

      Revision 1.168  2000/03/03 16:21:41  pl
      #73771# workaround for c50 intel compiler

      Revision 1.167  2000/03/03 16:18:43  pl
      #73771# workaround for c50 intel compiler

      Revision 1.166  2000/03/03 15:20:59  os
      StarView remainders removed

      Revision 1.165  2000/02/11 14:36:30  hr
      #70473# changes for unicode ( patched by automated patchtool )

*************************************************************************/


