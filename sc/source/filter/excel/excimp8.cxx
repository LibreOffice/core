/*************************************************************************
 *
 *  $RCSfile: excimp8.cxx,v $
 *
 *  $Revision: 1.84 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-24 11:54:22 $
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

#define ITEMID_FIELD EE_FEATURE_FIELD

#ifndef SC_ITEMS_HXX
#include <scitems.hxx>
#endif

#include <offmgr/fltrcfg.hxx>
#include <offmgr/app.hxx>

#include <svtools/wmf.hxx>

#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <svx/brshitem.hxx>
#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <svx/colritem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/postitem.hxx>
#include <svx/crsditem.hxx>
#include <svx/flditem.hxx>
#include <svx/xflclit.hxx>
#include <svx/svxmsbas.hxx>

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif

#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <rtl/math.hxx>

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#include <unotools/charclass.hxx>

#ifndef SC_DRWLAYER_HXX
#include <drwlayer.hxx>
#endif

#include "cell.hxx"
#include "document.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "attrib.hxx"
#include "conditio.hxx"
#include "dbcolect.hxx"
#include "editutil.hxx"
#include "markdata.hxx"
#include "rangenam.hxx"
#include "docoptio.hxx"
#ifndef SC_DETFUNC_HXX
#include "detfunc.hxx"
#endif
#ifndef __GLOBSTR_HRC_
#include "globstr.hrc"
#endif

#ifndef SC_XLOCX_HXX
#include "xlocx.hxx"
#endif
#ifndef SC_XIESCHER_HXX
#include "xiescher.hxx"
#endif
#ifndef SC_XILINK_HXX
#include "xilink.hxx"
#endif
#ifndef SC_XICONTENT_HXX
#include "xicontent.hxx"
#endif

#include "excimp8.hxx"
#include "excform.hxx"
#include "flttools.hxx"
#include "scextopt.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"

using namespace com::sun::star;

extern const sal_Char* pVBAStorageName;
extern const sal_Char* pVBASubStorageName;



#define INVALID_POS     0xFFFFFFFF




ExcCondForm::ExcCondForm( RootData* p, const XclImpRoot& rRoot ) :
    ExcRoot( p ),
    XclImpRoot( rRoot )
{
    nCol = nRow = 0;
    nNumOfConds = nCondCnt = 0;
    pScCondForm = NULL;
    pRangeList = new ScRangeList;
}


ExcCondForm::~ExcCondForm()
{
    delete pRangeList;
}


void ExcCondForm::Read( XclImpStream& rIn )
{
    rIn >> nNumOfConds;
    rIn.Ignore( 2 );
    rIn >> nRow;
    rIn.Ignore( 2 );
    rIn >> nCol;
    rIn.Ignore( 2 );

    UINT16  nRngCnt, nR1, nR2, nC1, nC2;
    rIn >> nRngCnt;
    while( nRngCnt )
    {
        rIn >> nR1 >> nR2 >> nC1 >> nC2;

        pRangeList->Append( ScRange( nC1, nR1, GetScTab(), nC2, nR2, GetScTab() ) );

        nRngCnt--;
    }
}


void ExcCondForm::ReadCf( XclImpStream& rIn, ExcelToSc& rConv, sal_uInt32 nListIndex )
{
    if( nNumOfConds )
    {
        nNumOfConds--;

//      const UINT32        nRecPos = rIn.Tell();
        UINT8               nFormType, nFormOperator;
        UINT16              nLenForm1, nLenForm2;
        ULONG               nDummy;

        rIn >> nFormType >> nFormOperator >> nLenForm1 >> nLenForm2;

        ScConditionMode     eMode;
        BOOL                bValid = FALSE;
        BOOL                bSingForm = TRUE;

        if( nFormType == 0x01 )
        {// compare
            bValid = TRUE;

            switch( nFormOperator )
            {
                case 0x01:  eMode = SC_COND_BETWEEN;    bSingForm = FALSE;  break;
                case 0x02:  eMode = SC_COND_NOTBETWEEN; bSingForm = FALSE;  break;
                case 0x03:  eMode = SC_COND_EQUAL;      break;
                case 0x04:  eMode = SC_COND_NOTEQUAL;   break;
                case 0x05:  eMode = SC_COND_GREATER;    break;
                case 0x06:  eMode = SC_COND_LESS;       break;
                case 0x07:  eMode = SC_COND_EQGREATER;  break;
                case 0x08:  eMode = SC_COND_EQLESS;     break;
                default:    eMode = SC_COND_NONE;
            }
        }
        else if( nFormType == 0x02 )
        {
            bValid = TRUE;
            eMode = SC_COND_DIRECT;
        }

        if( bValid )
        {
            ULONG               nFormatsLen = nLenForm1 + nLenForm2 + 6;
            if( nFormatsLen > rIn.GetRecSize() )
                return;

            nFormatsLen = rIn.GetRecSize() - nFormatsLen;    // >0!

            ScDocument& rDoc = GetDoc();

            const ScTokenArray* pFrmla1 = NULL;
            const ScTokenArray* pFrmla2 = NULL;

            ScAddress           aPos( nCol, nRow, GetScTab() );

            if( !pScCondForm )
            {
                nDummy = 0;
                pScCondForm = new ScConditionalFormat( nDummy, &rDoc );
            }

            // create style
            ULONG nPosF = rIn.GetRecPos();      // font
            ULONG nPosL = nPosF;                // line
            ULONG nPosP = nPosF;                // pattern (fill)

            switch( nFormatsLen )
            {
                case 10:    nPosF = 0;      nPosL = 0;      nPosP += 6;     break;  // P
                case 14:    nPosF = 0;      nPosL += 6;     nPosP = 0;      break;  // L
                case 18:    nPosF = 0;      nPosL += 6;     nPosP += 14;    break;  // L + P
                case 124:   nPosF += 74;    nPosL = 0;      nPosP = 0;      break;  // F
                case 128:   nPosF += 74;    nPosL = 0;      nPosP += 124;   break;  // F + P
                case 132:   nPosF += 74;    nPosL += 124;   nPosP = 0;      break;  // F + L
                case 136:   nPosF += 74;    nPosL += 124;   nPosP += 132;   break;  // F + L + P
                default:    nPosF = 0;      nPosL = 0;      nPosP = 0;
            }

            String aStyleName( XclTools::GetCondFormatStyleName( nListIndex, nCondCnt ) );
            SfxItemSet& rStyleItemSet = ScfTools::MakeCellStyleSheet( GetStyleSheetPool(), aStyleName, true ).GetItemSet();

            const XclImpPalette& rPalette = GetPalette();

            if( nPosF )     // font
            {
                UINT8           nAttr1, nAttr2, nAttr3, nUnder;
                UINT16          nBold;
                UINT32          nCol;
                rIn.Seek( nPosF );
                rIn >> nAttr1;          // italic / strike out
                rIn.Ignore( 3 );
                rIn >> nBold;           // boldness
                rIn.Ignore( 2 );
                rIn >> nUnder;          // num of underlines
                rIn.Ignore( 3 );
                rIn >> nCol;            // color
                rIn.Ignore( 4 );
                rIn >> nAttr2;          // strike out DC + italic/bold DC
                rIn.Ignore( 7 );
                rIn >> nAttr3;          // underline DC

                BOOL            bItalic = nAttr1 & 0x02;
                BOOL            bStrikeOut = nAttr1 & 0x80;

                BOOL            bHasColor = ( nCol != 0xFFFFFFFF );
                BOOL            bHasBoldItalic = !TRUEBOOL( nAttr2 & 0x02 );
                BOOL            bHasStrikeOut = !TRUEBOOL( nAttr2 & 0x80 );
                BOOL            bHasUnderline = !TRUEBOOL( nAttr3 & 0x01 );

                if( bHasBoldItalic )
                {
                    SvxWeightItem   aWeightItem( XclImpFont::GetScFontWeight( nBold ) );
                    rStyleItemSet.Put( aWeightItem );

                    SvxPostureItem  aAttr( bItalic? ITALIC_NORMAL : ITALIC_NONE );
                    rStyleItemSet.Put( aAttr );
                }

                if( bHasUnderline )
                {
                    FontUnderline   eUnder;
                    switch( nUnder )
                    {
                        case 1:     eUnder = UNDERLINE_SINGLE;      break;
                        case 2:     eUnder = UNDERLINE_DOUBLE;      break;
                        default:    eUnder = UNDERLINE_NONE;
                    }
                    SvxUnderlineItem    aUndItem( eUnder );
                    rStyleItemSet.Put( aUndItem );
                }

                if( bHasStrikeOut )
                {
                    SvxCrossedOutItem   aAttr( bStrikeOut? STRIKEOUT_SINGLE : STRIKEOUT_NONE );
                    rStyleItemSet.Put( aAttr );
                }

                if( bHasColor )
                    rStyleItemSet.Put( SvxColorItem( rPalette.GetColor( static_cast< sal_uInt16 >( nCol ) ) ) );
            }

            if( nPosL )     // line
            {
                sal_uInt16 nLineStyle;
                sal_uInt32 nLineColor;
                rIn.Seek( nPosL );
                rIn >> nLineStyle >> nLineColor;

                XclImpCellBorder aBorder;
                aBorder.FillFromCF8( nLineStyle, nLineColor );
                aBorder.FillToItemSet( rStyleItemSet, rPalette );
            }

            if( nPosP )     // pattern (fill)
            {
                sal_uInt16 nPattern, nColor;
                rIn.Seek( nPosP );
                rIn >> nPattern >> nColor;

                XclImpCellArea aArea;
                aArea.FillFromCF8( nPattern, nColor );
                aArea.FillToItemSet( rStyleItemSet, rPalette );
            }

            // convert formulas
            FORMULA_TYPE        eFT = FT_RangeName;
            if( nLenForm1 )
            {
                rIn.Seek( rIn.GetRecSize() - nLenForm1 - nLenForm2 );

                rConv.Reset( aPos );
                rConv.Convert( pFrmla1, nLenForm1, eFT );
            }

            ScTokenArray*       pHelp;

            if( nLenForm2 )
            {
                if( pFrmla1 )
                {
                    // copy unique ScTokenArry from formula converter!
                    pHelp = pFrmla1->Clone();
                    pFrmla1 = ( const ScTokenArray* ) pHelp;
                }

                rIn.Seek( rIn.GetRecSize() - nLenForm2 );

                rConv.Reset( aPos );
                rConv.Convert( pFrmla2, nLenForm2, eFT );
            }

            ScCondFormatEntry   aCFE( eMode, pFrmla1, pFrmla2, &rDoc, aPos, aStyleName );

            pScCondForm->AddEntry( aCFE );

            if( pFrmla1 && pFrmla2 )
            {
                // if both pointers are non null, 1 is a real copy
                pHelp = ( ScTokenArray* ) pFrmla1;
                delete pHelp;
            }
        }
        nCondCnt++;
    }
}


void ExcCondForm::Apply( void )
{
    if( pScCondForm )
    {
        ULONG           nCondFormat = GetDoc().AddCondFormat( *pScCondForm );
        ScPatternAttr   aPat( GetDoc().GetPool() );
        aPat.GetItemSet().Put( SfxUInt32Item( ATTR_CONDITIONAL, nCondFormat ) );

        const ScRange*  p = pRangeList->First();
        UINT16          nC1, nC2, nR1, nR2;

        while( p )
        {
            nC1 = p->aStart.Col();
            nR1 = p->aStart.Row();
            nC2 = p->aEnd.Col();
            nR2 = p->aEnd.Row();

            if( nC1 > MAXCOL )
                nC1 = MAXCOL;
            if( nC2 > MAXCOL )
                nC2 = MAXCOL;
            if( nR1 > MAXROW )
                nR1 = MAXROW;
            if( nR2 > MAXROW )
                nR2 = MAXROW;

            GetDoc().ApplyPatternAreaTab( nC1, nR1, nC2, nR2, p->aStart.Tab(), aPat );

            p = pRangeList->Next();
        }
    }
}




ExcCondFormList::~ExcCondFormList()
{
    ExcCondForm*    p = ( ExcCondForm* ) List::First();

    while( p )
    {
        delete p;
        p = ( ExcCondForm* ) List::Next();
    }
}


void ExcCondFormList::Apply( void )
{
    ExcCondForm*        p = ( ExcCondForm* ) List::First();

    while( p )
    {
        p->Apply();
        p = ( ExcCondForm* ) List::Next();
    }
}



ImportExcel8::ImportExcel8( SvStorage* pStorage, SvStream& rStream, ScDocument* pDoc, const String& rBasePath, SvStorage* pPivotCache ) :
    ImportExcel( rStream, pDoc, rBasePath )
{
    delete pFormConv;

    pFormConv = pExcRoot->pFmlaConverter = new ExcelToSc8( pExcRoot, aIn );

    pExcRoot->pPivotCacheStorage = pPivotCache;
    pCurrPivTab = NULL;
    pCurrPivotCache = NULL;

    pActCondForm = NULL;
    pCondFormList = NULL;

    pAutoFilterBuffer = NULL;

    pExcRoot->pRootStorage = pStorage;

    bHasBasic = FALSE;
}


ImportExcel8::~ImportExcel8()
{
    if( pCondFormList )
        delete pCondFormList;
    if( pAutoFilterBuffer )
        delete pAutoFilterBuffer;
}


void ImportExcel8::RecString( void )
{
    if( pLastFormCell )
    {
        pLastFormCell->SetString( aIn.ReadUniString() );

        pLastFormCell = NULL;
    }
}


void ImportExcel8::Calccount( void )
{
    ScDocOptions    aOpt = pD->GetDocOptions();
    aOpt.SetIterCount( aIn.ReaduInt16() );
    pD->SetDocOptions( aOpt );
}


void ImportExcel8::Delta( void )
{
    ScDocOptions    aOpt = pD->GetDocOptions();
    aOpt.SetIterEps( aIn.ReadDouble() );
    pD->SetDocOptions( aOpt );
}


void ImportExcel8::Iteration( void )
{
    ScDocOptions    aOpt = pD->GetDocOptions();
    aOpt.SetIter( aIn.ReaduInt16() == 1 );
    pD->SetDocOptions( aOpt );
}

void ImportExcel8:: WinProtection( void )
{
    if( aIn.ReaduInt16() )
    {
        pExcRoot->pExtDocOpt->SetWinProtection( true );
    }
}

void ImportExcel8::Verticalpagebreaks( void )
{
    UINT16      n;
    UINT16      nCol;

    aIn >> n;

    while( n )
    {
        aIn >> nCol;
        aIn.Ignore( 4 );        // beide Rows ueberlesen

        pColRowBuff->SetVertPagebreak( nCol );

        n--;
    }
}


void ImportExcel8::Horizontalpagebreaks( void )
{
    UINT16      n;
    UINT16      nRow;

    aIn >> n;

    while( n )
    {
        aIn >> nRow;
        aIn.Ignore( 4 );        // beide Cols ueberlesen

        pColRowBuff->SetHorizPagebreak( nRow );

        n--;
    }
}


void ImportExcel8::Note( void )
{
    UINT16  nCol, nRow, nFlags, nId;

    aIn >> nRow >> nCol >> nFlags >> nId;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        if( nId )
        {
            if( const XclImpEscherNote* pNoteObj = GetObjectManager().GetEscherNote( GetScTab(), nId ) )
            {
                if( const XclImpString* pString = pNoteObj->GetString() )
                {
                    bool bVisible = ::get_flag( nFlags, EXC_NOTE_VISIBLE );
                    ScPostIt aNote( pString->GetText() );
                    aNote.SetShown( bVisible );
                    GetDoc().SetNote( nCol, nRow, GetScTab(), aNote );
                    if( bVisible )
                    {
                        ScDocument* pDoc = GetDocPtr();
                        ScDetectiveFunc( pDoc, GetScTab() ).ShowComment( nCol, nRow, TRUE );
                    }
                }
            }
        }
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel8::Cont( void )
{
    if( bObjSection )
        GetObjectManager().ReadMsodrawing( aIn );
}


void ImportExcel8::Dconref( void )
{
    if( !pCurrPivotCache )
        return;

    UINT16  nR1, nR2;
    UINT8   nC1, nC2;
    String  aEncodedUrl, aUrl, aTabName;
    bool    bSelf;

    aIn >> nR1 >> nR2 >> nC1 >> nC2;


    aIn.AppendUniString( aEncodedUrl );
    XclImpUrlHelper::DecodeUrl( aUrl, aTabName, bSelf, *pExcRoot->pIR, aEncodedUrl );

    if( !aTabName.Len() )
    {
        aTabName = aUrl;
        aUrl.Erase();
    }
    ScfTools::ConvertToScSheetName( aTabName );
    pCurrPivotCache->SetSource( nC1, nR1, nC2, nR2, aUrl, aTabName, bSelf );
}


void ImportExcel8::Obj()
{
    GetObjectManager().ReadObj( maStrm );
}


void ImportExcel8::Boundsheet( void )
{
    UINT8           nLen;
    UINT16          nGrbit;

    aIn.Ignore( 4 );
    aIn >> nGrbit >> nLen;

    String aName( aIn.ReadUniString( nLen ) );

    ScfTools::ConvertToScSheetName( aName );
    *pExcRoot->pTabNameBuff << aName;

    if( nBdshtTab > 0 )
    {
        DBG_ASSERT( !pD->HasTable( nBdshtTab ),
            "*ImportExcel::Boundsheet8(): Tabelle schon vorhanden!" );

        pD->MakeTable( nBdshtTab );
    }

    if( ( nGrbit & 0x0001 ) || ( nGrbit & 0x0002 ) )
        pD->SetVisible( nBdshtTab, FALSE );
    else if( nFirstVisTab == 0xFFFF )
        nFirstVisTab = nBdshtTab;       // first visible for WINDOW2 import

    pD->RenameTab( nBdshtTab, aName );
    nBdshtTab++;
}


void ImportExcel8::Scenman( void )
{
    UINT16              nLastDispl;

    aIn.Ignore( 4 );
    aIn >> nLastDispl;

    aScenList.SetLast( nLastDispl );
}


void ImportExcel8::Scenario( void )
{
    aScenList.Append( new ExcScenario( aIn, *pExcRoot ) );
}


void ImportExcel8::Cellmerging( void )
{
    UINT16  nCount, nRow1, nRow2, nCol1, nCol2;
    aIn >> nCount;

    DBG_ASSERT( aIn.GetRecLeft() >= (ULONG)(nCount * 8), "ImportExcel8::Cellmerging - wrong record size" );

    while( nCount-- )
    {
        aIn >> nRow1 >> nRow2 >> nCol1 >> nCol2;
        bTabTruncated |= (nRow1 > MAXROW) || (nRow2 > MAXROW) || (nCol1 > MAXCOL) || (nCol2 > MAXCOL);
        if( (nRow1 <= MAXROW) && (nCol1 <= MAXCOL) )
        {
            nRow2 = Min( nRow2, static_cast< UINT16 >( MAXROW ) );
            nCol2 = Min( nCol2, static_cast< UINT16 >( MAXCOL ) );
            GetXFIndexBuffer().SetMerge( nCol1, nRow1, nCol2, nRow2 );
        }
    }
}


void ImportExcel8::Msodrawinggroup( void )
{
    GetObjectManager().ReadMsodrawinggroup( maStrm );
}


void ImportExcel8::Msodrawing( void )
{
    GetObjectManager().ReadMsodrawing( maStrm );
}


void ImportExcel8::Msodrawingselection( void )
{
    GetObjectManager().ReadMsodrawingselection( maStrm );
}

void ImportExcel8::Condfmt( void )
{
    if( !pCondFormList )
        pCondFormList = new ExcCondFormList;

    pActCondForm = new ExcCondForm( pExcRoot, *this );

    pCondFormList->Append( pActCondForm );

    pActCondForm->Read( aIn );
}


void ImportExcel8::Cf( void )
{
    if( pActCondForm )
        pActCondForm->ReadCf( aIn, *pFormConv, static_cast< sal_Int32 >( pCondFormList->Count() - 1 ) );
}


void ImportExcel8::Labelsst( void )
{
    UINT16                      nRow, nCol, nXF;
    UINT32                      nSst;

    aIn >> nRow >> nCol >> nXF >> nSst;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        GetXFIndexBuffer().SetXF( nCol, nRow, nXF );

        ScBaseCell* pCell = GetSst().CreateCell( nSst, nXF );
        if( pCell )
            GetDoc().PutCell( nCol, nRow, GetScTab(), pCell );

        pColRowBuff->Used( nCol, nRow );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel8::Rstring( void )
{
    UINT16 nRow, nCol, nXF;
    aIn >> nRow >> nCol >> nXF;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        GetXFIndexBuffer().SetXF( nCol, nRow, nXF );

        // unformatted Unicode string with separate formatting information
        XclImpString aString( maStrm );
        if( !aString.IsRich() )
            aString.ReadFormats( maStrm );

        ScBaseCell* pCell = XclImpStringHelper::CreateCell( *this, aString, nXF );
        if( pCell )
            GetDoc().PutCell( nCol, nRow, GetScTab(), pCell );

        pColRowBuff->Used( nCol, nRow );
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel8::Label( void )
{
    UINT16  nRow, nCol, nXF;
    aIn >> nRow >> nCol >> nXF;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        GetXFIndexBuffer().SetXF( nCol, nRow, nXF );

        XclImpString aString( maStrm );
        ScBaseCell* pCell = XclImpStringHelper::CreateCell( *this, aString, nXF );
        if( pCell )
            GetDoc().PutCell( nCol, nRow, GetScTab(), pCell );

        pColRowBuff->Used( nCol, nRow );

    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel8::Txo( void )
{
    GetObjectManager().ReadTxo( maStrm );
}


void ImportExcel8::Codename( BOOL bWorkbookGlobals )
{
    if( bHasBasic )
    {
        String aName( aIn.ReadUniString() );

        DBG_ASSERT( pExcRoot->pExtDocOpt, "-ImportExcel8::Codename(): nothing there to store!" );

        if( bWorkbookGlobals )
            pExcRoot->pExtDocOpt->SetCodename( aName );
        else
            pExcRoot->pExtDocOpt->AddCodename( aName );
    }
}


void ImportExcel8::Dimensions( void )
{
    UINT32  nRowFirst, nRowLast;
    UINT16  nColFirst, nColLast;

    aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;

    if( nRowLast > MAXROW )
        nRowLast = MAXROW;
    if( nColLast > MAXCOL )
        nColLast = MAXCOL;
    if( nRowFirst > nRowLast )
        nRowFirst = nRowLast;
    if( nColFirst > nColLast )
        nColFirst = nColLast;

    pColRowBuff->SetDimension(
        ScRange( nColFirst, ( UINT16 ) nRowFirst, GetScTab(), nColLast, ( UINT16 ) nRowLast, GetScTab() ) );
}


void ImportExcel8::Name( void )
{
    const ScTokenArray* pErgebnis;
    UINT16              nLenDef;
    BYTE                nLenName;
    BYTE                nLen;

    UINT16              nOpt;
    UINT16              nLenSeekRel = 0;
    UINT16              nSheet;

    aIn >> nOpt;
    aIn.Ignore( 1 );
    aIn >> nLenName >> nLenDef;
    aIn.Ignore( 2 );
    aIn >> nSheet           // sheet index (1-based)
        >> nLen;            // length of custom menu text
    nLenSeekRel += nLen;
    aIn >> nLen;            // length of description text
    nLenSeekRel += nLen;
    aIn >> nLen;            // length of help topic text
    nLenSeekRel += nLen;
    aIn >> nLen;            // length of status bar text
    nLenSeekRel += nLen;

    // Namen einlesen
    String              aName( aIn.ReadUniString( nLenName ) );
    // jetzt steht Lesemarke an der Formel

    sal_Unicode         cFirstChar = aName.GetChar( 0 );

    const BOOL          bHidden = TRUEBOOL( nOpt & EXC_NAME_HIDDEN );
    const BOOL          bBuiltIn = TRUEBOOL( nOpt & EXC_NAME_BUILTIN );
    const BOOL          bPrintArea = bBuiltIn && ( cFirstChar == EXC_BUILTIN_PRINTAREA );
    const BOOL          bPrintTitles = bBuiltIn && ( cFirstChar == EXC_BUILTIN_PRINTTITLES );
    const BOOL          bAutoFilter = bBuiltIn && ( cFirstChar == EXC_BUILTIN_AUTOFILTER );
    const BOOL          bCriteria = bBuiltIn && ( cFirstChar == EXC_BUILTIN_CRITERIA );
    const BOOL          bExtract = bBuiltIn && ( cFirstChar == EXC_BUILTIN_EXTRACT );
    BOOL                bAppendTabNum = FALSE;
    BOOL                bSkip = FALSE;
    RangeType           eNameType = RT_ABSAREA;

    if( bBuiltIn )
        aName = XclTools::GetBuiltInName( cFirstChar, nSheet );
    else
        ScfTools::ConvertToScDefinedName( aName );

    if(bPrintArea)
        eNameType = RT_PRINTAREA;
    if(bCriteria)
        eNameType = RT_CRITERIA;


    pFormConv->Reset();
    if( nOpt & (EXC_NAME_VB | EXC_NAME_PROC | EXC_NAME_BIG) )
        // function or command?
        pFormConv->GetDummy( pErgebnis );
    else if( bBuiltIn )
    {
        aIn.PushPosition();

        if( bPrintArea )
            pFormConv->Convert( *pPrintRanges, nLenDef, FT_RangeName );
        else if( bPrintTitles )
            pFormConv->Convert( *pPrintTitles, nLenDef, FT_RangeName );

        aIn.PopPosition();

        pFormConv->Convert( pErgebnis, nLenDef, FT_RangeName );

        // AutoFilter
        if( pErgebnis && (bAutoFilter || bCriteria || bExtract) )
        {
            ScRange aRange;
            if( pErgebnis->IsReference( aRange ) )  // test & get range
            {
//                aName += String::CreateFromInt32( (sal_Int32) aRange.aStart.Tab() );
                bSkip = bAutoFilter;

                if( !pAutoFilterBuffer )
                    pAutoFilterBuffer = new XclImpAutoFilterBuffer;
                if( bAutoFilter )
                    pAutoFilterBuffer->Insert( pExcRoot, aRange, aName );
                else if( bCriteria )
                    pAutoFilterBuffer->AddAdvancedRange( aRange );
                else if( bExtract )
                {
                    if( pErgebnis->IsValidReference( aRange ) )
                        pAutoFilterBuffer->AddExtractPos( aRange );
                    else
                        eNameType = RT_NAME;
                }
            }
        }
    }
    else
        pFormConv->Convert( pErgebnis, nLenDef, FT_RangeName );     // formula

    if( bHidden || bSkip )
        pExcRoot->pRNameBuff->Store( aName, NULL, nSheet );
    else
        // ohne hidden
        pExcRoot->pRNameBuff->Store( aName, pErgebnis, nSheet, eNameType );
}


void ImportExcel8::GetHFString( String& rStr )
{
    aIn.AppendUniString( rStr );
}


void ImportExcel8::EndSheet( void )
{
    pActCondForm = NULL;

    ImportExcel::EndSheet();
}


void ImportExcel8::PostDocLoad( void )
{
    if( pCondFormList )
        pCondFormList->Apply();
    if( pAutoFilterBuffer )
        pAutoFilterBuffer->Apply();
    GetWebQueryBuffer().Apply();    //! test if extant

    ApplyEscherObjects();

    ImportExcel::PostDocLoad();

    // Scenarien bemachen! ACHTUNG: Hier wird Tabellen-Anzahl im Dokument erhoeht!!
    if( !pD->IsClipboard() && aScenList.Count() )
    {
        pD->UpdateChartListenerCollection();    // references in charts must be updated

        aScenList.Apply( *pD );
    }

    SfxObjectShell* pShell = GetDocShell();

    // BASIC
    if( bHasBasic && pShell )
    {
        OfaFilterOptions*   pFiltOpt = OFF_APP()->GetFilterOptions();

        if( pFiltOpt )
        {
            if( pFiltOpt->IsLoadExcelBasicCode() || pFiltOpt->IsLoadExcelBasicStorage() )
            {
                DBG_ASSERT( pExcRoot->pRootStorage, "-ImportExcel8::PostDocLoad(): no storage, no cookies!" );

                SvxImportMSVBasic   aBasicImport( *pShell, *pExcRoot->pRootStorage,
                                                    pFiltOpt->IsLoadExcelBasicCode(),
                                                    pFiltOpt->IsLoadExcelBasicStorage() );

                aBasicImport.Import( String::CreateFromAscii( pVBAStorageName ),
                                     String::CreateFromAscii( pVBASubStorageName ) );
            }
        }
    }

    // read doc info
    // no docshell while pasting from clipboard
    if( pShell )
    {
        SfxDocumentInfo     aNewDocInfo;
        SfxDocumentInfo&    rOldDocInfo = pShell->GetDocInfo();

        aNewDocInfo.LoadPropertySet( pExcRoot->pRootStorage );

        rOldDocInfo = aNewDocInfo;
        pShell->Broadcast( SfxDocumentInfoHint( &rOldDocInfo ) );
    }

    // building pivot tables
    aPivotTabList.Apply();
}


void ImportExcel8::ApplyEscherObjects()
{
    XclImpObjectManager& rObjManager = GetObjectManager();
    if( rObjManager.HasEscherStream() )
    {
        XclImpDffManager& rDffManager = rObjManager.GetDffManager();
        if( const SvxMSDffShapeInfos* pShapeInfos = rDffManager.GetShapeInfos() )
        {
            for( sal_uInt16 nInfo = 0, nInfoCount = pShapeInfos->Count(); nInfo < nInfoCount; ++nInfo )
            {
                if( const SvxMSDffShapeInfo* pShapeInfo = pShapeInfos->GetObject( nInfo ) )
                {
                    sal_uInt32 nShapeId = pShapeInfo->nShapeId;
                    XclImpEscherObj* pEscherObj = rObjManager.GetEscherObjAcc( pShapeInfo->nFilePos );
                    if( pEscherObj && !pEscherObj->GetSdrObj() )
                    {
                        SvxMSDffImportData aDffImportData;
                        rDffManager.SetSdrObject( pEscherObj, nShapeId, aDffImportData );

                        // *** find some comboboxes to skip ***
                        if( const XclImpEscherTbxCtrl* pCtrlObj = PTR_CAST( XclImpEscherTbxCtrl, pEscherObj ) )
                        {
                            if( pCtrlObj->GetType() == EXC_OBJ_CMO_COMBOBOX )
                            {
                                if( const XclImpEscherAnchor* pAnchor = rObjManager.GetEscherAnchor( pShapeInfo->nFilePos ) )
                                {
                                    bool bSkipObj = aPivotTabList.IsInPivotRange( pAnchor->mnLCol, pAnchor->mnTRow, pAnchor->mnScTab );
                                    if( !bSkipObj && pAutoFilterBuffer )
                                        bSkipObj = pAutoFilterBuffer->HasDropDown( pAnchor->mnLCol, pAnchor->mnTRow, pAnchor->mnScTab );
                                    if( bSkipObj )
                                        pEscherObj->SetSkip();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    rObjManager.Apply();
}



void ImportExcel8::EndAllChartObjects( void )
{
}


//___________________________________________________________________
// pivot tables

void ImportExcel8::SXView( void )
{
    pCurrPivTab = new XclImpPivotTable( aIn, pExcRoot, (UINT8)GetScTab() );
    aPivotTabList.Append( pCurrPivTab );
}

void ImportExcel8::SXVd( void )
{
    if( !pCurrPivTab )
        return;
    pCurrPivTab->AddViewField( aIn );
}

void ImportExcel8::SXVi( void )
{
    if( !pCurrPivTab )
        return;

    UINT16  nItemType, nGrBit, nCache;
    aIn >> nItemType >> nGrBit >> nCache;
    pCurrPivTab->AddViewItem( nItemType, nCache, nGrBit );
}

void ImportExcel8::SXIvd( void )
{
    if( !pCurrPivTab )
        return;
    pCurrPivTab->ReadRCFieldIDs( aIn );
}

void ImportExcel8::SXLi( void )
{
}   // unnecessary to read this record

void ImportExcel8::SXPi( void )
{
    if( !pCurrPivTab )
        return;

    UINT16  nArrayCnt = (UINT16)(aIn.GetRecSize() / 6);      // SXPI contains 6-byte-arrays
    UINT16  nSXVD;
    UINT16  nSXVI;
    UINT16  nObjID;

    for( UINT16 iArray = 0; iArray < nArrayCnt; iArray++ )
    {
        aIn >> nSXVD >> nSXVI >> nObjID;
        pCurrPivTab->AddPageItemInfo( nSXVD, nSXVI );
    }
}

void ImportExcel8::SXDi( void )
{
    if( !pCurrPivTab )
        return;
    pCurrPivTab->AddDataItem( aIn );
}

void ImportExcel8::SXIdStm( void )
{
    UINT16 nStrId;
    aIn >> nStrId;

    if( !pExcRoot->pImpPivotCacheList )
        pExcRoot->pImpPivotCacheList = new XclImpPivotCacheList;

    pCurrPivotCache = new XclImpPivotCache( pExcRoot, nStrId );
    pExcRoot->pImpPivotCacheList->Append( pCurrPivotCache );
}

void ImportExcel8::SXVs( void )
{
    if( !pCurrPivotCache )
        return;

    UINT16 nSrcType;
    aIn >> nSrcType;
    pCurrPivotCache->SetSourceType( nSrcType );
}

void ImportExcel8::SXRule( void )
{
}

void ImportExcel8::SXEx( void )
{
}

void ImportExcel8::SXFilt( void )
{
}

void ImportExcel8::SXSelect( void )
{
}

void ImportExcel8::SXVdex( void )
{
    if( !pCurrPivTab )
        return;

    UINT32 nFlags;
    aIn >> nFlags;
    pCurrPivTab->SetShowEmpty( TRUEBOOL( nFlags & 0x00000001 ) );
}



//___________________________________________________________________
// autofilter

void ImportExcel8::FilterMode( void )
{
    // The FilterMode record exists: if either the AutoFilter
    // record exists or an Advanced Filter is saved and stored
    // in the sheet. Thus if the FilterMode records only exists
    // then the latter is true..
    if( !pAutoFilterBuffer ) return;

    pAutoFilterBuffer->IncrementActiveAF();

    XclImpAutoFilterData* pData = pAutoFilterBuffer->GetByTab( GetScTab() );
    if( pData )
        pData->SetAutoOrAdvanced();
}

void ImportExcel8::AutoFilterInfo( void )
{
    if( !pAutoFilterBuffer ) return;

    XclImpAutoFilterData* pData = pAutoFilterBuffer->GetByTab( GetScTab() );
    if( pData )
    {
        pData->SetAdvancedRange( NULL );
        pData->Activate();
    }
}

void ImportExcel8::AutoFilter( void )
{
    if( !pAutoFilterBuffer ) return;

    XclImpAutoFilterData* pData = pAutoFilterBuffer->GetByTab( GetScTab() );
    if( pData )
        pData->ReadAutoFilter( aIn );
}



XclImpAutoFilterData::XclImpAutoFilterData( RootData* pRoot, const ScRange& rRange, const String& rName ) :
        ExcRoot( pRoot ),
        pCurrDBData(NULL),
        nFirstEmpty( 0 ),
        bActive( FALSE ),
        bHasDropDown( FALSE ),
        bHasConflict( FALSE ),
        bCriteria( FALSE ),
        bAutoOrAdvanced(FALSE),
        aFilterName(rName)
{
    aParam.nCol1 = rRange.aStart.Col();
    aParam.nRow1 = rRange.aStart.Row();
    aParam.nTab = rRange.aStart.Tab();
    aParam.nCol2 = rRange.aEnd.Col();
    aParam.nRow2 = rRange.aEnd.Row();

    // Excel defaults to always in place regardless
    // of whether an extract record exists. The user
    // must choose to explicity set the Copy To in the UI.
    aParam.bInplace = TRUE;

}

void XclImpAutoFilterData::CreateFromDouble( String& rStr, double fVal )
{
    rStr += String( ::rtl::math::doubleToUString( fVal,
                rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0), TRUE));
}

void XclImpAutoFilterData::SetCellAttribs()
{
    bHasDropDown = TRUE;
    for ( UINT16 nCol = StartCol(); nCol <= EndCol(); nCol++ )
    {
        INT16 nFlag = ((ScMergeFlagAttr*) pExcRoot->pDoc->
            GetAttr( nCol, StartRow(), Tab(), ATTR_MERGE_FLAG ))->GetValue();
        pExcRoot->pDoc->ApplyAttr( nCol, StartRow(), Tab(),
            ScMergeFlagAttr( nFlag | SC_MF_AUTO) );
    }
}

void XclImpAutoFilterData::InsertQueryParam()
{
    if( pCurrDBData && !bHasConflict )
    {
        ScRange aAdvRange;
        BOOL    bHasAdv = pCurrDBData->GetAdvancedQuerySource( aAdvRange );
        if( bHasAdv )
            pExcRoot->pDoc->CreateQueryParam( aAdvRange.aStart.Col(),
                aAdvRange.aStart.Row(), aAdvRange.aEnd.Col(), aAdvRange.aEnd.Row(),
                aAdvRange.aStart.Tab(), aParam );

        pCurrDBData->SetQueryParam( aParam );
        if( bHasAdv )
            pCurrDBData->SetAdvancedQuerySource( &aAdvRange );
        else
        {
            pCurrDBData->SetAutoFilter( TRUE );
            SetCellAttribs();
        }
    }
}

BOOL XclImpAutoFilterData::HasDropDown( UINT16 nCol, UINT16 nRow, UINT16 nTab ) const
{
    return (bHasDropDown && (StartCol() <= nCol) && (nCol <= EndCol()) &&
            (nRow == StartRow()) && (nTab == Tab()));
}

void XclImpAutoFilterData::ReadAutoFilter( XclImpStream& rStrm )
{
    UINT16 nCol, nFlags;
    rStrm >> nCol >> nFlags;

    ScQueryConnect  eConn       = (nFlags & EXC_AFFLAG_ANDORMASK) ? SC_OR : SC_AND;
    BOOL            bTop10      = TRUEBOOL( nFlags & EXC_AFFLAG_TOP10 );
    BOOL            bTopOfTop10 = TRUEBOOL( nFlags & EXC_AFFLAG_TOP10TOP );
    BOOL            bPercent    = TRUEBOOL( nFlags & EXC_AFFLAG_TOP10PERC );
    UINT16          nCntOfTop10 = nFlags >> 7;
    UINT16          nCount      = aParam.GetEntryCount();

    if( bTop10 )
    {
        if( nFirstEmpty < nCount )
        {
            ScQueryEntry& aEntry = aParam.GetEntry( nFirstEmpty );
            aEntry.bDoQuery = TRUE;
            aEntry.bQueryByString = TRUE;
            aEntry.nField = StartCol() + nCol;
            aEntry.eOp = bTopOfTop10 ?
                (bPercent ? SC_TOPPERC : SC_TOPVAL) : (bPercent ? SC_BOTPERC : SC_BOTVAL);
            aEntry.eConnect = SC_AND;
            aEntry.pStr->Assign( String::CreateFromInt32( (sal_Int32) nCntOfTop10 ) );

            rStrm.Ignore( 20 );
            nFirstEmpty++;
        }
    }
    else
    {
        UINT8   nE, nType, nOper, nBoolErr, nVal;
        INT32   nRK;
        double  fVal;
        BOOL    bIgnore;

        UINT8   nStrLen[ 2 ]    = { 0, 0 };
        String* pEntryStr[ 2 ]  = { NULL, NULL };

        for( nE = 0; nE < 2; nE++ )
        {
            if( nFirstEmpty < nCount )
            {
                ScQueryEntry& aEntry = aParam.GetEntry( nFirstEmpty );
                pEntryStr[ nE ] = aEntry.pStr;
                bIgnore = FALSE;

                rStrm >> nType >> nOper;
                switch( nOper )
                {
                    case EXC_AFOPER_LESS:
                        aEntry.eOp = SC_LESS;
                    break;
                    case EXC_AFOPER_EQUAL:
                        aEntry.eOp = SC_EQUAL;
                    break;
                    case EXC_AFOPER_LESSEQUAL:
                        aEntry.eOp = SC_LESS_EQUAL;
                    break;
                    case EXC_AFOPER_GREATER:
                        aEntry.eOp = SC_GREATER;
                    break;
                    case EXC_AFOPER_NOTEQUAL:
                        aEntry.eOp = SC_NOT_EQUAL;
                    break;
                    case EXC_AFOPER_GREATEREQUAL:
                        aEntry.eOp = SC_GREATER_EQUAL;
                    break;
                    default:
                        aEntry.eOp = SC_EQUAL;
                }

                switch( nType )
                {
                    case EXC_AFTYPE_RK:
                        rStrm >> nRK;
                        rStrm.Ignore( 4 );
                        CreateFromDouble( *aEntry.pStr, XclTools::GetDoubleFromRK( nRK ) );
                    break;
                    case EXC_AFTYPE_DOUBLE:
                        rStrm >> fVal;
                        CreateFromDouble( *aEntry.pStr, fVal );
                    break;
                    case EXC_AFTYPE_STRING:
                        rStrm.Ignore( 4 );
                        rStrm >> nStrLen[ nE ];
                        rStrm.Ignore( 3 );
                        aEntry.pStr->Erase();
                    break;
                    case EXC_AFTYPE_BOOLERR:
                        rStrm >> nBoolErr >> nVal;
                        rStrm.Ignore( 6 );
                        aEntry.pStr->Assign( String::CreateFromInt32( (sal_Int32) nVal ) );
                        bIgnore = (BOOL) nBoolErr;
                    break;
                    case EXC_AFTYPE_EMPTY:
                        aEntry.bQueryByString = FALSE;
                        aEntry.nVal = SC_EMPTYFIELDS;
                        aEntry.eOp = SC_EQUAL;
                    break;
                    case EXC_AFTYPE_NOTEMPTY:
                        aEntry.bQueryByString = FALSE;
                        aEntry.nVal = SC_NONEMPTYFIELDS;
                        aEntry.eOp = SC_EQUAL;
                    break;
                    default:
                        rStrm.Ignore( 8 );
                        bIgnore = TRUE;
                }

                if( (nE > 0) && (nCol > 0) && (eConn == SC_OR) && !bIgnore )
                    bHasConflict = TRUE;
                if( !bHasConflict && !bIgnore )
                {
                    aEntry.bDoQuery = TRUE;
                    aEntry.bQueryByString = TRUE;
                    aEntry.nField = StartCol() + nCol;
                    aEntry.eConnect = nE ? eConn : SC_AND;
                    nFirstEmpty++;
                }
            }
            else
                rStrm.Ignore( 10 );
        }

        for( nE = 0; nE < 2; nE++ )
            if( nStrLen[ nE ] && pEntryStr[ nE ] )
                pEntryStr[ nE ]->Assign( rStrm.ReadUniString( nStrLen[ nE ] ) );
    }
}

void XclImpAutoFilterData::SetAdvancedRange( const ScRange* pRange )
{
    if (pRange)
    {
        aCriteriaRange = *pRange;
        bCriteria = TRUE;
    }
    else
        bCriteria = FALSE;
}

void XclImpAutoFilterData::SetExtractPos( const ScAddress& rAddr )
{
    aParam.nDestCol = rAddr.Col();
    aParam.nDestRow = rAddr.Row();
    aParam.nDestTab = rAddr.Tab();
    aParam.bDestPers = TRUE;

}

void XclImpAutoFilterData::Apply( const BOOL bUseUnNamed )
{
    CreateScDBData(bUseUnNamed);

    if( bActive )
    {
        InsertQueryParam();

        BYTE nFlags;
        for( UINT16 nRow = StartRow(); nRow <= EndRow(); nRow++ )
        {
            nFlags = pExcRoot->pDoc->GetRowFlags( nRow, Tab() );
            if( nFlags & CR_HIDDEN )
                nFlags |= CR_FILTERED;
            pExcRoot->pDoc->SetRowFlags( nRow, Tab(), nFlags );
        }
    }
}

void XclImpAutoFilterData::CreateScDBData( const BOOL bUseUnNamed )
{

    // Create the ScDBData() object if the AutoFilter is activated
    // or if we need to create the Advanced Filter.
    if( bActive || bCriteria)
    {
        ScDBCollection& rColl = *pExcRoot->pDoc->GetDBCollection();
        pCurrDBData = rColl.GetDBAtArea( Tab(), StartCol(), StartRow(), EndCol(), EndRow() );
        if( !pCurrDBData )
        {
            AmendAFName(bUseUnNamed);

            pCurrDBData = new ScDBData( aFilterName, Tab(), StartCol(), StartRow(), EndCol(), EndRow() );

            if( pCurrDBData )
            {
                if(bCriteria)
                {
                    EnableRemoveFilter();

                    pCurrDBData->SetQueryParam( aParam );
                    pCurrDBData->SetAdvancedQuerySource(&aCriteriaRange);
                }
                else
                    pCurrDBData->SetAdvancedQuerySource(NULL);
                rColl.Insert( pCurrDBData );
            }
        }
    }

}

void XclImpAutoFilterData::EnableRemoveFilter()
{
    // only if this is a saved Advanced filter
    if( !bActive && bAutoOrAdvanced )
    {
        ScQueryEntry& aEntry = aParam.GetEntry( nFirstEmpty );
        aEntry.bDoQuery = TRUE;
        ++nFirstEmpty;
    }

    // TBD: force the automatic activation of the
    // "Remove Filter" by setting a virtual mouse click
    // inside the advanced range
}

void XclImpAutoFilterData::AmendAFName(const BOOL bUseUnNamed)
{
    // If-and-only-if we have one AF filter then
    // use the Calc "unnamed" range name. Calc
    // only supports one in total while Excel
    // supports one per sheet.
    if( bUseUnNamed && bAutoOrAdvanced )
        aFilterName = ScGlobal::GetRscString(STR_DB_NONAME);
}

XclImpAutoFilterBuffer::XclImpAutoFilterBuffer() :
    nAFActiveCount( 0 )
{
}

XclImpAutoFilterBuffer::~XclImpAutoFilterBuffer()
{
    for( XclImpAutoFilterData* pData = _First(); pData; pData = _Next() )
        delete pData;
}

void XclImpAutoFilterBuffer::Insert( RootData* pRoot, const ScRange& rRange,
                                    const String& rName )
{
    if( !GetByTab( rRange.aStart.Tab() ) )
        Append( new XclImpAutoFilterData( pRoot, rRange, rName ) );
}

void XclImpAutoFilterBuffer::AddAdvancedRange( const ScRange& rRange )
{
    XclImpAutoFilterData* pData = GetByTab( rRange.aStart.Tab() );
    if( pData )
        pData->SetAdvancedRange( &rRange );
}

void XclImpAutoFilterBuffer::AddExtractPos( const ScRange& rRange )
{
    XclImpAutoFilterData* pData = GetByTab( rRange.aStart.Tab() );
    if( pData )
        pData->SetExtractPos( rRange.aStart );
}

void XclImpAutoFilterBuffer::Apply()
{
    for( XclImpAutoFilterData* pData = _First(); pData; pData = _Next() )
        pData->Apply(UseUnNamed());
}

XclImpAutoFilterData* XclImpAutoFilterBuffer::GetByTab( UINT16 nTab )
{
    for( XclImpAutoFilterData* pData = _First(); pData; pData = _Next() )
        if( pData->Tab() == nTab )
            return pData;
    return NULL;
}

BOOL XclImpAutoFilterBuffer::HasDropDown( UINT16 nCol, UINT16 nRow, UINT16 nTab )
{
    for( XclImpAutoFilterData* pData = _First(); pData; pData = _Next() )
        if( pData->HasDropDown( nCol, nRow, nTab ) )
            return TRUE;
    return FALSE;
}

