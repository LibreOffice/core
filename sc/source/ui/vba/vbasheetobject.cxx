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
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <ooo/vba/excel/Constants.hpp>
#include <ooo/vba/excel/XlOrientation.hpp>
#include <ooo/vba/excel/XlPlacement.hpp>
#include <rtl/ustrbuf.hxx>
#include <filter/msfilter/msvbahelper.hxx>
#include <svx/unoshape.hxx>
#include "vbafont.hxx"
#include "drwlayer.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

ScVbaButtonCharacters::ScVbaButtonCharacters(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< beans::XPropertySet >& rxPropSet,
        const ScVbaPalette& rPalette,
        const uno::Any& rStart,
        const uno::Any& rLength ) throw (uno::RuntimeException) :
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

OUString SAL_CALL ScVbaButtonCharacters::getCaption() throw (uno::RuntimeException, std::exception)
{
    // ignore invalid mnStart and/or mnLength members
    OUString aString = getFullString();
    sal_Int32 nStart = ::std::min( mnStart, aString.getLength() );
    sal_Int32 nLength = ::std::min( mnLength, aString.getLength() - nStart );
    return aString.copy( nStart, nLength );
}

void SAL_CALL ScVbaButtonCharacters::setCaption( const OUString& rCaption ) throw (uno::RuntimeException, std::exception)
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

sal_Int32 SAL_CALL ScVbaButtonCharacters::getCount() throw (uno::RuntimeException, std::exception)
{
    // always return the total length of the caption
    return getFullString().getLength();
}

OUString SAL_CALL ScVbaButtonCharacters::getText() throw (uno::RuntimeException, std::exception)
{
    // Text attribute same as Caption attribute?
    return getCaption();
}

void SAL_CALL ScVbaButtonCharacters::setText( const OUString& rText ) throw (uno::RuntimeException, std::exception)
{
    // Text attribute same as Caption attribute?
    setCaption( rText );
}

uno::Reference< excel::XFont > SAL_CALL ScVbaButtonCharacters::getFont() throw (uno::RuntimeException, std::exception)
{
    return new ScVbaFont( this, mxContext, maPalette, mxPropSet, nullptr, true );
}

void SAL_CALL ScVbaButtonCharacters::setFont( const uno::Reference< excel::XFont >& /*rxFont*/ ) throw (uno::RuntimeException, std::exception)
{
    // TODO
}

// XCharacters methods

void SAL_CALL ScVbaButtonCharacters::Insert( const OUString& rString ) throw (uno::RuntimeException, std::exception)
{
    /*  The Insert() operation is in fact "replace covered characters", at
        least for buttons... It seems there is no easy way to really insert a
        substring. This operation does not affect the mnLength parameter. */
    setCaption( rString );
}

void SAL_CALL ScVbaButtonCharacters::Delete() throw (uno::RuntimeException, std::exception)
{
    /*  The Delete() operation is nothing else than "replace with empty string".
        This does not affect the mnLength parameter, multiple calls of Delete()
        will remove characters as long as there are some more covered by this
        object. */
    setCaption( OUString() );
}

// XHelperInterface

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaButtonCharacters, "ooo.vba.excel.Characters" )

// private

OUString ScVbaButtonCharacters::getFullString() const throw (uno::RuntimeException)
{
    return mxPropSet->getPropertyValue( "Label" ).get< OUString >();
}

void ScVbaButtonCharacters::setFullString( const OUString& rString ) throw (uno::RuntimeException)
{
    mxPropSet->setPropertyValue( "Label", uno::Any( rString ) );
}

ScVbaSheetObjectBase::ScVbaSheetObjectBase(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< drawing::XShape >& rxShape ) throw (uno::RuntimeException) :
    ScVbaSheetObject_BASE( rxParent, rxContext ),
    maPalette( rxModel ),
    mxModel( rxModel, uno::UNO_SET_THROW ),
    mxShape( rxShape, uno::UNO_SET_THROW ),
    mxShapeProps( rxShape, uno::UNO_QUERY_THROW )
{
}

// XSheetObject attributes

double SAL_CALL ScVbaSheetObjectBase::getLeft() throw (uno::RuntimeException, std::exception)
{
    return HmmToPoints( mxShape->getPosition().X );
}

void SAL_CALL ScVbaSheetObjectBase::setLeft( double fLeft ) throw (uno::RuntimeException, std::exception)
{
    if( fLeft < 0.0 )
        throw uno::RuntimeException();
    mxShape->setPosition( awt::Point( PointsToHmm( fLeft ), mxShape->getPosition().Y ) );
}

