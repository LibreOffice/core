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
#include <ImageControl.hxx>
#include <strings.hxx>
#include <strings.hrc>
#include <core_resource.hxx>
#include <Tools.hxx>
#include <tools/color.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <FormatCondition.hxx>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <ReportHelperImpl.hxx>

namespace reportdesign
{

    using namespace com::sun::star;
static uno::Sequence< OUString > lcl_getImageOptionals()
{
    OUString pProps[] = {
            PROPERTY_CHARCOLOR
            ,PROPERTY_CHAREMPHASIS
            ,PROPERTY_CHARFONTCHARSET
            ,PROPERTY_CHARFONTFAMILY
            ,PROPERTY_CHARFONTNAME
            ,PROPERTY_CHARFONTPITCH
            ,PROPERTY_CHARFONTSTYLENAME
            ,PROPERTY_CHARHEIGHT
            ,PROPERTY_CHARPOSTURE
            ,PROPERTY_CHARRELIEF
            ,PROPERTY_FONTDESCRIPTOR
            ,PROPERTY_FONTDESCRIPTORASIAN
            ,PROPERTY_FONTDESCRIPTORCOMPLEX
            ,PROPERTY_CONTROLTEXTEMPHASISMARK
            ,PROPERTY_CHARROTATION
            ,PROPERTY_CHARSCALEWIDTH
            ,PROPERTY_CHARSTRIKEOUT
            ,PROPERTY_CHARUNDERLINECOLOR
            ,PROPERTY_CHARUNDERLINE
            ,PROPERTY_CHARWEIGHT
            ,PROPERTY_CHARWORDMODE
            ,PROPERTY_CHARFLASH
            ,PROPERTY_CHARAUTOKERNING
            ,PROPERTY_CHARESCAPEMENTHEIGHT
            ,PROPERTY_CHARLOCALE
            ,PROPERTY_CHARESCAPEMENT
            ,PROPERTY_CHARCASEMAP
            ,PROPERTY_CHARCOMBINEISON
            ,PROPERTY_CHARCOMBINEPREFIX
            ,PROPERTY_CHARCOMBINESUFFIX
            ,PROPERTY_CHARHIDDEN
            ,PROPERTY_CHARSHADOWED
            ,PROPERTY_CHARCONTOURED
            ,PROPERTY_VISITEDCHARSTYLENAME
            ,PROPERTY_UNVISITEDCHARSTYLENAME
            ,PROPERTY_CHARKERNING
            ,PROPERTY_MASTERFIELDS
            ,PROPERTY_DETAILFIELDS
            ,PROPERTY_PARAADJUST
            , PROPERTY_CHAREMPHASISASIAN
            , PROPERTY_CHARFONTNAMEASIAN
            , PROPERTY_CHARFONTSTYLENAMEASIAN
            , PROPERTY_CHARFONTFAMILYASIAN
            , PROPERTY_CHARFONTCHARSETASIAN
            , PROPERTY_CHARFONTPITCHASIAN
            , PROPERTY_CHARHEIGHTASIAN
            , PROPERTY_CHARUNDERLINEASIAN
            , PROPERTY_CHARWEIGHTASIAN
            , PROPERTY_CHARPOSTUREASIAN
            , PROPERTY_CHARWORDMODEASIAN
            , PROPERTY_CHARROTATIONASIAN
            , PROPERTY_CHARSCALEWIDTHASIAN
            , PROPERTY_CHARLOCALEASIAN
            , PROPERTY_CHAREMPHASISCOMPLEX
            , PROPERTY_CHARFONTNAMECOMPLEX
            , PROPERTY_CHARFONTSTYLENAMECOMPLEX
            , PROPERTY_CHARFONTFAMILYCOMPLEX
            , PROPERTY_CHARFONTCHARSETCOMPLEX
            , PROPERTY_CHARFONTPITCHCOMPLEX
            , PROPERTY_CHARHEIGHTCOMPLEX
            , PROPERTY_CHARUNDERLINECOMPLEX
            , PROPERTY_CHARWEIGHTCOMPLEX
            , PROPERTY_CHARPOSTURECOMPLEX
            , PROPERTY_CHARWORDMODECOMPLEX
            , PROPERTY_CHARROTATIONCOMPLEX
            , PROPERTY_CHARSCALEWIDTHCOMPLEX
            , PROPERTY_CHARLOCALECOMPLEX

    };
    return uno::Sequence< OUString >(pProps,SAL_N_ELEMENTS(pProps));
}


OImageControl::OImageControl(uno::Reference< uno::XComponentContext > const & _xContext)
:ImageControlBase(m_aMutex)
,ImageControlPropertySet(_xContext,IMPLEMENTS_PROPERTY_SET,lcl_getImageOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_nScaleMode(awt::ImageScaleMode::NONE)
,m_bPreserveIRI(true)
{
    m_aProps.aComponent.m_sName  = RptResId(RID_STR_IMAGECONTROL);
}

OImageControl::OImageControl(uno::Reference< uno::XComponentContext > const & _xContext
                           ,const uno::Reference< lang::XMultiServiceFactory>& _xFactory
                           ,uno::Reference< drawing::XShape >& _xShape)
:ImageControlBase(m_aMutex)
,ImageControlPropertySet(_xContext,IMPLEMENTS_PROPERTY_SET,lcl_getImageOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_nScaleMode(awt::ImageScaleMode::NONE)
,m_bPreserveIRI(true)
{
    m_aProps.aComponent.m_sName  = RptResId(RID_STR_IMAGECONTROL);
    m_aProps.aComponent.m_xFactory = _xFactory;
    osl_atomic_increment( &m_refCount );
    {
        m_aProps.aComponent.setShape(_xShape,this,m_refCount);
    }
    osl_atomic_decrement( &m_refCount );
}

OImageControl::~OImageControl()
{
}

//IMPLEMENT_FORWARD_XINTERFACE2(OImageControl,ImageControlBase,ImageControlPropertySet)
IMPLEMENT_FORWARD_REFCOUNT( OImageControl, ImageControlBase )

uno::Any SAL_CALL OImageControl::queryInterface( const uno::Type& _rType )
{
    uno::Any aReturn = ImageControlBase::queryInterface(_rType);
    if ( !aReturn.hasValue() )
        aReturn = ImageControlPropertySet::queryInterface(_rType);

    if ( !aReturn.hasValue() && OReportControlModel::isInterfaceForbidden(_rType) )
        return aReturn;

    return aReturn.hasValue() ? aReturn : (m_aProps.aComponent.m_xProxy.is() ? m_aProps.aComponent.m_xProxy->queryAggregation(_rType) : aReturn);
}


void SAL_CALL OImageControl::dispose()
{
    ImageControlPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}

OUString OImageControl::getImplementationName_Static(  )
{
    return u"com.sun.star.comp.report.OImageControl"_ustr;
}


OUString SAL_CALL OImageControl::getImplementationName(  )
{
    return getImplementationName_Static();
}

uno::Sequence< OUString > OImageControl::getSupportedServiceNames_Static(  )
{
    uno::Sequence< OUString > aServices { SERVICE_IMAGECONTROL };

    return aServices;
}

uno::Reference< uno::XInterface > OImageControl::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OImageControl(xContext));
}


uno::Sequence< OUString > SAL_CALL OImageControl::getSupportedServiceNames(  )
{
    return getSupportedServiceNames_Static();
}

sal_Bool SAL_CALL OImageControl::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XReportComponent
REPORTCOMPONENT_IMPL(OImageControl,m_aProps.aComponent)
REPORTCOMPONENT_IMPL2(OImageControl,m_aProps.aComponent)
REPORTCOMPONENT_NOMASTERDETAIL(OImageControl)
NO_REPORTCONTROLFORMAT_IMPL(OImageControl)
OUString SAL_CALL OImageControl::getHyperLinkURL()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.aFormatProperties.sHyperLinkURL;
}
void SAL_CALL OImageControl::setHyperLinkURL(const OUString & the_value)
{
    set(PROPERTY_HYPERLINKURL,the_value,m_aProps.aFormatProperties.sHyperLinkURL);
}
OUString SAL_CALL OImageControl::getHyperLinkTarget()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.aFormatProperties.sHyperLinkTarget;
}
void SAL_CALL OImageControl::setHyperLinkTarget(const OUString & the_value)
{
    set(PROPERTY_HYPERLINKTARGET,the_value,m_aProps.aFormatProperties.sHyperLinkTarget);
}
OUString SAL_CALL OImageControl::getHyperLinkName()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.aFormatProperties.sHyperLinkName;
}
void SAL_CALL OImageControl::setHyperLinkName(const OUString & the_value)
{
    set(PROPERTY_HYPERLINKNAME,the_value,m_aProps.aFormatProperties.sHyperLinkName);
}


