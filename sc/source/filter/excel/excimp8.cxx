/*************************************************************************
 *
 *  $RCSfile: excimp8.cxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: dr $ $Date: 2001-08-23 09:56:17 $
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

#ifndef _SC_XCLIMPSTREAM_HXX
#include "XclImpStream.hxx"
#endif
#ifndef _SC_XCLIMPHELPER_HXX
#include "XclImpHelper.hxx"
#endif
#ifndef _SC_XCLIMPEXTERNSHEET_HXX
#include "XclImpExternsheet.hxx"
#endif

#include "excimp8.hxx"
#include "xfbuff.hxx"
#include "vfbuff.hxx"
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
                    SvxWeightItem   aWeightItem( FontBuffer::GetWeight( nBold ) );
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

                XF_Buffer::SetBorder( rStyleItemSet, rColBuff,
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

                XF_Buffer::SetFill( rStyleItemSet, rColBuff, nP, nF, nB );
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



struct DVData
{
    ULONG               nHandle;

    UINT16              nCol1;
    UINT16              nCol2;
    UINT16              nRow1;
    UINT16              nRow2;
};




DVList::~DVList()
{
    for( DVData* p = ( DVData* ) List::First() ; p ; p = ( DVData* ) List::Next() )
        delete p;
}


void DVList::Add( DVData* p )
{
    List::Insert( p, LIST_APPEND );
}


void DVList::Reset( void )
{
    for( DVData* p = ( DVData* ) List::First() ; p ; p = ( DVData* ) List::Next() )
        delete p;

    List::Clear();
}


void DVList::Apply( ScDocument& rDoc, UINT16 nTab )
{
    for( DVData* p = ( DVData* ) List::First() ; p ; p = ( DVData* ) List::Next() )
    {
        ScPatternAttr   aPat( rDoc.GetPool() );
        aPat.GetItemSet().Put( SfxUInt32Item( ATTR_VALIDDATA, p->nHandle ) );

        if( p->nRow2 > MAXROW )
            p->nRow2 = MAXROW;

        rDoc.ApplyPatternAreaTab( p->nCol1, p->nRow1, p->nCol2, p->nRow2, nTab, aPat );
    }
}




ImportExcel8::ImportExcel8( SvStorage* pStorage, SvStream& rStream, ScDocument* pDoc, SvStorage* pPivotCache ) :
    ImportExcel( rStream, pDoc ),
    aObjManager( *pExcRoot )
{
    delete pFormConv;

    pExcRoot->pExtsheetBuffer = new XclImpExternsheetBuffer;
    pExcRoot->pImpTabIdBuffer = new XclImpTabIdBuffer;

    pFormConv = new ExcelToSc8( pExcRoot, aIn, nTab );

    pActChart = NULL;

    pExcRoot->pPivotCacheStorage = pPivotCache;
    pCurrPivTab = NULL;
    pCurrPivotCache = NULL;

    pActCondForm = NULL;
    pCondFormList = NULL;

    pDVList = NULL;

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
    if( pDVList )
        delete pDVList;
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

        aColRowBuff.SetVertPagebreak( nCol );

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

        aColRowBuff.SetHorizPagebreak( nRow );

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


void ImportExcel8::Format( void )
{
    UINT16  nInd = aIn.ReaduInt16();
    pValueFormBuffer->NewValueFormat( nInd, aIn.ReadUniString() );
}


void ImportExcel8::Font( void )
{
    UINT16  nHeight, nIndexCol, nScript;
    UINT8   nAttr0;
    BYTE    nUnderline, nFamily, nCharSet;
    UINT8   nLen;
    UINT16  nWeight;

    aIn >> nHeight >> nAttr0;
    aIn.Ignore( 1 );
    aIn >> nIndexCol >> nWeight >> nScript >> nUnderline >> nFamily >> nCharSet;
    aIn.Ignore( 1 );    // Reserved

    aIn >> nLen;

    String aName( aIn.ReadUniString( nLen ) );

    // Font in Pool batschen
    pExcRoot->pFontBuffer->NewFont(
        nHeight, nAttr0, nScript, nUnderline, nIndexCol, nWeight,
        nFamily, nCharSet, aName );
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

    XclImpHelper::DecodeExternsheetUni( aIn, aFileName, aTabName, bSelf );

    if( !aTabName.Len() )
    {
        aTabName = aFileName;
        aFileName.Erase();
    }
    ScFilterTools::ConvertName( aTabName );
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

    ScFilterTools::ConvertName( aName );
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


void ImportExcel8::Xf( void )
{
#define HASATTRSET(m)       (!(nW12&m))
    static UINT16   nXFCnt = 0;
    UINT16      nW4, nW6, nW8, nW10, nW12, nW14, nW16, nW22;
    UINT32      nL18;
    CellBorder  aBorder;
    CellFill    aFill;
    XF_Data*    pXFD = new XF_Data;

    aIn >> nW4 >> nW6 >> nW8 >> nW10 >> nW12 >> nW14 >> nW16 >> nL18 >> nW22;

    const BOOL  bCellXF = ( nW8 & 0x0004 ) == 0;

    pXFD->SetCellXF( bCellXF );

    if( bCellXF || HASATTRSET( 0x0800 ) )
        pXFD->SetFont( nW4 );
    if( bCellXF || HASATTRSET( 0x0400 ) )
        pXFD->SetValueFormat( pValueFormBuffer->GetValueFormat( nW6 ) );
    if( bCellXF || HASATTRSET( 0x8000 ) )
    {
        pXFD->SetLocked( TRUEBOOL( nW8 & EXC_XF_LOCKED ) );
        pXFD->SetHidden( TRUEBOOL( nW8 & EXC_XF_HIDDEN ) );
    }
    if( bCellXF )
        pXFD->SetParent( nW8 >> 4 );
    if( bCellXF || HASATTRSET( 0x1000 ) )
    {
        pXFD->SetAlign( ( ExcHorizAlign ) ( nW10 & 0x0007 ) );
        if( nW10 & 0x0008 )
            pXFD->SetWrap( EWT_Wrap );
        pXFD->SetAlign( ( ExcVertAlign ) ( ( nW10 & 0x0070 ) >> 4 ) );
        pXFD->SetTextOrient( ( UINT8 ) ( nW10 >> 8 ) );
    }

    // nW12 >> 5 : 1      fMergeCell

    pXFD->SetIndent( nW12 & 0x000F );

    if( bCellXF || HASATTRSET( 0x2000 ) )
    {
        aBorder.nLeftLine = ( BYTE ) nW14 & 0x000F;
        nW14 >>= 4;
        aBorder.nRightLine = ( BYTE ) nW14 & 0x000F;
        nW14 >>= 4;
        aBorder.nTopLine = ( BYTE ) nW14 & 0x000F;
        nW14 >>= 4;
        aBorder.nBottomLine = ( BYTE ) nW14 & 0x000F;

        aBorder.nLeftColor = nW16 & 0x007F;
        aBorder.nRightColor = ( nW16 >> 7 ) & 0x007F;
        aBorder.nTopColor = ( UINT16 ) ( nL18 & 0x007F );
        aBorder.nBottomColor = ( UINT16 ) ( ( nL18 >> 7 ) & 0x007F );

        pXFD->SetBorder( aBorder );
    }

    if( bCellXF || HASATTRSET( 0x4000 ) )
    {
        aFill.nPattern = ( BYTE ) ( nL18 >> 26 );
        aFill.nForeColor = nW22 & 0x007F;
        aFill.nBackColor = ( nW22 & 0x3F80 ) >> 7;

        pXFD->SetFill( aFill );
    }

    if( nW12 & 0x0020 )
        pXFD->Merge();

    pExcRoot->pXF_Buffer->NewXF( pXFD );
    nXFCnt++;

#undef  HASATTRSET
}


void ImportExcel8::Cellmerging( void )
{
    UINT16  n, nR1, nR2, nC1, nC2;

    aIn >> n;

    DBG_ASSERT( aIn.GetRecLeft() >= (ULONG)(n * 8), "*ImportExcel8::Cellmerging(): in die Hose!" );

    while( n )
    {
        aIn >> nR1 >> nR2 >> nC1 >> nC2;

        if( nR1 > MAXROW )
            nR1 = MAXROW;
        if( nR2 > MAXROW )
            nR2 = MAXROW;
        if( nC1 > MAXCOL )
            nC1 = MAXCOL;
        if( nC2 > MAXCOL )
            nC2 = MAXCOL;

        pFltTab->AppendMerge( nC1, nR1, nC2, nR2 );
        n--;
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
            else if( pExcRoot->pXF_Buffer->HasAttribSuperOrSubscript( nXF ) )
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


void ImportExcel8::Dval( void )
{
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

        aColRowBuff.Used( nCol, nRow );

        pFltTab->SetXF( nCol, nRow, nXF );
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

        aColRowBuff.Used( nCol, nRow );

        pFltTab->SetXF( nCol, nRow, nXF );

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
        pExcRoot->pImpTabIdBuffer->Fill( aIn, (UINT16)(aIn.GetRecLen() >> 1) );
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


void ImportExcel8::Dv( void )
{
    UINT32      nFlags;

    aIn >> nFlags;

    String      aPromptTitle( aIn.ReadUniString() );
    String      aErrorTitle( aIn.ReadUniString() );
    String      aPromptMessage( aIn.ReadUniString() );
    String      aErrorMessage( aIn.ReadUniString() );

    // vals
    if( aIn.GetRecLeft() > 8 )
    {
        DVData*             p = new DVData;

        const ScTokenArray* pFrmla1 = NULL;
        const ScTokenArray* pFrmla2 = NULL;
        ScTokenArray*       pHelp = NULL;

        UINT16              nLen;

        aIn >> nLen;
        aIn.Ignore( 2 );
        if( nLen )
        {
            pFormConv->Reset();
            pFormConv->Convert( pFrmla1, nLen, FT_RangeName );
        }

        aIn >> nLen;
        aIn.Ignore( 2 );
        if( nLen )
        {
            if( pFrmla1 )
            {
                // copy unique ScTokenArry from formula converter!
                pHelp = pFrmla1->Clone();
                pFrmla1 = ( const ScTokenArray* ) pHelp;
            }

            pFormConv->Reset();
            pFormConv->Convert( pFrmla2, nLen, FT_RangeName );
        }

        aIn.Ignore( 2 );

        aIn >> p->nRow1 >> p->nRow2 >> p->nCol1 >> p->nCol2;

        ScValidationMode    eValMode;// = ( ScValidationMode ) ( nFlags & 0x00000007 );
        switch( nFlags & 0x00000007 )
        {
            case 0: eValMode = SC_VALID_ANY;            break;
            case 1: eValMode = SC_VALID_WHOLE;          break;
            case 2: eValMode = SC_VALID_DECIMAL;        break;
            case 3: eValMode = SC_VALID_LIST;           break;
            case 4: eValMode = SC_VALID_DATE;           break;
            case 5: eValMode = SC_VALID_TIME;           break;
            case 6: eValMode = SC_VALID_TEXTLEN;        break;
            case 7: eValMode = SC_VALID_CUSTOM;         break;
        }

        ScConditionMode     eMode;
        switch( ( nFlags >> 20 )  & 0x00000007 )
        {
            case 0x00:  eMode = SC_COND_BETWEEN;    break;
            case 0x01:  eMode = SC_COND_NOTBETWEEN; break;
            case 0x02:  eMode = SC_COND_EQUAL;      break;
            case 0x03:  eMode = SC_COND_NOTEQUAL;   break;
            case 0x04:  eMode = SC_COND_GREATER;    break;
            case 0x05:  eMode = SC_COND_LESS;       break;
            case 0x06:  eMode = SC_COND_EQGREATER;  break;
            case 0x07:  eMode = SC_COND_EQLESS;     break;
        }

        ScValidationData    aValidData( eValMode, eMode, pFrmla1, pFrmla2, pD, ScAddress( p->nCol1, p->nRow1, nTab ) );

        if( /*( nFlags & 0x00040000 ) ||*/ aPromptTitle.Len() || aPromptMessage.Len() )
            // ignore flag so behavior is similar to Excel
            aValidData.SetInput( aPromptTitle, aPromptMessage );

        if( /*( nFlags & 0x00080000 ) ||*/ aErrorTitle.Len() || aErrorMessage.Len() )
        {
            // ignore flag so behavior is similar to Excel

            ScValidErrorStyle   eErrStyle = ScValidErrorStyle( ( nFlags >> 4 ) & 0x03 );

            if( eErrStyle > SC_VALERR_INFO )
                eErrStyle = SC_VALERR_STOP;

            aValidData.SetError( aErrorTitle, aErrorMessage, eErrStyle );
        }

        p->nHandle = pD->AddValidationEntry( aValidData );

        if( !pDVList )
            pDVList = new DVList;

        pDVList->Add( p );

        if( pHelp )
            delete pHelp;
    }
}


