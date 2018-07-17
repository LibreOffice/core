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

#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/PolygonKind.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <o3tl/any.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <osl/file.hxx>
#include <vcl/fltcall.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/wmf.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/GraphicLoader.hxx>

#include <svx/svdpool.hxx>

#include <editeng/unoprnms.hxx>
#include <svx/unoshape.hxx>
#include <svx/unopage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdouno.hxx>
#include "shapeimpl.hxx"
#include <svx/unoshprp.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdviter.hxx>
#include <svx/svdview.hxx>
#include <svx/svdopath.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <svx/svdograf.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <sfx2/fcontnr.hxx>


#include <memory>

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

#define QUERYINT( xint ) \
    if( rType == cppu::UnoType<xint>::get() ) \
        aAny <<= Reference< xint >(this)

SvxShapeGroup::SvxShapeGroup(SdrObject* pObj, SvxDrawPage* pDrawPage)
    : SvxShape(pObj, getSvxMapProvider().GetMap(SVXMAP_GROUP), getSvxMapProvider().GetPropertySet(SVXMAP_GROUP, SdrObject::GetGlobalDrawObjectItemPool()))
    , mxPage(pDrawPage)
{
}

SvxShapeGroup::~SvxShapeGroup() throw()
{
}

void SvxShapeGroup::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage )
{
    SvxShape::Create( pNewObj, pNewPage );
    mxPage = pNewPage;
}


