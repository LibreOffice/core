/*************************************************************************
 *
 *  $RCSfile: excimp8.cxx,v $
 *
 *  $Revision: 1.70 $
 *
 *  last change: $Author: dr $ $Date: 2002-05-22 11:11:18 $
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
#include <svx/linkmgr.hxx>
#include <svx/unoapi.hxx>

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif

#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <tools/solmath.hxx>

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
#include "validat.hxx"
#include "dbcolect.hxx"
#include "editutil.hxx"
#include "markdata.hxx"
#include "rangenam.hxx"
#include "arealink.hxx"
#include "docoptio.hxx"

#ifndef _SC_FILTERTOOLS_HXX
#include "FilterTools.hxx"
#endif
#ifndef _SC_XCLTOOLS_HXX
#include "XclTools.hxx"
#endif
#ifndef _SC_XCLIMPSTREAM_HXX
#include "XclImpStream.hxx"
#endif
#ifndef _SC_XCLIMPHELPER_HXX
#include "XclImpHelper.hxx"
#endif
#ifndef _SC_XCLIMPDOCCONTENT_HXX
#include "XclImpDocContent.hxx"
#endif
#ifndef _SC_XCLIMPEXTERNSHEET_HXX
#include "XclImpExternsheet.hxx"
#endif

#include "excimp8.hxx"
#include "fontbuff.hxx"
#include "excform.hxx"
#include "fltprgrs.hxx"
#include "flttools.hxx"
#include "scextopt.hxx"
#include "stlpool.hxx"
#include "scmsocximexp.hxx"
#include "XclAddInNameTrans.hxx"

using namespace com::sun::star;

extern const sal_Char* pVBAStorageName;
extern const sal_Char* pVBASubStorageName;



String      ImportExcel8::aSstErrTxt( _STRINGCONST( "*** ERROR IN SST ***" ) );

#define INVALID_POS     0xFFFFFFFF




ExcCondForm::ExcCondForm( RootData* p ) : ExcRoot( p )
{
    nTab = *p->pAktTab;
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

        pRangeList->Append( ScRange( nC1, nR1, nTab, nC2, nR2, nTab ) );

        nRngCnt--;
    }
}


void ExcCondForm::ReadCf( XclImpStream& rIn, ExcelToSc& rConv )
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
            if( nFormatsLen > rIn.GetRecLen() )
                return;

            nFormatsLen = rIn.GetRecLen() - nFormatsLen;    // >0!

            ScDocument*         pDoc = pExcRoot->pDoc;
            String              aStyle( pExcRoot->GetCondFormStyleName( nCondCnt ) );

            const ScTokenArray* pFrmla1 = NULL;
            const ScTokenArray* pFrmla2 = NULL;

            ScAddress           aPos( nCol, nRow, nTab );

            if( !pScCondForm )
            {
                nDummy = 0;
                pScCondForm = new ScConditionalFormat( nDummy, pDoc );
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

            SfxItemSet&         rStyleItemSet = pDoc->GetStyleSheetPool()->Make(
                                                    aStyle, SFX_STYLE_FAMILY_PARA,
                                                    SFXSTYLEBIT_USERDEF ).GetItemSet();

            ColorBuffer&        rColBuff = *pExcRoot->pColor;

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
                    rStyleItemSet.Put( *rColBuff.GetColor( (UINT16) nCol ) );
            }

            if( nPosL )     // line
            {
                UINT8           nLineH, nLineV;
                UINT16          nColH, nColV;
                rIn.Seek( nPosL );

                rIn >> nLineV >> nLineH >> nColV >> nColH;

                UINT8           nLineL = nLineV & 0x0F;
                UINT16          nColL = nColV & 0x007F;
                UINT8           nLineR = nLineV >> 4;
                UINT16          nColR = ( nColV >> 7 ) & 0x007F;
                UINT8           nLineT = nLineH & 0x0F;
                UINT16          nColT = nColH & 0x007F;
                UINT8           nLineB = nLineH >> 4;
                UINT16          nColB = ( nColH >> 7 ) & 0x007F;

                XclImpXF::SetBorder( rStyleItemSet, rColBuff,
                    nLineL, nColL, nLineR, nColR, nLineT, nColT, nLineB, nColB );
            }

            if( nPosP )     // pattern (fill)
            {
                UINT16          nPatt;
                UINT16          nCol;

                rIn.Seek( nPosP );
                rIn >> nPatt >> nCol;

                UINT8           nF = nCol & 0x007F;
                UINT8           nB = ( nCol >> 7 ) & 0x007F;
                UINT8           nP = (UINT8)((nPatt >> 10) & 0x003F);
                if( nP <= 1 )
                {
                    nP = nB; nB = nF; nF = nP; nP = 1;
                }

                XclImpXF::SetArea( rStyleItemSet, rColBuff, nP, nF, nB );
            }

            // convert formulas
            FORMULA_TYPE        eFT = FT_RangeName;
            if( nLenForm1 )
            {
                rIn.Seek( rIn.GetRecLen() - nLenForm1 - nLenForm2 );

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

                rIn.Seek( rIn.GetRecLen() - nLenForm2 );

                rConv.Reset( aPos );
                rConv.Convert( pFrmla2, nLenForm2, eFT );
            }

            ScCondFormatEntry   aCFE( eMode, pFrmla1, pFrmla2, pDoc, aPos, aStyle );

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
        ULONG           nCondFormat = pExcRoot->pDoc->AddCondFormat( *pScCondForm );
        ScPatternAttr   aPat( pExcRoot->pDoc->GetPool() );
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

            pExcRoot->pDoc->ApplyPatternAreaTab( nC1, nR1, nC2, nR2, nTab, aPat );

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



ImportExcel8::ImportExcel8( SvStorage* pStorage, SvStream& rStream, ScDocument* pDoc, SvStorage* pPivotCache ) :
    ImportExcel( rStream, pDoc ),
    aObjManager( *pExcRoot )
{
    delete pFormConv;

    pExcRoot->pExtsheetBuffer = new XclImpExtsheetBuffer;
    pExcRoot->pImpTabIdBuffer = new XclImpTabIdBuffer;

    pFormConv = new ExcelToSc8( pExcRoot, aIn, nTab );

    pExcRoot->pPivotCacheStorage = pPivotCache;
    pCurrPivTab = NULL;
    pCurrPivotCache = NULL;

    pActCondForm = NULL;
    pCondFormList = NULL;

    pAutoFilterBuffer = NULL;
    pWebQBuffer = NULL;

    pExcRoot->pRootStorage = pStorage;

    pExcRoot->pAddInNameTranslator = new XclAddInNameTranslator;

    bHasBasic = FALSE;
}


ImportExcel8::~ImportExcel8()
{
    if( pCondFormList )
        delete pCondFormList;
    if( pAutoFilterBuffer )
        delete pAutoFilterBuffer;
    if( pWebQBuffer )
        delete pWebQBuffer;
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


void ImportExcel8::Protect( void )
{
    if( aIn.ReaduInt16() )
    {
        uno::Sequence<sal_Int8> aEmptyPass;
        pD->SetTabProtection( nTab, TRUE, aEmptyPass );
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
    UINT16  nCol, nRow, nId;

    aIn >> nRow >> nCol;
    aIn.Ignore( 2 );
    aIn >> nId;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        if( nId )
        {
            const XclImpEscherNote* pObj = aObjManager.GetObjNote( nId, nTab );
            const String* pText = pObj ? pObj->GetText() : NULL;
            if( pText )
                pD->SetNote( nCol, nRow, nTab, ScPostIt( *pText ) );
        }
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel8::Cont( void )
{
    if( bObjSection )
        aObjManager.ReadMsodrawing( aIn );
}


void ImportExcel8::Dconref( void )
{
    if( !pCurrPivotCache )
        return;

    UINT16  nR1, nR2;
    UINT8   nC1, nC2;
    String  aFileName, aTabName;
    BOOL    bSelf;

    aIn >> nR1 >> nR2 >> nC1 >> nC2;

    XclImpURLDecoder::DecodeURL( aIn, aFileName, aTabName, bSelf );

    if( !aTabName.Len() )
    {
        aTabName = aFileName;
        aFileName.Erase();
    }
    ScfTools::ConvertName( aTabName );
    pCurrPivotCache->SetSource( nC1, nR1, nC2, nR2, aFileName, aTabName, bSelf );
}


void ImportExcel8::Obj()
{
    aObjManager.ReadObj( aIn );
}


void ImportExcel8::Boundsheet( void )
{
    UINT8           nLen;
    UINT16          nGrbit;

    aIn.Ignore( 4 );
    aIn >> nGrbit >> nLen;

    String aName( aIn.ReadUniString( nLen ) );

    ScfTools::ConvertName( aName );
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
            pCellStyleBuffer->SetMerge( nCol1, nRow1, nCol2, nRow2 );
        }
    }
}


struct BackgroundGraphic
{
    sal_uInt32 nMagicNumber;
    sal_uInt32 nUnknown1;
    sal_uInt32 nUnknown2;
    sal_uInt16 nWidth;
    sal_uInt16 nHeight;
    sal_uInt16 nPlanes;
    sal_uInt16 nBitsPerPixel;
};


static sal_Bool lcl_ImportBackgroundGraphic( XclImpStream& rIn, Graphic& rGraphic )
{
    sal_Bool            bRetValue = FALSE;
    BackgroundGraphic   aBackground;

    rIn >> aBackground.nMagicNumber
        >> aBackground.nUnknown1
        >> aBackground.nUnknown2
        >> aBackground.nWidth
        >> aBackground.nHeight
        >> aBackground.nPlanes
        >> aBackground.nBitsPerPixel;

    if( rIn.IsValid()
        && ( aBackground.nMagicNumber == 0x00010009 )
        && ( aBackground.nBitsPerPixel == 24 )
        && ( aBackground.nPlanes == 1 ) )
    {
        sal_uInt32              nWidth = aBackground.nWidth;
        sal_uInt32              nHeight = aBackground.nHeight;
        sal_uInt32              nPadding = nWidth % 4;

        if( rIn.GetRecLeft() == nHeight * (nWidth * 3 + nPadding) )
        {
            Bitmap              aBmp( Size( nWidth, nHeight ), aBackground.nBitsPerPixel );
            BitmapWriteAccess*  pAcc = aBmp.AcquireWriteAccess();

            sal_uInt8           nBlue, nGreen, nRed;
            sal_uInt32          x, y, ys;
            ys = nHeight - 1;
            for( y = 0 ; y < nHeight ; y++, ys-- )
            {
                for( x = 0 ; x < nWidth ; x++ )
                {
                    rIn >> nBlue >> nGreen >> nRed;
                    pAcc->SetPixel( ys, x, BitmapColor( nRed, nGreen, nBlue ) );
                }
                rIn.Ignore( nPadding );
            }

            aBmp.ReleaseAccess( pAcc );
            rGraphic = aBmp;
            bRetValue = TRUE;
        }
    }
    return bRetValue;
}


void ImportExcel8::BGPic( void )
{
    // no documentation available, but it might be, that it's only wmf format
    DBG_ASSERT( pStyleSheetItemSet, "-ImportExcel::BGPic(): f... no style sheet!" );

    Graphic             aGraphic;
    if( lcl_ImportBackgroundGraphic( aIn, aGraphic ) )
        pStyleSheetItemSet->Put( SvxBrushItem( aGraphic, GPOS_TILED ) );
}


void ImportExcel8::Msodrawinggroup( void )
{
    aObjManager.ReadMsodrawinggroup( aIn );
}


void ImportExcel8::Msodrawing( void )
{
    aObjManager.ReadMsodrawing( aIn );
}


void ImportExcel8::Msodrawingselection( void )
{
    aObjManager.ReadMsodrawingselection( aIn );
}

void ImportExcel8::Sst( void )
{
    aIn.Ignore( 8 );
    ShStrTabEntry* pEntry;

    while( aIn.GetRecLeft() )
    {
        pEntry = XclImpHelper::CreateUnicodeEntry( aIn );
        aSharedStringTable.Append( pEntry );
    }
}


ScBaseCell* ImportExcel8::CreateCellFromShStrTabEntry( const ShStrTabEntry* p, const UINT16 nXF )
{
    ScBaseCell*             pRet = NULL;
    if( p )
    {
        if( p->GetString().Len() > 0 )
        {
            if( p->HasFormats() )
            {
                ScEditEngineDefaulter&      rEdEng = GetEdEng();
                EditTextObject* pTextObj = p->CreateEditTextObject(
                                                rEdEng, *pExcRoot->pFontBuffer );

                DBG_ASSERT( pTextObj, "-ImportExcel8::Labelsst(): Keiner hat mich lieb!" );

                pRet = new ScEditCell( pTextObj, pD, rEdEng.GetEditTextObjectPool() );

                delete pTextObj;
            }
            else if( pExcRoot->pXFBuffer->HasEscapement( nXF ) )
            {
                EditTextObject*     pTObj = CreateFormText( 0, p->GetString(), nXF );

                pRet = new ScEditCell( pTObj, pD, GetEdEng().GetEditTextObjectPool() );

                delete pTObj;
            }
            else
                pRet = ScBaseCell::CreateTextCell( p->GetString(), pD );
        }
    }
    else
        pRet = ScBaseCell::CreateTextCell( aSstErrTxt, pD );

    return pRet;
}


void ImportExcel8::Condfmt( void )
{
    pActCondForm = new ExcCondForm( pExcRoot );

    if( !pCondFormList )
        pCondFormList = new ExcCondFormList;

    pCondFormList->Append( pActCondForm );

    pActCondForm->Read( aIn );
}


void ImportExcel8::Cf( void )
{
    if( pActCondForm )
        pActCondForm->ReadCf( aIn, *pFormConv );
}


void ImportExcel8::Labelsst( void )
{
    UINT16                      nRow, nCol, nXF;
    UINT32                      nSst;

    aIn >> nRow >> nCol >> nXF >> nSst;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        const ShStrTabEntry*    p = aSharedStringTable.Get( nSst );

        ScBaseCell*             pCell = CreateCellFromShStrTabEntry( p, nXF );
        if( pCell )
            pD->PutCell( nCol, nRow, nTab, pCell, ( BOOL ) TRUE );

        pColRowBuff->Used( nCol, nRow );

        pCellStyleBuffer->SetXF( nCol, nRow, nXF );
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
        ShStrTabEntry*  p = XclImpHelper::CreateUnicodeEntry( aIn );

        ScBaseCell*     pCell = CreateCellFromShStrTabEntry( p, nXF );
        if( pCell )
            pD->PutCell( nCol, nRow, nTab, pCell, ( BOOL ) TRUE );

        pColRowBuff->Used( nCol, nRow );

        pCellStyleBuffer->SetXF( nCol, nRow, nXF );

        delete p;
    }
    else
        bTabTruncated = TRUE;

    pLastFormCell = NULL;
}


void ImportExcel8::Tabid( void )
{
    DBG_ASSERT( pExcRoot->pImpTabIdBuffer, "ImportExcel8::Tabid - missing tab id buffer" );
    if( pExcRoot->pImpTabIdBuffer )
        pExcRoot->pImpTabIdBuffer->ReadTabid8( aIn );
}


void ImportExcel8::Txo( void )
{
    aObjManager.ReadTxo( aIn );
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


void ImportExcel8::Dval()
{
    aIn.Ignore( 10 );
    // drop down list object
    sal_uInt32 nObjId;
    aIn >> nObjId;
    if( nObjId < 0xFFFFFFFF )
        maIgnoreObjList.Append( nObjId );
}


void ImportExcel8::Dv()
{
    sal_uInt32 nFlags;
    aIn >> nFlags;

    // messages
    String aPromptTitle;
    aIn.AppendUniString( aPromptTitle );
    String aErrorTitle;
    aIn.AppendUniString( aErrorTitle );
    String aPromptMessage;
    aIn.AppendUniString( aPromptMessage );
    String aErrorMessage;
    aIn.AppendUniString( aErrorMessage );

    // formula(s)
    if( aIn.GetRecLeft() > 8 )
    {
        sal_uInt16 nLen;

        // first formula
        const ScTokenArray* pFmla1 = NULL;
        aIn >> nLen;
        aIn.Ignore( 2 );
        if( nLen )
        {
            pFormConv->Reset();
            pFormConv->Convert( pFmla1, nLen, FT_RangeName );
            // pFormConv owns pFmla1 -> create a copy of the token array
            pFmla1 = pFmla1->Clone();
        }

        // second formula
        const ScTokenArray* pFmla2 = NULL;
        aIn >> nLen;
        aIn.Ignore( 2 );
        if( nLen )
        {
            pFormConv->Reset();
            pFormConv->Convert( pFmla2, nLen, FT_RangeName );
            // we do not own pFmla2
        }

        // read all cell ranges
        ScRangeList aRanges;
        XclTools::ReadCellRangeList( aIn, aRanges, nTab );
        XclRangeState eRangeState = XclTools::CropCellRangeList( aRanges, ScAddress( MAXCOL, MAXROW, MAXTAB ) );
        bTabTruncated |= (eRangeState != xlRangeInside);

        if( aRanges.Count() )
        {
            sal_Bool bIsValid = sal_True;   // valid settings in flags field

            // create the Calc validation object
            ScValidationMode eValMode;
            switch( nFlags & EXC_DV_MODE_MASK )
            {
                case EXC_DV_MODE_ANY:       eValMode = SC_VALID_ANY;        break;
                case EXC_DV_MODE_WHOLE:     eValMode = SC_VALID_WHOLE;      break;
                case EXC_DV_MODE_DECIMAL:   eValMode = SC_VALID_DECIMAL;    break;
                case EXC_DV_MODE_LIST:      eValMode = SC_VALID_LIST;       break;
                case EXC_DV_MODE_DATE:      eValMode = SC_VALID_DATE;       break;
                case EXC_DV_MODE_TIME:      eValMode = SC_VALID_TIME;       break;
                case EXC_DV_MODE_TEXTLEN:   eValMode = SC_VALID_TEXTLEN;    break;
                case EXC_DV_MODE_CUSTOM:    eValMode = SC_VALID_CUSTOM;     break;
                default:                    bIsValid = sal_False;
            }

            ScConditionMode eCondMode;
            switch( nFlags & EXC_DV_COND_MASK )
            {
                case EXC_DV_COND_BETWEEN:   eCondMode = SC_COND_BETWEEN;    break;
                case EXC_DV_COND_NOTBETWEEN:eCondMode = SC_COND_NOTBETWEEN; break;
                case EXC_DV_COND_EQUAL:     eCondMode = SC_COND_EQUAL;      break;
                case EXC_DV_COND_NOTEQUAL:  eCondMode = SC_COND_NOTEQUAL;   break;
                case EXC_DV_COND_GREATER:   eCondMode = SC_COND_GREATER;    break;
                case EXC_DV_COND_LESS:      eCondMode = SC_COND_LESS;       break;
                case EXC_DV_COND_EQGREATER: eCondMode = SC_COND_EQGREATER;  break;
                case EXC_DV_COND_EQLESS:    eCondMode = SC_COND_EQLESS;     break;
                default:                    bIsValid = sal_False;
            }

            // first range for base address for relative references
            ScRange* pRange = aRanges.GetObject( 0 );
            if( bIsValid && pRange )
            {
                ScValidationData aValidData( eValMode, eCondMode, pFmla1, pFmla2, pD, pRange->aStart );

                aValidData.SetIgnoreBlank( ::hasFlag( nFlags, EXC_DV_IGNOREBLANK ) );

                // *** prompt box ***
                if( aPromptTitle.Len() || aPromptMessage.Len() )
                {
                    // set any text stored in the record
                    aValidData.SetInput( aPromptTitle, aPromptMessage );
                    if( !(nFlags & EXC_DV_SHOWPROMPT) )
                        aValidData.ResetInput();
                }

                // *** error box ***
                ScValidErrorStyle eErrStyle = SC_VALERR_STOP;
                switch( nFlags & EXC_DV_ERROR_MASK )
                {
                    case EXC_DV_ERROR_WARNING:  eErrStyle = SC_VALERR_WARNING;  break;
                    case EXC_DV_ERROR_INFO:     eErrStyle = SC_VALERR_INFO;     break;
                }
                // set texts and error style
                aValidData.SetError( aErrorTitle, aErrorMessage, eErrStyle );
                if( !(nFlags & EXC_DV_SHOWERROR) )
                    aValidData.ResetError();

                // set the handle ID
                sal_uInt32 nHandle = pD->AddValidationEntry( aValidData );
                ScPatternAttr aPattern( pD->GetPool() );
                aPattern.GetItemSet().Put( SfxUInt32Item( ATTR_VALIDDATA, nHandle ) );

                // apply all ranges
                for( pRange = aRanges.First(); pRange; pRange = aRanges.Next() )
                    pD->ApplyPatternAreaTab( pRange->aStart.Col(), pRange->aStart.Row(),
                        pRange->aEnd.Col(), pRange->aEnd.Row(), nTab, aPattern );
            }
        }
        delete pFmla1;
        // we do not own pFmla2
    }
}


void ImportExcel8::Labelranges()
{
    ScRangeList aRanges;
    ScAddress aMaxPos( MAXCOL, MAXROW, MAXTAB );
    XclRangeState eRangeState;
    ScRangePairListRef xLabelRangesRef;
    const ScRange* pRange;

    // row label ranges
    XclTools::ReadCellRangeList( aIn, aRanges, nTab );
    eRangeState = XclTools::CropCellRangeList( aRanges, aMaxPos );
    bTabTruncated |= (eRangeState != xlRangeInside);
    xLabelRangesRef = pExcRoot->pDoc->GetRowNameRangesRef();
    for( pRange = aRanges.First(); pRange; pRange = aRanges.Next() )
    {
        ScRange aDataRange( *pRange );
        if( aDataRange.aEnd.Col() < MAXCOL )
        {
            aDataRange.aStart.SetCol( aDataRange.aEnd.Col() + 1 );
            aDataRange.aEnd.SetCol( MAXCOL );
        }
        else if( aDataRange.aStart.Col() > 0 )
        {
            aDataRange.aEnd.SetCol( aDataRange.aStart.Col() - 1 );
            aDataRange.aStart.SetCol( 0 );
        }
        xLabelRangesRef->Append( ScRangePair( *pRange, aDataRange ) );
    }

    // column label ranges
    aRanges.RemoveAll();
    XclTools::ReadCellRangeList( aIn, aRanges, nTab );
    eRangeState = XclTools::CropCellRangeList( aRanges, aMaxPos );
    bTabTruncated |= (eRangeState != xlRangeInside);
    xLabelRangesRef = pExcRoot->pDoc->GetColNameRangesRef();
    for( pRange = aRanges.First(); pRange; pRange = aRanges.Next() )
    {
        ScRange aDataRange( *pRange );
        if( aDataRange.aEnd.Row() < MAXROW )
        {
            aDataRange.aStart.SetRow( aDataRange.aEnd.Row() + 1 );
            aDataRange.aEnd.SetRow( MAXROW );
        }
        else if( aDataRange.aStart.Row() > 0 )
        {
            aDataRange.aEnd.SetRow( aDataRange.aStart.Row() - 1 );
            aDataRange.aStart.SetRow( 0 );
        }
        xLabelRangesRef->Append( ScRangePair( *pRange, aDataRange ) );
    }
}


void ImportExcel8::Hlink()
{
    XclRangeState eRangeState = XclImpHlink::ReadHlink( aIn, *pExcRoot );
    bTabTruncated |= (eRangeState != xlRangeInside);
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
        ScRange( nColFirst, ( UINT16 ) nRowFirst, nTab, nColLast, ( UINT16 ) nRowLast, nTab ) );
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

    if( bBuiltIn )
        XclTools::GetBuiltInName( aName, cFirstChar, nSheet );
    else
        ScfTools::ConvertName( aName );

    pFormConv->Reset();
    if( nOpt & (EXC_NAME_VB | EXC_NAME_BIG) )
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
                    pAutoFilterBuffer->AddExtractPos( aRange );
            }
        }
    }
    else
        pFormConv->Convert( pErgebnis, nLenDef, FT_RangeName );     // formula

    if( bHidden || bSkip )
        pExcRoot->pRNameBuff->Store( aName, NULL, nSheet );
    else
        // ohne hidden
        pExcRoot->pRNameBuff->Store( aName, pErgebnis, nSheet, bPrintArea );
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
    if( pWebQBuffer )
        pWebQBuffer->Apply( pD );

    UINT32 nChartCnt = 0;

    if( aObjManager.HasEscherStream() )
    {
        Biff8MSDffManager*      pDffMan = new Biff8MSDffManager( *pExcRoot, aObjManager,
                                            0, 0, pD->GetDrawLayer(), 1440 );

        const String            aStrName( _STRING( "Ctls" ) );
        SvStorage&              rStrg = *pExcRoot->pRootStorage;
        const BOOL              bHasCtrls = rStrg.IsContained( aStrName ) && rStrg.IsStream( aStrName );
        ScMSConvertControls*    pCtrlConv;
        SvStorageStreamRef      xStStream;

        if( bHasCtrls )
        {
            pCtrlConv = new ScMSConvertControls( pD->GetDocumentShell() );
            xStStream = pExcRoot->pRootStorage->OpenStream( aStrName, STREAM_READ | STREAM_SHARE_DENYALL );
        }

        const XclImpAnchorData*         pAnch;
        const SvxMSDffShapeInfos*       pShpInf = pDffMan->GetShapeInfos();

        if( pShpInf )
        {
            const UINT32                nMax = pShpInf->Count();
            const SvxMSDffShapeInfo*    p;
            ULONG                       nShapeId;
            SvxMSDffImportData*         pMSDffImportData;
            UINT32                      n;
            BOOL                        bIgnoreObj;

            UINT32                      nOLEImpFlags = 0;
            OfaFilterOptions*           pFltOpts = OFF_APP()->GetFilterOptions();
            if( pFltOpts )
            {
                if( pFltOpts->IsMathType2Math() )
                    nOLEImpFlags |= OLE_MATHTYPE_2_STARMATH;

                if( pFltOpts->IsWinWord2Writer() )
                    nOLEImpFlags |= OLE_WINWORD_2_STARWRITER;

                if( pFltOpts->IsPowerPoint2Impress() )
                    nOLEImpFlags |= OLE_POWERPOINT_2_STARIMPRESS;
            }

            for( n = 0 ; n < nMax ; n++ )
            {
                p = pShpInf->GetObject( ( UINT16 ) n );
                DBG_ASSERT( p, "-ImportExcel8::PostDocLoad(): Immer diese falschen Versprechungen!" );

                nShapeId = p->nShapeId;

                XclImpEscherObj* pObj = aObjManager.GetObjFromStream( p->nFilePos );
                if( pObj && !pObj->GetSdrObj() )
                {
                    pMSDffImportData = new SvxMSDffImportData;
                    pDffMan->SetSdrObject( pObj, nShapeId, *pMSDffImportData );

                    if( pObj->GetSdrObj() )
                    {
                        pAnch = aObjManager.GetAnchorData( p->nFilePos );
                        bIgnoreObj = FALSE;

                        // *** find all objects to ignore ***
                        if( pAnch )
                        {
                            // pivot table dropdowns
                            bIgnoreObj |= aPivotTabList.IsInPivotRange( pAnch->nCol, pAnch->nRow, pAnch->nTab );
                            // auto filter drop downs
                            if( pAutoFilterBuffer )
                                bIgnoreObj |= pAutoFilterBuffer->HasDropDown( pAnch->nCol, pAnch->nRow, pAnch->nTab );
                        }
                        // other objects
                        if( !bIgnoreObj )
                            bIgnoreObj = maIgnoreObjList.Contains( pObj->GetId() );

                        if( bIgnoreObj )
                            pObj->SetSdrObj( NULL );      // delete SdrObject
                        else
                        {
                            switch ( pObj->GetObjType() )
                            {
                                case otOle:
                                    ((XclImpEscherOle*)pObj)->CreateSdrOle( *pDffMan, nOLEImpFlags );
                                break;
                                case otCtrl:
                                    if( bHasCtrls )
                                    {
                                        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                                                xShapeRef = GetXShapeForSdrObject( ( SdrObject* ) pObj->GetSdrObj() );
                                        if( pCtrlConv->ReadOCXExcelKludgeStream( xStStream, &xShapeRef, TRUE ) )
                                        {
                                            SdrObject*  pNewObj = GetSdrObjectFromXShape( xShapeRef );
                                            if( pNewObj )
                                                pObj->SetSdrObj( pNewObj );
                                        }
                                    }
                                break;
                            }
                        }
                    }
                    delete pMSDffImportData;
                }
            }
        }

        if( bHasCtrls )
            delete pCtrlConv;

        delete pDffMan;
    }

    aObjManager.Apply();

    // controls
/*
    ScMSConvertControls     aCtrlConverter( pD->GetDocumentShell() );
    String                  aStrName( String::CreateFromAscii( "Ctls" ) );
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape >*  pShapeRef = NULL;

    SvStorageStreamRef      xStStream = pExcRoot->pRootStorage->OpenStream(
                                aStrName, STREAM_READ | STREAM_SHARE_DENYALL );
    aCtrlConverter.ReadOCXExcelKludgeStream( xStStream, pShapeRef, TRUE );
                                                                // BOOL bFloatingCtrl
*/
    ImportExcel::PostDocLoad();

    // Scenarien bemachen! ACHTUNG: Hier wird Tabellen-Anzahl im Dokument erhoeht!!
    if( aScenList.Count() )
    {
        pD->UpdateChartListenerCollection();    // references in charts must be updated

        aScenList.Apply( *pD );
    }

    // BASIC
    if( bHasBasic )
    {
        OfaFilterOptions*   pFiltOpt = OFF_APP()->GetFilterOptions();

        if( pFiltOpt )
        {
            if( pFiltOpt->IsLoadExcelBasicCode() || pFiltOpt->IsLoadExcelBasicStorage() )
            {
                DBG_ASSERT( pExcRoot->pRootStorage, "-ImportExcel8::PostDocLoad(): no storage, no cookies!" );

                SvxImportMSVBasic   aBasicImport( *pD->GetDocumentShell(), *pExcRoot->pRootStorage,
                                                    pFiltOpt->IsLoadExcelBasicCode(),
                                                    pFiltOpt->IsLoadExcelBasicStorage() );

                aBasicImport.Import( String::CreateFromAscii( pVBAStorageName ),
                                     String::CreateFromAscii( pVBASubStorageName ) );
            }
        }
    }

    // read doc info
    SfxDocumentInfo     aNewDocInfo;
    SfxDocumentInfo&    rOldDocInfo = pD->GetDocumentShell()->GetDocInfo();

    aNewDocInfo.LoadPropertySet( pExcRoot->pRootStorage );

    rOldDocInfo = aNewDocInfo;
    pD->GetDocumentShell()->Broadcast( SfxDocumentInfoHint( &rOldDocInfo ) );

    // building pivot tables
    aPivotTabList.Apply();
}



