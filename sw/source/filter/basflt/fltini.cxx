/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fltini.cxx,v $
 *
 *  $Revision: 1.56 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:36:21 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#define _SVSTDARR_STRINGS

#include <string.h>
#include <stdio.h>          // sscanf

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _PARHTML_HXX //autogen
#include <svtools/parhtml.hxx>
#endif
#include <svtools/svstdarr.hxx>
#include <sot/storage.hxx>
#include <sot/clsids.hxx>
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

#ifndef _IODETECT_CXX
#include <iodetect.cxx>
#endif

using namespace utl;
using rtl::OUString;
using namespace com::sun::star::uno;

SwRead ReadRtf = 0, ReadAscii = 0, /*ReadSwg = 0, ReadSw3 = 0,*/
        ReadHTML = 0, ReadXML = 0;

/*SwRead SwGetReaderSw3() // SW_DLLPUBLIC
{
    return ReadSw3;
}
*/
SwRead SwGetReaderXML() // SW_DLLPUBLIC
{
    return ReadXML;
}

bool IsDocShellRegistered()
{
    return 0 != SwDocShell::_GetInterface();
}

inline void _SetFltPtr( USHORT& rPos, SwRead pReader
                        , const sal_Char*
#if OSL_DEBUG_LEVEL > 1
                            pNm
#endif
    /* pNm optimiert der Compiler weg, wird nur in der nicht PRODUCT benoetigt! */
                        )
{
#if OSL_DEBUG_LEVEL > 1
    ASSERT( !strcmp( aReaderWriter[ rPos ].pName, pNm ), "falscher Filter" );
    (void) pNm;
#endif
    aReaderWriter[ rPos++ ].pReader = pReader;
}

