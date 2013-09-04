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

#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svl/itemprop.hxx>
#include <svtools/unoevent.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/serviceinfohelper.hxx>

#include <cppuhelper/implbase2.hxx>
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

//-////////////////////////////////////////////////////////////////////

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;

//-////////////////////////////////////////////////////////////////////

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const uno::Reference< xint >*)0) ) \
        aAny <<= uno::Reference< xint >(this)

#define ITYPE( xint ) \
    ::getCppuType((const uno::Reference< xint >*)0)

//-////////////////////////////////////////////////////////////////////

class SvxUnoDrawPagesAccess : public ::cppu::WeakImplHelper2< ::com::sun::star::drawing::XDrawPages, ::com::sun::star::lang::XServiceInfo >
{
private:
    SvxUnoDrawingModel& mrModel;

public:
    SvxUnoDrawPagesAccess( SvxUnoDrawingModel& rMyModel ) throw();
    virtual ~SvxUnoDrawPagesAccess() throw();

    // XDrawPages
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL insertNewByIndex( sal_Int32 nIndex ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage ) throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};
//-////////////////////////////////////////////////////////////////////

static const SvEventDescription* ImplGetSupportedMacroItems()
{
    static const SvEventDescription aMacroDescriptionsImpl[] =
    {
        { SFX_EVENT_MOUSEOVER_OBJECT, "OnMouseOver" },
        { SFX_EVENT_MOUSEOUT_OBJECT, "OnMouseOut" },
        { 0, NULL }
    };

    return aMacroDescriptionsImpl;
}

//-////////////////////////////////////////////////////////////////////

/** fills the given EventObject from the given SdrHint.
    @returns
        true    if the SdrHint could be translated to an EventObject<br>
        false   if not
*/
sal_Bool SvxUnoDrawMSFactory::createEvent( const SdrModel* pDoc, const SdrHint* pSdrHint, ::com::sun::star::document::EventObject& aEvent )
{
    const SdrObject* pObj = NULL;
    const SdrPage* pPage = NULL;

    switch( pSdrHint->GetKind() )
    {
//              case HINT_LAYERCHG:             // Layerdefinition geaendert
//              case HINT_LAYERORDERCHG:        // Layerreihenfolge geaendert (Insert/Remove/ChangePos)
//              case HINT_LAYERSETCHG:          // Layerset geaendert
//              case HINT_LAYERSETORDERCHG:     // Layersetreihenfolge geaendert (Insert/Remove/ChangePos)

// #115423#
//      case HINT_PAGECHG:              // Page geaendert
//          aEvent.EventName = OUString( "PageModified" );
//          pPage = pSdrHint->GetPage();
//          break;
        case HINT_PAGEORDERCHG:         // Reihenfolge der Seiten (Zeichenseiten oder Masterpages) geaendert (Insert/Remove/ChangePos)
            aEvent.EventName = OUString( "PageOrderModified" );
            pPage = pSdrHint->GetPage();
            break;
        case HINT_OBJCHG:               // Objekt geaendert
            aEvent.EventName = OUString( "ShapeModified" );
            pObj = pSdrHint->GetObject();
            break;
        case HINT_OBJINSERTED:          // Neues Zeichenobjekt eingefuegt
            aEvent.EventName = OUString( "ShapeInserted" );
            pObj = pSdrHint->GetObject();
            break;
        case HINT_OBJREMOVED:           // Zeichenobjekt aus Liste entfernt
            aEvent.EventName = OUString( "ShapeRemoved" );
            pObj = pSdrHint->GetObject();
            break;
//                HINT_DEFAULTTABCHG,   // Default Tabulatorweite geaendert
//                HINT_DEFFONTHGTCHG,   // Default FontHeight geaendert
//                HINT_SWITCHTOPAGE,    // #94278# UNDO/REDO at an object evtl. on another page
//                HINT_OBJLISTCLEAR     // Is called before an SdrObjList will be cleared
        default:
            return sal_False;
    }

    if( pObj )
        aEvent.Source = const_cast<SdrObject*>(pObj)->getUnoShape();
    else if( pPage )
        aEvent.Source = const_cast<SdrPage*>(pPage)->getUnoPage();
    else
        aEvent.Source = (const_cast<SdrModel*>(pDoc))->getUnoModel();

    return sal_True;
}