uno::Any SAL_CALL SvxShapeGroup::queryInterface( const uno::Type & rType )
{
    return SvxShape::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeGroup::queryAggregation( const uno::Type & rType )
{
    uno::Any aAny;

    QUERYINT( drawing::XShapeGroup );
    else QUERYINT( drawing::XShapes );
    else QUERYINT( drawing::XShapes2 );
    else QUERYINT( container::XIndexAccess );
    else QUERYINT( container::XElementAccess );
    else
        return SvxShape::queryAggregation( rType );

    return aAny;
}

void SAL_CALL SvxShapeGroup::acquire() throw ( )
{
    SvxShape::acquire();
}

void SAL_CALL SvxShapeGroup::release() throw ( )
{
    SvxShape::release();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxShapeGroup::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// css::drawing::XShape


OUString SAL_CALL SvxShapeGroup::getShapeType()
{
    return SvxShape::getShapeType();
}

awt::Point SAL_CALL SvxShapeGroup::getPosition()
{
    return SvxShape::getPosition();
}


void SAL_CALL SvxShapeGroup::setPosition( const awt::Point& Position )
{
    SvxShape::setPosition(Position);
}


awt::Size SAL_CALL SvxShapeGroup::getSize()
{
    return SvxShape::getSize();
}


void SAL_CALL SvxShapeGroup::setSize( const awt::Size& rSize )
{
    SvxShape::setSize( rSize );
}

// drawing::XShapeGroup


void SAL_CALL SvxShapeGroup::enterGroup(  )
{
    // Todo
//  pDrView->EnterMarkedGroup();
}


void SAL_CALL SvxShapeGroup::leaveGroup(  )
{
    // Todo
//  pDrView->LeaveOneGroup();
}

void SvxShapeGroup::addUnoShape( const uno::Reference< drawing::XShape >& xShape, size_t nPos )
{
    if (!HasSdrObject() || !mxPage.is())
    {
        OSL_FAIL("could not add XShape to group shape!");
        return;
    }

    SvxShape* pShape = SvxShape::getImplementation( xShape );
    if (!pShape)
    {
        OSL_FAIL("could not add XShape to group shape!");
        return;
    }

    SdrObject* pSdrShape = pShape->GetSdrObject();
    if( pSdrShape == nullptr )
        pSdrShape = mxPage->CreateSdrObject_( xShape );

    if( pSdrShape->IsInserted() )
        pSdrShape->getParentSdrObjListFromSdrObject()->RemoveObject( pSdrShape->GetOrdNum() );

    GetSdrObject()->GetSubList()->InsertObject(pSdrShape, nPos);
    // TTTT Was created using mpModel in CreateSdrObject_ above
    // TTTT may be good to add a assertion here for the future
    // pSdrShape->SetModel(GetSdrObject()->GetModel());

    // #85922# It makes no sense to set the layer asked
    // from the group object since this is an iteration
    // over the contained objects. In consequence, this
    // statement erases all layer information from the draw
    // objects. Layers need to be set at draw objects directly
    // and have nothing to do with grouping at all.
    // pSdrShape->SetLayer(pObject->GetLayer());

    // Establish connection between new SdrObject and its wrapper before
    // inserting the new shape into the group.  There a new wrapper
    // would be created when this connection would not already exist.
    pShape->Create( pSdrShape, mxPage.get() );

    GetSdrObject()->getSdrModelFromSdrObject().SetChanged();
}

// XShapes
void SAL_CALL SvxShapeGroup::add( const uno::Reference< drawing::XShape >& xShape )
{
    ::SolarMutexGuard aGuard;

    // Add to the top of the stack (i.e. bottom of the list) by default.
    addUnoShape(xShape, SAL_MAX_SIZE);
}


void SAL_CALL SvxShapeGroup::remove( const uno::Reference< drawing::XShape >& xShape )
{
    ::SolarMutexGuard aGuard;

    SdrObject* pSdrShape = nullptr;
    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if( pShape )
        pSdrShape = pShape->GetSdrObject();

    if( !HasSdrObject() || pSdrShape == nullptr || pSdrShape->getParentSdrObjectFromSdrObject() != GetSdrObject() )
        throw uno::RuntimeException();

    SdrObjList& rList = *pSdrShape->getParentSdrObjListFromSdrObject();

    const size_t nObjCount = rList.GetObjCount();
    size_t nObjNum = 0;
    while( nObjNum < nObjCount )
    {
        if(rList.GetObj( nObjNum ) == pSdrShape )
            break;
        nObjNum++;
    }

    if( nObjNum < nObjCount )
    {
        // #i29181#
        // If the SdrObject which is about to be deleted is in any selection,
        // deselect it first.
        SdrViewIter aIter( pSdrShape );

        for ( SdrView* pView = aIter.FirstView(); pView; pView = aIter.NextView() )
        {
            if(SAL_MAX_SIZE != pView->TryToFindMarkedObject(pSdrShape))
            {
                pView->MarkObj(pSdrShape, pView->GetSdrPageView(), true);
            }
        }

        SdrObject* pObject = rList.NbcRemoveObject( nObjNum );
        SdrObject::Free( pObject );
    }
    else
    {
        SAL_WARN( "svx", "Fatality! SdrObject is not belonging to its SdrObjList! [CL]" );
    }

    GetSdrObject()->getSdrModelFromSdrObject().SetChanged();
}

void SAL_CALL SvxShapeGroup::addTop( const uno::Reference< drawing::XShape >& xShape )
{
    SolarMutexGuard aGuard;

    // Add to the top of the stack (i.e. bottom of the list).
    addUnoShape(xShape, SAL_MAX_SIZE);
}

void SAL_CALL SvxShapeGroup::addBottom( const uno::Reference< drawing::XShape >& xShape )
{
    SolarMutexGuard aGuard;

    // Add to the bottom of the stack (i.e. top of the list).
    addUnoShape(xShape, 0);
}

// XIndexAccess


sal_Int32 SAL_CALL SvxShapeGroup::getCount()
{
    ::SolarMutexGuard aGuard;

    if(!HasSdrObject() || !GetSdrObject()->GetSubList())
        throw uno::RuntimeException();

    sal_Int32 nRetval = GetSdrObject()->GetSubList()->GetObjCount();
    return nRetval;
}


uno::Any SAL_CALL SvxShapeGroup::getByIndex( sal_Int32 Index )
{
    ::SolarMutexGuard aGuard;

    if( !HasSdrObject() || GetSdrObject()->GetSubList() == nullptr )
        throw uno::RuntimeException();

    if( Index<0 || GetSdrObject()->GetSubList()->GetObjCount() <= static_cast<size_t>(Index) )
        throw lang::IndexOutOfBoundsException();

    SdrObject* pDestObj = GetSdrObject()->GetSubList()->GetObj( Index );

    if(pDestObj == nullptr)
        throw lang::IndexOutOfBoundsException();

    Reference< drawing::XShape > xShape( pDestObj->getUnoShape(), uno::UNO_QUERY );
    return uno::makeAny( xShape );
}

// css::container::XElementAccess


uno::Type SAL_CALL SvxShapeGroup::getElementType()
{
    return cppu::UnoType<drawing::XShape>::get();
}


sal_Bool SAL_CALL SvxShapeGroup::hasElements()
{
    ::SolarMutexGuard aGuard;

    return HasSdrObject() && GetSdrObject()->GetSubList() && (GetSdrObject()->GetSubList()->GetObjCount() > 0);
}

SvxShapeConnector::SvxShapeConnector(SdrObject* pObj)
    : SvxShapeText( pObj, getSvxMapProvider().GetMap(SVXMAP_CONNECTOR), getSvxMapProvider().GetPropertySet(SVXMAP_CONNECTOR, SdrObject::GetGlobalDrawObjectItemPool()) )
{
}


SvxShapeConnector::~SvxShapeConnector() throw()
{
}


uno::Any SAL_CALL SvxShapeConnector::queryInterface( const uno::Type & rType )
{
    return SvxShapeText::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeConnector::queryAggregation( const uno::Type & rType )
{
    uno::Any aAny;

    QUERYINT( drawing::XConnectorShape );
    else
        return SvxShapeText::queryAggregation( rType );

    return aAny;
}

void SAL_CALL SvxShapeConnector::acquire() throw ( )
{
    SvxShapeText::acquire();
}

void SAL_CALL SvxShapeConnector::release() throw ( )
{
    SvxShapeText::release();
}
// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxShapeConnector::getTypes()
{
    return SvxShape::getTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxShapeConnector::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// css::drawing::XShape


OUString SAL_CALL SvxShapeConnector::getShapeType()
{
    return SvxShapeText::getShapeType();
}

awt::Point SAL_CALL SvxShapeConnector::getPosition()
{
    return SvxShapeText::getPosition();
}


void SAL_CALL SvxShapeConnector::setPosition( const awt::Point& Position )
{
    SvxShapeText::setPosition(Position);
}


awt::Size SAL_CALL SvxShapeConnector::getSize()
{
    return SvxShapeText::getSize();
}


void SAL_CALL SvxShapeConnector::setSize( const awt::Size& rSize )
{
    SvxShapeText::setSize( rSize );
}


// XConnectorShape

void SAL_CALL SvxShapeConnector::connectStart( const uno::Reference< drawing::XConnectableShape >& xShape, drawing::ConnectionType )
{
    ::SolarMutexGuard aGuard;

    Reference< drawing::XShape > xRef( xShape, UNO_QUERY );
    SvxShape* pShape = SvxShape::getImplementation( xRef );

    if( pShape )
        GetSdrObject()->ConnectToNode( true, pShape->GetSdrObject() );

    GetSdrObject()->getSdrModelFromSdrObject().SetChanged();
}


void SAL_CALL SvxShapeConnector::connectEnd( const uno::Reference< drawing::XConnectableShape >& xShape, drawing::ConnectionType  )
{
    ::SolarMutexGuard aGuard;

    Reference< drawing::XShape > xRef( xShape, UNO_QUERY );
    SvxShape* pShape = SvxShape::getImplementation( xRef );

    if( HasSdrObject() && pShape )
        GetSdrObject()->ConnectToNode( false, pShape->GetSdrObject() );

    GetSdrObject()->getSdrModelFromSdrObject().SetChanged();
}


void SAL_CALL SvxShapeConnector::disconnectBegin( const uno::Reference< drawing::XConnectableShape >&  )
{
    ::SolarMutexGuard aGuard;

    if(HasSdrObject())
        GetSdrObject()->DisconnectFromNode( true );

    GetSdrObject()->getSdrModelFromSdrObject().SetChanged();
}


void SAL_CALL SvxShapeConnector::disconnectEnd( const uno::Reference< drawing::XConnectableShape >& )
{
    ::SolarMutexGuard aGuard;

    if(HasSdrObject())
        GetSdrObject()->DisconnectFromNode( false );

    GetSdrObject()->getSdrModelFromSdrObject().SetChanged();
}

SvxShapeControl::SvxShapeControl(SdrObject* pObj)
    : SvxShapeText( pObj, getSvxMapProvider().GetMap(SVXMAP_CONTROL), getSvxMapProvider().GetPropertySet(SVXMAP_CONTROL, SdrObject::GetGlobalDrawObjectItemPool()) )
{
    setShapeKind( OBJ_UNO );
}


SvxShapeControl::~SvxShapeControl() throw()
{
}


uno::Any SAL_CALL SvxShapeControl::queryInterface( const uno::Type & rType )
{
    return SvxShapeText::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeControl::queryAggregation( const uno::Type & rType )
{
    uno::Any aAny;

    QUERYINT( drawing::XControlShape );
    else
        return SvxShapeText::queryAggregation( rType );

    return aAny;
}

void SAL_CALL SvxShapeControl::acquire() throw ( )
{
    SvxShapeText::acquire();
}

void SAL_CALL SvxShapeControl::release() throw ( )
{
    SvxShapeText::release();
}
// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxShapeControl::getTypes()
{
    return SvxShape::getTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxShapeControl::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// css::drawing::XShape


OUString SAL_CALL SvxShapeControl::getShapeType()
{
    return SvxShapeText::getShapeType();
}

awt::Point SAL_CALL SvxShapeControl::getPosition()
{
    return SvxShapeText::getPosition();
}


void SAL_CALL SvxShapeControl::setPosition( const awt::Point& Position )
{
    SvxShapeText::setPosition(Position);
}


awt::Size SAL_CALL SvxShapeControl::getSize()
{
    return SvxShapeText::getSize();
}


void SAL_CALL SvxShapeControl::setSize( const awt::Size& rSize )
{
    SvxShapeText::setSize( rSize );
}


// XControlShape

Reference< awt::XControlModel > SAL_CALL SvxShapeControl::getControl()
{
    ::SolarMutexGuard aGuard;

    Reference< awt::XControlModel > xModel;

    SdrUnoObj* pUnoObj = dynamic_cast< SdrUnoObj * >(GetSdrObject());
    if( pUnoObj )
        xModel = pUnoObj->GetUnoControlModel();

    return xModel;
}


void SAL_CALL SvxShapeControl::setControl( const Reference< awt::XControlModel >& xControl )
{
    ::SolarMutexGuard aGuard;

    SdrUnoObj* pUnoObj = dynamic_cast< SdrUnoObj * >(GetSdrObject());
    if( pUnoObj )
        pUnoObj->SetUnoControlModel( xControl );

    GetSdrObject()->getSdrModelFromSdrObject().SetChanged();
}

static struct
{
    const sal_Char* mpAPIName;
    sal_uInt16 mnAPINameLen;

    const sal_Char* mpFormName;
    sal_uInt16 mnFormNameLen;
}
const SvxShapeControlPropertyMapping[] =
{
    // Warning: The first entry must be FontSlant because the any needs to be converted
    { RTL_CONSTASCII_STRINGPARAM(UNO_NAME_EDIT_CHAR_POSTURE), RTL_CONSTASCII_STRINGPARAM("FontSlant")  }, //  const sal_Int16 => css::awt::FontSlant
    { RTL_CONSTASCII_STRINGPARAM(UNO_NAME_EDIT_CHAR_FONTNAME), RTL_CONSTASCII_STRINGPARAM("FontName") },
    { RTL_CONSTASCII_STRINGPARAM(UNO_NAME_EDIT_CHAR_FONTSTYLENAME), RTL_CONSTASCII_STRINGPARAM("FontStyleName") },
    { RTL_CONSTASCII_STRINGPARAM(UNO_NAME_EDIT_CHAR_FONTFAMILY), RTL_CONSTASCII_STRINGPARAM("FontFamily") },
    { RTL_CONSTASCII_STRINGPARAM(UNO_NAME_EDIT_CHAR_FONTCHARSET), RTL_CONSTASCII_STRINGPARAM("FontCharset") },
    { RTL_CONSTASCII_STRINGPARAM(UNO_NAME_EDIT_CHAR_HEIGHT), RTL_CONSTASCII_STRINGPARAM("FontHeight") },
    { RTL_CONSTASCII_STRINGPARAM(UNO_NAME_EDIT_CHAR_FONTPITCH), RTL_CONSTASCII_STRINGPARAM("FontPitch" ) },
    { RTL_CONSTASCII_STRINGPARAM(UNO_NAME_EDIT_CHAR_WEIGHT), RTL_CONSTASCII_STRINGPARAM("FontWeight" ) },
    { RTL_CONSTASCII_STRINGPARAM(UNO_NAME_EDIT_CHAR_UNDERLINE), RTL_CONSTASCII_STRINGPARAM("FontUnderline") },
    { RTL_CONSTASCII_STRINGPARAM(UNO_NAME_EDIT_CHAR_STRIKEOUT), RTL_CONSTASCII_STRINGPARAM("FontStrikeout") },
    { RTL_CONSTASCII_STRINGPARAM("CharKerning"), RTL_CONSTASCII_STRINGPARAM("FontKerning") },
    { RTL_CONSTASCII_STRINGPARAM("CharWordMode"), RTL_CONSTASCII_STRINGPARAM("FontWordLineMode" ) },
    { RTL_CONSTASCII_STRINGPARAM(UNO_NAME_EDIT_CHAR_COLOR),   RTL_CONSTASCII_STRINGPARAM("TextColor") },
    { RTL_CONSTASCII_STRINGPARAM("CharBackColor"), RTL_CONSTASCII_STRINGPARAM("CharBackColor") },
    { RTL_CONSTASCII_STRINGPARAM("CharBackTransparent"), RTL_CONSTASCII_STRINGPARAM("CharBackTransparent") },
    { RTL_CONSTASCII_STRINGPARAM(UNO_NAME_TEXT_CHAINNEXTNAME), RTL_CONSTASCII_STRINGPARAM(UNO_NAME_TEXT_CHAINNEXTNAME) },
    { RTL_CONSTASCII_STRINGPARAM("CharRelief"),   RTL_CONSTASCII_STRINGPARAM("FontRelief") },
    { RTL_CONSTASCII_STRINGPARAM("CharUnderlineColor"),   RTL_CONSTASCII_STRINGPARAM("TextLineColor") },
    { RTL_CONSTASCII_STRINGPARAM(UNO_NAME_EDIT_PARA_ADJUST), RTL_CONSTASCII_STRINGPARAM("Align") },
    { RTL_CONSTASCII_STRINGPARAM("TextVerticalAdjust"), RTL_CONSTASCII_STRINGPARAM("VerticalAlign") },
    { RTL_CONSTASCII_STRINGPARAM("ControlBackground"), RTL_CONSTASCII_STRINGPARAM("BackgroundColor") },
    { RTL_CONSTASCII_STRINGPARAM("ControlSymbolColor"), RTL_CONSTASCII_STRINGPARAM("SymbolColor") },
    { RTL_CONSTASCII_STRINGPARAM("ControlBorder"), RTL_CONSTASCII_STRINGPARAM("Border") },
    { RTL_CONSTASCII_STRINGPARAM("ControlBorderColor"), RTL_CONSTASCII_STRINGPARAM("BorderColor") },
    { RTL_CONSTASCII_STRINGPARAM("ControlTextEmphasis"),  RTL_CONSTASCII_STRINGPARAM("FontEmphasisMark") },
    { RTL_CONSTASCII_STRINGPARAM("ImageScaleMode"),  RTL_CONSTASCII_STRINGPARAM("ScaleMode") },
    { RTL_CONSTASCII_STRINGPARAM("ControlWritingMode"), RTL_CONSTASCII_STRINGPARAM("WritingMode") },
    //added for exporting OCX control
    { RTL_CONSTASCII_STRINGPARAM("ControlTypeinMSO"), RTL_CONSTASCII_STRINGPARAM("ControlTypeinMSO") },
    { RTL_CONSTASCII_STRINGPARAM("ObjIDinMSO"), RTL_CONSTASCII_STRINGPARAM("ObjIDinMSO") },
    { RTL_CONSTASCII_STRINGPARAM("CharCaseMap"), RTL_CONSTASCII_STRINGPARAM("CharCaseMap") },
    { nullptr,0, nullptr, 0 }
};

namespace
{
    bool lcl_convertPropertyName( const OUString& rApiName, OUString& rInternalName )
    {
        sal_uInt16 i = 0;
        while( SvxShapeControlPropertyMapping[i].mpAPIName )
        {
            if( rApiName.reverseCompareToAsciiL( SvxShapeControlPropertyMapping[i].mpAPIName, SvxShapeControlPropertyMapping[i].mnAPINameLen ) == 0 )
            {
                rInternalName = OUString( SvxShapeControlPropertyMapping[i].mpFormName, SvxShapeControlPropertyMapping[i].mnFormNameLen, RTL_TEXTENCODING_ASCII_US );
            }
            ++i;
        }
        return !rInternalName.isEmpty();
    }

    struct EnumConversionMap
    {
        style::ParagraphAdjust   nAPIValue;
        sal_Int16                nFormValue;
    };

    EnumConversionMap const aMapAdjustToAlign[] =
    {
        // note that order matters:
        // lcl_convertTextAlignmentToParaAdjustment and lcl_convertParaAdjustmentToTextAlignment search this map from the _beginning_
        // and use the first matching entry
        {style::ParagraphAdjust_LEFT,           sal_Int16(awt::TextAlign::LEFT)},
        {style::ParagraphAdjust_CENTER,         sal_Int16(awt::TextAlign::CENTER)},
        {style::ParagraphAdjust_RIGHT,          sal_Int16(awt::TextAlign::RIGHT)},
        {style::ParagraphAdjust_BLOCK,          sal_Int16(awt::TextAlign::RIGHT)},
        {style::ParagraphAdjust_STRETCH,        sal_Int16(awt::TextAlign::LEFT)},
        {style::ParagraphAdjust(-1),-1}
    };

    void lcl_convertTextAlignmentToParaAdjustment( Any& _rValue )
    {
        sal_Int16 nValue = sal_Int16();
        OSL_VERIFY( _rValue >>= nValue );

        for ( auto const & rEntry : aMapAdjustToAlign )
            if ( nValue == rEntry.nFormValue )
            {
                _rValue <<= static_cast<sal_uInt16>(rEntry.nAPIValue);
                return;
            }
    }

    void lcl_convertParaAdjustmentToTextAlignment( Any& _rValue )
    {
        sal_Int32 nValue = 0;
        OSL_VERIFY( _rValue >>= nValue );

        for ( auto const & rEntry : aMapAdjustToAlign )
            if ( static_cast<style::ParagraphAdjust>(nValue) == rEntry.nAPIValue )
            {
                _rValue <<= rEntry.nFormValue;
                return;
            }
    }

    void convertVerticalAdjustToVerticalAlign( Any& _rValue )
    {
        if ( !_rValue.hasValue() )
            return;

        drawing::TextVerticalAdjust eAdjust = drawing::TextVerticalAdjust_TOP;
        style::VerticalAlignment    eAlign  = style::VerticalAlignment_TOP;
        if ( !( _rValue >>= eAdjust ) )
            throw lang::IllegalArgumentException();
        switch ( eAdjust )
        {
        case drawing::TextVerticalAdjust_TOP:    eAlign = style::VerticalAlignment_TOP; break;
        case drawing::TextVerticalAdjust_BOTTOM: eAlign = style::VerticalAlignment_BOTTOM; break;
        default:                                 eAlign = style::VerticalAlignment_MIDDLE; break;
        }
        _rValue <<= eAlign;
    }

    void convertVerticalAlignToVerticalAdjust( Any& _rValue )
    {
        if ( !_rValue.hasValue() )
            return;
        style::VerticalAlignment    eAlign  = style::VerticalAlignment_TOP;
        drawing::TextVerticalAdjust eAdjust = drawing::TextVerticalAdjust_TOP;
        OSL_VERIFY( _rValue >>= eAlign );
        switch ( eAlign )
        {
        case style::VerticalAlignment_TOP:    eAdjust = drawing::TextVerticalAdjust_TOP; break;
        case style::VerticalAlignment_BOTTOM: eAdjust = drawing::TextVerticalAdjust_BOTTOM; break;
        default:                              eAdjust = drawing::TextVerticalAdjust_CENTER; break;
        }
        _rValue <<= eAdjust;
    }
}

void SAL_CALL SvxShapeControl::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    OUString aFormsName;
    if ( lcl_convertPropertyName( aPropertyName, aFormsName ) )
    {
        uno::Reference< beans::XPropertySet > xControl( getControl(), uno::UNO_QUERY );
        if( xControl.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xInfo( xControl->getPropertySetInfo() );
            if( xInfo.is() && xInfo->hasPropertyByName( aFormsName ) )
            {
                uno::Any aConvertedValue( aValue );
                if ( aFormsName == "FontSlant" )
                {
                    awt::FontSlant nSlant;
                    if( !(aValue >>= nSlant ) )
                        throw lang::IllegalArgumentException();
                    aConvertedValue <<= static_cast<sal_Int16>(nSlant);
                }
                else if ( aFormsName == "Align" )
                {
                    lcl_convertParaAdjustmentToTextAlignment( aConvertedValue );
                }
                else if ( aFormsName == "VerticalAlign" )
                {
                    convertVerticalAdjustToVerticalAlign( aConvertedValue );
                }

                xControl->setPropertyValue( aFormsName, aConvertedValue );
            }
        }
    }
    else
    {
        SvxShape::setPropertyValue( aPropertyName, aValue );
    }
}

uno::Any SAL_CALL SvxShapeControl::getPropertyValue( const OUString& aPropertyName )
{
    OUString aFormsName;
    if ( lcl_convertPropertyName( aPropertyName, aFormsName ) )
    {
        uno::Reference< beans::XPropertySet > xControl( getControl(), uno::UNO_QUERY );

        uno::Any aValue;
        if( xControl.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xInfo( xControl->getPropertySetInfo() );
            if( xInfo.is() && xInfo->hasPropertyByName( aFormsName ) )
            {
                aValue = xControl->getPropertyValue( aFormsName );
                if ( aFormsName == "FontSlant" )
                {
                    awt::FontSlant eSlant = awt::FontSlant_NONE;
                    sal_Int16 nSlant = sal_Int16();
                    if ( aValue >>= nSlant )
                    {
                        eSlant = static_cast<awt::FontSlant>(nSlant);
                    }
                    else
                    {
                        OSL_VERIFY( aValue >>= eSlant );
                    }
                    aValue <<= eSlant;
                }
                else if ( aFormsName == "Align" )
                {
                    lcl_convertTextAlignmentToParaAdjustment( aValue );
                }
                else if ( aFormsName == "VerticalAlign" )
                {
                    convertVerticalAlignToVerticalAdjust( aValue );
                }
            }
        }

        return aValue;
    }
    else
    {
        return SvxShape::getPropertyValue( aPropertyName );
    }

}

// XPropertyState
beans::PropertyState SAL_CALL SvxShapeControl::getPropertyState( const OUString& PropertyName )
{
    OUString aFormsName;
    if ( lcl_convertPropertyName( PropertyName, aFormsName ) )
    {
        uno::Reference< beans::XPropertyState > xControl( getControl(), uno::UNO_QUERY );
        uno::Reference< beans::XPropertySet > xPropSet( getControl(), uno::UNO_QUERY );

        if( xControl.is() && xPropSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xInfo( xPropSet->getPropertySetInfo() );
            if( xInfo.is() && xInfo->hasPropertyByName( aFormsName ) )
            {
                return xControl->getPropertyState( aFormsName );
            }
        }

        return beans::PropertyState_DEFAULT_VALUE;
    }
    else
    {
        return SvxShape::getPropertyState( PropertyName );
    }
}

void SAL_CALL SvxShapeControl::setPropertyToDefault( const OUString& PropertyName )
{
    OUString aFormsName;
    if ( lcl_convertPropertyName( PropertyName, aFormsName ) )
    {
        uno::Reference< beans::XPropertyState > xControl( getControl(), uno::UNO_QUERY );
        uno::Reference< beans::XPropertySet > xPropSet( getControl(), uno::UNO_QUERY );

        if( xControl.is() && xPropSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xInfo( xPropSet->getPropertySetInfo() );
            if( xInfo.is() && xInfo->hasPropertyByName( aFormsName ) )
            {
                xControl->setPropertyToDefault( aFormsName );
            }
        }
    }
    else
    {
        SvxShape::setPropertyToDefault( PropertyName );
    }
}

uno::Any SAL_CALL SvxShapeControl::getPropertyDefault( const OUString& aPropertyName )
{
    OUString aFormsName;
    if ( lcl_convertPropertyName( aPropertyName, aFormsName ) )
    {
        uno::Reference< beans::XPropertyState > xControl( getControl(), uno::UNO_QUERY );

        if( xControl.is() )
        {
            Any aDefault( xControl->getPropertyDefault( aFormsName ) );
            if ( aFormsName == "FontSlant" )
            {
                sal_Int16 nSlant( 0 );
                aDefault >>= nSlant;
                aDefault <<= static_cast<awt::FontSlant>(nSlant);
            }
            else if ( aFormsName == "Align" )
            {
                lcl_convertTextAlignmentToParaAdjustment( aDefault );
            }
            else if ( aFormsName == "VerticalAlign" )
            {
                convertVerticalAlignToVerticalAdjust( aDefault );
            }
            return aDefault;
        }

        throw beans::UnknownPropertyException( aPropertyName, static_cast<cppu::OWeakObject*>(this));
    }
    else
    {
        return SvxShape::getPropertyDefault( aPropertyName );
    }
}

SvxShapeDimensioning::SvxShapeDimensioning(SdrObject* pObj)
    : SvxShapeText( pObj, getSvxMapProvider().GetMap(SVXMAP_DIMENSIONING), getSvxMapProvider().GetPropertySet(SVXMAP_DIMENSIONING, SdrObject::GetGlobalDrawObjectItemPool()) )
{
}

SvxShapeDimensioning::~SvxShapeDimensioning() throw()
{
}

SvxShapeCircle::SvxShapeCircle(SdrObject* pObj)
    : SvxShapeText( pObj, getSvxMapProvider().GetMap(SVXMAP_CIRCLE), getSvxMapProvider().GetPropertySet(SVXMAP_CIRCLE, SdrObject::GetGlobalDrawObjectItemPool()) )
{
}

SvxShapeCircle::~SvxShapeCircle() throw()
{
}

//////////////////////////////////////////////////////////////////////////////

SvxShapePolyPolygon::SvxShapePolyPolygon(
    SdrObject* pObj)
:   SvxShapeText(
        pObj,
        getSvxMapProvider().GetMap(SVXMAP_POLYPOLYGON),
        getSvxMapProvider().GetPropertySet(SVXMAP_POLYPOLYGON, SdrObject::GetGlobalDrawObjectItemPool()))
{
}

SvxShapePolyPolygon::~SvxShapePolyPolygon() throw()
{
}

bool SvxShapePolyPolygon::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_VALUE_POLYPOLYGONBEZIER:
    {
        if( auto s = o3tl::tryAccess<drawing::PolyPolygonBezierCoords>(rValue) )
        {
            basegfx::B2DPolyPolygon aNewPolyPolygon(
                basegfx::utils::UnoPolyPolygonBezierCoordsToB2DPolyPolygon(*s));

            // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
            ForceMetricToItemPoolMetric(aNewPolyPolygon);

            SetPolygon(aNewPolyPolygon);
            return true;
        }
        break;
    }
    case OWN_ATTR_VALUE_POLYPOLYGON:
    {
        if( auto s = o3tl::tryAccess<drawing::PointSequenceSequence>(rValue) )
        {
            basegfx::B2DPolyPolygon aNewPolyPolygon(
                basegfx::utils::UnoPointSequenceSequenceToB2DPolyPolygon(*s));

            // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
            ForceMetricToItemPoolMetric(aNewPolyPolygon);

            SetPolygon(aNewPolyPolygon);
            return true;
        }
        break;
    }
    case OWN_ATTR_BASE_GEOMETRY:
    {
        drawing::PointSequenceSequence aPointSequenceSequence;
        drawing::PolyPolygonBezierCoords aPolyPolygonBezierCoords;

        if( rValue >>= aPointSequenceSequence)
        {
            if( HasSdrObject() )
            {
                basegfx::B2DPolyPolygon aNewPolyPolygon;
                basegfx::B2DHomMatrix aNewHomogenMatrix;

                GetSdrObject()->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);
                aNewPolyPolygon = basegfx::utils::UnoPointSequenceSequenceToB2DPolyPolygon(aPointSequenceSequence);

                // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
                // Need to adapt aNewPolyPolygon from 100thmm to app-specific
                ForceMetricToItemPoolMetric(aNewPolyPolygon);

                GetSdrObject()->TRSetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);
            }
            return true;
        }
        else if( rValue >>= aPolyPolygonBezierCoords)
        {
            if( HasSdrObject() )
            {
                basegfx::B2DPolyPolygon aNewPolyPolygon;
                basegfx::B2DHomMatrix aNewHomogenMatrix;

                GetSdrObject()->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);
                aNewPolyPolygon = basegfx::utils::UnoPolyPolygonBezierCoordsToB2DPolyPolygon(aPolyPolygonBezierCoords);

                // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
                ForceMetricToItemPoolMetric(aNewPolyPolygon);

                GetSdrObject()->TRSetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);
            }
            return true;
        }
        break;
    }
    case OWN_ATTR_VALUE_POLYGON:
    {
        if( auto pSequence = o3tl::tryAccess<drawing::PointSequence>(rValue) )
        {
            // prepare new polygon
            basegfx::B2DPolygon aNewPolygon;

            // get pointer to arrays
            const awt::Point* pArray    = pSequence->getConstArray();
            const awt::Point* pArrayEnd = pArray + pSequence->getLength();

            for(;pArray != pArrayEnd;++pArray)
            {
                aNewPolygon.append(basegfx::B2DPoint(pArray->X, pArray->Y));
            }

            // check for closed state flag
            basegfx::utils::checkClosed(aNewPolygon);

            // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
            basegfx::B2DPolyPolygon aNewPolyPolygon(aNewPolygon);
            ForceMetricToItemPoolMetric(aNewPolyPolygon);

            // set polygon
            SetPolygon(aNewPolyPolygon);
            return true;
        }
        break;
    }
    default:
        return SvxShapeText::setPropertyValueImpl( rName, pProperty, rValue );
    }

    throw lang::IllegalArgumentException();
}

