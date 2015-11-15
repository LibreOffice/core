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
#include "FixedLine.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include "corestrings.hrc"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include <tools/debug.hxx>
#include <comphelper/property.hxx>
#include <cppuhelper/supportsservice.hxx>
#include "Tools.hxx"
#include "FormatCondition.hxx"
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#include "ReportHelperImpl.hxx"

#define MIN_WIDTH   80
#define MIN_HEIGHT  20

namespace reportdesign
{

    using namespace com::sun::star;
    using namespace comphelper;
uno::Sequence< OUString > lcl_getLineOptionals()
{
    OUString pProps[] = {
             OUString(PROPERTY_DATAFIELD)
            ,OUString(PROPERTY_DEFAULTCONTROL)
            ,OUString(PROPERTY_CONTROLBORDER)
            ,OUString(PROPERTY_CONTROLBORDERCOLOR)
            ,OUString(PROPERTY_CHARCOLOR)
            ,OUString(PROPERTY_CHAREMPHASIS)
            ,OUString(PROPERTY_CHARFONTCHARSET)
            ,OUString(PROPERTY_CHARFONTFAMILY)
            ,OUString(PROPERTY_CHARFONTNAME)
            ,OUString(PROPERTY_CHARFONTPITCH)
            ,OUString(PROPERTY_CHARFONTSTYLENAME)
            ,OUString(PROPERTY_CHARHEIGHT)
            ,OUString(PROPERTY_CHARPOSTURE)
            ,OUString(PROPERTY_CHARRELIEF)
            ,OUString(PROPERTY_FONTDESCRIPTOR)
            ,OUString(PROPERTY_FONTDESCRIPTORASIAN)
            ,OUString(PROPERTY_FONTDESCRIPTORCOMPLEX)
            ,OUString(PROPERTY_CONTROLTEXTEMPHASISMARK)
            ,OUString(PROPERTY_CHARROTATION)
            ,OUString(PROPERTY_CHARSCALEWIDTH)
            ,OUString(PROPERTY_CHARSTRIKEOUT)
            ,OUString(PROPERTY_CHARUNDERLINECOLOR)
            ,OUString(PROPERTY_CHARUNDERLINE)
            ,OUString(PROPERTY_CHARWEIGHT)
            ,OUString(PROPERTY_CHARWORDMODE)
            ,OUString(PROPERTY_CONTROLBACKGROUND)
            ,OUString(PROPERTY_CONTROLBACKGROUNDTRANSPARENT)
            ,OUString(PROPERTY_CHARFLASH)
            ,OUString(PROPERTY_CHARAUTOKERNING)
            ,OUString(PROPERTY_CHARESCAPEMENTHEIGHT)
            ,OUString(PROPERTY_CHARLOCALE)
            ,OUString(PROPERTY_CHARESCAPEMENT)
            ,OUString(PROPERTY_CHARCASEMAP)
            ,OUString(PROPERTY_CHARCOMBINEISON)
            ,OUString(PROPERTY_CHARCOMBINEPREFIX)
            ,OUString(PROPERTY_CHARCOMBINESUFFIX)
            ,OUString(PROPERTY_CHARHIDDEN)
            ,OUString(PROPERTY_CHARSHADOWED)
            ,OUString(PROPERTY_CHARCONTOURED)
            ,OUString(PROPERTY_HYPERLINKURL)
            ,OUString(PROPERTY_HYPERLINKTARGET)
            ,OUString(PROPERTY_HYPERLINKNAME)
            ,OUString(PROPERTY_VISITEDCHARSTYLENAME)
            ,OUString(PROPERTY_UNVISITEDCHARSTYLENAME)
            ,OUString(PROPERTY_CHARKERNING)
            ,OUString(PROPERTY_PRINTREPEATEDVALUES)
            ,OUString(PROPERTY_CONDITIONALPRINTEXPRESSION)
            ,OUString(PROPERTY_PRINTWHENGROUPCHANGE)
            ,OUString(PROPERTY_MASTERFIELDS)
            ,OUString(PROPERTY_DETAILFIELDS)
            ,OUString(PROPERTY_PARAADJUST)

            , OUString(PROPERTY_CHAREMPHASISASIAN)
            , OUString(PROPERTY_CHARFONTNAMEASIAN)
            , OUString(PROPERTY_CHARFONTSTYLENAMEASIAN)
            , OUString(PROPERTY_CHARFONTFAMILYASIAN)
            , OUString(PROPERTY_CHARFONTCHARSETASIAN)
            , OUString(PROPERTY_CHARFONTPITCHASIAN)
            , OUString(PROPERTY_CHARHEIGHTASIAN)
            , OUString(PROPERTY_CHARUNDERLINEASIAN)
            , OUString(PROPERTY_CHARWEIGHTASIAN)
            , OUString(PROPERTY_CHARPOSTUREASIAN)
            , OUString(PROPERTY_CHARWORDMODEASIAN)
            , OUString(PROPERTY_CHARROTATIONASIAN)
            , OUString(PROPERTY_CHARSCALEWIDTHASIAN)
            , OUString(PROPERTY_CHARLOCALEASIAN)
            , OUString(PROPERTY_CHAREMPHASISCOMPLEX)
            , OUString(PROPERTY_CHARFONTNAMECOMPLEX)
            , OUString(PROPERTY_CHARFONTSTYLENAMECOMPLEX)
            , OUString(PROPERTY_CHARFONTFAMILYCOMPLEX)
            , OUString(PROPERTY_CHARFONTCHARSETCOMPLEX)
            , OUString(PROPERTY_CHARFONTPITCHCOMPLEX)
            , OUString(PROPERTY_CHARHEIGHTCOMPLEX)
            , OUString(PROPERTY_CHARUNDERLINECOMPLEX)
            , OUString(PROPERTY_CHARWEIGHTCOMPLEX)
            , OUString(PROPERTY_CHARPOSTURECOMPLEX)
            , OUString(PROPERTY_CHARWORDMODECOMPLEX)
            , OUString(PROPERTY_CHARROTATIONCOMPLEX)
            , OUString(PROPERTY_CHARSCALEWIDTHCOMPLEX)
            , OUString(PROPERTY_CHARLOCALECOMPLEX)


    };
    return uno::Sequence< OUString >(pProps,sizeof(pProps)/sizeof(pProps[0]));
}

OFixedLine::OFixedLine(uno::Reference< uno::XComponentContext > const & _xContext)
:FixedLineBase(m_aMutex)
,FixedLinePropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),lcl_getLineOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_LineStyle( drawing::LineStyle_NONE )
,m_nOrientation(1)
,m_LineColor(0)
,m_LineTransparence(0)
,m_LineWidth(0)
{
    m_aProps.aComponent.m_sName  = RPT_RESSTRING(RID_STR_FIXEDLINE,m_aProps.aComponent.m_xContext->getServiceManager());
    m_aProps.aComponent.m_nWidth = MIN_WIDTH;
}

