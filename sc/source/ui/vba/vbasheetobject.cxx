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

#include "vbasheetobject.hxx"
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <comphelper/documentinfo.hxx>
#include <o3tl/unit_conversion.hxx>
#include <ooo/vba/excel/Constants.hpp>
#include <ooo/vba/excel/XlOrientation.hpp>
#include <ooo/vba/excel/XlPlacement.hpp>
#include <filter/msfilter/msvbahelper.hxx>
#include "vbafont.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

constexpr OUString gaListenerType = u"XActionListener"_ustr;
constexpr OUString gaEventMethod = u"actionPerformed"_ustr;

static double HmmToPoints(double nHmm)
{
    return o3tl::convert(nHmm, o3tl::Length::mm100, o3tl::Length::pt);
}

static sal_Int32 PointsToHmm(double fPoints)
{
    return std::round(o3tl::convert(fPoints, o3tl::Length::pt, o3tl::Length::mm100));
}

ScVbaButtonCharacters::ScVbaButtonCharacters(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< beans::XPropertySet >& rxPropSet,
        const ScVbaPalette& rPalette,
        const uno::Any& rStart,
        const uno::Any& rLength ) :
    ScVbaButtonCharacters_BASE( rxParent, rxContext ),
    maPalette( rPalette ),
    mxPropSet( rxPropSet, uno::UNO_SET_THROW )
{
    // extract optional start parameter (missing or invalid -> from beginning)
    if( !(rStart >>= mnStart) || (mnStart < 1) )
        mnStart = 1;
    --mnStart;  // VBA is 1-based, rtl string is 0-based

    // extract optional length parameter (missing or invalid -> to end)
    if( !(rLength >>= mnLength) || (mnLength < 1) )
        mnLength = SAL_MAX_INT32;
}

ScVbaButtonCharacters::~ScVbaButtonCharacters()
{
}

// XCharacters attributes

OUString SAL_CALL ScVbaButtonCharacters::getCaption()
{
    // ignore invalid mnStart and/or mnLength members
    OUString aString = getFullString();
    sal_Int32 nStart = ::std::min( mnStart, aString.getLength() );
    sal_Int32 nLength = ::std::min( mnLength, aString.getLength() - nStart );
    return aString.copy( nStart, nLength );
}

void SAL_CALL ScVbaButtonCharacters::setCaption( const OUString& rCaption )
{
    /*  Replace the covered text with the passed text, ignore invalid mnStart
        and/or mnLength members. This operation does not affect the mnLength
        parameter. If the inserted text is longer than mnLength, the additional
        characters are not covered by this object. If the inserted text is
        shorter than mnLength, other uncovered characters from the original
        string will be covered now, thus may be changed with subsequent
        operations. */
    OUString aString = getFullString();
    sal_Int32 nStart = ::std::min( mnStart, aString.getLength() );
    sal_Int32 nLength = ::std::min( mnLength, aString.getLength() - nStart );
    setFullString( aString.replaceAt( nStart, nLength, rCaption ) );
}

sal_Int32 SAL_CALL ScVbaButtonCharacters::getCount()
{
    // always return the total length of the caption
    return getFullString().getLength();
}

OUString SAL_CALL ScVbaButtonCharacters::getText()
{
    // Text attribute same as Caption attribute?
    return getCaption();
}

void SAL_CALL ScVbaButtonCharacters::setText( const OUString& rText )
{
    // Text attribute same as Caption attribute?
    setCaption( rText );
}

uno::Reference< excel::XFont > SAL_CALL ScVbaButtonCharacters::getFont()
{
    return new ScVbaFont( this, mxContext, maPalette, mxPropSet, nullptr, true );
}

void SAL_CALL ScVbaButtonCharacters::setFont( const uno::Reference< excel::XFont >& /*rxFont*/ )
{
    // TODO
}

// XCharacters methods

void SAL_CALL ScVbaButtonCharacters::Insert( const OUString& rString )
{
    /*  The Insert() operation is in fact "replace covered characters", at
        least for buttons... It seems there is no easy way to really insert a
        substring. This operation does not affect the mnLength parameter. */
    setCaption( rString );
}

void SAL_CALL ScVbaButtonCharacters::Delete()
{
    /*  The Delete() operation is nothing else than "replace with empty string".
        This does not affect the mnLength parameter, multiple calls of Delete()
        will remove characters as long as there are some more covered by this
        object. */
    setCaption( OUString() );
}

// XHelperInterface

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaButtonCharacters, u"ooo.vba.excel.Characters"_ustr )

// private

OUString ScVbaButtonCharacters::getFullString() const
{
    return mxPropSet->getPropertyValue( u"Label"_ustr ).get< OUString >();
}