bool SvxShapePolyPolygon::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty,
                                                css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_VALUE_POLYPOLYGONBEZIER:
    {
        // pack a tools::PolyPolygon in a struct tools::PolyPolygon
        basegfx::B2DPolyPolygon aPolyPoly(GetPolygon());

        // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
        ForceMetricTo100th_mm(aPolyPoly);

        drawing::PolyPolygonBezierCoords aRetval;
        basegfx::utils::B2DPolyPolygonToUnoPolyPolygonBezierCoords(aPolyPoly, aRetval);

        rValue <<= aRetval;
        break;
    }
    case OWN_ATTR_VALUE_POLYPOLYGON:
    {
        // pack a tools::PolyPolygon in a struct tools::PolyPolygon
        basegfx::B2DPolyPolygon aPolyPoly(GetPolygon());

        // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
        ForceMetricTo100th_mm(aPolyPoly);

        drawing::PointSequenceSequence aRetval( aPolyPoly.count() );
        basegfx::utils::B2DPolyPolygonToUnoPointSequenceSequence(aPolyPoly, aRetval);

        rValue <<= aRetval;
        break;
    }
    case OWN_ATTR_BASE_GEOMETRY:
    {
        // pack a tools::PolyPolygon in struct PolyPolygon
        basegfx::B2DPolyPolygon aPolyPoly;
        basegfx::B2DHomMatrix aNewHomogenMatrix;

        if(HasSdrObject())
        {
            GetSdrObject()->TRGetBaseGeometry(aNewHomogenMatrix, aPolyPoly);

            // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
            ForceMetricTo100th_mm(aPolyPoly);
        }

        if(aPolyPoly.areControlPointsUsed())
        {
            drawing::PolyPolygonBezierCoords aRetval;
            basegfx::utils::B2DPolyPolygonToUnoPolyPolygonBezierCoords(aPolyPoly, aRetval);
            rValue <<= aRetval;
        }
        else
        {
            drawing::PointSequenceSequence aRetval(aPolyPoly.count());
            basegfx::utils::B2DPolyPolygonToUnoPointSequenceSequence(aPolyPoly, aRetval);
            rValue <<= aRetval;
        }
        break;
    }
    case OWN_ATTR_VALUE_POLYGON:
    {
        // pack a tools::PolyPolygon in a struct tools::PolyPolygon
        basegfx::B2DPolyPolygon aPolyPoly(GetPolygon());

        // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
        ForceMetricTo100th_mm(aPolyPoly);

        const sal_Int32 nCount(0 == aPolyPoly.count() ? 0 : aPolyPoly.getB2DPolygon(0).count());
        drawing::PointSequence aRetval( nCount );

        if( nCount > 0 )
        {
            // get single polygon
            const basegfx::B2DPolygon aPoly(aPolyPoly.getB2DPolygon(0));

            // get pointer to arrays
            awt::Point* pSequence = aRetval.getArray();

            for(sal_Int32 b=0;b<nCount;b++)
            {
                const basegfx::B2DPoint aPoint(aPoly.getB2DPoint(b));
                *pSequence++ = awt::Point( basegfx::fround(aPoint.getX()), basegfx::fround(aPoint.getY()) );
            }
        }

        rValue <<= aRetval;
        break;
    }
    case OWN_ATTR_VALUE_POLYGONKIND:
    {
        rValue <<= GetPolygonKind();
        break;
    }
    default:
        return SvxShapeText::getPropertyValueImpl( rName, pProperty, rValue );
    }

    return true;
}