::sal_Int32 SAL_CALL OImageControl::getControlBackground()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.aFormatProperties.m_bBackgroundTransparent ? static_cast<sal_Int32>(COL_TRANSPARENT) : m_aProps.aFormatProperties.nBackgroundColor;
}

void SAL_CALL OImageControl::setControlBackground( ::sal_Int32 _backgroundcolor )
{
    bool bTransparent = _backgroundcolor == static_cast<sal_Int32>(COL_TRANSPARENT);
    setControlBackgroundTransparent(bTransparent);
    if ( !bTransparent )
        set(PROPERTY_CONTROLBACKGROUND,_backgroundcolor,m_aProps.aFormatProperties.nBackgroundColor);
}

sal_Bool SAL_CALL OImageControl::getControlBackgroundTransparent()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.aFormatProperties.m_bBackgroundTransparent;
}

void SAL_CALL OImageControl::setControlBackgroundTransparent( sal_Bool _controlbackgroundtransparent )
{
    set(PROPERTY_CONTROLBACKGROUNDTRANSPARENT,_controlbackgroundtransparent,m_aProps.aFormatProperties.m_bBackgroundTransparent);
    if ( _controlbackgroundtransparent )
        set(PROPERTY_CONTROLBACKGROUND,static_cast<sal_Int32>(COL_TRANSPARENT),m_aProps.aFormatProperties.nBackgroundColor);
}