OFixedLine::OFixedLine(uno::Reference< uno::XComponentContext > const & _xContext
                       ,const uno::Reference< lang::XMultiServiceFactory>& _xFactory
                       ,uno::Reference< drawing::XShape >& _xShape
                       ,sal_Int32 _nOrientation)
:FixedLineBase(m_aMutex)
,FixedLinePropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),lcl_getLineOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_LineStyle( drawing::LineStyle_NONE )
,m_nOrientation(_nOrientation)
,m_LineColor(0)
,m_LineTransparence(0)
,m_LineWidth(0)
{
    m_aProps.aComponent.m_sName  = RPT_RESSTRING(RID_STR_FIXEDLINE,m_aProps.aComponent.m_xContext->getServiceManager());
    m_aProps.aComponent.m_xFactory = _xFactory;
    osl_atomic_increment( &m_refCount );
    try
    {
        awt::Size aSize = _xShape->getSize();
        if ( m_nOrientation == 1 )
        {
            if ( aSize.Width < MIN_WIDTH )
            {
                aSize.Width = MIN_WIDTH;
                _xShape->setSize(aSize);
            }
        }
        else if ( MIN_HEIGHT > aSize.Height )
        {
            aSize.Height = MIN_HEIGHT;
            _xShape->setSize(aSize);
        }
        m_aProps.aComponent.setShape(_xShape,this,m_refCount);
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("OFixedLine::OFixedLine: Exception caught!");
    }
    osl_atomic_decrement( &m_refCount );
}