drawing::PolygonKind SvxShapePolyPolygon::GetPolygonKind() const
{
    ::SolarMutexGuard aGuard;
    drawing::PolygonKind aRetval(drawing::PolygonKind_LINE);

    if(HasSdrObject())
    {
        switch(GetSdrObject()->GetObjIdentifier())
        {
            case OBJ_POLY:      aRetval = drawing::PolygonKind_POLY; break;
            case OBJ_PLIN:      aRetval = drawing::PolygonKind_PLIN; break;
            case OBJ_SPLNLINE:
            case OBJ_PATHLINE:  aRetval = drawing::PolygonKind_PATHLINE; break;
            case OBJ_SPLNFILL:
            case OBJ_PATHFILL:  aRetval = drawing::PolygonKind_PATHFILL; break;
            case OBJ_FREELINE:  aRetval = drawing::PolygonKind_FREELINE; break;
            case OBJ_FREEFILL:  aRetval = drawing::PolygonKind_FREEFILL; break;
        }
    }

    return aRetval;
}

void SvxShapePolyPolygon::SetPolygon(const basegfx::B2DPolyPolygon& rNew)
{
    ::SolarMutexGuard aGuard;

    if(HasSdrObject())
        static_cast<SdrPathObj*>(GetSdrObject())->SetPathPoly(rNew);
}