double SAL_CALL ScVbaSheetObjectBase::getTop() throw (uno::RuntimeException, std::exception)
{
    return HmmToPoints( mxShape->getPosition().Y );
}

void SAL_CALL ScVbaSheetObjectBase::setTop( double fTop ) throw (uno::RuntimeException, std::exception)
{
    if( fTop < 0.0 )
        throw uno::RuntimeException();
    mxShape->setPosition( awt::Point( mxShape->getPosition().X, PointsToHmm( fTop ) ) );
}

double SAL_CALL ScVbaSheetObjectBase::getWidth() throw (uno::RuntimeException, std::exception)
{
    return HmmToPoints( mxShape->getSize().Width );
}

void SAL_CALL ScVbaSheetObjectBase::setWidth( double fWidth ) throw (uno::RuntimeException, std::exception)
{
    if( fWidth <= 0.0 )
        throw uno::RuntimeException();
    mxShape->setSize( awt::Size( PointsToHmm( fWidth ), mxShape->getSize().Height ) );
}

double SAL_CALL ScVbaSheetObjectBase::getHeight() throw (uno::RuntimeException, std::exception)
{
    return HmmToPoints( mxShape->getSize().Height );
}

void SAL_CALL ScVbaSheetObjectBase::setHeight( double fHeight ) throw (uno::RuntimeException, std::exception)
{
    if( fHeight <= 0.0 )
        throw uno::RuntimeException();
    mxShape->setSize( awt::Size( mxShape->getSize().Width, PointsToHmm( fHeight ) ) );
}

OUString SAL_CALL ScVbaSheetObjectBase::getName() throw (uno::RuntimeException, std::exception)
{
    return mxShapeProps->getPropertyValue( "Name" ).get< OUString >();
}

void SAL_CALL ScVbaSheetObjectBase::setName( const OUString& rName ) throw (uno::RuntimeException, std::exception)
{
    mxShapeProps->setPropertyValue( "Name", uno::Any( rName ) );
}