void ScVbaButtonCharacters::setFullString( const OUString& rString )
{
    mxPropSet->setPropertyValue( u"Label"_ustr, uno::Any( rString ) );
}

ScVbaSheetObjectBase::ScVbaSheetObjectBase(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< drawing::XShape >& rxShape ) :
    ScVbaSheetObject_BASE( rxParent, rxContext ),
    maPalette( rxModel ),
    mxModel( rxModel, uno::UNO_SET_THROW ),
    mxShape( rxShape, uno::UNO_SET_THROW ),
    mxShapeProps( rxShape, uno::UNO_QUERY_THROW )
{
}

// XSheetObject attributes

double SAL_CALL ScVbaSheetObjectBase::getLeft()
{
    return HmmToPoints( mxShape->getPosition().X );
}

void SAL_CALL ScVbaSheetObjectBase::setLeft( double fLeft )
{
    if( fLeft < 0.0 )
        throw uno::RuntimeException();
    mxShape->setPosition( awt::Point( PointsToHmm( fLeft ), mxShape->getPosition().Y ) );
}

double SAL_CALL ScVbaSheetObjectBase::getTop()
{
    return HmmToPoints( mxShape->getPosition().Y );
}

void SAL_CALL ScVbaSheetObjectBase::setTop( double fTop )
{
    if( fTop < 0.0 )
        throw uno::RuntimeException();
    mxShape->setPosition( awt::Point( mxShape->getPosition().X, PointsToHmm( fTop ) ) );
}

double SAL_CALL ScVbaSheetObjectBase::getWidth()
{
    return HmmToPoints( mxShape->getSize().Width );
}

void SAL_CALL ScVbaSheetObjectBase::setWidth( double fWidth )
{
    if( fWidth <= 0.0 )
        throw uno::RuntimeException();
    mxShape->setSize( awt::Size( PointsToHmm( fWidth ), mxShape->getSize().Height ) );
}

double SAL_CALL ScVbaSheetObjectBase::getHeight()
{
    return HmmToPoints( mxShape->getSize().Height );
}

void SAL_CALL ScVbaSheetObjectBase::setHeight( double fHeight )
{
    if( fHeight <= 0.0 )
        throw uno::RuntimeException();
    mxShape->setSize( awt::Size( mxShape->getSize().Width, PointsToHmm( fHeight ) ) );
}

OUString SAL_CALL ScVbaSheetObjectBase::getName()
{
    return mxShapeProps->getPropertyValue( u"Name"_ustr ).get< OUString >();
}

void SAL_CALL ScVbaSheetObjectBase::setName( const OUString& rName )
{
    mxShapeProps->setPropertyValue( u"Name"_ustr, uno::Any( rName ) );
}

sal_Int32 SAL_CALL ScVbaSheetObjectBase::getPlacement()
{
    sal_Int32 const nRet = excel::XlPlacement::xlMoveAndSize;
#if 0 // TODO: not working at the moment.
    SvxShape* pShape = SdrObject::getSdrObjectFromXShape( mxShape );
    if(pShape)
    {
        SdrObject* pObj = pShape->GetSdrObject();
        if (pObj)
        {
            ScAnchorType eType = ScDrawLayer::GetAnchor(pObj);
            if (eType == SCA_PAGE)
                nRet = excel::XlPlacement::xlFreeFloating;
        }
    }
#endif
    return nRet;
}

void SAL_CALL ScVbaSheetObjectBase::setPlacement( sal_Int32 /*nPlacement*/ )
{
#if 0 // TODO: not working at the moment.
    SvxShape* pShape = SdrObject::getSdrObjectFromXShape( mxShape );
    if(pShape)
    {
        SdrObject* pObj = pShape->GetSdrObject();
        if (pObj)
        {
            ScAnchorType eType = SCA_CELL;
            if ( nPlacement == excel::XlPlacement::xlFreeFloating )
                eType = SCA_PAGE;

            // xlMove is not supported, treated as SCA_CELL (xlMoveAndSize)

            ScDrawLayer::SetAnchor(pObj, eType);
        }
    }
#endif
}

sal_Bool SAL_CALL ScVbaSheetObjectBase::getPrintObject()
{
    // not supported
    return true;
}

void SAL_CALL ScVbaSheetObjectBase::setPrintObject( sal_Bool /*bPrintObject*/ )
{
    // not supported
}

// private

void ScVbaSheetObjectBase::setDefaultProperties( sal_Int32 nIndex )
{
    OUString aName = implGetBaseName() + OUStringChar(' ') + OUString::number( nIndex + 1 );
    setName( aName );
    implSetDefaultProperties();
}

void ScVbaSheetObjectBase::implSetDefaultProperties()
{
}

