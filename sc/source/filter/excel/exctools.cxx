/*************************************************************************
 *
 *  $RCSfile: exctools.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: gt $ $Date: 2001-02-27 14:05:40 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <vcl/svapp.hxx>

#include "document.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "globstr.hrc"
#include "scextopt.hxx"
#include "progress.hxx"
#include "editutil.hxx"
#include "addincol.hxx"

#include "root.hxx"
#include "imp_op.hxx"
#include "xfbuff.hxx"
#include "vfbuff.hxx"
#include "fontbuff.hxx"
#include "excimp8.hxx"
#include "otlnbuff.hxx"
#include "fltprgrs.hxx"
#include "excsst.hxx"
#include "excrecds.hxx"
#include "xcl97rec.hxx"
#include "XclAddInNameTrans.hxx"

#include "XclImpPivotTables.hxx"
#include "XclExpPivotTables.hxx"

// - ALLGEMEINE ----------------------------------------------------------

ScEditEngineDefaulter& RootData::GetEdEng( void )
{
    if( !pEdEng )
    {
        ScEditEngineDefaulter* p = new ScEditEngineDefaulter( pDoc->GetEnginePool() );

        p->SetRefMapMode( MAP_100TH_MM );
        p->SetEditTextObjectPool( pDoc->GetEditPool() );
        p->SetUpdateMode( FALSE );
        p->EnableUndo( FALSE );
        p->SetControlWord( ( p->GetControlWord() & ~EE_CNTRL_ALLOWBIGOBJS ) );

        pEdEng = p;
    }

    return *pEdEng;
}


ScEditEngineDefaulter& RootData::GetEdEngForHF( void )
{
    if( !pEdEngHF )
    {
        //  can't use document's edit engine pool here,
        //  because pool must have twips as default metric
        ScEditEngineDefaulter* p = new ScHeaderEditEngine( EditEngine::CreatePool(), TRUE );

        p->SetRefMapMode( MAP_TWIP );       // headers/footers are in twips
        p->SetUpdateMode( FALSE );
        p->EnableUndo( FALSE );
        p->SetControlWord( ( p->GetControlWord() & ~EE_CNTRL_ALLOWBIGOBJS ) );

        pEdEngHF = p;
    }

    return *pEdEngHF;
}


String RootData::GetCondFormStyleName( const UINT16 n )
{
    if( n <= nLastCond )
        nCondRangeCnt++;

    String  t( RTL_CONSTASCII_STRINGPARAM( "CndFrmStyl_" ) );

    t += UniString::CreateFromInt32( nCondRangeCnt );
    t.AppendAscii( "_" );
    t += UniString::CreateFromInt32( n );

    nLastCond = n;

    return t;
}


RootData::RootData( void )
{
    fColScale = fRowScale = 1.0;
    pDoc = NULL;
    pFormTable = NULL;
    pScRangeName = NULL;
    pColor = NULL;
    pXF_Buffer = NULL;
    pFontBuffer = NULL;
    eDefLanguage = ScGlobal::eLnge;
    eDateiTyp = eHauptDateiTyp = eGlobalDateiTyp = BiffX;
    pExtSheetBuff = NULL;
    pTabNameBuff = NULL;
    pRNameBuff = NULL;
    pShrfmlaBuff = NULL;
    pExtNameBuff = NULL;
    pAktTab = NULL;
    pCharset = NULL;
    pExtDocOpt = NULL;
    pProgress = NULL;

    pEdEng = pEdEngHF = NULL;

    bCellCut = FALSE;
    bBreakSharedFormula = TRUE;
    bDefaultPage = bChartTab = FALSE;

    pXtiBuffer = NULL;
    pSupbookBuffer = NULL;
    pCurrSupbook = NULL;
    pImpTabIdBuffer = NULL;

    pRootStorage = pPivotCacheStorage = /*pCtrlStorage = */NULL;
    pImpPivotCacheList = NULL;

    nCondRangeCnt = 0;
    nLastCond = 0;

    pRootStorage = NULL;
    pTabBuffer = NULL;
    pTabId = NULL;
    pUserBViewList = NULL;
    pNameList = NULL;
    pPalette2 = NULL;
    pFontRecs = NULL;
    pFormRecs = NULL;
    pExtSheetCntAndRecs = NULL;
    nRowMax = 0;
    nRangeNameIndex = 1;

    pSstRecs = NULL;
    pExternsheetRecs = NULL;
    pObjRecs = NULL;
    pNoteRecs = NULL;
    pEscher = NULL;

    pPivotCacheList = NULL;

    bWriteVBAStorage = FALSE;
    nCodenames = 0;

    pStyleSheet = NULL;
    pStyleSheetItemSet = NULL;

    pLastHlink = NULL;
    bStoreRel = FALSE;
    pBasePath = NULL;
}