basegfx::B2DPolyPolygon SvxShapePolyPolygon::GetPolygon() const throw()
{
    ::SolarMutexGuard aGuard;

    if(HasSdrObject())
    {
        return static_cast<SdrPathObj*>(GetSdrObject())->GetPathPoly();
    }
    else
    {
        return basegfx::B2DPolyPolygon();
    }
}

//////////////////////////////////////////////////////////////////////////////

SvxGraphicObject::SvxGraphicObject(SdrObject* pObj)
    : SvxShapeText( pObj, getSvxMapProvider().GetMap(SVXMAP_GRAPHICOBJECT), getSvxMapProvider().GetPropertySet(SVXMAP_GRAPHICOBJECT, SdrObject::GetGlobalDrawObjectItemPool()) )
{
}

SvxGraphicObject::~SvxGraphicObject() throw()
{
}

bool SvxGraphicObject::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    bool bOk = false;
    switch( pProperty->nWID )
    {
    case OWN_ATTR_VALUE_FILLBITMAP:
    {
        if( auto pSeq = o3tl::tryAccess<uno::Sequence<sal_Int8>>(rValue) )
        {
            SvMemoryStream  aMemStm;
            Graphic         aGraphic;

            aMemStm.SetBuffer( const_cast<css::uno::Sequence<sal_Int8> *>(pSeq)->getArray(), pSeq->getLength(), pSeq->getLength() );

            if( GraphicConverter::Import( aMemStm, aGraphic ) == ERRCODE_NONE )
            {
                static_cast<SdrGrafObj*>(GetSdrObject())->SetGraphic(aGraphic);
                bOk = true;
            }
        }
        else if (rValue.getValueType() == cppu::UnoType<graphic::XGraphic>::get())
        {
            auto xGraphic = rValue.get<uno::Reference<graphic::XGraphic>>();
            if (xGraphic.is())
            {
                static_cast<SdrGrafObj*>(GetSdrObject())->SetGraphic(Graphic(xGraphic));
                bOk = true;
            }
        }
        else if (rValue.getValueType() == cppu::UnoType<awt::XBitmap>::get())
        {
            auto xBitmap = rValue.get<uno::Reference<awt::XBitmap>>();
            if (xBitmap.is())
            {
                uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);
                Graphic aGraphic(xGraphic);
                static_cast<SdrGrafObj*>(GetSdrObject())->SetGraphic(aGraphic);
                bOk = true;
            }
        }
        break;
    }

    case OWN_ATTR_GRAFSTREAMURL:
    {
        OUString aStreamURL;

        if( rValue >>= aStreamURL )
        {
            if( !aStreamURL.startsWith( UNO_NAME_GRAPHOBJ_URLPKGPREFIX ) )
                aStreamURL.clear();

            if( HasSdrObject() )
            {
                static_cast<SdrGrafObj*>(GetSdrObject())->SetGrafStreamURL( aStreamURL );
            }
            bOk = true;
        }
        break;
    }

    case OWN_ATTR_GRAPHIC_URL:
    {
        OUString aURL;
        if (rValue >>= aURL)
        {
            Graphic aGraphic = vcl::graphic::loadFromURL(aURL);
            if (aGraphic)
            {
                static_cast<SdrGrafObj*>(GetSdrObject())->SetGraphic(aGraphic);
                bOk = true;
            }
        }
        break;
    }

    case OWN_ATTR_VALUE_GRAPHIC:
    {
        Reference< graphic::XGraphic > xGraphic( rValue, uno::UNO_QUERY );
        if( xGraphic.is() )
        {
            static_cast< SdrGrafObj*>( GetSdrObject() )->SetGraphic( xGraphic );
            bOk = true;
        }
        break;
    }

    case OWN_ATTR_IS_SIGNATURELINE:
    {
        bool bIsSignatureLine;
        if (rValue >>= bIsSignatureLine)
        {
            static_cast<SdrGrafObj*>(GetSdrObject())->setIsSignatureLine(bIsSignatureLine);
            bOk = true;
        }
        break;
    }

    case OWN_ATTR_SIGNATURELINE_ID:
    {
        OUString aSignatureLineId;
        if (rValue >>= aSignatureLineId)
        {
            static_cast<SdrGrafObj*>(GetSdrObject())->setSignatureLineId(aSignatureLineId);
            bOk = true;
        }
        break;
    }

    case OWN_ATTR_SIGNATURELINE_SUGGESTED_SIGNER_NAME:
    {
        OUString aSuggestedSignerName;
        if (rValue >>= aSuggestedSignerName)
        {
            static_cast<SdrGrafObj*>(GetSdrObject())->setSignatureLineSuggestedSignerName(aSuggestedSignerName);
            bOk = true;
        }
        break;
    }

    case OWN_ATTR_SIGNATURELINE_SUGGESTED_SIGNER_TITLE:
    {
        OUString aSuggestedSignerTitle;
        if (rValue >>= aSuggestedSignerTitle)
        {
            static_cast<SdrGrafObj*>(GetSdrObject())->setSignatureLineSuggestedSignerTitle(aSuggestedSignerTitle);
            bOk = true;
        }
        break;
    }

    case OWN_ATTR_SIGNATURELINE_SUGGESTED_SIGNER_EMAIL:
    {
        OUString aSuggestedSignerEmail;
        if (rValue >>= aSuggestedSignerEmail)
        {
            static_cast<SdrGrafObj*>(GetSdrObject())->setSignatureLineSuggestedSignerEmail(aSuggestedSignerEmail);
            bOk = true;
        }
        break;
    }

    case OWN_ATTR_SIGNATURELINE_SIGNING_INSTRUCTIONS:
    {
        OUString aSigningInstructions;
        if (rValue >>= aSigningInstructions)
        {
            static_cast<SdrGrafObj*>(GetSdrObject())->setSignatureLineSigningInstructions(aSigningInstructions);
            bOk = true;
        }
        break;
    }

    case OWN_ATTR_SIGNATURELINE_SHOW_SIGN_DATE:
    {
        bool bShowSignDate;
        if (rValue >>= bShowSignDate)
        {
            static_cast<SdrGrafObj*>(GetSdrObject())->setSignatureLineShowSignDate(bShowSignDate);
            bOk = true;
        }
        break;
    }

    case OWN_ATTR_SIGNATURELINE_CAN_ADD_COMMENT:
    {
        bool bCanAddComment;
        if (rValue >>= bCanAddComment)
        {
            static_cast<SdrGrafObj*>(GetSdrObject())->setSignatureLineCanAddComment(bCanAddComment);
            bOk = true;
        }
        break;
    }

    case OWN_ATTR_SIGNATURELINE_UNSIGNED_IMAGE:
    {
        Reference<graphic::XGraphic> xGraphic(rValue, uno::UNO_QUERY);
        if (xGraphic.is())
        {
            static_cast<SdrGrafObj*>(GetSdrObject())->setSignatureLineUnsignedGraphic(xGraphic);
            bOk = true;
        }
        break;
    }

    default:
        return SvxShapeText::setPropertyValueImpl( rName, pProperty, rValue );
    }

    if( !bOk )
        throw lang::IllegalArgumentException();

    GetSdrObject()->getSdrModelFromSdrObject().SetChanged();

    return true;
}