void ImportExcel8::CreateTmpCtrlStorage( void )
{
//  if( pExcRoot->pCtrlStorage )
    if( pExcRoot->xCtrlStorage.Is() )
        return;     // already done

    SvStorageStream*    pContrIn = pExcRoot->pRootStorage->OpenStream( _STRINGCONST( "Ctls" ), STREAM_STD_READ );
    if( pContrIn )
    {
        SvStorageRef    xStrg( new SvStorage( new SvMemoryStream(), TRUE ) );
        pExcRoot->xCtrlStorage = SvStorageRef( new SvStorage( new SvMemoryStream(), TRUE ) );
//      SvStorage*      pStrg = new SvStorage( new SvMemoryStream(), TRUE );

//      SvStorageStreamRef  xTemp = pStrg->OpenStream( _STRINGCONST( "contents" ) );
        SvStorageStreamRef  xTemp = xStrg->OpenStream( _STRINGCONST( "contents" ) );
        if ( xTemp.Is() && ( xTemp->GetError() == SVSTREAM_OK ) )
        {
            pContrIn->Seek( 16 );   // no need for class id at this point
            *xTemp << *pContrIn;

        SvGlobalName    aName( 0xD7053240, 0xCE69, 0x11CD, 0xA7, 0x77,
                                    0x00, 0xDD, 0x01, 0x14, 0x3C, 0x57 );
        UINT32              nClip = SotExchange::RegisterFormatName( _STRING( "Embedded Object" ) );
//      pStrg->SetClass( aName, nClip, _STRING( "Microsoft Forms 2.0 CommandButton" ) );
        xStrg->SetClass( aName, nClip, _STRING( "Microsoft Forms 2.0 CommandButton" ) );

        pExcRoot->xCtrlStorage = xStrg;
        }
/*      else
        {
            delete pStrg;
            pStrg = NULL;
        }*/

//      pExcRoot->pCtrlStorage = pStrg;
    }
}


