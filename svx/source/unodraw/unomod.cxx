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

#include <sal/config.h>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svl/itemprop.hxx>
#include <svtools/unoevent.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <cppuhelper/implbase.hxx>
#include <svx/unofill.hxx>
#include <editeng/unonrule.hxx>
#include <svtools/unoimap.hxx>
#include <svx/fmdpage.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <sfx2/sfx.hrc>
#include <svx/unoapi.hxx>

#include <svx/svdmodel.hxx>
#include "svx/globl3d.hxx"
#include <svx/svdtypes.hxx>
#include <svx/unoprov.hxx>
#include <svx/unopage.hxx>
#include <editeng/unofield.hxx>
#include <svx/unomod.hxx>
#include <svx/unomodel.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/unoshape.hxx>
#include <svx/xmlgrhlp.hxx>

#include <com/sun/star/text/textfield/Type.hpp>

//-

using namespace ::osl;
using namespace ::com::sun::star;

//-

#define QUERYINT( xint ) \
    if( rType == cppu::UnoType<xint>::get() ) \
        aAny <<= uno::Reference< xint >(this)

//-

class SvxUnoDrawPagesAccess : public ::cppu::WeakImplHelper< css::drawing::XDrawPages, css::lang::XServiceInfo >
{
private:
    SvxUnoDrawingModel& mrModel;

public:
    explicit SvxUnoDrawPagesAccess( SvxUnoDrawingModel& rMyModel ) throw();

    // XDrawPages
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL insertNewByIndex( sal_Int32 nIndex ) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XDrawPage >& xPage ) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};
//-

static const SvEventDescription* ImplGetSupportedMacroItems()
{
    static const SvEventDescription aMacroDescriptionsImpl[] =
    {
        { SFX_EVENT_MOUSEOVER_OBJECT, "OnMouseOver" },
        { SFX_EVENT_MOUSEOUT_OBJECT, "OnMouseOut" },
        { 0, nullptr }
    };

    return aMacroDescriptionsImpl;
}

//-

/** fills the given EventObject from the given SdrHint.
    @returns
        true    if the SdrHint could be translated to an EventObject<br>
        false   if not
*/
bool SvxUnoDrawMSFactory::createEvent( const SdrModel* pDoc, const SdrHint* pSdrHint, css::document::EventObject& aEvent )
{
    const SdrObject* pObj = nullptr;
    const SdrPage* pPage = nullptr;

    switch( pSdrHint->GetKind() )
    {
//              case SdrHintKind::LayerChange:             // layer definition changed
//              case SdrHintKind::LayerOrderChange:        // layer order changed (Insert/Remove/ChangePos)
//              case HINT_LAYERSETCHG:          // layer set changed
//              case HINT_LAYERSETORDERCHG:     // layer set order changed (Insert/Remove/ChangePos)

//      case HINT_PAGECHG:              // page changed
//          aEvent.EventName = "PageModified";
//          pPage = pSdrHint->GetPage();
//          break;
        case SdrHintKind::PageOrderChange:         // draw or master page order changed (Insert/Remove/ChangePos)
            aEvent.EventName = "PageOrderModified";
            pPage = pSdrHint->GetPage();
            break;
        case SdrHintKind::ObjectChange:               // object changed
            aEvent.EventName = "ShapeModified";
            pObj = pSdrHint->GetObject();
            break;
        case SdrHintKind::ObjectInserted:          // add new draw object
            aEvent.EventName = "ShapeInserted";
            pObj = pSdrHint->GetObject();
            break;
        case SdrHintKind::ObjectRemoved:           // removed draw object from list
            aEvent.EventName = "ShapeRemoved";
            pObj = pSdrHint->GetObject();
            break;
//                SdrHintKind::DefaultTabChange,   // default tab width changed
//                SdrHintKind::DefaultFontHeightChange,   // default FontHeight changed
//                SdrHintKind::SwitchToPage,    // #94278# UNDO/REDO at an object evtl. on another page
//                HINT_OBJLISTCLEAR     // Is called before an SdrObjList will be cleared
        default:
            return false;
    }

    if( pObj )
        aEvent.Source = const_cast<SdrObject*>(pObj)->getUnoShape();
    else if( pPage )
        aEvent.Source = const_cast<SdrPage*>(pPage)->getUnoPage();
    else
        aEvent.Source = (const_cast<SdrModel*>(pDoc))->getUnoModel();

    return true;
}

