/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DrawModelWrapper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2007-06-11 15:04:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "chartview/DrawModelWrapper.hxx"
#include "macros.hxx"
#include "ShapeFactory.hxx"
#include "ChartItemPool.hxx"
#include "ObjectIdentifier.hxx"

#ifndef SVX_UNOMODEL_HXX
#include <svx/unomodel.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
// header for class SfxBoolItem
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
// header for define EE_PARA_HYPHENATE
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
// header for class Svx3DPercentDiagonalItem
#ifndef _SVX3DITEMS_HXX
#include <svx/svx3ditems.hxx>
#endif
// header for class SvtPathOptions
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
// header E3dObjFactory
#ifndef _OBJFAC3D_HXX
#include <svx/objfac3d.hxx>
#endif
// header for class SdrObjList
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
// header for SvxUnoXDashTable_createInstance
#ifndef _SVX_XPROPERTYTABLE_HXX
#include <svx/XPropertyTable.hxx>
#endif
// header for class XDashList
#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif
// header for class SdrOutliner
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif
// header for class LinguMgr
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
// header for class Application
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
// header for class VirtualDevice
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <sfx2/objsh.hxx>

#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATOR_HPP_
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLCHECKER1_HPP_
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#endif

using namespace ::com::sun::star;

//.............................................................................
namespace
{
// this code is copied from sfx2/source/doc/objembed.cxx
SfxObjectShell * lcl_GetParentObjectShell( const uno::Reference< frame::XModel > & xModel )
{
    SfxObjectShell* pResult = NULL;

    try
    {
        uno::Reference< container::XChild > xChildModel( xModel, uno::UNO_QUERY );
        if ( xChildModel.is() )
        {
            uno::Reference< lang::XUnoTunnel > xParentTunnel( xChildModel->getParent(), uno::UNO_QUERY );
            if ( xParentTunnel.is() )
            {
                SvGlobalName aSfxIdent( SFX_GLOBAL_CLASSID );
                pResult = reinterpret_cast< SfxObjectShell * >(
                    xParentTunnel->getSomething( uno::Sequence< sal_Int8 >( aSfxIdent.GetByteSequence() ) ) );
            }
        }
    }
    catch( uno::Exception& )
    {
        // TODO: error handling
    }

    return pResult;
}

// this code is copied from sfx2/source/doc/objembed.cxx.  It is a workaround to
// get the reference device (e.g. printer) fromthe parent document
OutputDevice * lcl_GetParentRefDevice( const uno::Reference< frame::XModel > & xModel )
{
    SfxObjectShell * pParent = lcl_GetParentObjectShell( xModel );
    if ( pParent )
        return pParent->GetDocumentRefDev();
    return NULL;
}

} // anonymous namespace

//..    ...........................................................................
namespace chart
{
//.............................................................................

DrawModelWrapper::DrawModelWrapper(
        uno::Reference<uno::XComponentContext> const & xContext )
        : SdrModel( SvtPathOptions().GetPalettePath() )
        , m_xMCF(0)
        , m_pChartItemPool(0)
        , m_xMainDrawPage(0)
        , m_xHiddenDrawPage(0)
        , m_apRefDevice(0)
{
    m_pChartItemPool = ChartItemPool::CreateChartItemPool();

    m_xMCF = xContext->getServiceManager();

    SetScaleUnit(MAP_100TH_MM);
    SetScaleFraction(Fraction(1, 1));
    SetDefaultFontHeight(847);     // 24pt

    SfxItemPool* pMasterPool = &GetItemPool();
    pMasterPool->SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
    pMasterPool->SetPoolDefaultItem(SfxBoolItem(EE_PARA_HYPHENATE, TRUE) );
    pMasterPool->SetPoolDefaultItem(Svx3DPercentDiagonalItem (5));

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
        DBG_ERROR("Can't get Hyphenator or SpellChecker for chart");
    }

