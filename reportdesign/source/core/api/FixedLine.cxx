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
#include <FixedLine.hxx>
#include <strings.hxx>
#include <strings.hrc>
#include <core_resource.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <Tools.hxx>
#include <FormatCondition.hxx>
#include <ReportHelperImpl.hxx>

#define MIN_WIDTH   80
#define MIN_HEIGHT  20

namespace reportdesign
{

    using namespace com::sun::star;
static uno::Sequence< OUString > lcl_getLineOptionals()
{
    OUString pProps[] = {
             PROPERTY_DATAFIELD
            ,PROPERTY_DEFAULTCONTROL
            ,PROPERTY_CONTROLBORDER
            ,PROPERTY_CONTROLBORDERCOLOR
            ,PROPERTY_CHARCOLOR
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
            ,PROPERTY_CONTROLBACKGROUND
            ,PROPERTY_CONTROLBACKGROUNDTRANSPARENT
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
            ,PROPERTY_HYPERLINKURL
            ,PROPERTY_HYPERLINKTARGET
            ,PROPERTY_HYPERLINKNAME
            ,PROPERTY_VISITEDCHARSTYLENAME
            ,PROPERTY_UNVISITEDCHARSTYLENAME
            ,PROPERTY_CHARKERNING
            ,PROPERTY_PRINTREPEATEDVALUES
            ,PROPERTY_CONDITIONALPRINTEXPRESSION
            ,PROPERTY_PRINTWHENGROUPCHANGE
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

OFixedLine::OFixedLine(uno::Reference< uno::XComponentContext > const & _xContext)
:FixedLineBase(m_aMutex)
,FixedLinePropertySet(_xContext,IMPLEMENTS_PROPERTY_SET,lcl_getLineOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_LineStyle( drawing::LineStyle_NONE )
,m_nOrientation(1)
,m_LineColor(0)
,m_LineTransparence(0)
,m_LineWidth(0)
{
    m_aProps.aComponent.m_sName  = RptResId(RID_STR_FIXEDLINE);
    m_aProps.aComponent.m_nWidth = MIN_WIDTH;
}

OFixedLine::OFixedLine(uno::Reference< uno::XComponentContext > const & _xContext
                       ,const uno::Reference< lang::XMultiServiceFactory>& _xFactory
                       ,uno::Reference< drawing::XShape >& _xShape
                       ,sal_Int32 _nOrientation)
:FixedLineBase(m_aMutex)
,FixedLinePropertySet(_xContext,IMPLEMENTS_PROPERTY_SET,lcl_getLineOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_LineStyle( drawing::LineStyle_NONE )
,m_nOrientation(_nOrientation)
,m_LineColor(0)
,m_LineTransparence(0)
,m_LineWidth(0)
{
    m_aProps.aComponent.m_sName  = RptResId(RID_STR_FIXEDLINE);
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
        TOOLS_WARN_EXCEPTION( "reportdesign", "OFixedLine::OFixedLine");
    }
    osl_atomic_decrement( &m_refCount );
}

OFixedLine::~OFixedLine()
{
}

IMPLEMENT_FORWARD_REFCOUNT( OFixedLine, FixedLineBase )

uno::Any SAL_CALL OFixedLine::queryInterface( const uno::Type& _rType )
{
    uno::Any aReturn = FixedLineBase::queryInterface(_rType);
    if ( !aReturn.hasValue() )
        aReturn = FixedLinePropertySet::queryInterface(_rType);
    if ( !aReturn.hasValue() && OReportControlModel::isInterfaceForbidden(_rType) )
        return aReturn;

    return aReturn.hasValue() ? aReturn : (m_aProps.aComponent.m_xProxy.is() ? m_aProps.aComponent.m_xProxy->queryAggregation(_rType) : aReturn);
}

void SAL_CALL OFixedLine::dispose()
{
    FixedLinePropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}

OUString OFixedLine::getImplementationName_Static(  )
{
    return u"com.sun.star.comp.report.OFixedLine"_ustr;
}


OUString SAL_CALL OFixedLine::getImplementationName(  )
{
    return getImplementationName_Static();
}

uno::Sequence< OUString > OFixedLine::getSupportedServiceNames_Static(  )
{
    uno::Sequence< OUString > aServices { SERVICE_FIXEDLINE };

    return aServices;
}

uno::Reference< uno::XInterface > OFixedLine::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OFixedLine(xContext));
}


uno::Sequence< OUString > SAL_CALL OFixedLine::getSupportedServiceNames(  )
{
    return getSupportedServiceNames_Static();
}

sal_Bool SAL_CALL OFixedLine::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XReportComponent
REPORTCOMPONENT_IMPL3(OFixedLine,m_aProps.aComponent)
REPORTCOMPONENT_NOMASTERDETAIL(OFixedLine)

::sal_Int16  SAL_CALL OFixedLine::getControlBorder( )
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setControlBorder( ::sal_Int16 /*_border*/ )
{
    throw beans::UnknownPropertyException();
}

::sal_Int32 SAL_CALL OFixedLine::getControlBorderColor()
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setControlBorderColor( ::sal_Int32 /*_bordercolor*/ )
{
    throw beans::UnknownPropertyException();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL OFixedLine::getPropertySetInfo(  )
{
    return FixedLinePropertySet::getPropertySetInfo();
}

void SAL_CALL OFixedLine::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    FixedLinePropertySet::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL OFixedLine::getPropertyValue( const OUString& PropertyName )
{
    return FixedLinePropertySet::getPropertyValue( PropertyName);
}

void SAL_CALL OFixedLine::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
{
    FixedLinePropertySet::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL OFixedLine::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener )
{
    FixedLinePropertySet::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL OFixedLine::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
{
    FixedLinePropertySet::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL OFixedLine::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
{
    FixedLinePropertySet::removeVetoableChangeListener( PropertyName, aListener );
}

// XReportControlModel
OUString SAL_CALL OFixedLine::getDataField()
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setDataField( const OUString& /*_datafield*/ )
{
    throw beans::UnknownPropertyException();
}

::sal_Int32 SAL_CALL OFixedLine::getControlBackground()
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setControlBackground( ::sal_Int32 /*_backgroundcolor*/ )
{
    throw beans::UnknownPropertyException();
}

sal_Bool SAL_CALL OFixedLine::getControlBackgroundTransparent()
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setControlBackgroundTransparent( sal_Bool /*_controlbackgroundtransparent*/ )
{
    throw beans::UnknownPropertyException();
}

sal_Bool SAL_CALL OFixedLine::getPrintWhenGroupChange()
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setPrintWhenGroupChange( sal_Bool /*_printwhengroupchange*/ )
{
    throw beans::UnknownPropertyException();
}

OUString SAL_CALL OFixedLine::getConditionalPrintExpression()
{
    throw beans::UnknownPropertyException();
}

void SAL_CALL OFixedLine::setConditionalPrintExpression( const OUString& /*_conditionalprintexpression*/ )
{
    throw beans::UnknownPropertyException();
}

// XCloneable
uno::Reference< util::XCloneable > SAL_CALL OFixedLine::createClone(  )
{
    uno::Reference< report::XReportComponent> xSource = this;
    uno::Reference< report::XFixedLine> xSet(cloneObject(xSource,m_aProps.aComponent.m_xFactory,SERVICE_FIXEDLINE),uno::UNO_QUERY_THROW);
    return xSet;
}


// XFixedLine

::sal_Int32 SAL_CALL OFixedLine::getOrientation()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_nOrientation;
}

void SAL_CALL OFixedLine::setOrientation( ::sal_Int32 _orientation )
{
    set(PROPERTY_ORIENTATION,_orientation,m_nOrientation);
}

drawing::LineStyle SAL_CALL OFixedLine::getLineStyle()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineStyle;
}

void SAL_CALL OFixedLine::setLineStyle( drawing::LineStyle _linestyle )
{
    set(PROPERTY_LINESTYLE,_linestyle,m_LineStyle);
}

drawing::LineDash SAL_CALL OFixedLine::getLineDash()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineDash;
}

void SAL_CALL OFixedLine::setLineDash( const drawing::LineDash& _linedash )
{
    set(PROPERTY_LINEDASH,_linedash,m_LineDash);
}

::sal_Int32 SAL_CALL OFixedLine::getLineColor()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineColor;
}

void SAL_CALL OFixedLine::setLineColor( ::sal_Int32 _linecolor )
{
    set(PROPERTY_LINECOLOR,_linecolor,m_LineColor);
}

::sal_Int16 SAL_CALL OFixedLine::getLineTransparence()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineTransparence;
}