RootData::~RootData()
{
    delete pFontBuffer;
    delete pColor;
    delete pExtSheetBuff;
    delete pTabNameBuff;
    delete pRNameBuff;
    delete pShrfmlaBuff;
    delete pExtNameBuff;
    delete pXF_Buffer;

    if( pTabBuffer )
        delete pTabBuffer;
    if( pEdEng )
        delete pEdEng;
    if( pEdEngHF )
        delete pEdEngHF;
    if( pExtSheetCntAndRecs )
        delete pExtSheetCntAndRecs;

    if( pXtiBuffer )
        delete pXtiBuffer;
    if( pSupbookBuffer )
        delete pSupbookBuffer;
    if( pImpTabIdBuffer )
        delete pImpTabIdBuffer;

    if( pImpPivotCacheList )
        delete pImpPivotCacheList;
    if( pPivotCacheList )
        delete pPivotCacheList;

    if( pLastHlink )
        delete pLastHlink;
    if( pBasePath )
        delete pBasePath;

    if( pExtDocOpt )
        delete pExtDocOpt;

//  if( pCtrlStorage )
//      delete pCtrlStorage;
}




OutlineBuffer::OutlineBuffer( USHORT nNewSize )
{
    DBG_ASSERT( nNewSize > 0, "-OutlineBuffer::Ctor: nNewSize == 0!" );

    nSize = nNewSize + 1;
    pLevel = new BYTE[ nSize ];
    pOuted = new BOOL[ nSize ];
    pHidden = new BOOL[ nSize ];
    pOutlineArray = NULL;

    Reset();
}


OutlineBuffer::~OutlineBuffer()
{
    delete[] pLevel;
    delete[] pOuted;
    delete[] pHidden;
}


void OutlineBuffer::SetLevel( USHORT nIndex, BYTE nVal, BOOL bOuted, BOOL bHidden )
{
    if( nIndex < nSize )
    {
        pLevel[ nIndex ] = nVal;
        pOuted[ nIndex ] = bOuted;
        pHidden[ nIndex ] = bHidden;

        if( nIndex > nLast )
            nLast = nIndex;
        if( nVal > nMaxLevel )
            nMaxLevel = nVal;
    }
}


void OutlineBuffer::SetOuted( USHORT nIndex )
{
    if( nIndex < nSize )
    {
        pOuted[ nIndex ] = TRUE;
        if( nIndex > nLast )
            nLast = nIndex;
    }
}


void OutlineBuffer::SetOuted( USHORT nIndex, BOOL bOuted )
{
    if( nIndex < nSize )
    {
        pOuted[ nIndex ] = bOuted;
        if( nIndex > nLast )
            nLast = nIndex;
    }
}


void OutlineBuffer::SetOutlineArray( ScOutlineArray* pOArray )
{
    pOutlineArray = pOArray;
}