    //ref device for font rendering
    OutputDevice* pDefaultDevice = rOutliner.GetRefDevice();
    if( !pDefaultDevice )
        pDefaultDevice = Application::GetDefaultDevice();
    m_apRefDevice = std::auto_ptr< OutputDevice >( new VirtualDevice( *pDefaultDevice ) );
    MapMode aMapMode = m_apRefDevice->GetMapMode();
    aMapMode.SetMapUnit(MAP_100TH_MM);
    m_apRefDevice->SetMapMode(aMapMode);
    SetRefDevice(m_apRefDevice.get());
    rOutliner.SetRefDevice(m_apRefDevice.get());
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
                pPool->SetSecondaryPool (NULL);
                break;
            }
            pPool = pSecondary;
        }
        delete m_pChartItemPool;
    }
}

uno::Reference< uno::XInterface > DrawModelWrapper
::createUnoModel()
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

uno::Reference< drawing::XDrawPage > DrawModelWrapper::getMainDrawPage()
{
    //create draw page:
    if( !m_xMainDrawPage.is() )
    {
        uno::Reference< drawing::XDrawPagesSupplier > xDrawPagesSuplier( this->getUnoModel(), uno::UNO_QUERY );
        if( xDrawPagesSuplier.is() )
        {
            uno::Reference< drawing::XDrawPages > xDrawPages( xDrawPagesSuplier->getDrawPages () );
            if( xDrawPages->getCount()>1 )
            {
                uno::Any aPage = xDrawPages->getByIndex( 0 ) ;
                aPage >>= m_xMainDrawPage;
            }
            if(!m_xMainDrawPage.is())
            {
                m_xMainDrawPage = xDrawPages->insertNewByIndex( 0 );
            }
        }
    }
    //ensure that additional shapes are in front of the chart objects so create the chart root before
    ShapeFactory(this->getShapeFactory()).getOrCreateChartRootShape( m_xMainDrawPage );
    return m_xMainDrawPage;
}
uno::Reference< drawing::XDrawPage > DrawModelWrapper::getHiddenDrawPage()
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
    uno::Reference<drawing::XShapes> xChartRoot( ShapeFactory::getChartRootShape( m_xMainDrawPage ) );
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
    return ShapeFactory::getChartRootShape( xDrawPage );
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

void DrawModelWrapper::attachParentReferenceDevice( const uno::Reference< frame::XModel > & xChartModel )
{
    OutputDevice * pParentRefDev( lcl_GetParentRefDevice( xChartModel ));
    if( pParentRefDev )
    {
        SetRefDevice( pParentRefDev );
    }
}

OutputDevice* DrawModelWrapper::getReferenceDevice() const
{
    return SdrModel::GetRefDevice();
}

SfxItemPool& DrawModelWrapper::GetItemPool()
{
    return this->SdrModel::GetItemPool();
}
const SfxItemPool& DrawModelWrapper::GetItemPool() const
{
    return this->SdrModel::GetItemPool();
}
XColorTable* DrawModelWrapper::GetColorTable() const
{
    return this->SdrModel::GetColorTable();
}
XDashList* DrawModelWrapper::GetDashList() const
{
    return this->SdrModel::GetDashList();
}
XLineEndList* DrawModelWrapper::GetLineEndList() const
{
    return this->SdrModel::GetLineEndList();
}
XGradientList* DrawModelWrapper::GetGradientList() const
{
    return this->SdrModel::GetGradientList();
}
XHatchList* DrawModelWrapper::GetHatchList() const
{
    return this->SdrModel::GetHatchList();
}
XBitmapList* DrawModelWrapper::GetBitmapList() const
{
    return this->SdrModel::GetBitmapList();
}

SdrObject* DrawModelWrapper::getNamedSdrObject( const rtl::OUString& rName )
{
    if(rName.getLength()==0)
        return 0;
    return getNamedSdrObject( rName, GetPage(0) );
}