ScVbaControlObjectBase::ScVbaControlObjectBase(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< container::XIndexContainer >& rxFormIC,
        const uno::Reference< drawing::XControlShape >& rxControlShape ) :
    ScVbaControlObject_BASE( rxParent, rxContext, rxModel, uno::Reference< drawing::XShape >( rxControlShape, uno::UNO_QUERY_THROW ) ),
    mxFormIC( rxFormIC, uno::UNO_SET_THROW ),
    mxControlProps( rxControlShape->getControl(), uno::UNO_QUERY_THROW ),
    mbNotifyMacroEventRead(false)
{
}

// XSheetObject attributes

OUString SAL_CALL ScVbaControlObjectBase::getName()
{
    return mxControlProps->getPropertyValue( u"Name"_ustr ).get< OUString >();
}

void SAL_CALL ScVbaControlObjectBase::setName( const OUString& rName )
{
    mxControlProps->setPropertyValue( u"Name"_ustr, uno::Any( rName ) );
}

OUString SAL_CALL ScVbaControlObjectBase::getOnAction()
{
    uno::Reference< script::XEventAttacherManager > xEventMgr( mxFormIC, uno::UNO_QUERY_THROW );
    sal_Int32 nIndex = getModelIndexInForm();
    const uno::Sequence< script::ScriptEventDescriptor > aEvents = xEventMgr->getScriptEvents( nIndex );
    if( aEvents.hasElements() )
    {
        const script::ScriptEventDescriptor* pEvent = std::find_if(aEvents.begin(), aEvents.end(),
            [](const script::ScriptEventDescriptor& rEvent) {
                return (rEvent.ListenerType == gaListenerType)
                    && (rEvent.EventMethod == gaEventMethod)
                    && (rEvent.ScriptType == "Script");
            });
        if (pEvent != aEvents.end())
            return extractMacroName( pEvent->ScriptCode );
    }
    return OUString();
}

void ScVbaControlObjectBase::NotifyMacroEventRead()
{
    if (mbNotifyMacroEventRead)
        return;
    comphelper::DocumentInfo::notifyMacroEventRead(mxModel);
    mbNotifyMacroEventRead = true;
}

void SAL_CALL ScVbaControlObjectBase::setOnAction( const OUString& rMacroName )
{
    uno::Reference< script::XEventAttacherManager > xEventMgr( mxFormIC, uno::UNO_QUERY_THROW );
    sal_Int32 nIndex = getModelIndexInForm();

    // first, remove a registered event (try/catch just in case implementation throws)
    try { xEventMgr->revokeScriptEvent( nIndex, gaListenerType, gaEventMethod, OUString() ); } catch( uno::Exception& ) {}

    // if a macro name has been passed, try to attach it to the event
    if( rMacroName.isEmpty() )
        return;

    MacroResolvedInfo aResolvedMacro = resolveVBAMacro( getSfxObjShell( mxModel ), rMacroName );
    if( !aResolvedMacro.mbFound )
        throw uno::RuntimeException();
    script::ScriptEventDescriptor aDescriptor;
    aDescriptor.ListenerType = gaListenerType;
    aDescriptor.EventMethod = gaEventMethod;
    aDescriptor.ScriptType = "Script";
    aDescriptor.ScriptCode = makeMacroURL( aResolvedMacro.msResolvedMacro );
    NotifyMacroEventRead();
    xEventMgr->registerScriptEvent( nIndex, aDescriptor );
}

sal_Bool SAL_CALL ScVbaControlObjectBase::getPrintObject()
{
    return mxControlProps->getPropertyValue( u"Printable"_ustr ).get<bool>();
}

void SAL_CALL ScVbaControlObjectBase::setPrintObject( sal_Bool bPrintObject )
{
    mxControlProps->setPropertyValue( u"Printable"_ustr, uno::Any( bPrintObject ) );
}

// XControlObject attributes

sal_Bool SAL_CALL ScVbaControlObjectBase::getAutoSize()
{
    // not supported
    return false;
}

void SAL_CALL ScVbaControlObjectBase::setAutoSize( sal_Bool /*bAutoSize*/ )
{
    // not supported
}

// private

sal_Int32 ScVbaControlObjectBase::getModelIndexInForm() const
{
    for( sal_Int32 nIndex = 0, nCount = mxFormIC->getCount(); nIndex < nCount; ++nIndex )
    {
        uno::Reference< beans::XPropertySet > xProps( mxFormIC->getByIndex( nIndex ), uno::UNO_QUERY_THROW );
        if( mxControlProps.get() == xProps.get() )
            return nIndex;
    }
    throw uno::RuntimeException();
}

ScVbaButton::ScVbaButton(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< container::XIndexContainer >& rxFormIC,
        const uno::Reference< drawing::XControlShape >& rxControlShape ) :
    ScVbaButton_BASE( rxParent, rxContext, rxModel, rxFormIC, rxControlShape )
{
}

