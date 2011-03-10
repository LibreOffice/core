/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "ViewElementListProvider.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "chartview/DataPointSymbolSupplier.hxx"
#include "macros.hxx"
#include "DrawViewWrapper.hxx"
#include <svx/xtable.hxx>
#include <svx/XPropertyTable.hxx>
#include <svx/unofill.hxx>
#include <svx/unoapi.hxx>

// header for class NameOrIndex
#include <svx/xit.hxx>
// header for class XFillBitmapItem
#include <svx/xbtmpit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflgrit.hxx>
// header for class XLineStartItem
#include <svx/xlnstit.hxx>
// header for class XLineEndItem
#include <svx/xlnedit.hxx>

//------------
//oldChartModelWrapper

// header for class SfxItemPool
#include <svl/itempool.hxx>
// header for class FontList
#include <svtools/ctrltool.hxx>
// header for class Application
#include <vcl/svapp.hxx>
//------------
// header for class SdrObject
#include <svx/svdobj.hxx>


//---------------
//for creation of a symbol Graphic
// header for class VirtualDevice
#include <vcl/virdev.hxx>
// header for class SdrView
#include <svx/svdview.hxx>
//---------------

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

ViewElementListProvider::ViewElementListProvider( DrawModelWrapper* pDrawModelWrapper )
                        : m_pDrawModelWrapper( pDrawModelWrapper )
                        , m_pFontList(NULL)
{
}

ViewElementListProvider::~ViewElementListProvider()
{
    if(m_pFontList)
        delete m_pFontList;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

XColorTable*   ViewElementListProvider::GetColorTable() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetColorTable();
    return NULL;
}
XDashList*     ViewElementListProvider::GetDashList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetDashList();
    return NULL;
}
XLineEndList*  ViewElementListProvider::GetLineEndList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetLineEndList();
    return NULL;
}
XGradientList* ViewElementListProvider::GetGradientList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetGradientList();
    return NULL;
}
XHatchList*    ViewElementListProvider::GetHatchList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetHatchList();
    return NULL;
}
XBitmapList*   ViewElementListProvider::GetBitmapList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetBitmapList();
    return NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//create chartspecific symbols for linecharts
SdrObjList* ViewElementListProvider::GetSymbolList() const
{
    SdrObjList* m_pSymbolList = NULL;
    uno::Reference< drawing::XShapes > m_xSymbols(NULL);//@todo this keeps the first drawinglayer alive ...
    try
    {
        if(!m_pSymbolList || !m_pSymbolList->GetObjCount())
        {
            //@todo use mutex

            //get shape factory
            uno::Reference< lang::XMultiServiceFactory > xShapeFactory( m_pDrawModelWrapper->getShapeFactory() );

            //get hidden draw page (target):
            uno::Reference<drawing::XShapes> xTarget( m_pDrawModelWrapper->getHiddenDrawPage(), uno::UNO_QUERY );

            //create symbols via uno and convert to native sdr objects
            drawing::Direction3D aSymbolSize(220,220,0); // should be 250, but 250 -> 280 ??
            m_xSymbols =  DataPointSymbolSupplier::create2DSymbolList( xShapeFactory, xTarget, aSymbolSize );

            SdrObject* pSdrObject = DrawViewWrapper::getSdrObject( uno::Reference< drawing::XShape >( m_xSymbols, uno::UNO_QUERY ) );
            if(pSdrObject)
                m_pSymbolList = pSdrObject->GetSubList();
        }
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
    return m_pSymbolList;
}

Graphic ViewElementListProvider::GetSymbolGraphic( sal_Int32 nStandardSymbol, const SfxItemSet* pSymbolShapeProperties ) const
{
    SdrObjList* pSymbolList = this->GetSymbolList();
    if( !pSymbolList->GetObjCount() )
        return Graphic();
    if(nStandardSymbol<0)
        nStandardSymbol*=-1;
    if( nStandardSymbol >= static_cast<sal_Int32>(pSymbolList->GetObjCount()) )
        nStandardSymbol %= pSymbolList->GetObjCount();
    SdrObject* pObj = pSymbolList->GetObj(nStandardSymbol);

    VirtualDevice aVDev;
    aVDev.SetMapMode(MapMode(MAP_100TH_MM));
    SdrModel* pModel = new SdrModel();
    pModel->GetItemPool().FreezeIdRanges();
    SdrPage* pPage = new SdrPage( *pModel, sal_False );
    pPage->SetSize(Size(1000,1000));
    pModel->InsertPage( pPage, 0 );
    SdrView* pView = new SdrView( pModel, &aVDev );
    pView->hideMarkHandles();
    SdrPageView* pPageView = pView->ShowSdrPage(pPage);

    pObj=pObj->Clone();
    pPage->NbcInsertObject(pObj);
    pView->MarkObj(pObj,pPageView);
    if( pSymbolShapeProperties )
        pObj->SetMergedItemSet(*pSymbolShapeProperties);

    GDIMetaFile aMeta(pView->GetAllMarkedMetaFile());

    Graphic aGraph(aMeta);
    Size aSize = pObj->GetSnapRect().GetSize();
    aGraph.SetPrefSize(aSize);
    aGraph.SetPrefMapMode(MAP_100TH_MM);

    pView->UnmarkAll();
    pObj=pPage->RemoveObject(0);
    SdrObject::Free( pObj );
    delete pView;
    delete pModel;

    return aGraph;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

FontList* ViewElementListProvider::getFontList() const
{
    //was old chart:
    //SvxFontListItem* SfxObjectShell::.GetItem(SID_ATTR_CHAR_FONTLIST)

    if(!m_pFontList)
    {
        OutputDevice* pRefDev    = m_pDrawModelWrapper ? m_pDrawModelWrapper->getReferenceDevice() : NULL;
        OutputDevice* pDefaultOut = Application::GetDefaultDevice();
        m_pFontList = new FontList( pRefDev ? pRefDev    : pDefaultOut
                                , pRefDev ? pDefaultOut : NULL
                                , sal_False );
    }
    return m_pFontList;
}
//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
