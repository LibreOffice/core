/*************************************************************************
 *
 *  $RCSfile: excobj.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: gt $ $Date: 2001-02-20 15:19:02 $
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

#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM        // Transformationen, selten verwendet
#define _SDR_NOTOUCH            // Hit-Tests, selten verwendet
#define _SDR_NOEXTDEV           // ExtOutputDevice
//#define   _SDR_NOUNDO             // Undo-Objekte
#define _SDR_NOSURROGATEOBJ     // SdrObjSurrogate
#define _SDR_NOPAGEOBJ          // SdrPageObj
#define _SDR_NOVIRTOBJ          // SdrVirtObj
#define _SDR_NOGROUPOBJ         // SdrGroupObj
#define _SDR_NOTEXTOBJ          // SdrTextObj
#define _SDR_NOPATHOBJ          // SdrPathObj
#define _SDR_NOEDGEOBJ          // SdrEdgeObj
#define _SDR_NORECTOBJ          // SdrRectObj
#define _SDR_NOCAPTIONOBJ       // SdrCaptionObj
#define _SDR_NOCIRCLEOBJ        // SdrCircleObj
#define _SDR_NOGRAFOBJ          // SdrGrafObj
#define _SDR_NOOLE2OBJ          // SdrOle2Obj
#define SI_NOOTHERFORMS
#define SI_NOSBXCONTROLS
#define _VCBRW_HXX
#define _VCTRLS_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX

//------------------------------------------------------------------------

//#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
//#include <com/sun/star/form/XFormsSupplier.hpp>
//#endif
//#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
//#include <com/sun/star/form/XForm.hpp>
//#endif
//#ifndef _COM_SUN_STAR_FORM_XIMAGEPRODUCERSUPPLIER_HPP_
//#include <com/sun/star/form/XImageProducerSupplier.hpp>
//#endif
//#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
//#include <com/sun/star/form/XFormController.hpp>
//#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
//#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLERLISTENER_HPP_
//#include <com/sun/star/form/XFormControllerListener.hpp>
//#endif
//#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
//#include <com/sun/star/frame/XStorable.hpp>
//#endif
//#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
//#include <com/sun/star/frame/XModel.hpp>
//#endif
//#ifndef _COM_SUN_STAR_DRAWING_XCONNECTABLESHAPE_HPP_
//#include <com/sun/star/drawing/XConnectableShape.hpp>
//#endif
//#ifndef _COM_SUN_STAR_DRAWING_XCONNECTORSHAPE_HPP_
//#include <com/sun/star/drawing/XConnectorShape.hpp>
//#endif
//#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
//#include <com/sun/star/drawing/XShape.hpp>
//#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
//#ifndef _COM_SUN_STAR_DRAWING_XSHAPEALIGNER_HPP_
//#include <com/sun/star/drawing/XShapeAligner.hpp>
//#endif
//#ifndef _COM_SUN_STAR_DRAWING_XSHAPEGROUP_HPP_
//#include <com/sun/star/drawing/XShapeGroup.hpp>
//#endif
//#ifndef _COM_SUN_STAR_DRAWING_XSHAPEDESCRIPTOR_HPP_
//#include <com/sun/star/drawing/XShapeDescriptor.hpp>
//#endif
//#ifndef _COM_SUN_STAR_DRAWING_XUNIVERSALSHAPEDESCRIPTOR_HPP_
//#include <com/sun/star/drawing/XUniversalShapeDescriptor.hpp>
//#endif
//#ifndef _COM_SUN_STAR_DRAWING_XSHAPEMIRROR_HPP_
//#include <com/sun/star/drawing/XShapeMirror.hpp>
//#endif
//#ifndef _COM_SUN_STAR_DRAWING_XSHAPEARRANGER_HPP_
//#include <com/sun/star/drawing/XShapeArranger.hpp>
//#endif
//#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
//#include <com/sun/star/drawing/XDrawPage.hpp>
//#endif
//#ifndef _COM_SUN_STAR_DRAWING_XSHAPEBINDER_HPP_
//#include <com/sun/star/drawing/XShapeBinder.hpp>
//#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
//#ifndef _COM_SUN_STAR_DRAWING_XSHAPEGROUPER_HPP_
//#include <com/sun/star/drawing/XShapeGrouper.hpp>
//#endif
//#ifndef _COM_SUN_STAR_DRAWING_XSHAPECOMBINER_HPP_
//#include <com/sun/star/drawing/XShapeCombiner.hpp>
//#endif
//#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
//#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
//#endif
//#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
//#include <com/sun/star/lang/XMultiServiceFactory.hpp>
//#endif
//#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
//#include <com/sun/star/lang/XSingleServiceFactory.hpp>
//#endif
//#ifndef _COM_SUN_STAR_LOADER_XIMPLEMENTATIONLOADER_HPP_
//#include <com/sun/star/loader/XImplementationLoader.hpp>
//#endif
//#ifndef _COM_SUN_STAR_LOADER_CANNOTACTIVATEFACTORYEXCEPTION_HPP_
//#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
//#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
//#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
//#include <com/sun/star/text/VertOrientation.hpp>
//#endif
//#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
//#include <com/sun/star/text/TextContentAnchorType.hpp>
//#endif


#include <stdio.h>

#include "scitems.hxx"
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
#ifndef _SCHDLL0_HXX
#include <sch/schdll0.hxx>
#endif
#include <svtools/itemset.hxx>
#include <svtools/moduleoptions.hxx>
#include <sfx2/app.hxx>
#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#ifndef _SVDOGRAF_HXX //autogen wg. SdrGrafObj
#include <svx/svdograf.hxx>
#endif

#include <sfx2/interno.hxx>

#include "document.hxx"
#include "drwlayer.hxx"
#include "chartarr.hxx"
#include "editutil.hxx"

#include "imp_op.hxx"
#include "excimp8.hxx"
#include "fontbuff.hxx"
#include "fltprgrs.hxx"
#include "scmsocximexp.hxx"

#ifndef _SC_XCLIMPCHARTS_HXX
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

    Point aUL(  CalcX( nTab, nCol1, nColOff1, HMM_PER_TWIPS, pD ),
                CalcY( nTab, nRow1, nRowOff1, HMM_PER_TWIPS, pD ) );

    Point aLR(  CalcX( nTab, nCol2, nColOff2, HMM_PER_TWIPS, pD ),
                CalcY( nTab, nRow2, nRowOff2, HMM_PER_TWIPS, pD ) );

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
        pD->GetDrawLayer()->GetPage( nTab )->InsertObject( pObj );
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
    {
        const SvxColorItem* pColor = pExcRoot->pColor->GetColor( nColor );
        rSet.Put( XLineColorItem( String(), pColor->GetValue() ) );
    }
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
    {
        const SvxColorItem* pColor = pExcRoot->pColor->GetColor( nFg );
        rSet.Put( XFillColorItem( String(), pColor->GetValue() ) );
    }
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

    ExcelChartData* pData = new ExcelChartData( pD, rUL, rLR, nTab );
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

                const XclImpXti* pXti = pExcRoot->pXtiBuffer->Get( nIxti );

                if( pXti )
                {
                    const XclImpSupbook* pSbE = pExcRoot->pSupbookBuffer->Get( pXti->nSupbook );

                    if( pSbE /*&& pSbE->IsSameSheet()*/ )
                    {// in aktuellem Workbook
                        nTab1 = pXti->nFirst;
                        nTab2 = pXti->nLast;
                        bValues = TRUE;
                    }
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
        aIn.AppendByteString( pChart->aLastLabel, eQuellChar, FALSE );
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
    ExcelChartData*             p = pUsedChartFirst;
    ExcelChartData*             pDelete;

    while( p )
    {
        if( p->nRow1 <= p->nRow2 && p->nCol1 <= p->nCol2 )
        {
            SfxObjectShell*     pSh = pD->GetDocumentShell();
            SvStorageRef        aStor = new SvStorage( String() );
            SvInPlaceObjectRef  aIPObj;
            //  wenn Chart nicht installiert ist, darf nicht auf SCH_MOD zugegriffen werden!
            //! Warnung am Storage setzen?
            if ( SvtModuleOptions().IsChart() )
            {
#ifndef SO3
                aIPObj = &SvInPlaceObject::ClassFactory()->CreateAndInit
                    ( *SCH_MOD()->pSchChartDocShellFactory, aStor );
#else
                aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit
                    ( *SCH_MOD()->pSchChartDocShellFactory, aStor );
#endif
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




void ImportExcel8::EndAllChartObjects( void )
{
}


void ImportExcel8::EndChartObj( void )
{
}




ExcEscherObj::ExcEscherObj( const UINT32 nS, const UINT32 nE, const UINT16 nT, RootData* p ) : ExcRoot( p )
{
    pAnchor = NULL;
    pSdrObj = NULL;
    nStrPosStart = nS;
    nStrPosEnd = nE;
    nTab = nT;
    nId = 0xFFFF;
}


ExcEscherObj::ExcEscherObj( ExcEscherObj& r ) : ExcRoot( r.pExcRoot )
{
    if( r.pAnchor )
        pAnchor = new Rectangle( *r.pAnchor );
    else
        pAnchor = NULL;

    if( r.pSdrObj )
    {
        pSdrObj = r.pSdrObj;
        r.pSdrObj = NULL;
    }
    else
        pSdrObj = NULL;

    nStrPosStart = r.nStrPosStart;
    nStrPosEnd = r.nStrPosEnd;
    nTab = r.nTab;
    nId = r.nId;
}


ExcEscherObj::~ExcEscherObj()
{
    if( pAnchor )
        delete pAnchor;
    if( pSdrObj )
        delete pSdrObj;
}


OBJECTTYPE ExcEscherObj::GetObjType( void ) const
{
    return OT_UNKNOWN;
}


void ExcEscherObj::SetAnchor( const Rectangle& r )
{
    if( pAnchor )
        *pAnchor = r;
    else
        pAnchor = new Rectangle( r );
}


void ExcEscherObj::SetObj( SdrObject* p )
{
    if( pSdrObj )
        delete pSdrObj;

    pSdrObj = p;
}


void ExcEscherObj::Apply( void )
{
    if( pSdrObj && pAnchor )
    {
        pExcRoot->pDoc->GetDrawLayer()->GetPage( nTab )->InsertObject( pSdrObj );

        pSdrObj = NULL;
    }
}


void ExcEscherObj::MorpheFrom( ExcEscherObj*& rpMAD )
{
    ExcEscherObj&   r = *rpMAD;

    nStrPosStart = r.nStrPosStart;
    nStrPosEnd = r.nStrPosEnd;
    nTab = r.nTab;
    nId = r.nId;

    pAnchor = r.pAnchor;
    r.pAnchor = NULL;

    pSdrObj = r.pSdrObj;
    r.pSdrObj = NULL;

    delete rpMAD;
    rpMAD = NULL;
}


ExcChart* ExcEscherObj::GetChartData( void )
{
    return NULL;
}


void ExcEscherObj::SetChartData( ExcChart* p )
{
}




ExcEscherChart::ExcEscherChart( ExcEscherObj*& rpCAD ) : ExcEscherObj( *rpCAD )
{
    delete rpCAD;
    rpCAD = NULL;

    pChrtData = new ExcChart( pExcRoot );

    pChrtData->nBaseTab = nTab;
}


ExcEscherChart::~ExcEscherChart()
{
    if( pChrtData )
    {
        delete pChrtData;
#ifdef DEBUG
        pChrtData = ( ExcChart* ) 0xFFFF7777;
#endif
    }
}


OBJECTTYPE ExcEscherChart::GetObjType( void ) const
{
    return OT_CHART;
}


ExcChart* ExcEscherChart::GetChartData( void )
{
    return pChrtData;
}


void ExcEscherChart::SetChartData( ExcChart* p )
{
    if( pChrtData )
        delete pChrtData;

    pChrtData = p;
}


void ExcEscherChart::Apply( void )
{
    // Setz-Teil vom alten EndAllChartObjects()
    ExcChart*                   p = pChrtData;

    if( p && p->GetSourceData() && pAnchor )
    {
        p->CloseSourceData();

        const ExcChart_LinkedData*  p1 = p->GetSourceData();

        ScDocument*                 pD = pExcRoot->pDoc;

        SfxObjectShell*             pSh = pD->GetDocumentShell();
        SvStorageRef                aStor = new SvStorage( String() );
        SvInPlaceObjectRef          aIPObj;
        Rectangle&                  rRect = *pAnchor;

        //  wenn Chart nicht installiert ist, darf nicht auf SCH_MOD zugegriffen werden!
        //! Warnung am Storage setzen?
        if ( SvtModuleOptions().IsChart() )
    {
#ifndef SO3
            aIPObj = &SvInPlaceObject::ClassFactory()->CreateAndInit
                ( *SCH_MOD()->pSchChartDocShellFactory, aStor );
#else
            aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit
                ( *SCH_MOD()->pSchChartDocShellFactory, aStor );
#endif
    }
        if( aIPObj.Is() )
        {
            pSh->InsertObject(aIPObj, String());

            String              aName;
            SvInfoObject*       pInfoObj = pSh->Find( aIPObj );

            if( pInfoObj )
                aName = pInfoObj->GetObjName();
            else
                DBG_ERROR( "IP-Object not found :-/" );

            BOOL bEnabled = aIPObj->IsEnableSetModified();
            if (bEnabled)
                aIPObj->EnableSetModified(FALSE);

            Size                aSize = aIPObj->GetVisArea().GetSize();
            if( aSize.Height() == 0 || aSize.Width() == 0 )
            {
                aSize.Width() = 5000;
                aSize.Height() = 5000;
                aSize = Window::LogicToLogic
                            ( aSize, MapMode( MAP_100TH_MM ), MapMode( aIPObj->GetMapUnit() ) );
                aIPObj->SetVisAreaSize( aSize );
            }

            SdrOle2Obj*         pSdrObj = new SdrOle2Obj( aIPObj, aName, rRect );

            pSdrObj->SetLayer( SC_LAYER_FRONT );

            pD->GetDrawLayer()->GetPage( p->nBaseTab )->InsertObject( pSdrObj );

            pSdrObj->NbcSetLogicRect( rRect );

            ScChartArray        aChartObj( pD, p1->GetRangeList(), aName );

            BOOL bSwap = ( p1 && p1->GetDir() );
            BOOL bColHdr = bSwap ? p->bHasSeriesNames : p->bHasCategoryNames;
            BOOL bRowHdr = bSwap ? p->bHasCategoryNames : p->bHasSeriesNames;
            aChartObj.SetHeaders( bColHdr, bRowHdr );

            SchMemChart*        pMemChart = aChartObj.CreateMemChart();
            SchDLL::Update( aIPObj, pMemChart );
            delete pMemChart;

            SfxInPlaceObjectRef aSfxObj( aIPObj );
            if( aSfxObj.Is() )
            {
                SfxObjectShell* pObjSh = aSfxObj->GetObjectShell();
                if( pObjSh )
                {
                    uno::Reference<lang::XComponent>    x = pObjSh->GetModel().get();
                    if( x.is() )
                        p->Apply( x, rRect );
                }
            }

            if (bEnabled)
                aIPObj->EnableSetModified(TRUE);

            aIPObj->SetModified();
            aIPObj->DoSave();
            aIPObj->DoSaveCompleted();
            aIPObj->GetStorage()->Commit();
        }
    }
}




ExcEscherOle::ExcEscherOle( ExcEscherObj*& rpCAD ):
        ExcEscherObj( *rpCAD ),
        nBlipId( 0 ),
        bAsSymbol( FALSE ),
        bLinked( FALSE ),
        eType( OT_OLE )
{
    delete rpCAD;
    rpCAD = NULL;
}


ExcEscherOle::~ExcEscherOle()
{
}


OBJECTTYPE ExcEscherOle::GetObjType( void ) const
{
    return eType;
}


void ExcEscherOle::ReadPictFmla( XclImpStream& rIn, UINT16 nLen )
{
    UINT32  nStorageId;
    UINT16  nFmlaLen;
    rIn >> nFmlaLen;

    String  aUserName;
    ULONG   nPos0 = rIn.GetRecPos();        // fmla start
    BOOL    bOk = TRUE;
    if ( bLinked )
    {
        BOOL    bSizeOk = (sizeof(nFmlaLen) + nFmlaLen == nLen);
        DBG_ASSERT( bSizeOk, "ExcEscherOle::ReadPictFmla: bad linked size" );
        UINT16  n16;
        rIn >> n16;     // should be 7 but who knows ...
        bOk = (n16 + 3 <= nFmlaLen);
        DBG_ASSERT( bOk, "ExcEscherOle::ReadPictFmla: linked length mismatch" );
        if ( bOk )
        {
            rIn.Ignore( n16 );
            UINT8 n8;
            rIn >> n8;
            bOk = (n8 == 0x01);
            DBG_ASSERT( bOk, "ExcEscherOle::ReadPictFmla: no sheet start" );
            if ( bOk )
            {
                rIn >> n16;     // EXTERNSHEET based 0, EXTERNNAME based 1
                const ExtName*  pExtName = pExcRoot->pExtNameBuff->GetName( n16 + 1 );
                bOk = (pExtName && pExtName->IsOLE());
                DBG_ASSERT( bOk, "ExcEscherOle::ReadPictFmla: EXTERNNAME not found or not OLE" );
                if ( bOk )
                    nStorageId = pExtName->nStorageId;
            }
        }
    }
    else
    {
        UINT16  n16;
        rIn >> n16;     // should be 5 but who knows ...
        DBG_ASSERT( n16 + 4 <= nFmlaLen, "ExcEscherOle::ReadPictFmla: embedded length mismatch" );
        if ( n16 + 4 <= nFmlaLen )
        {
            rIn.Ignore( n16 + 4 );
            UINT8 n8;
            rIn >> n8;
            DBG_ASSERT( n8 == 0x03, "ExcEscherOle::ReadPictFmla: no name start" );
            if ( n8 == 0x03 )
            {
                rIn >> n16;     // string length
                if ( n16 )
                {   // the 4th way Xcl stores a unicode string: not even a Grbit byte present if length 0
                    INT32 nLeft = INT32(nFmlaLen) - (rIn.GetRecPos() - nPos0);
                    rIn.AppendUniString( aUserName, *pExcRoot->pCharset, n16 );
                    // 0:= ID follows, 1:= pad byte + ID
                    DBG_ASSERT( nLeft == 0 || nLeft == 1, "ExcEscherOle::ReadPictFmla: unknown left over" );

                    if( aUserName.EqualsAscii( "Forms.", 0, 6 ) )
                        eType = OT_CTRL;
                }
            }
        }
        rIn.Seek( nPos0 + nFmlaLen );
        rIn >> nStorageId;
        if ( nStorageId )
        {
            BOOL bSizeOk = sizeof(nFmlaLen) + nFmlaLen + sizeof(nStorageId) == nLen;
            DBG_ASSERT( bSizeOk, "ExcEscherOle::ReadPictFmla: bad embedded size" );
        }
        else
            bOk = FALSE;        // no storage, internal
    }
    if ( bOk )
    {
        aStorageName.AssignAscii( bLinked ? "LNK" : "MBD" );
        sal_Char    aBuf[ sizeof(UINT32) * 2 + 1 ];
        sprintf( aBuf, "%08X", nStorageId );
        aStorageName.AppendAscii( aBuf );
    }
}


void ExcEscherOle::CreateSdrOle( Biff8MSDffManager& rDffMan, UINT32 nOLEImpFlgs )
{
    if( pAnchor && aStorageName.Len() )
    {
        Graphic             aGraph;
        if( rDffMan.GetBLIP( nBlipId, aGraph ) )
        {
            SvStorageRef        xSrc = pExcRoot->pRootStorage;
            SvStorageRef        xDst( pExcRoot->pDoc->GetDocumentShell()->GetStorage() );
            UINT32              nOLEImpFlgs = 0;

            SdrOle2Obj*     pRet = SvxMSDffManager::CreateSdrOLEFromStorage(
                    aStorageName, xSrc, xDst, aGraph, *pAnchor, NULL, nOLEImpFlgs );
            if( pRet )
                SetObj( pRet );
        }
    }
}


void ExcEscherOle::Apply()
{
    if( pSdrObj && pAnchor )
    {
        if( pSdrObj->ISA( SdrOle2Obj ) )
        {
            SvInfoObject*   pInfoObj =
                pExcRoot->pDoc->GetDocumentShell()->InsertObject(
                ((SdrOle2Obj*)pSdrObj)->GetObjRef(), EMPTY_STRING );

            DBG_ASSERT( pInfoObj, "ExcEscherOle::Apply: no InfoObject" );

            if( pInfoObj )
                ((SdrOle2Obj*)pSdrObj)->SetName( pInfoObj->GetObjName() );
        }

        pExcRoot->pDoc->GetDrawLayer()->GetPage( nTab )->InsertObject( pSdrObj );
        pSdrObj = NULL;
    }
}




ExcEscherDrwObj::ExcEscherDrwObj( ExcEscherObj*& rpCAD ): ExcEscherObj( *rpCAD )
{
    delete rpCAD;
    rpCAD = NULL;
}


ExcEscherDrwObj::~ExcEscherDrwObj()
{
}


OBJECTTYPE ExcEscherDrwObj::GetObjType( void ) const
{
    return OT_PICTURE;
}




void ExcEscherTxo::ApplyTextOnObject( SdrObject* p_Obj )
{
    if( bIsApplied )
        return;

    SdrObject*  pObj = p_Obj? p_Obj : pSdrObj;

    if( pObj && pObj->ISA( SdrTextObj ) )
    {
        SdrTextObj*             pTxtObj = ( SdrTextObj* ) pObj;

        bIsApplied = TRUE;

        if( pFormText )
        {
            OutlinerParaObject* pOPO = new OutlinerParaObject( *pFormText );
            pOPO->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );
            pTxtObj->NbcSetOutlinerParaObject( pOPO );
        }
        else if( pText )
            pTxtObj->SetText( *pText );
        else
            bIsApplied = FALSE;
    }
}


ExcEscherTxo::ExcEscherTxo( ExcEscherObj*& rpCAD ): ExcEscherObj( *rpCAD )
{
    pText = NULL;
    pFormText = NULL;

    delete rpCAD;
    rpCAD = NULL;

    bIsApplied = FALSE;
}


ExcEscherTxo::~ExcEscherTxo()
{
    if( pText )
        delete pText;
}


OBJECTTYPE ExcEscherTxo::GetObjType( void ) const
{
    return OT_TXO;
}


void ExcEscherTxo::SetObj( SdrObject* p )
{
    ExcEscherObj::SetObj( p );

    ApplyTextOnObject();
}


void ExcEscherTxo::SetText( const String& r )
{
    if( pText )
        *pText = r;
    else
        pText = new String( r );

    ApplyTextOnObject();
}


void ExcEscherTxo::TakeTxo( TxoCont& r )
{
    if( r.HasFormText() )
    {
        if( pFormText )
            delete pFormText;

        pFormText = r.pFormText;

        r.pFormText = NULL;         // jetzt hab ich's!
    }

    if( r.HasText() )
    {
        if( pText )
            delete pText;

        pText = r.pText;

        r.pText = NULL;             // noch einen!
    }

    ApplyTextOnObject();
}


void ExcEscherTxo::Apply( void )
{
    ApplyTextOnObject();

    ExcEscherObj::Apply();
}


void ExcEscherTxo::Apply( SdrObject* p )
{
    ApplyTextOnObject( p );
}


ExcEscherNote::ExcEscherNote( ExcEscherObj*& rpCAD ) : ExcEscherTxo( rpCAD )
{
}


OBJECTTYPE ExcEscherNote::GetObjType( void ) const
{
    return OT_NOTE;
}


void ExcEscherNote::Apply( void )
{
}




BOOL ExcEscherObjList::SetTabStart( const UINT16 n )
{
    if( n == nLastReqTab )
        return TRUE;

    UINT32          nSrchIndx;
    if( n > nLastReqTab )
        nSrchIndx = nLastReqTabStart + 1;
    else
        nSrchIndx = 0;

    ExcEscherObj*   p = ( ExcEscherObj* ) GetObject( nSrchIndx );

    while( p )
    {
        if( p->nTab == n )
        {
            nLastReqTab = n;
            nLastReqTabStart = nSrchIndx;

            return TRUE;
        }

        nSrchIndx++;
        p = ( ExcEscherObj* ) GetObject( nSrchIndx );
    }

    return FALSE;
}


ExcEscherObjList::ExcEscherObjList( PosBuffer& r, RootData* p ) : rPosBuffer( r ), ExcRoot( p )
{
    nLastReqTabStart = 0;
    nLastReqTab = 0;
}


ExcEscherObjList::~ExcEscherObjList()
{
    ExcEscherObj*   p = ( ExcEscherObj* ) First();

    while( p )
    {
        delete p;
        p = ( ExcEscherObj* ) Next();
    }
}


void ExcEscherObjList::Append( ExcEscherObj* p )
{
    rPosBuffer.Append( p->nStrPosStart, p->nStrPosEnd, Count(), p->nTab );

    List::Insert( p, LIST_APPEND );
}


void ExcEscherObjList::MorpheLastObj( ExcEscherObj* p )
{
    UINT32          n = Count();

    if( n )
    {
        n--;

        ExcEscherObj*   pOld = ( ExcEscherObj* ) GetObject( n );

        p->MorpheFrom( pOld );

        Replace( p, n );
    }
}


const ExcEscherObj* ExcEscherObjList::Get( const UINT32 n, const UINT16 nTab ) const
{
    if( nTab == 0xFFFF )
        return ( const ExcEscherObj* ) List::GetObject( n );
    else if( ( ( ExcEscherObjList* ) this )->SetTabStart( nTab ) )
    {
            UINT32 nPos = nLastReqTabStart;
            const ExcEscherObj* p;
            while ( (p = ( const ExcEscherObj* ) List::GetObject( nPos )) &&
                    p->nTab == nTab )
        {
                if ( n == p->GetId() )
                    return p;
                ++nPos;
        }
    }
    return NULL;
}


ExcEscherTxo* ExcEscherObjList::GetTxo( const UINT32 nObjNum, const UINT16 nTab ) const
{
    const ExcEscherObj*     p = Get( nObjNum, nTab );

    if( p->GetObjType() == OT_TXO )
        return ( ExcEscherTxo* ) p;
    else
        return NULL;
}


void ExcEscherObjList::SetData( const UINT32 n, SdrObject* pDrObj )
{
    ExcEscherObj*   p = ( ExcEscherObj* ) Get( n );

    if( p )
        p ->SetObj( pDrObj );
    else
        delete pDrObj;
}


void ExcEscherObjList::SetData( const UINT32 n, const Rectangle& r )
{
    ExcEscherObj*   p = ( ExcEscherObj* ) Get( n );

    if( p )
        p->SetAnchor( r );
}


void ExcEscherObjList::SetData( const UINT32 n, const Rectangle& rA, SdrObject* pDrObj )
{
    ExcEscherObj*   p = ( ExcEscherObj* ) Get( n );

    if( p )
    {
        p->SetAnchor( rA );

        p->SetObj( pDrObj );
    }
    else
        delete pDrObj;
}


void ExcEscherObjList::Apply( void )
{
    ExcEscherObj*   p = ( ExcEscherObj* ) First();

    while( p )
    {
        p->Apply();

        if( p->GetObjType() == OT_CHART && pExcRoot->pProgress )
            pExcRoot->pProgress->Progress();

        p = ( ExcEscherObj* ) Next();
    }
}




using namespace ::com::sun::star;

BOOL ScMSConvertControls::InsertControl(
    const uno::Reference< form::XFormComponent >& rFComp,
    const awt::Size& rSize,
    uno::Reference< drawing::XShape >* pShape,BOOL bFloatingCtrl )
{
    return FALSE;
}