bool SvxGraphicObject::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_VALUE_FILLBITMAP:
    {
        const Graphic& rGraphic = static_cast<SdrGrafObj*>(GetSdrObject())->GetGraphic();

        if (rGraphic.GetType() != GraphicType::GdiMetafile)
        {
            uno::Reference<awt::XBitmap> xBitmap(rGraphic.GetXGraphic(), uno::UNO_QUERY);
            rValue <<= xBitmap;
        }
        else
        {
            SvMemoryStream aDestStrm( 65535, 65535 );

            ConvertGDIMetaFileToWMF( rGraphic.GetGDIMetaFile(), aDestStrm, nullptr, false );
            const uno::Sequence<sal_Int8> aSeq(
                static_cast< const sal_Int8* >(aDestStrm.GetData()),
                aDestStrm.GetEndOfData());
            rValue <<= aSeq;
        }
        break;
    }

    case OWN_ATTR_REPLACEMENT_GRAPHIC:
    {
        const GraphicObject* pGrafObj = static_cast< SdrGrafObj* >(GetSdrObject())->GetReplacementGraphicObject();

        if (pGrafObj)
        {
            rValue <<= pGrafObj->GetGraphic().GetXGraphic();
        }

        break;
    }

    case OWN_ATTR_GRAFSTREAMURL:
    {
        const OUString  aStreamURL( static_cast<SdrGrafObj*>( GetSdrObject() )->GetGrafStreamURL() );
        if( !aStreamURL.isEmpty() )
            rValue <<= aStreamURL;
        break;
    }

    case OWN_ATTR_GRAPHIC_URL:
    {
        throw uno::RuntimeException("Getting from this property is not unsupported");
        break;
    }

    case OWN_ATTR_VALUE_GRAPHIC:
    {
        Reference<graphic::XGraphic> xGraphic;
        auto pSdrGraphicObject = static_cast<SdrGrafObj*>(GetSdrObject());
        if (pSdrGraphicObject->GetGraphicObject().GetType() != GraphicType::NONE)
            xGraphic = pSdrGraphicObject->GetGraphic().GetXGraphic();
        rValue <<= xGraphic;
        break;
    }

    case OWN_ATTR_GRAPHIC_STREAM:
    {
        rValue <<= static_cast< SdrGrafObj* >( GetSdrObject() )->getInputStream();
        break;
    }

    case OWN_ATTR_IS_SIGNATURELINE:
    {
        rValue <<= static_cast<SdrGrafObj*>(GetSdrObject())->isSignatureLine();
        break;
    }

    case OWN_ATTR_SIGNATURELINE_ID:
    {
        rValue <<= static_cast<SdrGrafObj*>(GetSdrObject())->getSignatureLineId();
        break;
    }

    case OWN_ATTR_SIGNATURELINE_SUGGESTED_SIGNER_NAME:
    {
        rValue <<= static_cast<SdrGrafObj*>(GetSdrObject())->getSignatureLineSuggestedSignerName();
        break;
    }

    case OWN_ATTR_SIGNATURELINE_SUGGESTED_SIGNER_TITLE:
    {
        rValue <<= static_cast<SdrGrafObj*>(GetSdrObject())->getSignatureLineSuggestedSignerTitle();
        break;
    }

    case OWN_ATTR_SIGNATURELINE_SUGGESTED_SIGNER_EMAIL:
    {
        rValue <<= static_cast<SdrGrafObj*>(GetSdrObject())->getSignatureLineSuggestedSignerEmail();
        break;
    }

    case OWN_ATTR_SIGNATURELINE_SIGNING_INSTRUCTIONS:
    {
        rValue <<= static_cast<SdrGrafObj*>(GetSdrObject())->getSignatureLineSigningInstructions();
        break;
    }

    case OWN_ATTR_SIGNATURELINE_SHOW_SIGN_DATE:
    {
        rValue <<= static_cast<SdrGrafObj*>(GetSdrObject())->isSignatureLineShowSignDate();
        break;
    }

    case OWN_ATTR_SIGNATURELINE_CAN_ADD_COMMENT:
    {
        rValue <<= static_cast<SdrGrafObj*>(GetSdrObject())->isSignatureLineCanAddComment();
        break;
    }

    case OWN_ATTR_SIGNATURELINE_UNSIGNED_IMAGE:
    {
        Reference<graphic::XGraphic> xGraphic(
            static_cast<SdrGrafObj*>(GetSdrObject())->getSignatureLineUnsignedGraphic());
        rValue <<= xGraphic;
        break;
    }

    default:
        return SvxShapeText::getPropertyValueImpl(rName, pProperty,rValue);
    }

    return true;
}