void SAL_CALL OFixedLine::setLineTransparence( ::sal_Int16 _linetransparence )
{
    set(PROPERTY_LINETRANSPARENCE,_linetransparence,m_LineTransparence);
}

::sal_Int32 SAL_CALL OFixedLine::getLineWidth()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineWidth;
}

void SAL_CALL OFixedLine::setLineWidth( ::sal_Int32 _linewidth )
{
    set(PROPERTY_LINEWIDTH,_linewidth,m_LineWidth);
}


// XChild
uno::Reference< uno::XInterface > SAL_CALL OFixedLine::getParent(  )
{
    return OShapeHelper::getParent(this);
}

void SAL_CALL OFixedLine::setParent( const uno::Reference< uno::XInterface >& Parent )
{
    OShapeHelper::setParent(Parent,this);
}

uno::Reference< report::XFormatCondition > SAL_CALL OFixedLine::createFormatCondition(  )
{
    return new OFormatCondition(m_aProps.aComponent.m_xContext);
}

// XContainer
void SAL_CALL OFixedLine::addContainerListener( const uno::Reference< container::XContainerListener >& xListener )
{
    m_aProps.addContainerListener(xListener);
}

void SAL_CALL OFixedLine::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener )
{
    m_aProps.removeContainerListener(xListener);
}

