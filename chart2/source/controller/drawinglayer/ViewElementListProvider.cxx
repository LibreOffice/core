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

#include <memory>
#include <ViewElementListProvider.hxx>
#include <chartview/DrawModelWrapper.hxx>
#include <chartview/DataPointSymbolSupplier.hxx>
#include <DrawViewWrapper.hxx>

#include <com/sun/star/drawing/Direction3D.hpp>
#include <o3tl/safeint.hxx>
#include <svx/xtable.hxx>
#include <svtools/ctrltool.hxx>
#include <vcl/svapp.hxx>
#include <svx/svdobj.hxx>
#include <vcl/virdev.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpage.hxx>
#include <comphelper/diagnose_ex.hxx>

namespace chart
{
using namespace ::com::sun::star;

ViewElementListProvider::ViewElementListProvider( DrawModelWrapper* pDrawModelWrapper )
                        : m_pDrawModelWrapper( pDrawModelWrapper )
{
}

ViewElementListProvider::ViewElementListProvider(ViewElementListProvider&& rOther) noexcept
{
    m_pDrawModelWrapper = rOther.m_pDrawModelWrapper;
    m_pFontList = std::move(rOther.m_pFontList);
}

ViewElementListProvider::~ViewElementListProvider()
{
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
    return nullptr;
}

XBitmapListRef   ViewElementListProvider::GetBitmapList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetBitmapList();
    return XBitmapListRef();
}

XPatternListRef   ViewElementListProvider::GetPatternList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetPatternList();
    return XPatternListRef();
}

//create chartspecific symbols for linecharts
SdrObjList* ViewElementListProvider::GetSymbolList() const
{
    SdrObjList* pSymbolList = nullptr;
    try
    {
        //@todo use mutex

        //get hidden draw page (target):
        rtl::Reference<SvxDrawPage> xTarget = m_pDrawModelWrapper->getHiddenDrawPage();

        //create symbols via uno and convert to native sdr objects
        drawing::Direction3D aSymbolSize(220, 220, 0); // should be 250, but 250 -> 280 ??
        rtl::Reference< SvxShapeGroup > xSymbols
            = DataPointSymbolSupplier::create2DSymbolList(xTarget, aSymbolSize);

        SdrObject* pSdrObject = DrawViewWrapper::getSdrObject(
            uno::Reference<drawing::XShape>(static_cast<cppu::OWeakObject*>(xSymbols.get()), uno::UNO_QUERY));
        if (pSdrObject)
            pSymbolList = pSdrObject->GetSubList();
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return pSymbolList;
}

Graphic ViewElementListProvider::GetSymbolGraphic( sal_Int32 nStandardSymbol, const SfxItemSet* pSymbolShapeProperties ) const
{
    SdrObjList* pSymbolList = GetSymbolList();
    if( !pSymbolList->GetObjCount() )
        return Graphic();
    if(nStandardSymbol<0)
        nStandardSymbol*=-1;
    if( o3tl::make_unsigned(nStandardSymbol) >= pSymbolList->GetObjCount() )
        nStandardSymbol %= pSymbolList->GetObjCount();
    rtl::Reference<SdrObject> pObj = pSymbolList->GetObj(nStandardSymbol);

    ScopedVclPtrInstance< VirtualDevice > pVDev;
    pVDev->SetMapMode(MapMode(MapUnit::Map100thMM));

    std::unique_ptr<SdrModel> pModel(
        new SdrModel());

    rtl::Reference<SdrPage> pPage = new SdrPage( *pModel, false );
    pPage->SetSize(Size(1000,1000));
    pModel->InsertPage( pPage.get(), 0 );
    SdrView aView(*pModel, pVDev);
    aView.hideMarkHandles();
    SdrPageView* pPageView = aView.ShowSdrPage(pPage.get());

    // directly clone to target SdrModel
    pObj = pObj->CloneSdrObject(*pModel);

    pPage->NbcInsertObject(pObj.get());
    aView.MarkObj(pObj.get(),pPageView);
    if( pSymbolShapeProperties )
        pObj->SetMergedItemSet(*pSymbolShapeProperties);

    GDIMetaFile aMeta(aView.GetMarkedObjMetaFile());

    Graphic aGraph(aMeta);
    Size aSize = pObj->GetSnapRect().GetSize();
    aGraph.SetPrefSize(aSize);
    aGraph.SetPrefMapMode(MapMode(MapUnit::Map100thMM));

    aView.UnmarkAll();
    pObj=pPage->RemoveObject(0);
    // these need to die before the associated SdrModel
    pObj.clear();
    pPage.clear();

    return aGraph;
}

FontList* ViewElementListProvider::getFontList() const
{
    //was old chart:
    //SvxFontListItem* SfxObjectShell::.GetItem(SID_ATTR_CHAR_FONTLIST)

    if(!m_pFontList)
    {
        OutputDevice* pRefDev    = m_pDrawModelWrapper ? m_pDrawModelWrapper->getReferenceDevice() : nullptr;
        OutputDevice* pDefaultOut = Application::GetDefaultDevice();
        m_pFontList.reset( new FontList( pRefDev ? pRefDev    : pDefaultOut
                                       , pRefDev ? pDefaultOut : nullptr) );
    }
    return m_pFontList.get();
}
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
