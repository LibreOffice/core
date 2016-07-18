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

#include "chartview/DrawModelWrapper.hxx"
#include "macros.hxx"
#include "AbstractShapeFactory.hxx"
#include "ChartItemPool.hxx"
#include "ObjectIdentifier.hxx"
#include <svx/unomodel.hxx>
#include <svl/itempool.hxx>
#include <svl/eitem.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svx3ditems.hxx>
#include <unotools/pathoptions.hxx>
#include <svx/objfac3d.hxx>
#include <svx/svdpage.hxx>
#include <svx/XPropertyTable.hxx>
#include <svx/xtable.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/unolingu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <sfx2/objsh.hxx>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>

using namespace ::com::sun::star;


namespace chart
{

DrawModelWrapper::DrawModelWrapper( const uno::Reference<uno::XComponentContext>& /*xContext*/ )
        : SdrModel( SvtPathOptions().GetPalettePath(), nullptr, nullptr, false )
        , m_pChartItemPool(nullptr)
        , m_xMainDrawPage(nullptr)
        , m_xHiddenDrawPage(nullptr)
{
    m_pChartItemPool = ChartItemPool::CreateChartItemPool();

    SetScaleUnit(MAP_100TH_MM);
    SetScaleFraction(Fraction(1, 1));
    SetDefaultFontHeight(423);     // 12pt

    SfxItemPool* pMasterPool = &GetItemPool();
    pMasterPool->SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
    pMasterPool->SetPoolDefaultItem(SfxBoolItem(EE_PARA_HYPHENATE, true) );
    pMasterPool->SetPoolDefaultItem(makeSvx3DPercentDiagonalItem (5));

    SfxItemPool* pPool = pMasterPool;
    // append chart pool to end of pool chain
    for (;;)
    {
        SfxItemPool* pSecondary = pPool->GetSecondaryPool();
        if (!pSecondary)
            break;

        pPool = pSecondary;
    }
    pPool->SetSecondaryPool(m_pChartItemPool);
    pMasterPool->FreezeIdRanges();

    //this factory needs to be created before first use of 3D scenes once upon an office runtime
    //@todo in future this should be done by drawing engine itself on demand
    static bool b3dFactoryInitialized = false;
    if(!b3dFactoryInitialized)
    {
        E3dObjFactory aObjFactory;
        b3dFactoryInitialized = true;
    }

    //Hyphenyation and spellchecking
    SdrOutliner& rOutliner = GetDrawOutliner();
    try
    {
        uno::Reference< linguistic2::XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
        if( xHyphenator.is() )
            rOutliner.SetHyphenator( xHyphenator );

        uno::Reference< linguistic2::XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
        if ( xSpellChecker.is() )
            rOutliner.SetSpeller( xSpellChecker );
    }
    catch(...)
    {
        OSL_FAIL("Can't get Hyphenator or SpellChecker for chart");
    }

    //ref device for font rendering
    OutputDevice* pDefaultDevice = rOutliner.GetRefDevice();
    if( !pDefaultDevice )
        pDefaultDevice = Application::GetDefaultDevice();
    m_pRefDevice.disposeAndClear();
    m_pRefDevice = VclPtr<VirtualDevice>::Create(*pDefaultDevice);
    MapMode aMapMode = m_pRefDevice->GetMapMode();
    aMapMode.SetMapUnit(MAP_100TH_MM);
    m_pRefDevice->SetMapMode(aMapMode);
    SetRefDevice(m_pRefDevice.get());
    rOutliner.SetRefDevice(m_pRefDevice.get());
}

DrawModelWrapper::~DrawModelWrapper()
{
    //remove m_pChartItemPool from pool chain
    if(m_pChartItemPool)
    {
        SfxItemPool* pPool = &GetItemPool();
        for (;;)
        {
            SfxItemPool* pSecondary = pPool->GetSecondaryPool();
            if(pSecondary == m_pChartItemPool)
            {
                pPool->SetSecondaryPool (nullptr);
                break;
            }
            pPool = pSecondary;
        }
        SfxItemPool::Free(m_pChartItemPool);
    }
    m_pRefDevice.disposeAndClear();
}

uno::Reference< uno::XInterface > DrawModelWrapper::createUnoModel()
{
    uno::Reference< lang::XComponent > xComponent = new SvxUnoDrawingModel( this ); //tell Andreas Schluens if SvxUnoDrawingModel is not needed anymore -> remove export from svx to avoid link problems in writer
    return uno::Reference< uno::XInterface >::query( xComponent );
}

uno::Reference< frame::XModel > DrawModelWrapper::getUnoModel()
{
    uno::Reference< uno::XInterface > xI = this->SdrModel::getUnoModel();
    return uno::Reference<frame::XModel>::query( xI );
}

SdrModel& DrawModelWrapper::getSdrModel()
{
    return *this;
}

uno::Reference< lang::XMultiServiceFactory > DrawModelWrapper::getShapeFactory()
{
    uno::Reference< lang::XMultiServiceFactory > xShapeFactory( this->getUnoModel(), uno::UNO_QUERY );
    return xShapeFactory;
}

uno::Reference< drawing::XDrawPage > const & DrawModelWrapper::getMainDrawPage()
{
    if (m_xMainDrawPage.is())
        return m_xMainDrawPage;

    // Create draw page.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSuplier(getUnoModel(), uno::UNO_QUERY);
    if (!xDrawPagesSuplier.is())
        return m_xMainDrawPage;

    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSuplier->getDrawPages();
    if (xDrawPages->getCount() > 1)
    {
        // Take the first page in case of multiple pages.
        uno::Any aPage = xDrawPages->getByIndex(0);
        aPage >>= m_xMainDrawPage;
    }

    if (!m_xMainDrawPage.is())
    {
        m_xMainDrawPage = xDrawPages->insertNewByIndex(0);
    }

    //ensure that additional shapes are in front of the chart objects so create the chart root before
    // let us disable this call for now
    // TODO:moggi
    // AbstractShapeFactory::getOrCreateShapeFactory(this->getShapeFactory())->getOrCreateChartRootShape( m_xMainDrawPage );
    return m_xMainDrawPage;
}
uno::Reference< drawing::XDrawPage > const & DrawModelWrapper::getHiddenDrawPage()
{
    if( !m_xHiddenDrawPage.is() )
    {
        uno::Reference< drawing::XDrawPagesSupplier > xDrawPagesSuplier( this->getUnoModel(), uno::UNO_QUERY );
        if( xDrawPagesSuplier.is() )
        {
            uno::Reference< drawing::XDrawPages > xDrawPages( xDrawPagesSuplier->getDrawPages () );
            if( xDrawPages->getCount()>1 )
            {
                uno::Any aPage = xDrawPages->getByIndex( 1 ) ;
                aPage >>= m_xHiddenDrawPage;
            }

            if(!m_xHiddenDrawPage.is())
            {
                if( xDrawPages->getCount()==0 )
                    m_xMainDrawPage = xDrawPages->insertNewByIndex( 0 );
                m_xHiddenDrawPage = xDrawPages->insertNewByIndex( 1 );
            }
        }
    }
    return m_xHiddenDrawPage;
}
void DrawModelWrapper::clearMainDrawPage()
{
    //uno::Reference<drawing::XShapes> xChartRoot( m_xMainDrawPage, uno::UNO_QUERY );
    uno::Reference<drawing::XShapes> xChartRoot( AbstractShapeFactory::getChartRootShape( m_xMainDrawPage ) );
    if( xChartRoot.is() )
    {
        sal_Int32 nSubCount = xChartRoot->getCount();
        uno::Reference< drawing::XShape > xShape;
        for( sal_Int32 nS = nSubCount; nS--; )
        {
            if( xChartRoot->getByIndex( nS ) >>= xShape )
                xChartRoot->remove( xShape );
        }
    }
}

uno::Reference< drawing::XShapes > DrawModelWrapper::getChartRootShape(
    const uno::Reference< drawing::XDrawPage>& xDrawPage )
{
    return AbstractShapeFactory::getChartRootShape( xDrawPage );
}

void DrawModelWrapper::lockControllers()
{
    uno::Reference< frame::XModel > xDrawModel( this->getUnoModel() );
    if( xDrawModel.is())
        xDrawModel->lockControllers();
}
void DrawModelWrapper::unlockControllers()
{
    uno::Reference< frame::XModel > xDrawModel( this->getUnoModel() );
    if( xDrawModel.is())
        xDrawModel->unlockControllers();
}

OutputDevice* DrawModelWrapper::getReferenceDevice() const
{
    return SdrModel::GetRefDevice();
}

SfxItemPool& DrawModelWrapper::GetItemPool()
{
    return this->SdrModel::GetItemPool();
}
XColorListRef DrawModelWrapper::GetColorList() const
{
    return this->SdrModel::GetColorList();
}
XDashListRef DrawModelWrapper::GetDashList() const
{
    return this->SdrModel::GetDashList();
}
XLineEndListRef DrawModelWrapper::GetLineEndList() const
{
    return this->SdrModel::GetLineEndList();
}
XGradientListRef DrawModelWrapper::GetGradientList() const
{
    return this->SdrModel::GetGradientList();
}
XHatchListRef DrawModelWrapper::GetHatchList() const
{
    return this->SdrModel::GetHatchList();
}
XBitmapListRef DrawModelWrapper::GetBitmapList() const
{
    return this->SdrModel::GetBitmapList();
}

SdrObject* DrawModelWrapper::getNamedSdrObject( const OUString& rName )
{
    if( rName.isEmpty() )
        return nullptr;
    return getNamedSdrObject( rName, GetPage(0) );
}

SdrObject* DrawModelWrapper::getNamedSdrObject( const OUString& rObjectCID, SdrObjList* pSearchList )
{
    if(!pSearchList || rObjectCID.isEmpty())
        return nullptr;
    const size_t nCount = pSearchList->GetObjCount();
    for( size_t nN=0; nN<nCount; ++nN )
    {
        SdrObject* pObj = pSearchList->GetObj(nN);
        if(!pObj)
            continue;
        if( ObjectIdentifier::areIdenticalObjects( rObjectCID, pObj->GetName() ) )
            return pObj;
        pObj = DrawModelWrapper::getNamedSdrObject( rObjectCID, pObj->GetSubList() );
        if(pObj)
            return pObj;
    }
    return nullptr;
}

bool DrawModelWrapper::removeShape( const uno::Reference< drawing::XShape >& xShape )
{
    uno::Reference< container::XChild > xChild( xShape, uno::UNO_QUERY );
    if( xChild.is() )
    {
        uno::Reference<drawing::XShapes> xShapes( xChild->getParent(), uno::UNO_QUERY );
        if( xShapes.is() )
        {
            xShapes->remove(xShape);
            return true;
        }
    }
    return false;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