void ImportExcel8::EndAllChartObjects( void )
{
}



//___________________________________________________________________
// 3D references, external references
//  --> records SUPBOOK, XCT, CRN, EXTERNNAME, EXTERNSHEET

void ImportExcel8::Supbook( void )
{
    pExcRoot->pExtsheetBuffer->ReadSupbook8( aIn );
}

void ImportExcel8::Xct( void )
{
    pExcRoot->pExtsheetBuffer->ReadXct8( aIn );
}

void ImportExcel8::Crn( void )
{
    pExcRoot->pExtsheetBuffer->ReadCrn8( aIn, *pFormConv );
}

void ImportExcel8::Externname( void )
{
    pExcRoot->pExtsheetBuffer->ReadExternname8( aIn, *pExcRoot );
}

void ImportExcel8::Externsheet( void )
{
    pExcRoot->pExtsheetBuffer->ReadExternsheet8( aIn );
    pExcRoot->pExtsheetBuffer->CreateTables( *pExcRoot );
}



//___________________________________________________________________
// web queries
//  --> records QSI, PARAMQRY, SXSTRING, WEBQRYSETTINGS, WEBQRYTABLES

void ImportExcel8::Qsi()
{
    aIn.Ignore( 10 );
    String aName( aIn.ReadUniString() );
    USHORT nIndex;
    if( pExcRoot->pScRangeName->SearchName( aName, nIndex ) )
    {
        const ScRangeData* pRangeData = (*pExcRoot->pScRangeName)[ nIndex ];
        ScRange aRange;
        if( pRangeData && pRangeData->IsReference( aRange ) )
        {
            if( !pWebQBuffer )
                pWebQBuffer = new XclImpWebQueryBuffer;
            XclImpWebQuery* pQuery = new XclImpWebQuery;
            pQuery->aDestRange = aRange;
            pWebQBuffer->Append( pQuery );
        }
    }
}

