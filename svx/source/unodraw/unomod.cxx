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
#include <vcl/svapp.hxx>
#include <svl/itempool.hxx>
#include <svtools/unoevent.hxx>
#include <comphelper/sequence.hxx>
#include <o3tl/string_view.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <cppuhelper/implbase.hxx>
#include <svx/unofill.hxx>
#include <editeng/unonrule.hxx>
#include <svtools/unoimap.hxx>
#include <sfx2/event.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <svx/unoapi.hxx>

#include <svx/svdmodel.hxx>
#include <svx/unoprov.hxx>
#include <svx/unopage.hxx>
#include <editeng/unofield.hxx>
#include <svx/unomod.hxx>
#include <svx/unomodel.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/SvxXTextColumns.hxx>
#include <svx/unoshape.hxx>
#include <svx/xmlgrhlp.hxx>

#include <com/sun/star/text/textfield/Type.hpp>

//-

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
    explicit SvxUnoDrawPagesAccess( SvxUnoDrawingModel& rMyModel ) noexcept;

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
        { SvMacroItemId::OnMouseOver, "OnMouseOver" },
        { SvMacroItemId::OnMouseOut,  "OnMouseOut" },
        { SvMacroItemId::NONE, nullptr }
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
        aEvent.Source = const_cast<SdrModel*>(pDoc)->getUnoModel();

    return true;
}

namespace {

css::uno::Reference<css::uno::XInterface> create(
    OUString const & rServiceSpecifier, OUString const & referer)
{
    if( rServiceSpecifier.startsWith("com.sun.star.drawing.") )
    {
        std::optional<SdrObjKind> nType = UHashMap::getId( rServiceSpecifier );
        if( nType )
        {
            SdrInventor nI = IsInventorE3D(*nType) ? SdrInventor::E3d : SdrInventor::Default;

            return cppu::getXWeak(SvxDrawPage::CreateShapeByTypeAndInventor( *nType, nI, nullptr, nullptr, referer ).get());
        }
    }
    else if (rServiceSpecifier == "com.sun.star.document.ImportGraphicStorageHandler")
    {
        return cppu::getXWeak( SvXMLGraphicHelper::Create( SvXMLGraphicHelperMode::Read ).get() );
    }
    else if (rServiceSpecifier == "com.sun.star.text.TextColumns")
    {
        return SvxXTextColumns_createInstance();
    }

    uno::Reference< uno::XInterface > xRet( SvxUnoDrawMSFactory::createTextField( rServiceSpecifier ) );
    if( !xRet.is() )
        throw lang::ServiceNotRegisteredException("unknown service: " + rServiceSpecifier);

    return xRet;
}

}

uno::Reference< uno::XInterface > SAL_CALL SvxUnoDrawMSFactory::createInstance( const OUString& rServiceSpecifier )
{
    return create(rServiceSpecifier, u""_ustr);
}

uno::Reference< uno::XInterface > SvxUnoDrawMSFactory::createTextField( std::u16string_view ServiceSpecifier )
{
    return SvxUnoTextCreateTextField( ServiceSpecifier );
}