uno::Reference< uno::XInterface > SAL_CALL SvxUnoDrawMSFactory::createInstance( const OUString& rServiceSpecifier )
    throw( uno::Exception, uno::RuntimeException )
{
    const OUString aDrawingPrefix("com.sun.star.drawing.");

    if( rServiceSpecifier.startsWith( aDrawingPrefix ) )
    {
        sal_uInt32 nType = UHashMap::getId( rServiceSpecifier );
        if( nType != UHASHMAP_NOTFOUND )
        {
            sal_uInt16 nT = (sal_uInt16)(nType & ~E3D_INVENTOR_FLAG);
            sal_uInt32 nI = (nType & E3D_INVENTOR_FLAG)?E3dInventor:SdrInventor;

            return uno::Reference< uno::XInterface >( (drawing::XShape*) SvxDrawPage::CreateShapeByTypeAndInventor( nT, nI ) );
        }
    }
    else if ( rServiceSpecifier == "com.sun.star.document.ImportGraphicObjectResolver" )
    {
        SvXMLGraphicHelper* pGraphicHelper = SvXMLGraphicHelper::Create( GRAPHICHELPER_MODE_READ );
        uno::Reference< uno::XInterface> xRet( static_cast< ::cppu::OWeakObject* >( pGraphicHelper ) );
        pGraphicHelper->release();
        return xRet;
    }

    uno::Reference< uno::XInterface > xRet( createTextField( rServiceSpecifier ) );
    if( !xRet.is() )
        throw lang::ServiceNotRegisteredException();

    return xRet;
}

uno::Reference< uno::XInterface > SAL_CALL SvxUnoDrawMSFactory::createTextField( const OUString& ServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return SvxUnoTextCreateTextField( ServiceSpecifier );
}

uno::Reference< uno::XInterface > SAL_CALL SvxUnoDrawMSFactory::createInstanceWithArguments( const OUString&, const uno::Sequence< ::com::sun::star::uno::Any >& )
    throw( uno::Exception, uno::RuntimeException )
{
    throw lang::NoSupportException();
}

uno::Sequence< OUString > SAL_CALL SvxUnoDrawMSFactory::getAvailableServiceNames()
    throw( uno::RuntimeException )
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


///
SvxUnoDrawingModel::SvxUnoDrawingModel( SdrModel* pDoc ) throw()
: mpDoc( pDoc )
{
}

SvxUnoDrawingModel::~SvxUnoDrawingModel() throw()
{
}

uno::Any SAL_CALL SvxUnoDrawingModel::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT(lang::XServiceInfo);
    else QUERYINT(lang::XMultiServiceFactory);
    else QUERYINT(drawing::XDrawPagesSupplier);
    else QUERYINT(com::sun::star::ucb::XAnyCompareFactory);
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
uno::Sequence< uno::Type > SAL_CALL SvxUnoDrawingModel::getTypes(  ) throw(uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        const uno::Sequence< uno::Type > aBaseTypes( SfxBaseModel::getTypes() );
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();

        const sal_Int32 nOwnTypes = 4;      // !DANGER! Keep this updated!

        maTypeSequence.realloc(  nBaseTypes + nOwnTypes );
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ITYPE(lang::XServiceInfo);
        *pTypes++ = ITYPE(lang::XMultiServiceFactory);
        *pTypes++ = ITYPE(drawing::XDrawPagesSupplier);
        *pTypes++ = ITYPE(com::sun::star::ucb::XAnyCompareFactory);

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }

    return maTypeSequence;
}

namespace
{
    class theSvxUnoDrawingModelImplementationId : public rtl::Static< UnoTunnelIdInit, theSvxUnoDrawingModelImplementationId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoDrawingModel::getImplementationId(  ) throw(uno::RuntimeException)
{
    return theSvxUnoDrawingModelImplementationId::get().getSeq();
}

void SAL_CALL SvxUnoDrawingModel::lockControllers(  )
    throw(uno::RuntimeException)
{
    if( mpDoc )
        mpDoc->setLock(true);
}

void SAL_CALL SvxUnoDrawingModel::unlockControllers(  )
    throw(uno::RuntimeException)
{
    if( mpDoc && mpDoc->isLocked() )
    {
        mpDoc->setLock(false);
    }
}

sal_Bool SAL_CALL SvxUnoDrawingModel::hasControllersLocked(  )
    throw(uno::RuntimeException)
{
    return mpDoc && mpDoc->isLocked();
}

// XDrawPagesSupplier
uno::Reference< drawing::XDrawPages > SAL_CALL SvxUnoDrawingModel::getDrawPages()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    uno::Reference< drawing::XDrawPages >  xDrawPages( mxDrawPagesAccess );

    if( !xDrawPages.is() )
        mxDrawPagesAccess = xDrawPages = (drawing::XDrawPages*)new SvxUnoDrawPagesAccess(*this);

    return xDrawPages;
}