void ImportExcel8::SXExt_ParamQry()
{
    XclImpWebQuery* pQuery = pWebQBuffer ? pWebQBuffer->Last() : NULL;
    if( !pQuery ) return;

    UINT16 nFlags;
    aIn >> nFlags;
    if( nFlags & EXC_PQRY_TABLES )
    {
        pQuery->eMode = xiwqAllTables;
        pQuery->aTables = ScfTools::GetHTMLTablesName();
    }
    else
    {
        pQuery->eMode = xiwqDoc;
        pQuery->aTables = ScfTools::GetHTMLDocName();
    }
}

void ImportExcel8::SXString()
{
    XclImpWebQuery* pQuery = pWebQBuffer ? pWebQBuffer->Last() : NULL;
    if( !pQuery ) return;

    pQuery->aFilename.Erase();
    aIn.AppendUniString( pQuery->aFilename );
}

void ImportExcel8::WebQrySettings()
{
    XclImpWebQuery* pQuery = pWebQBuffer ? pWebQBuffer->Last() : NULL;
    if( !pQuery ) return;

    UINT16 nFlags;
    aIn.Ignore( 10 );
    aIn >> nFlags;
    if( (nFlags & EXC_WQSETT_SPECTABLES) && (pQuery->eMode == xiwqAllTables) )
        pQuery->eMode = xiwqSpecTables;

    aIn.Ignore( 10 );
    aIn >> pQuery->nRefresh;
}