SvxShapeCaption::SvxShapeCaption(SdrObject* pObj)
: SvxShapeText( pObj, getSvxMapProvider().GetMap(SVXMAP_CAPTION), getSvxMapProvider().GetPropertySet(SVXMAP_CAPTION, SdrObject::GetGlobalDrawObjectItemPool()) )
{
}

SvxShapeCaption::~SvxShapeCaption() throw()
{
}

SvxCustomShape::SvxCustomShape(SdrObject* pObj)
    : SvxShapeText( pObj, getSvxMapProvider().GetMap( SVXMAP_CUSTOMSHAPE ), getSvxMapProvider().GetPropertySet(SVXMAP_CUSTOMSHAPE, SdrObject::GetGlobalDrawObjectItemPool()) )
{
}

SvxCustomShape::~SvxCustomShape() throw()
{
}

uno::Any SAL_CALL SvxCustomShape::queryInterface( const uno::Type & rType )
{
    return SvxShapeText::queryInterface( rType );
}

uno::Any SAL_CALL SvxCustomShape::queryAggregation( const uno::Type & rType )
{
    css::uno::Any aReturn = SvxShapeText::queryAggregation( rType );
    if ( !aReturn.hasValue() )
        aReturn = ::cppu::queryInterface(rType, static_cast<drawing::XEnhancedCustomShapeDefaulter*>(this) );
    return aReturn;
}

void SAL_CALL SvxCustomShape::acquire() throw ( )
{
    SvxShapeText::acquire();
}

void SAL_CALL SvxCustomShape::release() throw ( )
{
    SvxShapeText::release();
}