// XButton attributes

OUString SAL_CALL ScVbaButton::getCaption()
{
    return mxControlProps->getPropertyValue( u"Label"_ustr ).get< OUString >();
}

void SAL_CALL ScVbaButton::setCaption( const OUString& rCaption )
{
    mxControlProps->setPropertyValue( u"Label"_ustr, uno::Any( rCaption ) );
}

uno::Reference< excel::XFont > SAL_CALL ScVbaButton::getFont()
{
    return new ScVbaFont( this, mxContext, maPalette, mxControlProps, nullptr, true );
}

void SAL_CALL ScVbaButton::setFont( const uno::Reference< excel::XFont >& /*rxFont*/ )
{
    // TODO
}

sal_Int32 SAL_CALL ScVbaButton::getHorizontalAlignment()
{
    switch( mxControlProps->getPropertyValue( u"Align"_ustr ).get< sal_Int16 >() )
    {
        case awt::TextAlign::LEFT:      return excel::Constants::xlLeft;
        case awt::TextAlign::RIGHT:     return excel::Constants::xlRight;
        case awt::TextAlign::CENTER:    return excel::Constants::xlCenter;
    }
    return excel::Constants::xlCenter;
}

void SAL_CALL ScVbaButton::setHorizontalAlignment( sal_Int32 nAlign )
{
    sal_Int32 nAwtAlign = awt::TextAlign::CENTER;
    switch( nAlign )
    {
        case excel::Constants::xlLeft:      nAwtAlign = awt::TextAlign::LEFT;   break;
        case excel::Constants::xlRight:     nAwtAlign = awt::TextAlign::RIGHT;  break;
        case excel::Constants::xlCenter:    nAwtAlign = awt::TextAlign::CENTER; break;
    }
    // form controls expect short value
    mxControlProps->setPropertyValue( u"Align"_ustr, uno::Any( static_cast< sal_Int16 >( nAwtAlign ) ) );
}

sal_Int32 SAL_CALL ScVbaButton::getVerticalAlignment()
{
    switch( mxControlProps->getPropertyValue( u"VerticalAlign"_ustr ).get< style::VerticalAlignment >() )
    {
        case style::VerticalAlignment_TOP:      return excel::Constants::xlTop;
        case style::VerticalAlignment_BOTTOM:   return excel::Constants::xlBottom;
        case style::VerticalAlignment_MIDDLE:   return excel::Constants::xlCenter;
        default:;
    }
    return excel::Constants::xlCenter;
}

void SAL_CALL ScVbaButton::setVerticalAlignment( sal_Int32 nAlign )
{
    style::VerticalAlignment eAwtAlign = style::VerticalAlignment_MIDDLE;
    switch( nAlign )
    {
        case excel::Constants::xlTop:       eAwtAlign = style::VerticalAlignment_TOP;       break;
        case excel::Constants::xlBottom:    eAwtAlign = style::VerticalAlignment_BOTTOM;    break;
        case excel::Constants::xlCenter:    eAwtAlign = style::VerticalAlignment_MIDDLE;    break;
    }
    mxControlProps->setPropertyValue( u"VerticalAlign"_ustr, uno::Any( eAwtAlign ) );
}

sal_Int32 SAL_CALL ScVbaButton::getOrientation()
{
    // not supported
    return excel::XlOrientation::xlHorizontal;
}

void SAL_CALL ScVbaButton::setOrientation( sal_Int32 /*nOrientation*/ )
{
    // not supported
}

uno::Any SAL_CALL ScVbaButton::getValue()
{
    return mxControlProps->getPropertyValue( u"State"_ustr );
}

void SAL_CALL ScVbaButton::setValue( const uno::Any &nValue )
{
    return mxControlProps->setPropertyValue( u"State"_ustr, nValue );
}

OUString SAL_CALL ScVbaButton::getText()
{
    return mxControlProps->getPropertyValue( u"Label"_ustr ).get< OUString >();
}

void SAL_CALL ScVbaButton::setText( const OUString &aText )
{
    return mxControlProps->setPropertyValue( u"Label"_ustr, uno::Any( aText ) );
}

// XButton methods

uno::Reference< excel::XCharacters > SAL_CALL ScVbaButton::Characters( const uno::Any& rStart, const uno::Any& rLength )
{
    return new ScVbaButtonCharacters( this, mxContext, mxControlProps, maPalette, rStart, rLength );
}

// XHelperInterface

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaButton, u"ooo.vba.excel.Button"_ustr )

// private

OUString ScVbaButton::implGetBaseName() const
{
    return u"Button"_ustr;
}

void ScVbaButton::implSetDefaultProperties()
{
    setCaption( getName() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