uno::Reference< uno::XInterface > SAL_CALL SvxUnoDrawMSFactory::createInstanceWithArguments( const OUString& ServiceSpecifier, const uno::Sequence< css::uno::Any >& Arguments )
{
    OUString arg;
    if ((ServiceSpecifier == "com.sun.star.drawing.GraphicObjectShape"
         || ServiceSpecifier == "com.sun.star.drawing.AppletShape"
         || ServiceSpecifier == "com.sun.star.drawing.FrameShape"
         || ServiceSpecifier == "com.sun.star.drawing.OLE2Shape"
         || ServiceSpecifier == "com.sun.star.drawing.MediaShape"
         || ServiceSpecifier == "com.sun.star.drawing.PluginShape")
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

SdrModel& SvxUnoDrawingModel::getSdrModelFromUnoModel() const
{
    OSL_ENSURE(mpDoc, "No SdrModel in UnoDrawingModel, should not happen");
    return *mpDoc;
}

SvxUnoDrawingModel::SvxUnoDrawingModel(SdrModel* pDoc) noexcept
:   SfxBaseModel(nullptr),
    mpDoc(pDoc)
{
}

SvxUnoDrawingModel::~SvxUnoDrawingModel() noexcept
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

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SvxUnoDrawingModel::getTypes(  )
{
    if( !maTypeSequence.hasElements() )
    {
        maTypeSequence = comphelper::concatSequences( SfxBaseModel::getTypes(),
            uno::Sequence {
                cppu::UnoType<lang::XServiceInfo>::get(),
                cppu::UnoType<lang::XMultiServiceFactory>::get(),
                cppu::UnoType<drawing::XDrawPagesSupplier>::get(),
                cppu::UnoType<css::ucb::XAnyCompareFactory>::get() });
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
        mxDrawPagesAccess = xDrawPages = new SvxUnoDrawPagesAccess(*this);

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
        return cppu::getXWeak(new SvxUnoTextField(text::textfield::Type::DATE));
    }

    uno::Reference< uno::XInterface > xRet;

    static constexpr OUString aPackagePrefix( u"com.sun.star.presentation."_ustr );
    if( aServiceSpecifier.startsWith( aPackagePrefix ) )
    {
        SvxShape* pShape = nullptr;

        SdrObjKind nType = SdrObjKind::Text;
        std::u16string_view aTypeName = aServiceSpecifier.subView( aPackagePrefix.getLength() );
        // create a shape wrapper
        if( o3tl::starts_with(aTypeName, u"TitleTextShape") )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aTypeName, u"OutlinerShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aTypeName, u"SubtitleShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aTypeName, u"GraphicObjectShape" ) )
        {
            nType = SdrObjKind::Graphic;
        }
        else if( o3tl::starts_with(aTypeName, u"PageShape" ) )
        {
            nType = SdrObjKind::Page;
        }
        else if( o3tl::starts_with(aTypeName, u"OLE2Shape" ) )
        {
            nType = SdrObjKind::OLE2;
        }
        else if( o3tl::starts_with(aTypeName, u"ChartShape" ) )
        {
            nType = SdrObjKind::OLE2;
        }
        else if( o3tl::starts_with(aTypeName, u"OrgChartShape" ) )
        {
            nType = SdrObjKind::OLE2;
        }
        else if( o3tl::starts_with(aTypeName, u"NotesShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aTypeName, u"HandoutShape" ) )
        {
            nType = SdrObjKind::Page;
        }
        else if( o3tl::starts_with(aTypeName, u"FooterShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aTypeName, u"HeaderShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aTypeName, u"SlideNumberShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aTypeName, u"DateTimeShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aTypeName, u"TableShape" ) )
        {
            nType = SdrObjKind::Table;
        }
        else
        {
            throw lang::ServiceNotRegisteredException();
        }

        // create the API wrapper
        rtl::Reference<SvxShape> xNewShape = CreateSvxShapeByTypeAndInventor( nType, SdrInventor::Default, u""_ustr );
        pShape = xNewShape.get();

        // set shape type
        if( pShape )
            pShape->SetShapeType(aServiceSpecifier);

        xRet = cppu::getXWeak(pShape);
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

    uno::Sequence< OUString > aSNS{
        u"com.sun.star.drawing.DashTable"_ustr,
        u"com.sun.star.drawing.GradientTable"_ustr,
        u"com.sun.star.drawing.HatchTable"_ustr,
        u"com.sun.star.drawing.BitmapTable"_ustr,
        u"com.sun.star.drawing.TransparencyGradientTable"_ustr,
        u"com.sun.star.drawing.MarkerTable"_ustr,
        u"com.sun.star.text.NumberingRules"_ustr,
        u"com.sun.star.image.ImageMapRectangleObject"_ustr,
        u"com.sun.star.image.ImageMapCircleObject"_ustr,
        u"com.sun.star.image.ImageMapPolygonObject"_ustr,

        u"com.sun.star.presentation.TitleTextShape"_ustr,
        u"com.sun.star.presentation.OutlinerShape"_ustr,
        u"com.sun.star.presentation.SubtitleShape"_ustr,
        u"com.sun.star.presentation.GraphicObjectShape"_ustr,
        u"com.sun.star.presentation.ChartShape"_ustr,
        u"com.sun.star.presentation.PageShape"_ustr,
        u"com.sun.star.presentation.OLE2Shape"_ustr,
        u"com.sun.star.presentation.TableShape"_ustr,
        u"com.sun.star.presentation.OrgChartShape"_ustr,
        u"com.sun.star.presentation.NotesShape"_ustr,
        u"com.sun.star.presentation.HandoutShape"_ustr
    };

    return comphelper::concatSequences( aSNS_ORG, aSNS );
}

// lang::XServiceInfo
OUString SAL_CALL SvxUnoDrawingModel::getImplementationName()
{
    return u"SvxUnoDrawingModel"_ustr;
}

sal_Bool SAL_CALL SvxUnoDrawingModel::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SvxUnoDrawingModel::getSupportedServiceNames()
{
    return { u"com.sun.star.drawing.DrawingDocument"_ustr };
}

// XAnyCompareFactory
uno::Reference< css::ucb::XAnyCompare > SAL_CALL SvxUnoDrawingModel::createAnyCompareByName( const OUString& )
{
    return SvxCreateNumRuleCompare();
}

SvxUnoDrawPagesAccess::SvxUnoDrawPagesAccess( SvxUnoDrawingModel& rMyModel )  noexcept
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

        SdrPage* pPage = mrModel.mpDoc->GetPage( static_cast<sal_uInt16>(Index) );
        if( pPage )
        {
            uno::Reference< uno::XInterface > xPage( pPage->mxUnoPage );

            if( !xPage.is() )
            {
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
        rtl::Reference<SdrPage> pPage;

        if( auto pFormModel = dynamic_cast<FmFormModel*>( mrModel.mpDoc )  )
            pPage = new FmFormPage(*pFormModel);
        else
            pPage = new SdrPage(*mrModel.mpDoc);

        mrModel.mpDoc->InsertPage( pPage.get(), static_cast<sal_uInt16>(nIndex) );
        xDrawPage.set( pPage->getUnoPage(), uno::UNO_QUERY );
    }

    return xDrawPage;
}

void SAL_CALL SvxUnoDrawPagesAccess::remove( const uno::Reference< drawing::XDrawPage >& xPage )
{
    ::SolarMutexGuard aGuard;

    sal_uInt16 nPageCount = mrModel.mpDoc->GetPageCount();
    if( nPageCount <= 1 )
        return;

    // get pPage from xPage and get Id (nPos)
    SvxDrawPage* pSvxPage = comphelper::getFromUnoTunnel<SvxDrawPage>( xPage );
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

// XServiceInfo

OUString SAL_CALL SvxUnoDrawPagesAccess::getImplementationName(  )
{
    return u"SvxUnoDrawPagesAccess"_ustr;
}

sal_Bool SAL_CALL SvxUnoDrawPagesAccess::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SvxUnoDrawPagesAccess::getSupportedServiceNames(  )
{
    return { u"com.sun.star.drawing.DrawPages"_ustr };
}

css::uno::Reference< css::container::XIndexReplace > SvxCreateNumRule(SdrModel* pModel)
{
    const SvxNumRule* pDefaultRule = nullptr;
    if( pModel )
    {
        const SvxNumBulletItem* pItem = pModel->GetItemPool().GetSecondaryPool()->GetUserDefaultItem(EE_PARA_NUMBULLET);
        if( pItem )
        {
            pDefaultRule = &pItem->GetNumRule();
        }
    }

    if( pDefaultRule )
    {
        return SvxCreateNumRule( *pDefaultRule );
    }
    else
    {
        SvxNumRule aTempRule( SvxNumRuleFlags::NONE, 10, false );
        return SvxCreateNumRule( aTempRule );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