namespace {

css::uno::Reference<css::uno::XInterface> create(
    OUString const & rServiceSpecifier, OUString const & referer)
{
    if( rServiceSpecifier.startsWith("com.sun.star.drawing.") )
    {
        sal_uInt32 nType = UHashMap::getId( rServiceSpecifier );
        if( nType != UHASHMAP_NOTFOUND )
        {
            sal_uInt16 nT = (sal_uInt16)(nType & ~E3D_INVENTOR_FLAG);
            SdrInventor nI = (nType & E3D_INVENTOR_FLAG) ? SdrInventor::E3d : SdrInventor::Default;

            return uno::Reference< uno::XInterface >( static_cast<drawing::XShape*>(SvxDrawPage::CreateShapeByTypeAndInventor( nT, nI, nullptr, nullptr, referer )) );
        }
    }
    else if ( rServiceSpecifier == "com.sun.star.document.ImportGraphicObjectResolver" )
    {
        SvXMLGraphicHelper* pGraphicHelper = SvXMLGraphicHelper::Create( SvXMLGraphicHelperMode::Read );
        uno::Reference< uno::XInterface> xRet( static_cast< ::cppu::OWeakObject* >( pGraphicHelper ) );
        pGraphicHelper->release();
        return xRet;
    }

    uno::Reference< uno::XInterface > xRet( SvxUnoDrawMSFactory::createTextField( rServiceSpecifier ) );
    if( !xRet.is() )
        throw lang::ServiceNotRegisteredException("unknown service: " + rServiceSpecifier);

    return xRet;
}

}

uno::Reference< uno::XInterface > SAL_CALL SvxUnoDrawMSFactory::createInstance( const OUString& rServiceSpecifier )
{
    return create(rServiceSpecifier, "");
}

uno::Reference< uno::XInterface > SAL_CALL SvxUnoDrawMSFactory::createTextField( const OUString& ServiceSpecifier )
{
    return SvxUnoTextCreateTextField( ServiceSpecifier );
}

uno::Reference< uno::XInterface > SAL_CALL SvxUnoDrawMSFactory::createInstanceWithArguments( const OUString& ServiceSpecifier, const uno::Sequence< css::uno::Any >& Arguments )
{
    OUString arg;
    if ((ServiceSpecifier == "com.sun.star.drawing.GraphicObjectShape"
         || ServiceSpecifier == "com.sun.star.drawing.MediaShape")
        && Arguments.getLength() == 1 && (Arguments[0] >>= arg))
    {
        return create(ServiceSpecifier, arg);
    }
    throw lang::NoSupportException();
}

uno::Sequence< OUString > SAL_CALL SvxUnoDrawMSFactory::getAvailableServiceNames()
{
    return UHashMap::getServiceNames();
}

uno::Sequence< OUString > SvxUnoDrawMSFactory::concatServiceNames( uno::Sequence< OUString >& rServices1, uno::Sequence< OUString >& rServices2 ) throw()
{
    const sal_Int32 nLen1 = rServices1.getLength();
    const sal_Int32 nLen2 = rServices2.getLength();

    uno::Sequence< OUString > aSeq( nLen1+nLen2 );
    OUString* pStrings = aSeq.getArray();

    sal_Int32 nIdx;
    OUString* pStringDst = pStrings;
    const OUString* pStringSrc = rServices1.getArray();

    for( nIdx = 0; nIdx < nLen1; nIdx++ )
        *pStringDst++ = *pStringSrc++;

    pStringSrc = rServices2.getArray();

    for( nIdx = 0; nIdx < nLen2; nIdx++ )
        *pStringDst++ = *pStringSrc++;

    return aSeq;
}

SvxUnoDrawingModel::SvxUnoDrawingModel( SdrModel* pDoc ) throw()
: mpDoc( pDoc )
{
}

SvxUnoDrawingModel::~SvxUnoDrawingModel() throw()
{
}

uno::Any SAL_CALL SvxUnoDrawingModel::queryInterface( const uno::Type & rType )
{
    uno::Any aAny;

    QUERYINT(lang::XServiceInfo);
    else QUERYINT(lang::XMultiServiceFactory);
    else QUERYINT(drawing::XDrawPagesSupplier);
    else QUERYINT(css::ucb::XAnyCompareFactory);
    else
        return SfxBaseModel::queryInterface( rType );

    return aAny;
}

void SAL_CALL SvxUnoDrawingModel::acquire() throw ( )
{
    SfxBaseModel::acquire();
}