void ImportExcel8::WebQryTables()
{
    XclImpWebQuery* pQuery = pWebQBuffer ? pWebQBuffer->Last() : NULL;
    if( !pQuery ) return;

    if( pQuery->eMode == xiwqSpecTables )
    {
        aIn.Ignore( 4 );
        pQuery->aTables.Erase();
        aIn.AppendUniString( pQuery->aTables );
        pQuery->ConvertTableNames();
    }
}



BOOL XclImpWebQuery::IsValid()
{
    return (aFilename.Len() != 0) && (eMode != xiwqUnknown);
}

void XclImpWebQuery::ConvertTableNames()
{
    const sal_Unicode cSep = ';';
    aTables.SearchAndReplaceAll( ',', cSep );
    String aQuotedPairs( RTL_CONSTASCII_USTRINGPARAM( "\"\"" ) );
    xub_StrLen nTokenCnt = aTables.GetQuotedTokenCount( aQuotedPairs, cSep );
    String aNewTables;
    xub_StrLen nStringIx = 0;
    for( xub_StrLen nToken = 0; nToken < nTokenCnt; nToken++ )
    {
        String aToken( aTables.GetQuotedToken( 0, aQuotedPairs, cSep, nStringIx ) );
        sal_Int32 nTabNum = CharClass::isAsciiNumeric( aToken ) ? aToken.ToInt32() : 0;
        if( nTabNum > 0 )
            ScfTools::AddToken( aNewTables, ScfTools::GetNameFromHTMLIndex( (ULONG)nTabNum ), cSep );
        else
        {
            ScfTools::EraseQuotes( aToken );
            if( aToken.Len() )
                ScfTools::AddToken( aNewTables, ScfTools::GetNameFromHTMLName( aToken ), cSep );
        }
    }
    aTables = aNewTables;
}