// XMultiServiceFactory ( SvxFmMSFactory )
uno::Reference< uno::XInterface > SAL_CALL SvxUnoDrawingModel::createInstance( const OUString& aServiceSpecifier )
    throw(uno::Exception, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.DashTable") ) )
    {
        if( !mxDashTable.is() )
            mxDashTable = SvxUnoDashTable_createInstance( mpDoc );
        return mxDashTable;
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.GradientTable") ) )
    {
        if( !mxGradientTable.is() )
            mxGradientTable = SvxUnoGradientTable_createInstance( mpDoc );
        return mxGradientTable;
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.HatchTable") ) )
    {
        if( !mxHatchTable.is() )
            mxHatchTable = SvxUnoHatchTable_createInstance( mpDoc );
        return mxHatchTable;
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.BitmapTable") ) )
    {
        if( !mxBitmapTable.is() )
            mxBitmapTable = SvxUnoBitmapTable_createInstance( mpDoc );
        return mxBitmapTable;
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.TransparencyGradientTable") ) )
    {
        if( !mxTransGradientTable.is() )
            mxTransGradientTable = SvxUnoTransGradientTable_createInstance( mpDoc );
        return mxTransGradientTable;
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.MarkerTable") ) )
    {
        if( !mxMarkerTable.is() )
            mxMarkerTable = SvxUnoMarkerTable_createInstance( mpDoc );
        return mxMarkerTable;
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.NumberingRules" ) ) )
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

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.TextField.DateTime") ) )
    {
        return (::cppu::OWeakObject * )new SvxUnoTextField(text::textfield::Type::DATE);
    }

    uno::Reference< uno::XInterface > xRet;

    const OUString aType( aServiceSpecifier );
    const OUString aPackagePrefix( "com.sun.star.presentation." );
    if( aType.startsWith( aPackagePrefix ) )
    {
        SvxShape* pShape = NULL;

        sal_uInt16 nType = OBJ_TEXT;
        OUString aTypeName = aType.copy( aPackagePrefix.getLength() );
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
        else if( aTypeName.startsWith( "TableShape" ) )
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
        pShape = CreateSvxShapeByTypeAndInventor( nType, SdrInventor );

        // set shape type
        if( pShape )
            pShape->SetShapeType(aServiceSpecifier);

        xRet = (uno::XWeak*)pShape;
    }
    else
    {
        xRet = SvxFmMSFactory::createInstance( aServiceSpecifier );
    }

    return xRet;
}

uno::Sequence< OUString > SAL_CALL SvxUnoDrawingModel::getAvailableServiceNames()
    throw(uno::RuntimeException)
{
    const uno::Sequence< OUString > aSNS_ORG( SvxFmMSFactory::getAvailableServiceNames() );

    uno::Sequence< OUString > aSNS( 21 );

    sal_uInt16 i = 0;

    aSNS[i++] = OUString("com.sun.star.drawing.DashTable");
    aSNS[i++] = OUString("com.sun.star.drawing.GradientTable");
    aSNS[i++] = OUString("com.sun.star.drawing.HatchTable");
    aSNS[i++] = OUString("com.sun.star.drawing.BitmapTable");
    aSNS[i++] = OUString("com.sun.star.drawing.TransparencyGradientTable");
    aSNS[i++] = OUString("com.sun.star.drawing.MarkerTable");
    aSNS[i++] = OUString("com.sun.star.text.NumberingRules");
    aSNS[i++] = OUString("com.sun.star.image.ImageMapRectangleObject");
    aSNS[i++] = OUString("com.sun.star.image.ImageMapCircleObject");
    aSNS[i++] = OUString("com.sun.star.image.ImageMapPolygonObject");

    aSNS[i++] = OUString("com.sun.star.presentation.TitleTextShape");
    aSNS[i++] = OUString("com.sun.star.presentation.OutlinerShape");
    aSNS[i++] = OUString("com.sun.star.presentation.SubtitleShape");
    aSNS[i++] = OUString("com.sun.star.presentation.GraphicObjectShape");
    aSNS[i++] = OUString("com.sun.star.presentation.ChartShape");
    aSNS[i++] = OUString("com.sun.star.presentation.PageShape");
    aSNS[i++] = OUString("com.sun.star.presentation.OLE2Shape");
    aSNS[i++] = OUString("com.sun.star.presentation.TableShape");
    aSNS[i++] = OUString("com.sun.star.presentation.OrgChartShape");
    aSNS[i++] = OUString("com.sun.star.presentation.NotesShape");
    aSNS[i++] = OUString("com.sun.star.presentation.HandoutShape");

    DBG_ASSERT( i == aSNS.getLength(), "Sequence overrun!" );

    return comphelper::concatSequences( aSNS_ORG, aSNS );
}

// lang::XServiceInfo
OUString SAL_CALL SvxUnoDrawingModel::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString("SvxUnoDrawingModel");
}

sal_Bool SAL_CALL SvxUnoDrawingModel::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return comphelper::ServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SvxUnoDrawingModel::getSupportedServiceNames() throw(uno::RuntimeException)
{
    OUString aSN("com.sun.star.drawing.DrawingDocument");
    uno::Sequence< OUString > aSeq( &aSN, 1 );
    return aSeq;
}

// XAnyCompareFactory
uno::Reference< com::sun::star::ucb::XAnyCompare > SAL_CALL SvxUnoDrawingModel::createAnyCompareByName( const OUString& )
    throw(uno::RuntimeException)
{
    return SvxCreateNumRuleCompare();
}

//=============================================================================
// class SvxUnoDrawPagesAccess
//=============================================================================

SvxUnoDrawPagesAccess::SvxUnoDrawPagesAccess( SvxUnoDrawingModel& rMyModel )  throw()
:   mrModel(rMyModel)
{
}

SvxUnoDrawPagesAccess::~SvxUnoDrawPagesAccess() throw()
{
}

// XIndexAccess
sal_Int32 SAL_CALL SvxUnoDrawPagesAccess::getCount()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    sal_Int32 nCount = 0;

    if( mrModel.mpDoc )
        nCount = mrModel.mpDoc->GetPageCount();

    return( nCount );
}

uno::Any SAL_CALL SvxUnoDrawPagesAccess::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
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
                if( PTR_CAST( FmFormModel, mrModel.mpDoc ) )
                    xPage = (drawing::XDrawPage*)new SvxFmDrawPage( pPage );
                else
                    xPage = (drawing::XDrawPage*)new SvxDrawPage( pPage );

                pPage->mxUnoPage = xPage;
            }

            aAny <<= xPage;
        }
    }
    return aAny;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoDrawPagesAccess::getElementType()
    throw(uno::RuntimeException)
{
    return ITYPE( drawing::XDrawPage );
}

sal_Bool SAL_CALL SvxUnoDrawPagesAccess::hasElements()
    throw(uno::RuntimeException)
{
    return getCount() > 0;
}

// XDrawPages

/******************************************************************************
* Erzeugt eine neue Seite mit Model an der angegebennen Position und gibt die *
* dazugehoerige SdDrawPage zurueck.                                           *
******************************************************************************/
uno::Reference< drawing::XDrawPage > SAL_CALL SvxUnoDrawPagesAccess::insertNewByIndex( sal_Int32 nIndex )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    uno::Reference< drawing::XDrawPage > xDrawPage;

    if( mrModel.mpDoc )
    {
        SdrPage* pPage;

        if( PTR_CAST( FmFormModel, mrModel.mpDoc ) )
            pPage = new FmFormPage(*(FmFormModel*)mrModel.mpDoc, NULL);
        else
            pPage = new SdrPage(*mrModel.mpDoc);

        mrModel.mpDoc->InsertPage( pPage, (sal_uInt16)nIndex );
        xDrawPage = uno::Reference< drawing::XDrawPage >::query( pPage->getUnoPage() );
    }

    return xDrawPage;
}