OFixedLine::~OFixedLine()
{
}

IMPLEMENT_FORWARD_REFCOUNT( OFixedLine, FixedLineBase )

uno::Any SAL_CALL OFixedLine::queryInterface( const uno::Type& _rType ) throw (uno::RuntimeException, std::exception)
{
    uno::Any aReturn = FixedLineBase::queryInterface(_rType);
    if ( !aReturn.hasValue() )
        aReturn = FixedLinePropertySet::queryInterface(_rType);
    if ( !aReturn.hasValue() && OReportControlModel::isInterfaceForbidden(_rType) )
        return aReturn;

    return aReturn.hasValue() ? aReturn : (m_aProps.aComponent.m_xProxy.is() ? m_aProps.aComponent.m_xProxy->queryAggregation(_rType) : aReturn);
}

void SAL_CALL OFixedLine::dispose() throw(uno::RuntimeException, std::exception)
{
    FixedLinePropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}

OUString OFixedLine::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return OUString("com.sun.star.comp.report.OFixedLine");
}


OUString SAL_CALL OFixedLine::getImplementationName(  ) throw(uno::RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

uno::Sequence< OUString > OFixedLine::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aServices { SERVICE_FIXEDLINE };

    return aServices;
}

uno::Reference< uno::XInterface > OFixedLine::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OFixedLine(xContext));
}