XclImpWebQueryBuffer::~XclImpWebQueryBuffer()
{
    for( XclImpWebQuery* pQuery = First(); pQuery; pQuery = Next() )
        delete pQuery;
}

void XclImpWebQueryBuffer::Apply( ScDocument* pDoc )
{
    DBG_ASSERT( pDoc, "XclImpWebQueryBuffer::Apply - no document" );

    for( XclImpWebQuery* pQuery = First(); pQuery; pQuery = Next() )
    {
        if( pQuery->IsValid() )
        {
            String sFilterName( RTL_CONSTASCII_USTRINGPARAM( EXC_WEBQRY_FILTER ) );
            ScAreaLink* pLink = new ScAreaLink( pDoc->GetDocumentShell(),
                pQuery->aFilename, sFilterName, EMPTY_STRING, pQuery->aTables,
                pQuery->aDestRange, (ULONG)pQuery->nRefresh * 60 );
            pDoc->GetLinkManager()->InsertFileLink( *pLink, OBJECT_CLIENT_FILE,
                pQuery->aFilename, &sFilterName, &pQuery->aTables );
        }
    }
}



//___________________________________________________________________
// pivot tables

void ImportExcel8::SXView( void )
{
    pCurrPivTab = new XclImpPivotTable( aIn, pExcRoot, (UINT8) nTab );
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

    UINT16  nArrayCnt = (UINT16)(aIn.GetRecLen() / 6);      // SXPI contains 6-byte-arrays
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
}