uno::Reference< beans::XPropertySetInfo > SAL_CALL OImageControl::getPropertySetInfo(  )
{
    return ImageControlPropertySet::getPropertySetInfo();
}

void SAL_CALL OImageControl::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    ImageControlPropertySet::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL OImageControl::getPropertyValue( const OUString& PropertyName )
{
    return ImageControlPropertySet::getPropertyValue( PropertyName);
}

void SAL_CALL OImageControl::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
{
    ImageControlPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL OImageControl::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener )
{
    ImageControlPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL OImageControl::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
{
    ImageControlPropertySet::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL OImageControl::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
{
    ImageControlPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}

// XReportControlModel
OUString SAL_CALL OImageControl::getDataField()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.aDataField;
}

void SAL_CALL OImageControl::setDataField( const OUString& _datafield )
{
    set(PROPERTY_DATAFIELD,_datafield,m_aProps.aDataField);
}


sal_Bool SAL_CALL OImageControl::getPrintWhenGroupChange()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.bPrintWhenGroupChange;
}

void SAL_CALL OImageControl::setPrintWhenGroupChange( sal_Bool _printwhengroupchange )
{
    set(PROPERTY_PRINTWHENGROUPCHANGE,_printwhengroupchange,m_aProps.bPrintWhenGroupChange);
}

OUString SAL_CALL OImageControl::getConditionalPrintExpression()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.aConditionalPrintExpression;
}

void SAL_CALL OImageControl::setConditionalPrintExpression( const OUString& _conditionalprintexpression )
{
    set(PROPERTY_CONDITIONALPRINTEXPRESSION,_conditionalprintexpression,m_aProps.aConditionalPrintExpression);
}


// XCloneable
uno::Reference< util::XCloneable > SAL_CALL OImageControl::createClone(  )
{
    uno::Reference< report::XReportComponent> xSource = this;
    uno::Reference< report::XImageControl> xSet(cloneObject(xSource,m_aProps.aComponent.m_xFactory,SERVICE_IMAGECONTROL),uno::UNO_QUERY_THROW);
    return xSet;
}