// transtorm xcl-outline into SC-outline
void OutlineBuffer::MakeScOutline( void )
{
    if( !pOutlineArray || !HasOutline() )
        return;

    const UINT16    nNumLev         = 8;
    BOOL            bPreOutedLevel  = FALSE;
    BYTE            nCurrLevel      = 0;
    UINT16          nC;
    BOOL            bMakeHidden[ nNumLev ];
    BOOL            bMakeVisible[ nNumLev + 1 ];

    for( nC = 0; nC < nNumLev; nC++ )
        bMakeHidden[ nC ] = FALSE;
    for( nC = 0; nC <= nNumLev; nC++ )
        bMakeVisible[ nC ] = TRUE;
    if( nLast < (nSize - 1) )
        nLast++;

    // search for hidden attributes at end of level, move them to begin
    if( bButtonNormal )
    {
        for( BYTE nWorkLevel = 1; nWorkLevel <= nMaxLevel; nWorkLevel++ )
        {
            UINT16  nStartPos;
            BYTE    nCurrLevel  = 0;
            BYTE    nPrevLevel  = 0;

            for( UINT16 nC = 0 ; nC <= nLast ; nC++ )
            {
                nPrevLevel = nCurrLevel;
                nCurrLevel = pLevel[ nC ];
                if( (nPrevLevel < nWorkLevel) && (nCurrLevel >= nWorkLevel) )
                    nStartPos = nC;
                else if( (nPrevLevel >= nWorkLevel) && (nCurrLevel < nWorkLevel) )
                {
                    if( pOuted[ nC ] && pHidden[ nStartPos ] )
                    {
                        if( nStartPos )
                            pOuted[ nStartPos - 1 ] = TRUE;
                        else
                            bPreOutedLevel = TRUE;
                        pOuted[ nC ] = FALSE;
                    }
                }
            }
        }
    }
    else
        bPreOutedLevel = pHidden[ 0 ];

    // generate SC outlines
    UINT16  nPrevC;
    UINT16  nStart[ nNumLev ];
    BOOL    bDummy;
    BOOL    bPrevOuted  = bPreOutedLevel;
    BOOL    bCurrHidden = FALSE;
    BOOL    bPrevHidden = FALSE;

    for( nC = 0; nC <= nLast; nC++ )
    {
        BYTE nWorkLevel = pLevel[ nC ];

        nPrevC      = nC ? nC - 1 : 0;
        bPrevHidden = bCurrHidden;
        bCurrHidden = pHidden[ nC ];

        // open new levels
        while( nWorkLevel > nCurrLevel )
        {
            nCurrLevel++;
            bMakeHidden[ nCurrLevel ] = bPrevOuted;
            bMakeVisible[ nCurrLevel + 1 ] =
                bMakeVisible[ nCurrLevel ] && !bMakeHidden[ nCurrLevel ];
            nStart[ nCurrLevel ] = nC;
        }
        // close levels
        while( nWorkLevel < nCurrLevel )
        {
            BOOL bLastLevel     = (nWorkLevel == (nCurrLevel - 1));
            BOOL bRealHidden    = (bMakeHidden[ nCurrLevel ] && bPrevHidden );
            BOOL bRealVisible   = (bMakeVisible[ nCurrLevel ] ||
                                    (!bCurrHidden && bLastLevel));

            pOutlineArray->Insert( nStart[ nCurrLevel ], nPrevC , bDummy,
                bRealHidden, bRealVisible );
            nCurrLevel--;
        }

        bPrevOuted = pOuted[ nC ];
    }
}


void OutlineBuffer::SetLevelRange( USHORT nF, USHORT nL, BYTE nVal,
                                    BOOL bOuted, BOOL bHidden )
{
    DBG_ASSERT( nF <= nL, "+OutlineBuffer::SetLevelRange(): Last < First!" );

    if( nL < nSize )
    {
        if( nL > nLast )
            nLast = nL;

        BYTE*   pLevelCount;
        BYTE*   pLast;
        BOOL*   pOutedCount;
        BOOL*   pHiddenCount;

        pLevelCount = &pLevel[ nF ];
        pLast = &pLevel[ nL ];
        pOutedCount = &pOuted[ nF ];
        pHiddenCount = &pHidden[ nF ];

        while( pLevelCount <= pLast )
        {
            *( pLevelCount++ ) = nVal;
            *( pOutedCount++ ) = bOuted;
            *( pHiddenCount++ ) = bHidden;
        }

        if( nVal > nMaxLevel )
            nMaxLevel = nVal;
    }
}


void OutlineBuffer::Reset( void )
{
    for( USHORT nC = 0 ; nC < nSize ; nC++  )
    {
        pLevel[ nC ] = 0;
        pOuted[ nC ] = pHidden[ nC ] = FALSE;
    }
    nLast = 0;
    nMaxLevel = 0;
}


void OutlineBuffer::Reset( ScOutlineArray *pOArray )
{
    for( USHORT nC = 0 ; nC < nSize ; nC++  )
    {
        pLevel[ nC ] = 0;
        pOuted[ nC ] = pHidden[ nC ] = FALSE;
    }
    pOutlineArray = pOArray;
}




INT32   FilterProgressBar::nInstances = 0;