sal_Int32 SAL_CALL ScVbaSheetObjectBase::getPlacement() throw (uno::RuntimeException, std::exception)
{
    sal_Int32 nRet = excel::XlPlacement::xlMoveAndSize;
#if 0 // TODO: not working at the moment.
    SvxShape* pShape = SvxShape::getImplementation( mxShape );
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

void SAL_CALL ScVbaSheetObjectBase::setPlacement( sal_Int32 /*nPlacement*/ ) throw (uno::RuntimeException, std::exception)
{
#if 0 // TODO: not working at the moment.
    SvxShape* pShape = SvxShape::getImplementation( mxShape );
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

sal_Bool SAL_CALL ScVbaSheetObjectBase::getPrintObject() throw (uno::RuntimeException, std::exception)
{
    // not supported
    return true;
}

void SAL_CALL ScVbaSheetObjectBase::setPrintObject( sal_Bool /*bPrintObject*/ ) throw (uno::RuntimeException, std::exception)
{
    // not supported
}

// private

void ScVbaSheetObjectBase::setDefaultProperties( sal_Int32 nIndex ) throw (uno::RuntimeException)
{
    OUString aName = OUStringBuffer( implGetBaseName() ).append( ' ' ).append( nIndex + 1 ).makeStringAndClear();
    setName( aName );
    implSetDefaultProperties();
}

void ScVbaSheetObjectBase::implSetDefaultProperties() throw (uno::RuntimeException)
{
}

ScVbaControlObjectBase::ScVbaControlObjectBase(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< frame::XModel >& rxModel,
        const uno::Reference< container::XIndexContainer >& rxFormIC,
        const uno::Reference< drawing::XControlShape >& rxControlShape,
        ListenerType eListenerType ) throw (uno::RuntimeException) :
    ScVbaControlObject_BASE( rxParent, rxContext, rxModel, uno::Reference< drawing::XShape >( rxControlShape, uno::UNO_QUERY_THROW ) ),
    mxFormIC( rxFormIC, uno::UNO_SET_THROW ),
    mxControlProps( rxControlShape->getControl(), uno::UNO_QUERY_THROW )
{
    // set listener and event name to be used for OnAction attribute
    switch( eListenerType )
    {
        case LISTENER_ACTION:
            maListenerType = "XActionListener";
            maEventMethod = "actionPerformed";
        break;
        case LISTENER_MOUSE:
            maListenerType = "XMouseListener";
            maEventMethod = "mouseReleased";
        break;
        case LISTENER_TEXT:
            maListenerType = "XTextListener";
            maEventMethod = "textChanged";
        break;
        case LISTENER_VALUE:
            maListenerType = "XAdjustmentListener";
            maEventMethod = "adjustmentValueChanged";
        break;
        case LISTENER_CHANGE:
            maListenerType = "XChangeListener";
            maEventMethod = "changed";
        break;
        // no default, to let the compiler complain about missing case
    }
}

// XSheetObject attributes

OUString SAL_CALL ScVbaControlObjectBase::getName() throw (uno::RuntimeException, std::exception)
{
    return mxControlProps->getPropertyValue( "Name" ).get< OUString >();
}

void SAL_CALL ScVbaControlObjectBase::setName( const OUString& rName ) throw (uno::RuntimeException, std::exception)
{
    mxControlProps->setPropertyValue( "Name", uno::Any( rName ) );
}

OUString SAL_CALL ScVbaControlObjectBase::getOnAction() throw (uno::RuntimeException, std::exception)
{
    uno::Reference< script::XEventAttacherManager > xEventMgr( mxFormIC, uno::UNO_QUERY_THROW );
    sal_Int32 nIndex = getModelIndexInForm();
    uno::Sequence< script::ScriptEventDescriptor > aEvents = xEventMgr->getScriptEvents( nIndex );
    if( aEvents.hasElements() )
    {
        const script::ScriptEventDescriptor* pEvent = aEvents.getConstArray();
        const script::ScriptEventDescriptor* pEventEnd = pEvent + aEvents.getLength();
        const OUString aScriptType = "Script";
        for( ; pEvent < pEventEnd; ++pEvent )
            if( (pEvent->ListenerType == maListenerType) && (pEvent->EventMethod == maEventMethod) && (pEvent->ScriptType == aScriptType) )
                return extractMacroName( pEvent->ScriptCode );
    }
    return OUString();
}

void SAL_CALL ScVbaControlObjectBase::setOnAction( const OUString& rMacroName ) throw (uno::RuntimeException, std::exception)
{
    uno::Reference< script::XEventAttacherManager > xEventMgr( mxFormIC, uno::UNO_QUERY_THROW );
    sal_Int32 nIndex = getModelIndexInForm();

    // first, remove a registered event (try/catch just in case implementation throws)
    try { xEventMgr->revokeScriptEvent( nIndex, maListenerType, maEventMethod, OUString() ); } catch( uno::Exception& ) {}

    // if a macro name has been passed, try to attach it to the event
    if( !rMacroName.isEmpty() )
    {
        MacroResolvedInfo aResolvedMacro = resolveVBAMacro( getSfxObjShell( mxModel ), rMacroName );
        if( !aResolvedMacro.mbFound )
            throw uno::RuntimeException();
        script::ScriptEventDescriptor aDescriptor;
        aDescriptor.ListenerType = maListenerType;
        aDescriptor.EventMethod = maEventMethod;
        aDescriptor.ScriptType = "Script";
        aDescriptor.ScriptCode = makeMacroURL( aResolvedMacro.msResolvedMacro );
        xEventMgr->registerScriptEvent( nIndex, aDescriptor );
    }
}

sal_Bool SAL_CALL ScVbaControlObjectBase::getPrintObject() throw (uno::RuntimeException, std::exception)
{
    return mxControlProps->getPropertyValue( "Printable" ).get< sal_Bool >();
}

void SAL_CALL ScVbaControlObjectBase::setPrintObject( sal_Bool bPrintObject ) throw (uno::RuntimeException, std::exception)
{
    mxControlProps->setPropertyValue( "Printable", uno::Any( bPrintObject ) );
}

// XControlObject attributes

sal_Bool SAL_CALL ScVbaControlObjectBase::getAutoSize() throw (uno::RuntimeException, std::exception)
{
    // not supported
    return false;
}

void SAL_CALL ScVbaControlObjectBase::setAutoSize( sal_Bool /*bAutoSize*/ ) throw (uno::RuntimeException, std::exception)
{
    // not supported
}

// private

sal_Int32 ScVbaControlObjectBase::getModelIndexInForm() const throw (uno::RuntimeException)
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
        const uno::Reference< drawing::XControlShape >& rxControlShape ) throw (uno::RuntimeException) :
    ScVbaButton_BASE( rxParent, rxContext, rxModel, rxFormIC, rxControlShape, LISTENER_ACTION )
{
}

// XButton attributes

OUString SAL_CALL ScVbaButton::getCaption() throw (uno::RuntimeException, std::exception)
{
    return mxControlProps->getPropertyValue( "Label" ).get< OUString >();
}

void SAL_CALL ScVbaButton::setCaption( const OUString& rCaption ) throw (uno::RuntimeException, std::exception)
{
    mxControlProps->setPropertyValue( "Label", uno::Any( rCaption ) );
}

uno::Reference< excel::XFont > SAL_CALL ScVbaButton::getFont() throw (uno::RuntimeException, std::exception)
{
    return new ScVbaFont( this, mxContext, maPalette, mxControlProps, nullptr, true );
}

void SAL_CALL ScVbaButton::setFont( const uno::Reference< excel::XFont >& /*rxFont*/ ) throw (uno::RuntimeException, std::exception)
{
    // TODO
}

sal_Int32 SAL_CALL ScVbaButton::getHorizontalAlignment() throw (uno::RuntimeException, std::exception)
{
    switch( mxControlProps->getPropertyValue( "Align" ).get< sal_Int16 >() )
    {
        case awt::TextAlign::LEFT:      return excel::Constants::xlLeft;
        case awt::TextAlign::RIGHT:     return excel::Constants::xlRight;
        case awt::TextAlign::CENTER:    return excel::Constants::xlCenter;
    }
    return excel::Constants::xlCenter;
}

void SAL_CALL ScVbaButton::setHorizontalAlignment( sal_Int32 nAlign ) throw (uno::RuntimeException, std::exception)
{
    sal_Int32 nAwtAlign = awt::TextAlign::CENTER;
    switch( nAlign )
    {
        case excel::Constants::xlLeft:      nAwtAlign = awt::TextAlign::LEFT;   break;
        case excel::Constants::xlRight:     nAwtAlign = awt::TextAlign::RIGHT;  break;
        case excel::Constants::xlCenter:    nAwtAlign = awt::TextAlign::CENTER; break;
    }
    // form controls expect short value
    mxControlProps->setPropertyValue( "Align", uno::Any( static_cast< sal_Int16 >( nAwtAlign ) ) );
}

sal_Int32 SAL_CALL ScVbaButton::getVerticalAlignment() throw (uno::RuntimeException, std::exception)
{
    switch( mxControlProps->getPropertyValue( "VerticalAlign" ).get< style::VerticalAlignment >() )
    {
        case style::VerticalAlignment_TOP:      return excel::Constants::xlTop;
        case style::VerticalAlignment_BOTTOM:   return excel::Constants::xlBottom;
        case style::VerticalAlignment_MIDDLE:   return excel::Constants::xlCenter;
        default:;
    }
    return excel::Constants::xlCenter;
}

void SAL_CALL ScVbaButton::setVerticalAlignment( sal_Int32 nAlign ) throw (uno::RuntimeException, std::exception)
{
    style::VerticalAlignment eAwtAlign = style::VerticalAlignment_MIDDLE;
    switch( nAlign )
    {
        case excel::Constants::xlTop:       eAwtAlign = style::VerticalAlignment_TOP;       break;
        case excel::Constants::xlBottom:    eAwtAlign = style::VerticalAlignment_BOTTOM;    break;
        case excel::Constants::xlCenter:    eAwtAlign = style::VerticalAlignment_MIDDLE;    break;
    }
    mxControlProps->setPropertyValue( "VerticalAlign", uno::Any( eAwtAlign ) );
}

sal_Int32 SAL_CALL ScVbaButton::getOrientation() throw (uno::RuntimeException, std::exception)
{
    // not supported
    return excel::XlOrientation::xlHorizontal;
}

void SAL_CALL ScVbaButton::setOrientation( sal_Int32 /*nOrientation*/ ) throw (uno::RuntimeException, std::exception)
{
    // not supported
}

// XButton methods

uno::Reference< excel::XCharacters > SAL_CALL ScVbaButton::Characters( const uno::Any& rStart, const uno::Any& rLength ) throw (uno::RuntimeException, std::exception)
{
    return new ScVbaButtonCharacters( this, mxContext, mxControlProps, maPalette, rStart, rLength );
}

// XHelperInterface

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaButton, "ooo.vba.excel.Button" )

// private

OUString ScVbaButton::implGetBaseName() const
{
    return OUString( "Button" );
}

void ScVbaButton::implSetDefaultProperties() throw (uno::RuntimeException)
{
    setCaption( getName() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
