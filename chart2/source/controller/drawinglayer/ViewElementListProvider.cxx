/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

//oldChartModelWrapper

// header for class SfxItemPool
#include <svl/itempool.hxx>
// header for class FontList
#include <svtools/ctrltool.hxx>
// header for class Application
#include <vcl/svapp.hxx>
// header for class SdrObject
#include <svx/svdobj.hxx>

//for creation of a symbol Graphic
// header for class VirtualDevice
#include <vcl/virdev.hxx>
// header for class SdrView
#include <svx/svdview.hxx>

namespace chart
{
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

XColorListRef   ViewElementListProvider::GetColorTable() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetColorList();
    return XColorListRef();
}

XDashListRef     ViewElementListProvider::GetDashList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetDashList();
    return XDashListRef();
}

XLineEndListRef  ViewElementListProvider::GetLineEndList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetLineEndList();
    return XLineEndListRef();
}

XGradientListRef ViewElementListProvider::GetGradientList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetGradientList();
    return XGradientListRef();
}
XHatchListRef    ViewElementListProvider::GetHatchList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetHatchList();
    return NULL;
}

XBitmapListRef   ViewElementListProvider::GetBitmapList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetBitmapList();
    return XBitmapListRef();
}

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
    catch( const uno::Exception& e )
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

    GDIMetaFile aMeta(pView->GetMarkedObjMetaFile());

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
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