uno::Sequence< uno::Type > SAL_CALL SvxCustomShape::getTypes()
{
    return SvxShapeText::getTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxCustomShape::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// css::drawing::XShape


awt::Point SAL_CALL SvxCustomShape::getPosition()
{
    ::SolarMutexGuard aGuard;
    if ( HasSdrObject() )
    {
        SdrAShapeObjGeoData aCustomShapeGeoData;
        static_cast<SdrObjCustomShape*>(GetSdrObject())->SaveGeoData( aCustomShapeGeoData );

        bool bMirroredX = false;
        bool bMirroredY = false;

        if ( HasSdrObject() )
        {
            bMirroredX = static_cast<SdrObjCustomShape*>(GetSdrObject())->IsMirroredX();
            bMirroredY = static_cast<SdrObjCustomShape*>(GetSdrObject())->IsMirroredY();
        }
        // get aRect, this is the unrotated snaprect
        tools::Rectangle aRect(static_cast<SdrObjCustomShape*>(GetSdrObject())->GetLogicRect());
        tools::Rectangle aRectangle( aRect );

        if ( bMirroredX || bMirroredY )
        {   // we have to retrieve the unmirrored rect

            GeoStat aNewGeo( aCustomShapeGeoData.aGeo );
            if ( bMirroredX )
            {
                tools::Polygon aPol( Rect2Poly( aRect, aNewGeo ) );
                tools::Rectangle aBoundRect( aPol.GetBoundRect() );

                Point aRef1( ( aBoundRect.Left() + aBoundRect.Right() ) >> 1, aBoundRect.Top() );
                Point aRef2( aRef1.X(), aRef1.Y() + 1000 );
                sal_uInt16 i;
                sal_uInt16 nPointCount=aPol.GetSize();
                for (i=0; i<nPointCount; i++)
                {
                    MirrorPoint(aPol[i],aRef1,aRef2);
                }
                // turn and move polygon
                tools::Polygon aPol0(aPol);
                aPol[0]=aPol0[1];
                aPol[1]=aPol0[0];
                aPol[2]=aPol0[3];
                aPol[3]=aPol0[2];
                aPol[4]=aPol0[1];
                Poly2Rect(aPol,aRectangle,aNewGeo);
            }
            if ( bMirroredY )
            {
                tools::Polygon aPol( Rect2Poly( aRectangle, aNewGeo ) );
                tools::Rectangle aBoundRect( aPol.GetBoundRect() );

                Point aRef1( aBoundRect.Left(), ( aBoundRect.Top() + aBoundRect.Bottom() ) >> 1 );
                Point aRef2( aRef1.X() + 1000, aRef1.Y() );
                sal_uInt16 i;
                sal_uInt16 nPointCount=aPol.GetSize();
                for (i=0; i<nPointCount; i++)
                {
                    MirrorPoint(aPol[i],aRef1,aRef2);
                }
                // turn and move polygon
                tools::Polygon aPol0(aPol);
                aPol[0]=aPol0[1];
                aPol[1]=aPol0[0];
                aPol[2]=aPol0[3];
                aPol[3]=aPol0[2];
                aPol[4]=aPol0[1];
                Poly2Rect( aPol, aRectangle, aNewGeo );
            }
        }
        Point aPt( aRectangle.TopLeft() );

        if( GetSdrObject()->getSdrModelFromSdrObject().IsWriter() )
            aPt -= GetSdrObject()->GetAnchorPos();

        ForceMetricTo100th_mm(aPt);
        return css::awt::Point( aPt.X(), aPt.Y() );
    }
    else
        return SvxShape::getPosition();
}


void SAL_CALL SvxCustomShape::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    ::SolarMutexGuard aGuard;

    SdrObject* pObject = GetSdrObject();

    // tdf#98163 Use a custom slot to have filter code flush the UNO
    // API implementations of SdrObjCustomShape. Used e.g. by
    // ~SdXMLCustomShapeContext, see there for more information
    const OUString sFlushCustomShapeUnoApiObjects("FlushCustomShapeUnoApiObjects");
    if(sFlushCustomShapeUnoApiObjects == aPropertyName)
    {
        SdrObjCustomShape* pTarget = dynamic_cast< SdrObjCustomShape* >(pObject);
        if(pTarget)
        {
            // Luckily, the object causing problems in tdf#93994 is not the
            // UNO API object, but the XCustomShapeEngine involved. This
            // object is on-demand replaceable and can be reset here. This
            // will free the involved EditEngine and VirtualDevice.
            pTarget->mxCustomShapeEngine.set(nullptr);
        }
        // since this case is only for the application cores
        // we should return from this function now
        return;
    }

    bool bCustomShapeGeometry = pObject && aPropertyName == "CustomShapeGeometry";

    bool bMirroredX = false;
    bool bMirroredY = false;

    if ( bCustomShapeGeometry )
    {
        bMirroredX = static_cast<SdrObjCustomShape*>(pObject)->IsMirroredX();
        bMirroredY = static_cast<SdrObjCustomShape*>(pObject)->IsMirroredY();
    }

    SvxShape::setPropertyValue( aPropertyName, aValue );

    if ( bCustomShapeGeometry )
    {
        static_cast<SdrObjCustomShape*>(pObject)->MergeDefaultAttributes();
        tools::Rectangle aRect( pObject->GetSnapRect() );

        // #i38892#
        bool bNeedsMirrorX = static_cast<SdrObjCustomShape*>(pObject)->IsMirroredX() != bMirroredX;
        bool bNeedsMirrorY = static_cast<SdrObjCustomShape*>(pObject)->IsMirroredY() != bMirroredY;

        std::unique_ptr< SdrGluePointList > pListCopy;
        if( bNeedsMirrorX || bNeedsMirrorY )
        {
            const SdrGluePointList* pList = pObject->GetGluePointList();
            if( pList )
                pListCopy.reset( new SdrGluePointList(*pList) );
        }

        if ( bNeedsMirrorX )
        {
            Point aTop( ( aRect.Left() + aRect.Right() ) >> 1, aRect.Top() );
            Point aBottom( aTop.X(), aTop.Y() + 1000 );
            pObject->NbcMirror( aTop, aBottom );
            // NbcMirroring is flipping the current mirror state,
            // so we have to set the correct state again
            static_cast<SdrObjCustomShape*>(pObject)->SetMirroredX( !bMirroredX );
        }
        if ( bNeedsMirrorY )
        {
            Point aLeft( aRect.Left(), ( aRect.Top() + aRect.Bottom() ) >> 1 );
            Point aRight( aLeft.X() + 1000, aLeft.Y() );
            pObject->NbcMirror( aLeft, aRight );
            // NbcMirroring is flipping the current mirror state,
            // so we have to set the correct state again
            static_cast<SdrObjCustomShape*>(pObject)->SetMirroredY( !bMirroredY );
        }

        if( pListCopy )
        {
            SdrGluePointList* pNewList = const_cast< SdrGluePointList* >( pObject->GetGluePointList() );
            if(pNewList)
                *pNewList = *pListCopy;
        }
    }
}

bool SvxCustomShape::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case SDRATTR_ROTATEANGLE:
    {
        double fAngle = static_cast<SdrObjCustomShape*>(GetSdrObject())->GetObjectRotation();
        fAngle *= 100;
        rValue <<= static_cast<sal_Int32>(fAngle);
        return true;
    }
    default:
        return SvxShape::getPropertyValueImpl( rName, pProperty, rValue );
    }
}

void SvxCustomShape::createCustomShapeDefaults( const OUString& rValueType )
{
    if (!HasSdrObject())
    {
        OSL_FAIL("could not create Custom Shape Defaults!");
        return;
    }

    static_cast<SdrObjCustomShape*>(GetSdrObject())->MergeDefaultAttributes( &rValueType );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
