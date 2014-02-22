/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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


#include <svx/xit.hxx>

#include <svx/xbtmpit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflgrit.hxx>

#include <svx/xlnstit.hxx>

#include <svx/xlnedit.hxx>




#include <svl/itempool.hxx>

#include <svtools/ctrltool.hxx>

#include <vcl/svapp.hxx>

#include <svx/svdobj.hxx>



#include <vcl/virdev.hxx>

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


SdrObjList* ViewElementListProvider::GetSymbolList() const
{
    SdrObjList* m_pSymbolList = NULL;
    uno::Reference< drawing::XShapes > m_xSymbols(NULL);
    try
    {
        if(!m_pSymbolList || !m_pSymbolList->GetObjCount())
        {
            

            
            uno::Reference< lang::XMultiServiceFactory > xShapeFactory( m_pDrawModelWrapper->getShapeFactory() );

            
            uno::Reference<drawing::XShapes> xTarget( m_pDrawModelWrapper->getHiddenDrawPage(), uno::UNO_QUERY );

            
            drawing::Direction3D aSymbolSize(220,220,0); 
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
    SdrPage* pPage = new SdrPage( *pModel, false );
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
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