FilterProgressBar::FilterProgressBar( SvStream& rStream ) : pStr( &rStream ), pXIStr( NULL )
{
    ULONG nOldPos = rStream.Tell();
    rStream.Seek( STREAM_SEEK_TO_END );
    ULONG nStrmLen = rStream.Tell();
    rStream.Seek( nOldPos );
    Init( nOldPos, nStrmLen );
}


FilterProgressBar::FilterProgressBar( XclImpStream& rStream ) : pStr( NULL ), pXIStr( &rStream )
{
    Init( rStream.GetStreamPos(), rStream.GetStreamLen() );
}


FilterProgressBar::~FilterProgressBar()
{
    nInstances--;

    if( pPrgrs )
        delete pPrgrs;
}


void FilterProgressBar::Init( ULONG nStreamPos, ULONG nStreamLen )
{
    nInstances++;

    nCnt = 0;

    if( nInstances == 1 )
        pPrgrs = new ScProgress( NULL, ScGlobal::GetRscString( STR_LOAD_DOC ), nStreamLen );
    else
        pPrgrs = NULL;

    if( pPrgrs )
        pPrgrs->SetState( nStreamPos );
}


void FilterProgressBar::Progress( void )
{
    if( pPrgrs )
    {
        if( pStr )
            pPrgrs->SetState( pStr->Tell() );
        else if( pXIStr )
            pPrgrs->SetState( pXIStr->GetStreamPos() );
        else
        {
            nCnt++;
            pPrgrs->SetState( nCnt );
        }
    }
}


void FilterProgressBar::StartPostLoadProgress( const UINT32 nObj )
{
    if( pPrgrs )
    {
        pStr = NULL;
        pXIStr = NULL;
        nCnt = 0;

        delete pPrgrs;
        if( nObj )
        {
            pPrgrs = new ScProgress( NULL,
                    ScGlobal::GetRscString( STR_PROGRESS_CALCULATING ), nObj );
        }
        else
            pPrgrs = NULL;

    }
}




ShStrTabEntry::ShStrTabEntry( const String& r ) : aString( r )
{
}


ShStrTabEntry::~ShStrTabEntry()
{
}


BOOL ShStrTabEntry::HasFormats( void ) const
{
    return FALSE;
}


EditTextObject* ShStrTabEntry::CreateEditTextObject( ScEditEngineDefaulter&, FontBuffer& ) const
{
    return NULL;
}




struct ShStrTabFormData
{
    EditTextObject*     pEdTxtObj;
    UINT16*             pForms;
    UINT16              nFormCnt;

                        ShStrTabFormData( XclImpStream& rIn, UINT16 nFormCnt );
                            // wenn Ctor fehlschlaegt, ist pForms NULL!
                        ~ShStrTabFormData();
};


ShStrTabFormData::ShStrTabFormData( XclImpStream& r, UINT16 n )
{
    pEdTxtObj = NULL;
    if( n )
    {
        pForms = new UINT16[ n * 2 ];
        nFormCnt = n;

        UINT16*     p = pForms;

        while( n )
        {
            r >> *p;
            p++;
            r >> *p;
            p++;
            n--;
        }
    }
    else
        pForms = NULL;
}


ShStrTabFormData::~ShStrTabFormData()
{
    if( pEdTxtObj )
        delete pEdTxtObj;

    if( pForms )
        delete[] pForms;
}




ShStrTabFormEntry::ShStrTabFormEntry( const String& r, XclImpStream& rIn, const UINT16 nFormCnt ) :
    ShStrTabEntry( r )
{
    pData = new ShStrTabFormData( rIn, nFormCnt );
    if( !pData->pForms )
    {
        delete pData;
        pData = NULL;
    }
}


ShStrTabFormEntry::~ShStrTabFormEntry()
{
    if( pData )
        delete pData;
}


BOOL ShStrTabFormEntry::HasFormats( void ) const
{
    return pData && pData->pForms;
}




#define READFORM()      nChar = *pRead; pRead++; nFont = *pRead; pRead++; nAnzFrms--;


