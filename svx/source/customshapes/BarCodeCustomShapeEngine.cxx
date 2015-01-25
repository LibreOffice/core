/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <basegfx/tools/unotools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <svx/unoapi.hxx>
#include <svx/unopage.hxx>
#include <svx/unoshape.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdasitm.hxx>

#include "BarCodeRender.hxx"

using namespace css;
using namespace css::uno;

namespace {

class BarCodeCustomShapeEngine : public cppu::WeakImplHelper3
        < lang::XInitialization, lang::XServiceInfo,
          drawing::XCustomShapeEngine >
{
    Reference< drawing::XShape > mxShape;

public:
             BarCodeCustomShapeEngine() {}
    virtual ~BarCodeCustomShapeEngine() {}

    // XInitialization
    virtual void SAL_CALL initialize( const uno::Sequence< uno::Any >& aArguments )
        throw ( uno::Exception, uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw ( uno::RuntimeException, std::exception ) SAL_OVERRIDE
    {
        return OUString( "org.libreoffice.draw.barcode" );
    }
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName )
        throw ( uno::RuntimeException, std::exception ) SAL_OVERRIDE
    {
        return cppu::supportsService(this, rServiceName);
    }
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw ( uno::RuntimeException, std::exception ) SAL_OVERRIDE
    {
        Sequence< OUString > aRet(1);
        OUString* pArray = aRet.getArray();
        pArray[0] = "com.sun.star.drawing.CustomShapeEngine";
        return aRet;
    }

    // XCustomShapeEngine
    virtual uno::Reference< drawing::XShape > SAL_CALL render()
        throw ( uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual awt::Rectangle SAL_CALL getTextBounds()
        throw ( uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual drawing::PolyPolygonBezierCoords SAL_CALL getLineGeometry()
        throw ( uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual uno::Sequence< uno::Reference< drawing::XCustomShapeHandle > > SAL_CALL getInteraction()
        throw ( uno::RuntimeException, std::exception ) SAL_OVERRIDE;
};

// XInitialization
void SAL_CALL BarCodeCustomShapeEngine::initialize( const Sequence< Any >& aArgs )
    throw ( Exception, RuntimeException, std::exception )
{
    sal_Int32 i;
    Sequence< beans::PropertyValue > aParameter;
    for ( i = 0; i < aArgs.getLength(); i++ )
    {
        if ( aArgs[ i ] >>= aParameter )
            break;
    }
    for ( i = 0; i < aParameter.getLength(); i++ )
    {
        const beans::PropertyValue& rProp = aParameter[ i ];
        if ( rProp.Name == "CustomShape" )
            rProp.Value >>= mxShape;
    }
}

Reference< drawing::XShape > SAL_CALL BarCodeCustomShapeEngine::render()
    throw ( RuntimeException, std::exception )
{
    Reference< drawing::XShape > xShape;
    SAL_INFO("svx", "barcode engine render");
    SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxShape ) );
    if ( pSdrObjCustomShape )
    {
        Point aP( pSdrObjCustomShape->GetSnapRect().Center() );
        Size aS( pSdrObjCustomShape->GetLogicRect().GetSize() );
        aP.X() -= aS.Width() / 2;
        aP.Y() -= aS.Height() / 2;

        Rectangle aLogicRect;
        aLogicRect = Rectangle( aP, aS );

        OUString aData(static_cast<const SdrCustomShapeDataItem&>(pSdrObjCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_DATA )).GetValue());

        SdrObjGroup *pShape = svx::BarCodeRender(aLogicRect, aData);
        if (pShape)
        {
            SfxItemSet aSet( pSdrObjCustomShape->GetMergedItemSet() );
            pShape->SetMergedItemSet( aSet );

            pShape->NbcSetStyleSheet( pSdrObjCustomShape->GetStyleSheet(), true );
            pShape->RecalcSnapRect();

            xShape = SvxDrawPage::CreateShapeByTypeAndInventor(
                        pShape->GetObjIdentifier(),
                        pShape->GetObjInventor(), pShape, NULL );
        }

        if ( xShape.is() )
        {
            SvxShape* pTake = SvxShape::getImplementation( xShape );
            if ( pTake )
                pTake->TakeSdrObjectOwnership();
        }
    }
    return xShape;
}

awt::Rectangle SAL_CALL BarCodeCustomShapeEngine::getTextBounds()
    throw ( RuntimeException, std::exception )
{
    awt::Rectangle aTextRect;
    SAL_INFO("svx", "barcode engine get text bounds: stub");
    return aTextRect;
}

drawing::PolyPolygonBezierCoords SAL_CALL BarCodeCustomShapeEngine::getLineGeometry()
    throw ( RuntimeException, std::exception )
{
    drawing::PolyPolygonBezierCoords aPolyPolygonBezierCoords;
    SAL_INFO("svx", "barcode engine get line geometry: stub");
    return aPolyPolygonBezierCoords;
}

Sequence< Reference< drawing::XCustomShapeHandle > > SAL_CALL BarCodeCustomShapeEngine::getInteraction()
    throw ( RuntimeException, std::exception )
{
    return Sequence< Reference< drawing::XCustomShapeHandle > >();
}

}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface * SAL_CALL
org_libreoffice_draw_barcode_get_implementation(
    uno::XComponentContext *,
    uno::Sequence<uno::Any> const &)
{
    return cppu::acquire(new BarCodeCustomShapeEngine);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