uno::Sequence< OUString > SAL_CALL OFixedLine::getSupportedServiceNames(  ) throw(uno::RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

sal_Bool SAL_CALL OFixedLine::supportsService(const OUString& ServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

// XReportComponent
REPORTCOMPONENT_IMPL3(OFixedLine,m_aProps.aComponent)
REPORTCOMPONENT_NOMASTERDETAIL(OFixedLine)

::sal_Int16  SAL_CALL OFixedLine::getControlBorder( ) throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setControlBorder( ::sal_Int16 /*_border*/ ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}

::sal_Int32 SAL_CALL OFixedLine::getControlBorderColor() throw (beans::UnknownPropertyException,uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setControlBorderColor( ::sal_Int32 /*_bordercolor*/ ) throw (beans::UnknownPropertyException,lang::IllegalArgumentException,uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL OFixedLine::getPropertySetInfo(  ) throw(uno::RuntimeException, std::exception)
{
    return FixedLinePropertySet::getPropertySetInfo();
}

void SAL_CALL OFixedLine::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FixedLinePropertySet::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL OFixedLine::getPropertyValue( const OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    return FixedLinePropertySet::getPropertyValue( PropertyName);
}

void SAL_CALL OFixedLine::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FixedLinePropertySet::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL OFixedLine::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FixedLinePropertySet::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL OFixedLine::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FixedLinePropertySet::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL OFixedLine::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FixedLinePropertySet::removeVetoableChangeListener( PropertyName, aListener );
}

// XReportControlModel
OUString SAL_CALL OFixedLine::getDataField() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setDataField( const OUString& /*_datafield*/ ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}

::sal_Int32 SAL_CALL OFixedLine::getControlBackground() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setControlBackground( ::sal_Int32 /*_backgroundcolor*/ ) throw (uno::RuntimeException,beans::UnknownPropertyException, std::exception)
{
    throw beans::UnknownPropertyException();
}

sal_Bool SAL_CALL OFixedLine::getControlBackgroundTransparent() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setControlBackgroundTransparent( sal_Bool /*_controlbackgroundtransparent*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}

sal_Bool SAL_CALL OFixedLine::getPrintWhenGroupChange() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setPrintWhenGroupChange( sal_Bool /*_printwhengroupchange*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}

OUString SAL_CALL OFixedLine::getConditionalPrintExpression() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setConditionalPrintExpression( const OUString& /*_conditionalprintexpression*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}

// XCloneable
uno::Reference< util::XCloneable > SAL_CALL OFixedLine::createClone(  ) throw (uno::RuntimeException, std::exception)
{
    uno::Reference< report::XReportComponent> xSource = this;
    uno::Reference< report::XFixedLine> xSet(cloneObject(xSource,m_aProps.aComponent.m_xFactory,SERVICE_FIXEDLINE),uno::UNO_QUERY_THROW);
    return xSet.get();
}


// XFixedLine

::sal_Int32 SAL_CALL OFixedLine::getOrientation() throw (beans::UnknownPropertyException,uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_nOrientation;
}

void SAL_CALL OFixedLine::setOrientation( ::sal_Int32 _orientation ) throw (beans::UnknownPropertyException,uno::RuntimeException, std::exception)
{
    set(PROPERTY_ORIENTATION,_orientation,m_nOrientation);
}

drawing::LineStyle SAL_CALL OFixedLine::getLineStyle() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineStyle;
}

void SAL_CALL OFixedLine::setLineStyle( drawing::LineStyle _linestyle ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_LINESTYLE,_linestyle,m_LineStyle);
}

drawing::LineDash SAL_CALL OFixedLine::getLineDash() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineDash;
}

void SAL_CALL OFixedLine::setLineDash( const drawing::LineDash& _linedash ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_LINEDASH,_linedash,m_LineDash);
}

::sal_Int32 SAL_CALL OFixedLine::getLineColor() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineColor;
}

void SAL_CALL OFixedLine::setLineColor( ::sal_Int32 _linecolor ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_LINECOLOR,_linecolor,m_LineColor);
}

::sal_Int16 SAL_CALL OFixedLine::getLineTransparence() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineTransparence;
}

void SAL_CALL OFixedLine::setLineTransparence( ::sal_Int16 _linetransparence ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_LINETRANSPARENCE,_linetransparence,m_LineTransparence);
}

::sal_Int32 SAL_CALL OFixedLine::getLineWidth() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineWidth;
}

void SAL_CALL OFixedLine::setLineWidth( ::sal_Int32 _linewidth ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_LINEWIDTH,_linewidth,m_LineWidth);
}


// XChild
uno::Reference< uno::XInterface > SAL_CALL OFixedLine::getParent(  ) throw (uno::RuntimeException, std::exception)
{
    return OShapeHelper::getParent(this);
}

void SAL_CALL OFixedLine::setParent( const uno::Reference< uno::XInterface >& Parent ) throw (lang::NoSupportException, uno::RuntimeException, std::exception)
{
    OShapeHelper::setParent(Parent,this);
}

uno::Reference< report::XFormatCondition > SAL_CALL OFixedLine::createFormatCondition(  ) throw (uno::Exception, uno::RuntimeException, std::exception)
{
    return new OFormatCondition(m_aProps.aComponent.m_xContext);
}

// XContainer
void SAL_CALL OFixedLine::addContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException, std::exception)
{
    m_aProps.addContainerListener(xListener);
}

void SAL_CALL OFixedLine::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException, std::exception)
{
    m_aProps.removeContainerListener(xListener);
}

// XElementAccess
uno::Type SAL_CALL OFixedLine::getElementType(  ) throw (uno::RuntimeException, std::exception)
{
    return cppu::UnoType<report::XFormatCondition>::get();
}

sal_Bool SAL_CALL OFixedLine::hasElements(  ) throw (uno::RuntimeException, std::exception)
{
    return m_aProps.hasElements();
}

// XIndexContainer
void SAL_CALL OFixedLine::insertByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    m_aProps.insertByIndex(Index,Element);
}

void SAL_CALL OFixedLine::removeByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    m_aProps.removeByIndex(Index);
}

// XIndexReplace
void SAL_CALL OFixedLine::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    m_aProps.replaceByIndex(Index,Element);
}

// XIndexAccess
::sal_Int32 SAL_CALL OFixedLine::getCount(  ) throw (uno::RuntimeException, std::exception)
{
    return m_aProps.getCount();
}

uno::Any SAL_CALL OFixedLine::getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    return m_aProps.getByIndex( Index );
}

// XShape
awt::Point SAL_CALL OFixedLine::getPosition(  ) throw (uno::RuntimeException, std::exception)
{
    return OShapeHelper::getPosition(this);
}

void SAL_CALL OFixedLine::setPosition( const awt::Point& aPosition ) throw (uno::RuntimeException, std::exception)
{
    OShapeHelper::setPosition(aPosition,this);
}

awt::Size SAL_CALL OFixedLine::getSize(  ) throw (uno::RuntimeException, std::exception)
{
    return OShapeHelper::getSize(this);
}

void SAL_CALL OFixedLine::setSize( const awt::Size& aSize ) throw (beans::PropertyVetoException, uno::RuntimeException, std::exception)
{
    const char hundredthmmC[] = "0\xe2\x80\x89\xC2\xB5""m"; // in UTF-8: 0, thin space, µ (micro), m (meter)
    const OUString hundredthmm(hundredthmmC, sizeof(hundredthmmC)-1, RTL_TEXTENCODING_UTF8);
    if ( aSize.Width < MIN_WIDTH && m_nOrientation == 1 )
        throw beans::PropertyVetoException("Too small width for FixedLine; minimum is "  + OUString::number(MIN_WIDTH)  + hundredthmm, static_cast<cppu::OWeakObject*>(this));
    else if ( aSize.Height < MIN_HEIGHT && m_nOrientation == 0 )
        throw beans::PropertyVetoException("Too small height for FixedLine; minimum is " + OUString::number(MIN_HEIGHT) + hundredthmm, static_cast<cppu::OWeakObject*>(this));
    OShapeHelper::setSize(aSize,this);
}

// XShapeDescriptor
OUString SAL_CALL OFixedLine::getShapeType(  ) throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_aProps.aComponent.m_xShape.is() )
        return m_aProps.aComponent.m_xShape->getShapeType();
    return OUString("com.sun.star.drawing.ControlShape");
}

OUString SAL_CALL OFixedLine::getHyperLinkURL() throw (uno::RuntimeException, beans::UnknownPropertyException, std::exception)
{
    throw beans::UnknownPropertyException();
}
void SAL_CALL OFixedLine::setHyperLinkURL(const OUString & /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException, std::exception)
{
    throw beans::UnknownPropertyException();
}
OUString SAL_CALL OFixedLine::getHyperLinkTarget() throw (uno::RuntimeException, beans::UnknownPropertyException, std::exception)
{
    throw beans::UnknownPropertyException();
}
void SAL_CALL OFixedLine::setHyperLinkTarget(const OUString & /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException, std::exception)
{
    throw beans::UnknownPropertyException();
}
OUString SAL_CALL OFixedLine::getHyperLinkName() throw (uno::RuntimeException, beans::UnknownPropertyException, std::exception)
{
    throw beans::UnknownPropertyException();
}
void SAL_CALL OFixedLine::setHyperLinkName(const OUString & /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException, std::exception)
{
    throw beans::UnknownPropertyException();
}

NO_REPORTCONTROLFORMAT_IMPL(OFixedLine)

sal_Bool SAL_CALL OFixedLine::getPrintRepeatedValues() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}
void SAL_CALL OFixedLine::setPrintRepeatedValues( sal_Bool /*_printrepeatedvalues*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException();
}



} // namespace reportdesign


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
