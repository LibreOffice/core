/*************************************************************************
 *
 *  $RCSfile: excobj.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:21:48 $
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

#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif


#include <stdio.h>

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/colritem.hxx>
#include <svx/svddef.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpage.hxx>
#include <svx/xcolit.hxx>
#include <svx/xfillit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xsetit.hxx>
#include <svx/outlobj.hxx>
#include <svx/outliner.hxx>
#include <sfx2/objsh.hxx>
#include <so3/ipobj.hxx>
#include <so3/svstor.hxx>
#include <sch/schdll.hxx>
#include <sch/memchrt.hxx>
#include <svtools/itemset.hxx>
#include <svtools/moduleoptions.hxx>
#include <sfx2/app.hxx>
#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <sot/clsids.hxx>
#ifndef _SVDOGRAF_HXX //autogen wg. SdrGrafObj
#include <svx/svdograf.hxx>
#endif

#include <sfx2/interno.hxx>

#include "document.hxx"
#include "drwlayer.hxx"
#include "chartarr.hxx"
#include "editutil.hxx"

#include "imp_op.hxx"

#ifndef SC_XILINK_HXX
#include "xilink.hxx"
#endif
#ifndef SC_XCLIMPCHARTS_HXX
#include "XclImpCharts.hxx"
#endif

using namespace com::sun::star;

// Object Flags

#define OBJF_SELECTED   0x0001          // ---
#define OBJF_AUTOSIZE   0x0002          // zellgebunden
#define OBJF_MOVE       0x0004          // ---
#define OBJF_LOCKED     0x0010          // ---
#define OBJF_GROUPED    0x0080          // ---
#define OBJF_HIDDEN     0x0100          // ---
#define OBJF_VISIBLE    0x0200          // ---
#define OBJF_PRINTABLE  0x0400          // ---


void ImportExcel::Obj()
{
    UINT32 nObj;
    UINT16 nType, nId, nFlags;
    // Zeilen, Spalten, Offsets (in 1/1024 Zeile/Spalte)
    UINT16 nCol1, nColOff1, nRow1, nRowOff1;
    UINT16 nCol2, nColOff2, nRow2, nRowOff2;
    UINT16 nMacroLen;

    aIn >> nObj >> nType >> nId >> nFlags
        >> nCol1 >> nColOff1 >> nRow1 >> nRowOff1
        >> nCol2 >> nColOff2 >> nRow2 >> nRowOff2
        >> nMacroLen;
    BOOL bBiff5 = BOOL( pExcRoot->eHauptDateiTyp == Biff5 );
    short nReserved = bBiff5 ? 6 : 2;
    aIn.Ignore( nReserved );

    ScDocument& rDoc = GetDoc();
    sal_uInt16 nScTab = GetScTab();

    Point aUL(  XclTools::CalcX( rDoc, nScTab, nCol1, nColOff1, HMM_PER_TWIPS ),
                XclTools::CalcY( rDoc, nScTab, nRow1, nRowOff1, HMM_PER_TWIPS ) );

    Point aLR(  XclTools::CalcX( rDoc, nScTab, nCol2, nColOff2, HMM_PER_TWIPS ),
                XclTools::CalcY( rDoc, nScTab, nRow2, nRowOff2, HMM_PER_TWIPS ) );

    SfxItemSet aSet
        ( pD->GetDrawLayer()->GetItemPool(), SDRATTR_START, SDRATTR_END );
    SdrObject* pObj = NULL;
    switch( nType )
    {
        case 0x01:  pObj = LineObj( aSet, aUL, aLR ); break;
        case 0x02:  pObj = RectObj( aSet, aUL, aLR ); break;
        case 0x05:  pObj = BeginChartObj( aSet, aUL, aLR ); break;
    }
    if( pObj )
    {
//-/        pObj->SetAttributes( aSet, TRUE );
        pObj->ClearItem();
        pObj->SetItemSetAndBroadcast(aSet);

        pObj->NbcSetLogicRect(Rectangle( aUL, aLR ) );
        pObj->SetLayer( SC_LAYER_FRONT );
        pD->GetDrawLayer()->GetPage( nScTab )->InsertObject( pObj );
        if( bBiff5 && aIn.GetRecLeft() )
        {
            BYTE nNameLen;
            aIn >> nNameLen;
            aIn.Ignore( nNameLen + nMacroLen );
        }
    }
}


void ImportExcel::SetLineStyle( SfxItemSet& rSet, short nColor, short nStyle, short nWidth )
{
    if( nColor >= 0 )
        rSet.Put( XLineColorItem( String(), GetPalette().GetColor( nColor ) ) );
    if( nStyle >= 0 )
    {
        XLineStyle eStyle = ( nStyle == 1 || nStyle == 2 )
                          ? XLINE_DASH : XLINE_SOLID;
        rSet.Put( XLineStyleItem( eStyle ) );
    }
    if( nWidth >= 0 )
        rSet.Put( XLineWidthItem( nWidth * 40 ) );
}


void ImportExcel::SetFillStyle( SfxItemSet& rSet, short nBg, short nFg, short nStyle )
{
    if( nBg >= 0 )
        rSet.Put( XFillColorItem( String(), GetPalette().GetColor( nFg ) ) );
    if( nStyle >= 0 )
        // Entweder mit Farbe oder leer, keine Patterns!
        rSet.Put( XFillStyleItem( nStyle ? XFILL_SOLID : XFILL_NONE ) );
}


SdrObject* ImportExcel::LineObj( SfxItemSet& rSet, const Point& rUL, const Point& rLR )
{
    BYTE        nLc, nStyle, nWeight, nAuto;
    UINT16      nEndStyles;
    BYTE        nQuadrant;
    aIn >> nLc >> nStyle >> nWeight >> nAuto >> nEndStyles >> nQuadrant;
    aIn.Ignore( 1 );

    Point       aPointArr[2] = { rUL, rLR };
    SdrPathObj* pObj = new SdrPathObj( OBJ_LINE, XPolyPolygon( XPolygon( Polygon( 2, aPointArr ) ) ) );
    SetLineStyle( rSet, nLc, nStyle, nWeight );
    return pObj;
}


SdrObject* ImportExcel::RectObj( SfxItemSet& rSet, const Point& rUL, const Point& rLR )
{
    BYTE        nBg, nFg, nPat, fAuto1, nLc, nLStyle, nLWeight, fAuto2;
    UINT16      nFRS;
    aIn >> nBg >> nFg >> nPat >> fAuto1 >> nLc >> nLStyle
        >> nLWeight >> fAuto2 >> nFRS;

    SdrRectObj* pObj = new SdrRectObj( Rectangle( rUL, rLR ) );
    SetLineStyle( rSet, nLc, nLStyle, nLWeight );
    SetFillStyle( rSet, nBg, nFg, nPat );
    return pObj;
}


SdrObject* ImportExcel::BeginChartObj( SfxItemSet&, const Point& rUL, const Point& rLR )
{
    BYTE        nBg, nFg, nPat, fAuto1, nLc, nLStyle, nLWeight, fAuto2;
    UINT16      nFRS;
    aIn >> nBg >> nFg >> nPat >> fAuto1 >> nLc >> nLStyle
        >> nLWeight >> fAuto2 >> nFRS;
    aIn.Ignore( 18 );

    ExcelChartData* pData = new ExcelChartData( pD, rUL, rLR, GetScTab() );
    SetLineStyle( *pData->pAttrs, nLc, nLStyle, nLWeight );
    SetFillStyle( *pData->pAttrs, nBg, nFg, nPat );
    pData->pNext = pChart;
    pChart = pData;
    return NULL;
}


void ImportExcel::EndChartObj()
{
    DBG_ASSERT( pChart, "Kein Chart definiert!" );

    ExcelChartData*         pData = pChart;
    if( !pData )
        return;

    pChart = pData->pNext;

    if( pUsedChartFirst )
        pUsedChartLast->pNext = pData;
    else
        pUsedChartFirst = pData;

    pUsedChartLast = pData;

    pData->pNext = NULL;
}


ExcelChartData::ExcelChartData( ScDocument* p, const Point& rUL, const Point& rLR, const UINT16 nBT ) :
    aRect( rUL, rLR ),
    nRow1( 32767 ), nCol1( 32767 ), nTab1( 32767 ), nRow2( 0 ), nCol2( 0 ), nTab2( 0 ), pNext( NULL ),
    nBaseTab( nBT ), nObjNum( 0xFFFFFFFF )
{
    pAttrs = new SfxItemSet( p->GetDrawLayer()->GetItemPool(), SDRATTR_START, SDRATTR_END );
}


ExcelChartData::~ExcelChartData()
{
    delete pAttrs;
}


void ImportExcel::ChartSelection( void )
{
    UINT8       nId, nRef;
    UINT16      nFlags, nFmt, nSize;
    aIn >> nId >> nRef >> nFlags >> nFmt >> nSize;
    // Nur Links To Worksheet auswerten
    if( nRef == 2 && nSize )
    {
        // Der AI-Record enthaelt eine 3D Cell Reference (Opcode 0x3A)
        // oder eine 3D Area Reference (Opcode 0x3B) (oder auch nicht...)
        UINT8       nOp;
        UINT16      nLink;
        UINT16      nTab1, nTab2, nRow1, nRow2;
        UINT16      nCol1, nCol2;
        BOOL        bValues = FALSE;

        aIn >> nOp;

        if( pExcRoot->eHauptDateiTyp == Biff5 )
        {
            aIn >> nLink;

            if( nLink == 0xFFFF )
            {
                aIn.Ignore( 8 );
                switch( nOp )
                {
                    case 0x3A:
                    {
                        UINT8   nC;
                        aIn >> nTab1 >> nTab2 >> nRow1 >> nC;
                        nRow2 = nRow1;
                        nCol2 = nCol1 = nC;
                        bValues = TRUE;
                    }
                        break;
                    case 0x3B:
                    {
                        UINT8   nC1, nC2;
                        aIn >> nTab1 >> nTab2 >> nRow1 >> nRow2 >> nC1 >> nC2;
                        nCol1 = nC1;
                        nCol2 = nC2;
                        bValues = TRUE;
                    }
                        break;
                }

                nRow1 &= 0x3FFF;
                nRow2 &= 0x3FFF;
            }
        }
        else                                                    // Biff8 (?)
        {
            UINT16              nIxti;

            DBG_ASSERT( pExcRoot->eHauptDateiTyp == Biff8,
                "*ImportExcel::ChartSelection(): Den kenn' ich nicht!" );

            if( nOp == 0x3A || nOp == 0x3B )
            {
                if( nOp == 0x3A )
                {
                    aIn >> nIxti >> nRow1 >> nCol1;
                    nCol1 &= 0x3FFF;
                    nCol2 = nCol1;
                    nRow2 = nRow1;
                }
                else
                {
                    aIn >> nIxti >> nRow1 >> nRow2 >> nCol1 >> nCol2;
                    nCol1 &= 0x3FFF;
                    nCol2 &= 0x3FFF;
                }

                const XclImpXti* pXti = pExcRoot->pIR->GetLinkManager().GetXti( nIxti );
                const XclImpSupbook* pSupbook = pExcRoot->pIR->GetLinkManager().GetSupbook( nIxti );

                if( pXti && pSupbook /*&& pSupbook->IsSameSheet()*/ )
                {// in aktuellem Workbook
                    nTab1 = pXti->mnFirst;
                    nTab2 = pXti->mnLast;
                    bValues = TRUE;
                }
            }
        }   // Ende Biff8

        if( bValues )
        {
            DBG_ASSERT( pChart, "Keine Chartdaten!" );
            if( pChart )
            {
                if( pChart->nRow1 > nRow1 )
                    pChart->nRow1 = nRow1;
                if( pChart->nCol1 > nCol1 )
                    pChart->nCol1 = nCol1;
                if( pChart->nTab1 > nTab1 )
                    pChart->nTab1 = nTab1;
                if( pChart->nRow2 < nRow2 )
                    pChart->nRow2 = nRow2;
                if( pChart->nCol2 < nCol2 )
                    pChart->nCol2 = nCol2;
                if( pChart->nTab2 > nTab2 )
                    pChart->nTab2 = nTab2;
            }
        }
    }
}