// XImageControl

OUString SAL_CALL OImageControl::getImageURL()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aImageURL;
}

void SAL_CALL OImageControl::setImageURL( const OUString& _imageurl )
{
    set(PROPERTY_IMAGEURL,_imageurl,m_aImageURL);
}

uno::Reference< awt::XImageProducer > SAL_CALL OImageControl::getImageProducer(  )
{
    return uno::Reference< awt::XImageProducer >();
}

// XChild
uno::Reference< uno::XInterface > SAL_CALL OImageControl::getParent(  )
{
    return OShapeHelper::getParent(this);
}

void SAL_CALL OImageControl::setParent( const uno::Reference< uno::XInterface >& Parent )
{
    OShapeHelper::setParent(Parent,this);
}
uno::Reference< report::XFormatCondition > SAL_CALL OImageControl::createFormatCondition(  )
{
    return new OFormatCondition(m_aProps.aComponent.m_xContext);
}

// XContainer
void SAL_CALL OImageControl::addContainerListener( const uno::Reference< container::XContainerListener >& xListener )
{
    m_aProps.addContainerListener(xListener);
}

void SAL_CALL OImageControl::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener )
{
    m_aProps.removeContainerListener(xListener);
}

// XElementAccess
uno::Type SAL_CALL OImageControl::getElementType(  )
{
    return cppu::UnoType<report::XFormatCondition>::get();
}

sal_Bool SAL_CALL OImageControl::hasElements(  )
{
    return m_aProps.hasElements();
}

// XIndexContainer
void SAL_CALL OImageControl::insertByIndex( ::sal_Int32 Index, const uno::Any& Element )
{
    m_aProps.insertByIndex(Index,Element);
}

void SAL_CALL OImageControl::removeByIndex( ::sal_Int32 Index )
{
    m_aProps.removeByIndex(Index);
}

// XIndexReplace
void SAL_CALL OImageControl::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element )
{
    m_aProps.replaceByIndex(Index,Element);
}

// XIndexAccess
::sal_Int32 SAL_CALL OImageControl::getCount(  )
{
    return m_aProps.getCount();
}

uno::Any SAL_CALL OImageControl::getByIndex( ::sal_Int32 Index )
{
    return m_aProps.getByIndex( Index );
}


// XShape
awt::Point SAL_CALL OImageControl::getPosition(  )
{
    return OShapeHelper::getPosition(this);
}

void SAL_CALL OImageControl::setPosition( const awt::Point& aPosition )
{
    OShapeHelper::setPosition(aPosition,this);
}

awt::Size SAL_CALL OImageControl::getSize(  )
{
    return OShapeHelper::getSize(this);
}

void SAL_CALL OImageControl::setSize( const awt::Size& aSize )
{
    OShapeHelper::setSize(aSize,this);
}


// XShapeDescriptor
OUString SAL_CALL OImageControl::getShapeType(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_aProps.aComponent.m_xShape.is() )
        return m_aProps.aComponent.m_xShape->getShapeType();
    return u"com.sun.star.drawing.ControlShape"_ustr;
}

::sal_Int16 SAL_CALL OImageControl::getScaleMode()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_nScaleMode;
}

void SAL_CALL OImageControl::setScaleMode( ::sal_Int16 _scalemode )
{
    if ( _scalemode < awt::ImageScaleMode::NONE ||_scalemode > awt::ImageScaleMode::ANISOTROPIC )
        throw lang::IllegalArgumentException();
    set(PROPERTY_SCALEMODE,_scalemode,m_nScaleMode);
}

sal_Bool SAL_CALL OImageControl::getPreserveIRI()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_bPreserveIRI;
}

void SAL_CALL OImageControl::setPreserveIRI( sal_Bool _preserveiri )
{
    set(PROPERTY_PRESERVEIRI,_preserveiri,m_bPreserveIRI);
}

} // namespace reportdesign


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