EditTextObject* ShStrTabFormEntry::CreateEditTextObject( ScEditEngineDefaulter& rEdEng, FontBuffer& rFB ) const
{
    if( !pData || !pData->pForms )
        return NULL;

    if( !pData->pEdTxtObj )
    {
        rEdEng.SetText( aString );

        SfxItemSet          aItemSet( rEdEng.GetEmptyItemSet() );

        UINT16              nChar, nFont;
        const sal_Unicode*  pAktChar = aString.GetBuffer();
        sal_Unicode         cAkt = *pAktChar;
        UINT16              nCnt = 0;
        const UINT16*       pRead = pData->pForms;
        UINT32              nAnzFrms = pData->nFormCnt;

        READFORM();

        ESelection          aSel( 0, 0 );

        while( cAkt )
        {
            if( nCnt >= nChar )
            {// neuer Item-Set
                rEdEng.QuickSetAttribs( aItemSet, aSel );

                aItemSet.ClearItem( 0 );

                rFB.Fill( nFont, aItemSet, FALSE );
                if( nAnzFrms )
                {
                    READFORM();
                }
                else
                    nChar = 0xFFFF;

                aSel.nStartPara = aSel.nEndPara;
                aSel.nStartPos = aSel.nEndPos;
            }

            if( cAkt == '\n' )
            {// new Paragraph
                aSel.nEndPara++;
                aSel.nEndPos = 0;
            }
            else
                aSel.nEndPos++;

            pAktChar++;
            cAkt = *pAktChar;
            nCnt++;
        }

        // letzten ItemSet setzten
        rEdEng.QuickSetAttribs( aItemSet, aSel );

        pData->pEdTxtObj = rEdEng.CreateTextObject();
    }

    return pData->pEdTxtObj->Clone();
}




SharedStringTable::SharedStringTable( void )
{
}


SharedStringTable::~SharedStringTable()
{
    Clear();
}


void SharedStringTable::Append( ShStrTabEntry* p )
{
    List::Insert( p, LIST_APPEND );
}


void SharedStringTable::Clear( void )
{
    ShStrTabEntry*      p = ( ShStrTabEntry* ) List::First();

    while( p )
    {
        delete p;
        p = ( ShStrTabEntry* ) List::Next();
    }

    List::Clear();
}


//___________________________________________________________________


ExcScenarioCell::ExcScenarioCell( const UINT16 nC, const UINT16 nR ) : nCol( nC ), nRow( nR )
{
}


void ExcScenarioCell::SetValue( const String& r )
{
    aValue = r;
}




#define EXCSCAPPEND(EXCSCCELL)  (List::Insert(EXCSCCELL,LIST_APPEND))
#define EXCSCFIRST()            ((ExcScenarioCell*)List::First())
#define EXCSCNEXT()             ((ExcScenarioCell*)List::Next())


ExcScenario::ExcScenario( XclImpStream& rIn, const RootData& rR ) : nTab( *rR.pAktTab )
{
    const CharSet   eSrc = *rR.pCharset;

    UINT16          nCref;
    UINT8           nName, nComment;

    rIn >> nCref;
    rIn.Ignore( 2 );
    rIn >> nName >> nComment;
    rIn.Ignore( 1 );        // statt nUser!

    if( nName )
        pName = new String( rIn.ReadUniString( eSrc, nName ) );
    else
    {
        pName = new String( RTL_CONSTASCII_STRINGPARAM( "Scenery" ) );
        rIn.Ignore( 1 );
    }

    pUserName = new String( rIn.ReadUniString( eSrc ) );

    if( nComment )
        pComment = new String( rIn.ReadUniString( eSrc ) );
    else
        pComment = new String;

    UINT16          n = nCref;
    UINT16          nC, nR;
    while( n )
    {
        rIn >> nR >> nC;

        EXCSCAPPEND( new ExcScenarioCell( nC, nR ) );

        n--;
    }

    n = nCref;
    ExcScenarioCell*    p = EXCSCFIRST();
    while( p )
    {
        p->SetValue( rIn.ReadUniString( eSrc ) );

        p = EXCSCNEXT();
    }
}


ExcScenario::~ExcScenario()
{
    ExcScenarioCell*    p = EXCSCFIRST();

    while( p )
    {
        delete p;
        p = EXCSCNEXT();
    }

    if( pName )
        delete pName;
    if( pComment )
        delete pComment;
    if( pUserName )
        delete pUserName;
}