void SAL_CALL SvxUnoDrawPagesAccess::remove( const uno::Reference< drawing::XDrawPage >& xPage )
        throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    sal_uInt16 nPageCount = mrModel.mpDoc->GetPageCount();
    if( nPageCount > 1 )
    {
        // pPage von xPage besorgen und dann die Id (nPos )ermitteln
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
const char pSvxUnoDrawPagesAccessService[] = "com.sun.star.drawing.DrawPages";

OUString SAL_CALL SvxUnoDrawPagesAccess::getImplementationName(  ) throw(uno::RuntimeException)
{
    return OUString( "SvxUnoDrawPagesAccess" );
}

sal_Bool SAL_CALL SvxUnoDrawPagesAccess::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    return ServiceName == pSvxUnoDrawPagesAccessService;
}

uno::Sequence< OUString > SAL_CALL SvxUnoDrawPagesAccess::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    OUString aService( RTL_CONSTASCII_USTRINGPARAM( pSvxUnoDrawPagesAccessService ) );
    uno::Sequence< OUString > aSeq( &aService, 1 );
    return aSeq;
}

com::sun::star::uno::Reference< com::sun::star::container::XIndexReplace > SvxCreateNumRule( SdrModel* pModel ) throw()
{
    SvxNumRule* pDefaultRule = NULL;
    if( pModel )
    {
        SvxNumBulletItem* pItem = (SvxNumBulletItem*) pModel->GetItemPool().GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET);
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
        SvxNumRule aTempRule( 0, 10, false );
        return SvxCreateNumRule( &aTempRule );
    }
}

///////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