void ImportExcel8::AutoFilterInfo( void )
{
    if( !pAutoFilterBuffer ) return;

    XclImpAutoFilterData* pData = pAutoFilterBuffer->GetByTab( nTab );
    if( pData )
    {
        pData->SetAdvancedRange( NULL );
        pData->Activate();
    }
}

void ImportExcel8::AutoFilter( void )
{
    if( !pAutoFilterBuffer ) return;

    XclImpAutoFilterData* pData = pAutoFilterBuffer->GetByTab( nTab );
    if( pData )
        pData->ReadAutoFilter( aIn );
}



XclImpAutoFilterData::XclImpAutoFilterData( RootData* pRoot, const ScRange& rRange, const String& rName ) :
        ExcRoot( pRoot ),
        nFirstEmpty( 0 ),
        bActive( FALSE ),
        bHasDropDown( FALSE ),
        bHasConflict( FALSE )
{
    aParam.nCol1 = rRange.aStart.Col();
    aParam.nRow1 = rRange.aStart.Row();
    aParam.nTab = rRange.aStart.Tab();
    aParam.nCol2 = rRange.aEnd.Col();
    aParam.nRow2 = rRange.aEnd.Row();

    ScDBCollection& rColl = *pRoot->pDoc->GetDBCollection();

    pCurrDBData = rColl.GetDBAtArea( Tab(), StartCol(), StartRow(), EndCol(), EndRow() );
    if( !pCurrDBData )
    {
        pCurrDBData = new ScDBData( rName, Tab(), StartCol(), StartRow(), EndCol(), EndRow() );
        if( pCurrDBData )
            rColl.Insert( pCurrDBData );
    }
}

void XclImpAutoFilterData::CreateFromDouble( String& rStr, double fVal )
{
    SolarMath::DoubleToString( rStr, fVal, 'A', INT_MAX,
        ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0), TRUE );
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
        UINT32  nRK;
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
    if( pCurrDBData )
        pCurrDBData->SetAdvancedQuerySource( pRange );
}

void XclImpAutoFilterData::SetExtractPos( const ScAddress& rAddr )
{
    aParam.nDestCol = rAddr.Col();
    aParam.nDestRow = rAddr.Row();
    aParam.nDestTab = rAddr.Tab();
    aParam.bInplace = FALSE;
    aParam.bDestPers = TRUE;
}

void XclImpAutoFilterData::Apply()
{
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
        pData->Apply();
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