void ExcScenario::Apply( ScDocument& r, const BOOL bLast )
{
    UINT32              nNumCells = List::Count();

    ExcScenarioCell*    p = EXCSCFIRST();
    String              aSzenName( *pName );
    r.CreateValidTabName( aSzenName );
    UINT16              nNewTab = nTab + 1;

    if( !r.InsertTab( nNewTab, aSzenName ) )
        return;

    r.SetScenario( nNewTab, TRUE );
    r.SetScenarioData( nNewTab, *pComment, COL_LIGHTGRAY, SC_SCENARIO_SHOWFRAME|SC_SCENARIO_COPYALL );

    while( p )
    {
        UINT16          nCol = p->nCol;
        UINT16          nRow = p->nRow;
        String          aVal = p->GetValue();

        r.ApplyFlagsTab( nCol, nRow, nCol, nRow, nNewTab, SC_MF_SCENARIO );

        r.SetString( nCol, nRow, nNewTab, aVal );

        p = EXCSCNEXT();
    }

    if( bLast )
        r.SetActiveScenario( nNewTab, TRUE );
}




ExcScenarioList::~ExcScenarioList()
{
    ExcScenario*    p = _First();

    while( p )
    {
        delete p;
        p = _Next();
    }
}


void ExcScenarioList::Apply( ScDocument& r )
{
    ExcScenario*    p = _Last();
    UINT16          n = ( UINT16 ) Count();

    while( p )
    {
        n--;
        p->Apply( r, ( BOOL ) ( n == nLastScenario ) );
        p = _Prev();
    }
}


XclAddInNameTranslator::XclAddInNameTranslator( void ) : rAddInColl( *ScGlobal::GetAddInCollection() )
{
    eLng = ::GetpApp()->GetAppInternational().GetLanguage();
}


XclAddInNameTranslator::~XclAddInNameTranslator()
{
}


String XclAddInNameTranslator::GetScName( const String& rExcelName )
{
    String  aRet;

    if( rAddInColl.GetCalcName( rExcelName, aRet ) )
        return aRet;
    else
        return rExcelName;
}


String XclAddInNameTranslator::GetXclName( const String& rScName )
{
    String  aRet;

    if( rAddInColl.GetExcelName( rScName, eLng, aRet ) )
        return aRet;
    else
        return rScName;
}


struct CountryLanguageEntry
{
    UINT16          nCntry;
    LanguageType    eLng;
};


BOOL XclAddInNameTranslator::SetLanguage( UINT16 n )
{
    static const CountryLanguageEntry   aTransTab[] =
    {
        {   1, LANGUAGE_ENGLISH_US },
        {   2, LANGUAGE_ENGLISH_CAN },
        {   3, LANGUAGE_SPANISH },              // Latin Americam except Brasil
        {  31, LANGUAGE_DUTCH },
        {  32, LANGUAGE_DUTCH_BELGIAN },        // Belgium
        {  33, LANGUAGE_FRENCH },
        {  34, LANGUAGE_SPANISH },
        {  39, LANGUAGE_ITALIAN },
        {  41, LANGUAGE_GERMAN_SWISS },
        {  43, LANGUAGE_GERMAN_AUSTRIAN },
        {  44, LANGUAGE_ENGLISH_UK },
        {  45, LANGUAGE_DANISH },
        {  46, LANGUAGE_SWEDISH },
        {  47, LANGUAGE_NORWEGIAN },
        {  49, LANGUAGE_GERMAN },
        {  52, LANGUAGE_SPANISH_MEXICAN },
        {  55, LANGUAGE_PORTUGUESE_BRAZILIAN },
        {  61, LANGUAGE_ENGLISH_AUS },
        {  64, LANGUAGE_ENGLISH_NZ },
        {  81, LANGUAGE_JAPANESE },
        {  82, LANGUAGE_KOREAN },
        { 351, LANGUAGE_PORTUGUESE },
        { 354, LANGUAGE_ICELANDIC },
        { 358, LANGUAGE_SWEDISH_FINLAND },
        { 785, LANGUAGE_ARABIC_SAUDI_ARABIA },
        { 886, LANGUAGE_CHINESE },
        { 972, LANGUAGE_HEBREW },               // Israel
        NULL
    };

    const CountryLanguageEntry*     p = aTransTab;

    while( p )
    {
        if( p->nCntry == n )
        {
            eLng = p->eLng;
            return TRUE;
        }
        p++;
    }

    return FALSE;
}