void _InitFilter()
{
    SwRead pWW8Rd = new WW8Reader;

    USHORT nCnt = 0;
    _SetFltPtr( nCnt, (ReadRtf = new RtfReader), FILTER_RTF );
    _SetFltPtr( nCnt, (ReadAscii = new AsciiReader), FILTER_BAS );
    _SetFltPtr( nCnt, pWW8Rd, sWW6 );
    _SetFltPtr( nCnt, pWW8Rd, FILTER_WW8 );
    _SetFltPtr( nCnt, ReadRtf, sRtfWH );
    _SetFltPtr( nCnt, (ReadHTML = new HTMLReader), sHTML);
    _SetFltPtr( nCnt, new WW1Reader, sWW1 );
    _SetFltPtr( nCnt, pWW8Rd, sWW5 );
    _SetFltPtr( nCnt, (ReadXML = new XMLReader), FILTER_XML );

#ifdef NEW_WW97_EXPORT
    aReaderWriter[ 8-3 ].fnGetWriter =  &::GetWW8Writer;
    aReaderWriter[ 9-3 ].fnGetWriter = &::GetWW8Writer;
#endif

    _SetFltPtr( nCnt, ReadAscii, FILTER_TEXT_DLG );
    _SetFltPtr( nCnt, ReadAscii, FILTER_TEXT );

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

void SwIoSystem::GetWriter( const String& rFltName, const String& rBaseURL, WriterRef& xRet )
{
    for( USHORT n = 0; n < MAXFILTER; ++n )
        if( aReaderWriter[n].IsFilter( rFltName ) )
        {
            aReaderWriter[n].GetWriter( rFltName, rBaseURL, xRet );
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

/*  */

/////////////// die Storage Reader/Writer ////////////////////////////////

/*void GetSw3Writer( const String&, const String& rBaseURL, WriterRef& xRet )
{
    DBG_ERROR( "Shouldn't happen!");
    xRet = new Sw3Writer;
}
*/

ULONG StgReader::OpenMainStream( SvStorageStreamRef& rRef, USHORT& rBuffSize )
{
    ULONG nRet = ERR_SWG_READ_ERROR;
    ASSERT( pStg, "wo ist mein Storage?" );
    const SfxFilter* pFltr = SwIoSystem::GetFilterOfFormat( aFltName );
    if( pFltr )
    {
        rRef = pStg->OpenSotStream( SwIoSystem::GetSubStorageName( *pFltr ),
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
/*
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

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
USHORT Sw3Reader::GetSectionList( SfxMedium& rMedium,
                                    SvStrings& rStrings ) const
{
    SvStorageRef aStg( rMedium.GetStorage() );
    const SfxFilter* pFlt = rMedium.GetFilter();
    ASSERT( pFlt && pFlt->GetVersion(),
                                "Kein Filter oder Filter ohne FF-Version" );
    if( pFlt && pFlt->GetVersion() )
        aStg->SetVersion( (long)pFlt->GetVersion() );

    if( pIO )
        pIO->GetSectionList( &aStg, rStrings );
    return rStrings.Count();
    return 0;
}
*/

/*ULONG Sw3Writer::WriteStorage()
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

ULONG Sw3Writer::WriteMedium( SfxMedium& )
{
    DBG_ERROR( "Shouldn't be used currently!");
    return WriteStorage();
}

BOOL Sw3Writer::IsSw3Writer() const { return TRUE; }
*/

void Writer::SetPasswd( const String& ) {}


void Writer::SetVersion( const String&, long ) {}


BOOL Writer::IsStgWriter() const { return FALSE; }
//BOOL Writer::IsSw3Writer() const { return FALSE; }

BOOL StgWriter::IsStgWriter() const { return TRUE; }

/*  */



BOOL SwReader::NeedsPasswd( const Reader& /*rOptions*/ )
{
    BOOL bRes = FALSE;
    return bRes;
}


BOOL SwReader::CheckPasswd( const String& /*rPasswd*/, const Reader& /*rOptions*/ )
{
    return TRUE;
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

#define FILTER_OPTION_ROOT      String::CreateFromAscii( \
                RTL_CONSTASCII_STRINGPARAM( "Office.Writer/FilterFlags" ) )

SwFilterOptions::SwFilterOptions( sal_uInt16 nCnt, const sal_Char** ppNames,
                                  sal_uInt32* pValues )
    : ConfigItem( FILTER_OPTION_ROOT )
{
    GetValues( nCnt, ppNames, pValues );
}

void SwFilterOptions::GetValues( sal_uInt16 nCnt, const sal_Char** ppNames,
                                      sal_uInt32* pValues )
{
    Sequence<OUString> aNames( nCnt );
    OUString* pNames = aNames.getArray();
    USHORT n;

    for( n = 0; n < nCnt; ++n )
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
    const SwNumRuleTbl* pRuleTbl = NULL;

    if( !bNewDoc )
    {
        // jetzt alle schon vorhanden NumRules aus dem Array entfernen,
        // damit nur die neuen angepasst werden
        SwNumRuleTbl aNumRuleTbl;
        aNumRuleTbl.Insert( pNumRuleTbl, 0 );
        pNumRuleTbl->Remove( 0, pNumRuleTbl->Count() );
        const SwNumRuleTbl& rRuleTbl = rDoc.GetNumRuleTbl();
        SwNumRule* pRule;

        for( USHORT n = 0; n < rRuleTbl.Count(); ++n )
            if( USHRT_MAX == aNumRuleTbl.GetPos( ( pRule = rRuleTbl[ n ] )))
                // war noch nicht vorhanden, also neu
                pNumRuleTbl->Insert( pRule, pNumRuleTbl->Count() );

        aNumRuleTbl.Remove( 0, aNumRuleTbl.Count() );

        pRuleTbl = pNumRuleTbl;
    }
    else
    {
        pRuleTbl = &rDoc.GetNumRuleTbl();
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
            if( pNd->IsOutline() && !pNd->GetNumRule() )
                SetNumLSpace( *pNd, *pDoc->GetOutlineNumRule() );
        }
    }
}

void SwRelNumRuleSpaces::SetNumLSpace( SwTxtNode& rNd, const SwNumRule& rRule )
{
    BOOL bOutlineRule = OUTLINE_RULE == rRule.GetRuleType();
    // --> OD 2005-11-18 #128056#
    // correction of refactoring done by cws swnumtree:
    // - assure a correct level for retrieving numbering format.
//    BYTE nLvl = rNd.GetLevel();
    BYTE nLvl = 0;
    if ( rNd.GetLevel() >= 0 && rNd.GetLevel() < MAXLEVEL )
    {
        nLvl = static_cast< BYTE >(rNd.GetLevel());
    }
    // <--
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


void CalculateFlySize(SfxItemSet& rFlySet, const SwNodeIndex& rAnchor,
    SwTwips nPageWidth)
{
    const SfxPoolItem* pItem = 0;
    if( SFX_ITEM_SET != rFlySet.GetItemState( RES_FRM_SIZE, TRUE, &pItem ) ||
        MINFLY > ((SwFmtFrmSize*)pItem)->GetWidth() )
    {
        SwFmtFrmSize aSz((SwFmtFrmSize&)rFlySet.Get(RES_FRM_SIZE, TRUE));
        if (pItem)
            aSz = (SwFmtFrmSize&)(*pItem);

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
                        USHORT nWidthTmp = pLn->GetOutWidth() + pLn->GetInWidth();
                        nWidthTmp = nWidthTmp + rBoxItem.GetDistance( nLine );
                        nMinFrm += nWidthTmp;
                        nMaxFrm += nWidthTmp;
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
        IMPLENTRY(PT154),
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
String NameFromCharSet(rtl_TextEncoding nChrSet)
{
    const CharSetNameMap *pStart = GetCharSetNameMap();
    const char *pRet = pStart->pName;

    for(const CharSetNameMap *pMap = pStart; pMap->pName; ++pMap)
    {
        if (nChrSet == pMap->eCode)
        {
            pRet = pMap->pName;
            break;
        }
    }

    ASSERT(pRet != pStart->pName, "TXT: That was an unknown language!");

    return String::CreateFromAscii(pRet);
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
                nLanguage = MsLangId::convertIsoStringToLanguage( sToken );
                break;
            }
        }
        ++nCnt;
    } while( STRING_NOTFOUND != nToken );
}

void SwAsciiOptions::WriteUserData( String& rStr )
{
    // 1. charset
    rStr = NameFromCharSet(eCharSet);
    rStr += ',';

    // 2. LineEnd
    switch(eCRLF_Flag)
    {
        case LINEEND_CRLF:
            rStr.AppendAscii( "CRLF" );
            break;
        case LINEEND_CR:
            rStr.AppendAscii(  "CR" );
            break;
        case LINEEND_LF:
            rStr.AppendAscii(  "LF" );
            break;
    }
    rStr += ',';

    // 3. Fontname
    rStr += sFont;
    rStr += ',';

    // 4. Language
    if (nLanguage)
    {
        rtl::OUString sTmp = MsLangId::convertLanguageToIsoString( nLanguage );
        rStr += (String)sTmp;
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