void ImportExcel::ChartSeriesText()
{
    UINT16  nId;
    aIn >> nId;
    if( pChart )
        aIn.AppendByteString( pChart->aLastLabel, FALSE );
}


void ImportExcel::ChartObjectLink()
{
    UINT16 nType, nVar1, nVar2;
    aIn >> nType >> nVar1 >> nVar2;
    if( pChart && pChart->aLastLabel.Len() )
    {
        switch( nType )
        {
            case 1: pChart->aTitle = pChart->aLastLabel; break;
            case 2: pChart->aYTitle = pChart->aLastLabel; break;
            case 3: pChart->aXTitle = pChart->aLastLabel; break;
            case 7: pChart->aZTitle = pChart->aLastLabel; break;
        }
        pChart->aLastLabel.Erase();
    }
}


#ifdef WNT
#pragma optimize("",off)
#endif

void ImportExcel::EndAllChartObjects( void )
{
    SfxObjectShell* pSh = pD->GetDocumentShell();
    if( !pSh ) return;

    ExcelChartData*             p = pUsedChartFirst;
    ExcelChartData*             pDelete;

    while( p )
    {
        if( p->nRow1 <= p->nRow2 && p->nCol1 <= p->nCol2 )
        {
            SvInPlaceObjectRef  aIPObj;
            //  wenn Chart nicht installiert ist, darf nicht auf SCH_MOD zugegriffen werden!
            //! Warnung am Storage setzen?
            if ( SvtModuleOptions().IsChart() )
            {
                aIPObj = SvInPlaceObject::CreateObject( SvGlobalName( SO3_SCH_CLASSID ) );
            }
            if( aIPObj.Is() )
            {
                pSh->InsertObject(aIPObj, String());
                //          String aName = aIPObj->GetName()->GetName();

                String          aName;
                SvInfoObject*   pInfoObj = pSh->Find( aIPObj );

                if( pInfoObj )
                    aName = pInfoObj->GetObjName();
                else
                    DBG_ERROR( "IP-Object not found :-/" );


                Size            aSize = aIPObj->GetVisArea().GetSize();
                if( aSize.Height() == 0 || aSize.Width() == 0 )
                {
                    aSize.Width() = 5000;
                    aSize.Height() = 5000;
                    aSize = Window::LogicToLogic
                                ( aSize, MapMode( MAP_100TH_MM ), MapMode( aIPObj->GetMapUnit() ) );
                    aIPObj->SetVisAreaSize( aSize );
                }

                pD->LimitChartArea( p->nTab1, p->nCol1, p->nRow1, p->nCol2, p->nRow2 );

                SdrOle2Obj*     pSdrObj = new SdrOle2Obj( aIPObj, aName, p->aRect );

//-/                pSdrObj->SetAttributes( *p->pAttrs, TRUE );
                pSdrObj->ClearItem();
                pSdrObj->SetItemSetAndBroadcast(*p->pAttrs);

                pSdrObj->SetLayer( SC_LAYER_FRONT );

                pD->GetDrawLayer()->GetPage( p->nBaseTab )->InsertObject( pSdrObj );

                pSdrObj->NbcSetLogicRect( p->aRect );

                ScChartArray    aChartObj
                                ( pD, p->nTab1, p->nCol1, p->nRow1, p->nCol2, p->nRow2, aName );

                SchMemChart*    pMemChart = aChartObj.CreateMemChart();
                SchDLL::Update( aIPObj, pMemChart );
                delete pMemChart;
            }
        }

        pDelete = p;
        p = p->pNext;

        delete pDelete;
    }

    pUsedChartFirst = pUsedChartLast = NULL;

    p = pChart;
    while( p )
    {
        pDelete = p;
        p = p->pNext;
        delete pDelete;
    }

}



#ifdef WNT
#pragma optimize("",on)
#endif