//static
SdrObject* DrawModelWrapper::getNamedSdrObject( const String& rObjectCID, SdrObjList* pSearchList )
{
    if(!pSearchList || rObjectCID.Len()==0)
        return 0;
    ULONG nCount = pSearchList->GetObjCount();
    for( ULONG nN=0; nN<nCount; nN++  )
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
    return 0;
}

namespace
{
    void impl_addOrExchangeElements( const uno::Reference< uno::XInterface >& xSource
        , const uno::Reference< uno::XInterface >& xTarget )
    {
        uno::Reference< container::XNameContainer > xSourceContainer( xSource, uno::UNO_QUERY );
        uno::Reference< container::XNameContainer > xTargetContainer( xTarget, uno::UNO_QUERY );

        if(!xSourceContainer.is()||!xTargetContainer.is())
            return;

        try
        {
            //put each element of xSourceContainer to xTargetContainer
            uno::Sequence< rtl::OUString > aNames( xSourceContainer->getElementNames() );
            for( sal_Int32 nN = aNames.getLength(); nN--; )
            {
                rtl::OUString aName(aNames[nN]);
                uno::Any aNewValue( xSourceContainer->getByName( aName ) );
                if( xTargetContainer->hasByName(aName) )
                {
                    uno::Any aOldValue( xTargetContainer->getByName( aName ) );
                    if( aOldValue != aNewValue )
                        xTargetContainer->replaceByName(aName,aNewValue);
                }
                else
                    xTargetContainer->insertByName( aName, aNewValue );
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

void DrawModelWrapper::updateTablesFromChartModel( const uno::Reference< frame::XModel >& xChartModel )
{
    //all gradients, etc contained in the model need to be offered fot the view and GUI also:

    DBG_ASSERT( xChartModel.is(),"need a chart model for updateTablesFromChartModel");
    if( !xChartModel.is() )
        return;

    uno::Reference< lang::XMultiServiceFactory > xTableFactory( xChartModel, uno::UNO_QUERY );
    DBG_ASSERT( xTableFactory.is(), "new model is expected to implement service factory for gradient table etc" );
    if( !xTableFactory.is()  )
        return;

    //todo? colorTable...
    //impl_addOrExchangeElements( xTableFactory->createInstance( C2U("com.sun.star.drawing.XXX") )
    //                          , SvxUnoXColorTable_createInstance( this->GetColorTable() ) );
    impl_addOrExchangeElements( xTableFactory->createInstance( C2U("com.sun.star.drawing.DashTable") )
                              , SvxUnoXDashTable_createInstance( this->GetDashList() ) );
    impl_addOrExchangeElements( xTableFactory->createInstance( C2U("com.sun.star.drawing.MarkerTable") )
                              , SvxUnoXLineEndTable_createInstance( this->GetLineEndList() ) );
    impl_addOrExchangeElements( xTableFactory->createInstance( C2U("com.sun.star.drawing.GradientTable") )
                              , SvxUnoXGradientTable_createInstance( this->GetGradientList() ) );
    impl_addOrExchangeElements( xTableFactory->createInstance( C2U("com.sun.star.drawing.HatchTable") )
                              , SvxUnoXHatchTable_createInstance( this->GetHatchList() ) );
    impl_addOrExchangeElements( xTableFactory->createInstance( C2U("com.sun.star.drawing.BitmapTable") )
                              , SvxUnoXBitmapTable_createInstance( this->GetBitmapList() ) );

    // transparency gradients
    uno::Reference< uno::XInterface > xSource(
        xTableFactory->createInstance( C2U("com.sun.star.drawing.TransparencyGradientTable")));
    uno::Reference< uno::XInterface > xTarget(
        this->getShapeFactory()->createInstance( C2U("com.sun.star.drawing.TransparencyGradientTable")));
    impl_addOrExchangeElements( xSource, xTarget );
}

//static
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

//.............................................................................
} //namespace chart
//.............................................................................