void SAL_CALL SvxUnoDrawingModel::release() throw ( )
{
    SfxBaseModel::release();
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SvxUnoDrawingModel::getTypes(  )
{
    if( maTypeSequence.getLength() == 0 )
    {
        const uno::Sequence< uno::Type > aBaseTypes( SfxBaseModel::getTypes() );
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();

        const sal_Int32 nOwnTypes = 4;      // !DANGER! Keep this updated!

        maTypeSequence.realloc(  nBaseTypes + nOwnTypes );
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = cppu::UnoType<lang::XServiceInfo>::get();
        *pTypes++ = cppu::UnoType<lang::XMultiServiceFactory>::get();
        *pTypes++ = cppu::UnoType<drawing::XDrawPagesSupplier>::get();
        *pTypes++ = cppu::UnoType<css::ucb::XAnyCompareFactory>::get();

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }

    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoDrawingModel::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

void SAL_CALL SvxUnoDrawingModel::lockControllers(  )
{
    if( mpDoc )
        mpDoc->setLock(true);
}

void SAL_CALL SvxUnoDrawingModel::unlockControllers(  )
{
    if( mpDoc && mpDoc->isLocked() )
    {
        mpDoc->setLock(false);
    }
}

sal_Bool SAL_CALL SvxUnoDrawingModel::hasControllersLocked(  )
{
    return mpDoc && mpDoc->isLocked();
}

// XDrawPagesSupplier
uno::Reference< drawing::XDrawPages > SAL_CALL SvxUnoDrawingModel::getDrawPages()
{
    ::SolarMutexGuard aGuard;

    uno::Reference< drawing::XDrawPages >  xDrawPages( mxDrawPagesAccess );

    if( !xDrawPages.is() )
        mxDrawPagesAccess = xDrawPages = static_cast<drawing::XDrawPages*>(new SvxUnoDrawPagesAccess(*this));

    return xDrawPages;
}

// XMultiServiceFactory ( SvxFmMSFactory )
uno::Reference< uno::XInterface > SAL_CALL SvxUnoDrawingModel::createInstance( const OUString& aServiceSpecifier )
{
    ::SolarMutexGuard aGuard;

    if( aServiceSpecifier == "com.sun.star.drawing.DashTable" )
    {
        if( !mxDashTable.is() )
            mxDashTable = SvxUnoDashTable_createInstance( mpDoc );
        return mxDashTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.GradientTable" )
    {
        if( !mxGradientTable.is() )
            mxGradientTable = SvxUnoGradientTable_createInstance( mpDoc );
        return mxGradientTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.HatchTable" )
    {
        if( !mxHatchTable.is() )
            mxHatchTable = SvxUnoHatchTable_createInstance( mpDoc );
        return mxHatchTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.BitmapTable" )
    {
        if( !mxBitmapTable.is() )
            mxBitmapTable = SvxUnoBitmapTable_createInstance( mpDoc );
        return mxBitmapTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.TransparencyGradientTable" )
    {
        if( !mxTransGradientTable.is() )
            mxTransGradientTable = SvxUnoTransGradientTable_createInstance( mpDoc );
        return mxTransGradientTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.MarkerTable" )
    {
        if( !mxMarkerTable.is() )
            mxMarkerTable = SvxUnoMarkerTable_createInstance( mpDoc );
        return mxMarkerTable;
    }
    if( aServiceSpecifier == "com.sun.star.text.NumberingRules" )
    {
        return uno::Reference< uno::XInterface >( SvxCreateNumRule( mpDoc ), uno::UNO_QUERY );
    }

    if ( aServiceSpecifier == "com.sun.star.image.ImageMapRectangleObject" )
    {
        return SvUnoImageMapRectangleObject_createInstance( ImplGetSupportedMacroItems() );
    }

    if ( aServiceSpecifier == "com.sun.star.image.ImageMapCircleObject" )
    {
        return SvUnoImageMapCircleObject_createInstance( ImplGetSupportedMacroItems() );
    }

    if ( aServiceSpecifier == "com.sun.star.image.ImageMapPolygonObject" )
    {
        return SvUnoImageMapPolygonObject_createInstance( ImplGetSupportedMacroItems() );
    }

    if( aServiceSpecifier == "com.sun.star.text.TextField.DateTime" )
    {
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField(text::textfield::Type::DATE));
    }

    uno::Reference< uno::XInterface > xRet;

    const OUString aPackagePrefix( "com.sun.star.presentation." );
    if( aServiceSpecifier.startsWith( aPackagePrefix ) )
    {
        SvxShape* pShape = nullptr;

        sal_uInt16 nType = OBJ_TEXT;
        OUString aTypeName = aServiceSpecifier.copy( aPackagePrefix.getLength() );
        // create a shape wrapper
        if( aTypeName.startsWith("TitleTextShape") )
        {
            nType = OBJ_TEXT;
        }
        else if( aTypeName.startsWith( "OutlinerShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aTypeName.startsWith( "SubtitleShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aTypeName.startsWith( "GraphicObjectShape" ) )
        {
            nType = OBJ_GRAF;
        }
        else if( aTypeName.startsWith( "PageShape" ) )
        {
            nType = OBJ_PAGE;
        }
        else if( aTypeName.startsWith( "OLE2Shape" ) )
        {
            nType = OBJ_OLE2;
        }
        else if( aTypeName.startsWith( "ChartShape" ) )
        {
            nType = OBJ_OLE2;
        }
        else if( aTypeName.startsWith( "OrgChartShape" ) )
        {
            nType = OBJ_OLE2;
        }
        else if( aTypeName.startsWith( "NotesShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aTypeName.startsWith( "HandoutShape" ) )
        {
            nType = OBJ_PAGE;
        }
        else if( aTypeName.startsWith( "FooterShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aTypeName.startsWith( "HeaderShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aTypeName.startsWith( "SlideNumberShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aTypeName.startsWith( "DateTimeShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aTypeName.startsWith( "TableShape" ) )
        {
            nType = OBJ_TABLE;
        }
        else
        {
            throw lang::ServiceNotRegisteredException();
        }

        // create the API wrapper
        pShape = CreateSvxShapeByTypeAndInventor( nType, SdrInventor::Default, "" );

        // set shape type
        if( pShape )
            pShape->SetShapeType(aServiceSpecifier);

        xRet = static_cast<uno::XWeak*>(pShape);
    }
    else
    {
        xRet = SvxFmMSFactory::createInstance( aServiceSpecifier );
    }

    return xRet;
}

uno::Sequence< OUString > SAL_CALL SvxUnoDrawingModel::getAvailableServiceNames()
{
    const uno::Sequence< OUString > aSNS_ORG( SvxFmMSFactory::getAvailableServiceNames() );

    uno::Sequence< OUString > aSNS( 21 );

    sal_uInt16 i = 0;

    aSNS[i++] = "com.sun.star.drawing.DashTable";
    aSNS[i++] = "com.sun.star.drawing.GradientTable";
    aSNS[i++] = "com.sun.star.drawing.HatchTable";
    aSNS[i++] = "com.sun.star.drawing.BitmapTable";
    aSNS[i++] = "com.sun.star.drawing.TransparencyGradientTable";
    aSNS[i++] = "com.sun.star.drawing.MarkerTable";
    aSNS[i++] = "com.sun.star.text.NumberingRules";
    aSNS[i++] = "com.sun.star.image.ImageMapRectangleObject";
    aSNS[i++] = "com.sun.star.image.ImageMapCircleObject";
    aSNS[i++] = "com.sun.star.image.ImageMapPolygonObject";

    aSNS[i++] = "com.sun.star.presentation.TitleTextShape";
    aSNS[i++] = "com.sun.star.presentation.OutlinerShape";
    aSNS[i++] = "com.sun.star.presentation.SubtitleShape";
    aSNS[i++] = "com.sun.star.presentation.GraphicObjectShape";
    aSNS[i++] = "com.sun.star.presentation.ChartShape";
    aSNS[i++] = "com.sun.star.presentation.PageShape";
    aSNS[i++] = "com.sun.star.presentation.OLE2Shape";
    aSNS[i++] = "com.sun.star.presentation.TableShape";
    aSNS[i++] = "com.sun.star.presentation.OrgChartShape";
    aSNS[i++] = "com.sun.star.presentation.NotesShape";
    aSNS[i++] = "com.sun.star.presentation.HandoutShape";

    DBG_ASSERT( i == aSNS.getLength(), "Sequence overrun!" );

    return comphelper::concatSequences( aSNS_ORG, aSNS );
}

// lang::XServiceInfo
OUString SAL_CALL SvxUnoDrawingModel::getImplementationName()
{
    return OUString("SvxUnoDrawingModel");
}

sal_Bool SAL_CALL SvxUnoDrawingModel::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SvxUnoDrawingModel::getSupportedServiceNames()
{
    OUString aSN("com.sun.star.drawing.DrawingDocument");
    uno::Sequence< OUString > aSeq( &aSN, 1 );
    return aSeq;
}

// XAnyCompareFactory
uno::Reference< css::ucb::XAnyCompare > SAL_CALL SvxUnoDrawingModel::createAnyCompareByName( const OUString& )
{
    return SvxCreateNumRuleCompare();
}

SvxUnoDrawPagesAccess::SvxUnoDrawPagesAccess( SvxUnoDrawingModel& rMyModel )  throw()
:   mrModel(rMyModel)
{
}

// XIndexAccess
sal_Int32 SAL_CALL SvxUnoDrawPagesAccess::getCount()
{
    ::SolarMutexGuard aGuard;

    sal_Int32 nCount = 0;

    if( mrModel.mpDoc )
        nCount = mrModel.mpDoc->GetPageCount();

    return nCount;
}

uno::Any SAL_CALL SvxUnoDrawPagesAccess::getByIndex( sal_Int32 Index )
{
    ::SolarMutexGuard aGuard;

    uno::Any aAny;

    if( mrModel.mpDoc )
    {
        if( (Index < 0) || (Index >= mrModel.mpDoc->GetPageCount() ) )
            throw lang::IndexOutOfBoundsException();

        SdrPage* pPage = mrModel.mpDoc->GetPage( (sal_uInt16)Index );
        if( pPage )
        {
            uno::Reference< uno::XInterface > xPage( pPage->mxUnoPage );

            if( !xPage.is() )
            {
                if( dynamic_cast<FmFormModel*>( mrModel.mpDoc )  )
                    xPage = static_cast<drawing::XDrawPage*>(new SvxFmDrawPage( pPage ));
                else
                    xPage = static_cast<drawing::XDrawPage*>(new SvxDrawPage( pPage ));

                pPage->mxUnoPage = xPage;
            }

            aAny <<= xPage;
        }
    }
    return aAny;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoDrawPagesAccess::getElementType()
{
    return cppu::UnoType<drawing::XDrawPage>::get();
}

sal_Bool SAL_CALL SvxUnoDrawPagesAccess::hasElements()
{
    return getCount() > 0;
}

// XDrawPages

// create a new page with model at given position
// and return corresponding SdDrawPage
uno::Reference< drawing::XDrawPage > SAL_CALL SvxUnoDrawPagesAccess::insertNewByIndex( sal_Int32 nIndex )
{
    ::SolarMutexGuard aGuard;

    uno::Reference< drawing::XDrawPage > xDrawPage;

    if( mrModel.mpDoc )
    {
        SdrPage* pPage;

        if( dynamic_cast<FmFormModel*>( mrModel.mpDoc )  )
            pPage = new FmFormPage(*static_cast<FmFormModel*>(mrModel.mpDoc));
        else
            pPage = new SdrPage(*mrModel.mpDoc);

        mrModel.mpDoc->InsertPage( pPage, (sal_uInt16)nIndex );
        xDrawPage.set( pPage->getUnoPage(), uno::UNO_QUERY );
    }

    return xDrawPage;
}

void SAL_CALL SvxUnoDrawPagesAccess::remove( const uno::Reference< drawing::XDrawPage >& xPage )
{
    ::SolarMutexGuard aGuard;

    sal_uInt16 nPageCount = mrModel.mpDoc->GetPageCount();
    if( nPageCount > 1 )
    {
        // get pPage from xPage and get Id (nPos)
        SvxDrawPage* pSvxPage = SvxDrawPage::getImplementation( xPage );
        if( pSvxPage )
        {
            SdrPage* pPage = pSvxPage->GetSdrPage();
            if(pPage)
            {
                sal_uInt16 nPage = pPage->GetPageNum();
                mrModel.mpDoc->DeletePage( nPage );
            }
        }
    }
}

// XServiceInfo

OUString SAL_CALL SvxUnoDrawPagesAccess::getImplementationName(  )
{
    return OUString( "SvxUnoDrawPagesAccess" );
}

sal_Bool SAL_CALL SvxUnoDrawPagesAccess::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SvxUnoDrawPagesAccess::getSupportedServiceNames(  )
{
    OUString aService( "com.sun.star.drawing.DrawPages" );
    uno::Sequence< OUString > aSeq( &aService, 1 );
    return aSeq;
}

css::uno::Reference< css::container::XIndexReplace > SvxCreateNumRule( SdrModel* pModel ) throw()
{
    const SvxNumRule* pDefaultRule = nullptr;
    if( pModel )
    {
        const SvxNumBulletItem* pItem = static_cast<const SvxNumBulletItem*>( pModel->GetItemPool().GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET) );
        if( pItem )
        {
            pDefaultRule = pItem->GetNumRule();
        }
    }

    if( pDefaultRule )
    {
        return SvxCreateNumRule( pDefaultRule );
    }
    else
    {
        SvxNumRule aTempRule( SvxNumRuleFlags::NONE, 10, false );
        return SvxCreateNumRule( &aTempRule );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