static void lcl_GetAbs( String& rPath, UINT16 nDl, SfxObjectShell* pDocShell )
{
    String      aTmpStr;

    if( nDl )
    {
        while( nDl )
        {
            aTmpStr.AppendAscii( "../" );
            nDl--;
        }
    }

    aTmpStr += rPath;

    bool bWasAbs = false;
    rPath = pDocShell->GetMedium()->GetURLObject().smartRel2Abs( aTmpStr, bWasAbs ).GetMainURL(INetURLObject::NO_DECODE);
    // full path as stored in SvxURLField must be encoded
}


void ImportExcel8::Hlink( void )
{
    UINT16 nRF, nRL, nCF, nCL;
    UINT32 nFlags;

    aIn >> nRF >> nRL >> nCF >> nCL;
    aIn.Ignore( 20 );
    aIn >> nFlags;

    UINT16  nLevel = 0;             // counter for level to climb down in path
    String* pLongname = NULL;       // link / file name
    String* pShortname = NULL;      // 8.3-representation from file name
    String* pTextmark = NULL;       // text mark
    UINT32  nStrLen = 0;

    // description (ignore)
    if( nFlags & EXC_HLINK_DESCR )
    {
        aIn >> nStrLen;
        aIn.IgnoreRawUniString( (UINT16) nStrLen, TRUE );
    }

    // network path
    if( nFlags & EXC_HLINK_NET )
    {
        aIn >> nStrLen;
        pLongname = new String( aIn.ReadRawUniString( (UINT16) nStrLen, TRUE ) );
        lcl_GetAbs( *pLongname, 0, pD->GetDocumentShell() );
    }
    // file link or URL
    else if( nFlags & EXC_HLINK_BODY )
    {
        UINT32 nID;
        aIn >> nID;
        switch( nID )
        {
            case EXC_HLINK_ID_FILE:
            {
                aIn.Ignore( 12 );
                aIn >> nLevel >> nStrLen;
                pShortname = new String( aIn.ReadRawUniString( (UINT16) nStrLen, FALSE ) );
                aIn.Ignore( 24 );
                aIn >> nStrLen;
                if( nStrLen )
                {
                    aIn >> nStrLen;
                    aIn.Ignore( 2 );
                    pLongname = new String( aIn.ReadRawUniString( (UINT16)(nStrLen >> 1), TRUE ) );
                    lcl_GetAbs( *pLongname, nLevel, pD->GetDocumentShell() );
                }
                else
                    lcl_GetAbs( *pShortname, nLevel, pD->GetDocumentShell() );
            }
            break;
            case EXC_HLINK_ID_URL:
            {
                aIn.Ignore( 12 );
                aIn >> nStrLen;
                pLongname = new String( aIn.ReadRawUniString( (UINT16)(nStrLen >> 1), TRUE ) );
            }
            break;
            default:
                DBG_ERROR( "ImportExcel8::HLink - unknown content id" );
        }
    }

    // text mark
    if( nFlags & EXC_HLINK_MARK )
    {
        aIn >> nStrLen;
        pTextmark = new String( aIn.ReadRawUniString( (UINT16) nStrLen, TRUE ) );
    }

    DBG_ASSERT( !aIn.GetRecLeft(), "ImportExcel8::HLink - record size mismatch" );

    if( !pLongname && pShortname )
    {
        pLongname = pShortname;
        pShortname = NULL;
    }
    else if( !pLongname && pTextmark )
        pLongname = new String;

    if( pLongname )
    {
        if( pTextmark )
        {
            if( !pLongname->Len() )
                pTextmark->SearchAndReplaceAll( '!', '.' );
            *pLongname += '#';
            *pLongname += *pTextmark;
        }

        for( UINT16 nCol = nCF ; nCol <= nCL ; nCol++ )
            for( UINT16 nRow = nRF ; nRow <= nRL ; nRow++ )
                InsertHyperlink( nCol, nRow, *pLongname );
    }

    if( pLongname )
        delete pLongname;
    if( pShortname )
        delete pShortname;
    if( pTextmark )
        delete pTextmark;
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

    aColRowBuff.SetDimension(
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
        ScFilterTools::GetBuiltInName( aName, cFirstChar, nSheet );
    else
        ScFilterTools::ConvertName( aName );

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


void ImportExcel8::Style( void )
{
    UINT16      nXf;

    aIn >> nXf;

    if( !( nXf & 0x8000 ) )
    {
        nXf &= 0x0FFF;  // only bit 0...11 is used for XF-index

        pExcRoot->pXF_Buffer->SetStyle( nXf, aIn.ReadUniString() );
    }
}


void ImportExcel8::GetHFString( String& rStr )
{
    aIn.AppendUniString( rStr );
}


void ImportExcel8::EndSheet( void )
{
    pActCondForm = NULL;

    ImportExcel::EndSheet();

    if( pDVList )
    {
        pDVList->Apply( *pD, nTab );
        pDVList->Reset();
    }
}


void ImportExcel8::PostDocLoad( void )
{
    if( pCondFormList )
        pCondFormList->Apply();
    if( pAutoFilterBuffer )
        pAutoFilterBuffer->Apply();
    if( pWebQBuffer )
        pWebQBuffer->Apply( pD );

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
            UINT32                      nChartCnt = 0;
            const SvxMSDffShapeInfo*    p;
            ULONG                       nShapeId;
            SdrObject*                  pSdrObj = NULL;
            SvxMSDffImportData*         pMSDffImportData;
            UINT32                      n;
            BOOL                        bRangeTest;

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
                        bRangeTest = FALSE;
                        if( pAnch )
                        {
                            bRangeTest = aPivotTabList.IsInPivotRange( pAnch->nCol, pAnch->nRow, pAnch->nTab );
                            if( pAutoFilterBuffer )
                                bRangeTest |= pAutoFilterBuffer->HasDropDown( pAnch->nCol, pAnch->nRow, pAnch->nTab );
                        }
                        if( bRangeTest )
                            pObj->SetSdrObj( NULL );      // delete SdrObject
                        else
                        {
                            switch ( pObj->GetObjType() )
                            {
                                case otChart:
                                    nChartCnt++;
                                break;
                                case otOle:
                                    ((XclImpEscherOle*)pObj)->CreateSdrOle( *pDffMan, nOLEImpFlags );
                                break;
                                case otCtrl:
                                    if( bHasCtrls )
                                    {
                                        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                                                xShapeRef = GetXShapeForSdrObject( pSdrObj );
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
            if( pExcRoot->pProgress )
                pExcRoot->pProgress->StartPostLoadProgress( nChartCnt );
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
}



void ImportExcel8::InsertHyperlink( const UINT16 nCol, const UINT16 nRow, const String& rURL )
{
    ScAddress       aAddr( nCol, nRow, nTab );

    const CellType  e = pD->GetCellType( aAddr );

    if( e == CELLTYPE_FORMULA || e == CELLTYPE_VALUE )
        return;

    String          aOrgText;
    pD->GetString( nCol, nRow, nTab, aOrgText );
    if( !aOrgText.Len() )
        aOrgText = rURL;

    EditEngine&     rEdEng = GetEdEng();
    ESelection      aAppSel( 0xFFFF, 0xFFFF );

    rEdEng.SetText( EMPTY_STRING );

    SvxURLField     aUrlField( rURL, aOrgText, SVXURLFORMAT_APPDEFAULT );

    rEdEng.QuickInsertField( SvxFieldItem( aUrlField ), aAppSel );

    EditTextObject* pTextObj = rEdEng.CreateTextObject();

    ScBaseCell*     pCell = new ScEditCell( pTextObj, pD, GetEdEng().GetEditTextObjectPool() );

    delete pTextObj;

    pD->PutCell( aAddr, pCell, ( BOOL ) TRUE );
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
    pExcRoot->pExtsheetBuffer->AppendSupbook( new XclImpSupbook( aIn ) );
}

void ImportExcel8::Xct( void )
{
    XclImpSupbook* pSupbook = pExcRoot->pExtsheetBuffer->GetCurrSupbook();
    if( pSupbook )
        pSupbook->ReadXct( aIn );
}

void ImportExcel8::Crn( void )
{
    XclImpSupbook* pSupbook = pExcRoot->pExtsheetBuffer->GetCurrSupbook();
    if( pSupbook )
        pSupbook->ReadCrn( aIn, pFormConv );
}

void ImportExcel8::Externname( void )
{
    XclImpSupbook* pSupbook = pExcRoot->pExtsheetBuffer->GetCurrSupbook();
    if( pSupbook )
        pSupbook->ReadExternname( aIn, *pExcRoot );
}

void ImportExcel8::Externsheet( void )
{
    pExcRoot->pExtsheetBuffer->Read( aIn );
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
        pQuery->aTables = ScFilterTools::GetHTMLTablesName();
    }
    else
    {
        pQuery->eMode = xiwqDoc;
        pQuery->aTables = ScFilterTools::GetHTMLDocName();
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
            ScFilterTools::AddToken( aNewTables, ScFilterTools::GetNameFromHTMLIndex( (ULONG)nTabNum ), cSep );
        else
        {
            ScFilterTools::EraseQuotes( aToken );
            if( aToken.Len() )
                ScFilterTools::AddToken( aNewTables, ScFilterTools::GetNameFromHTMLName( aToken ), cSep );
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
                        CreateFromDouble( *aEntry.pStr, XclImpHelper::GetDoubleFromRK( nRK ) );
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