// XElementAccess
uno::Type SAL_CALL OFixedLine::getElementType(  )
{
    return cppu::UnoType<report::XFormatCondition>::get();
}

sal_Bool SAL_CALL OFixedLine::hasElements(  )
{
    return m_aProps.hasElements();
}

// XIndexContainer
void SAL_CALL OFixedLine::insertByIndex( ::sal_Int32 Index, const uno::Any& Element )
{
    m_aProps.insertByIndex(Index,Element);
}

void SAL_CALL OFixedLine::removeByIndex( ::sal_Int32 Index )
{
    m_aProps.removeByIndex(Index);
}

// XIndexReplace
void SAL_CALL OFixedLine::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element )
{
    m_aProps.replaceByIndex(Index,Element);
}

// XIndexAccess
::sal_Int32 SAL_CALL OFixedLine::getCount(  )
{
    return m_aProps.getCount();
}

uno::Any SAL_CALL OFixedLine::getByIndex( ::sal_Int32 Index )
{
    return m_aProps.getByIndex( Index );
}

// XShape
awt::Point SAL_CALL OFixedLine::getPosition(  )
{
    return OShapeHelper::getPosition(this);
}

void SAL_CALL OFixedLine::setPosition( const awt::Point& aPosition )
{
    OShapeHelper::setPosition(aPosition,this);
}

awt::Size SAL_CALL OFixedLine::getSize(  )
{
    return OShapeHelper::getSize(this);
}

void SAL_CALL OFixedLine::setSize( const awt::Size& aSize )
{
    if ( aSize.Width < MIN_WIDTH && m_nOrientation == 1 )
        throw beans::PropertyVetoException("Too small width for FixedLine; minimum is "  + OUString::number(MIN_WIDTH)  + "0 micrometer", getXWeak());
    else if ( aSize.Height < MIN_HEIGHT && m_nOrientation == 0 )
        throw beans::PropertyVetoException("Too small height for FixedLine; minimum is " + OUString::number(MIN_HEIGHT) + "0 micrometer", getXWeak());
    OShapeHelper::setSize(aSize,this);
}

// XShapeDescriptor
OUString SAL_CALL OFixedLine::getShapeType(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_aProps.aComponent.m_xShape.is() )
        return m_aProps.aComponent.m_xShape->getShapeType();
    return u"com.sun.star.drawing.ControlShape"_ustr;
}

OUString SAL_CALL OFixedLine::getHyperLinkURL()
{
    throw beans::UnknownPropertyException();
}
void SAL_CALL OFixedLine::setHyperLinkURL(const OUString & /*the_value*/)
{
    throw beans::UnknownPropertyException();
}
OUString SAL_CALL OFixedLine::getHyperLinkTarget()
{
    throw beans::UnknownPropertyException();
}
void SAL_CALL OFixedLine::setHyperLinkTarget(const OUString & /*the_value*/)
{
    throw beans::UnknownPropertyException();
}
OUString SAL_CALL OFixedLine::getHyperLinkName()
{
    throw beans::UnknownPropertyException();
}
void SAL_CALL OFixedLine::setHyperLinkName(const OUString & /*the_value*/)
{
    throw beans::UnknownPropertyException();
}

NO_REPORTCONTROLFORMAT_IMPL(OFixedLine)

sal_Bool SAL_CALL OFixedLine::getPrintRepeatedValues()
{
    throw beans::UnknownPropertyException();
}
void SAL_CALL OFixedLine::setPrintRepeatedValues( sal_Bool /*_printrepeatedvalues*/ )
{
    throw beans::UnknownPropertyException();
}


} // namespace reportdesign


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
