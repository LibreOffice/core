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
#include <comphelper/sequence.hxx>
#include <tools/debug.hxx>
#include <comphelper/property.hxx>
#include "Tools.hxx"
#include "FormatCondition.hxx"
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#include "ReportHelperImpl.hxx"

#define MIN_WIDTH   80
#define MIN_HEIGHT  20
// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;
uno::Sequence< ::rtl::OUString > lcl_getLineOptionals()
{
    ::rtl::OUString pProps[] = {
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
    return uno::Sequence< ::rtl::OUString >(pProps,sizeof(pProps)/sizeof(pProps[0]));
}
DBG_NAME(rpt_OFixedLine)
// -----------------------------------------------------------------------------
OFixedLine::OFixedLine(uno::Reference< uno::XComponentContext > const & _xContext)
:FixedLineBase(m_aMutex)
,FixedLinePropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),lcl_getLineOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_nOrientation(1)
,m_LineColor(0)
,m_LineTransparence(0)
,m_LineWidth(0)
{
    DBG_CTOR(rpt_OFixedLine,NULL);
    m_aProps.aComponent.m_sName  = RPT_RESSTRING(RID_STR_FIXEDLINE,m_aProps.aComponent.m_xContext->getServiceManager());
    m_aProps.aComponent.m_nWidth = MIN_WIDTH;
}
// -----------------------------------------------------------------------------
OFixedLine::OFixedLine(uno::Reference< uno::XComponentContext > const & _xContext
                       ,const uno::Reference< lang::XMultiServiceFactory>& _xFactory
                       ,uno::Reference< drawing::XShape >& _xShape
                       ,sal_Int32 _nOrientation)
:FixedLineBase(m_aMutex)
,FixedLinePropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),lcl_getLineOptionals())
,m_aProps(m_aMutex,static_cast< container::XContainer*>( this ),_xContext)
,m_nOrientation(_nOrientation)
,m_LineColor(0)
,m_LineTransparence(0)
,m_LineWidth(0)
{
    DBG_CTOR(rpt_OFixedLine,NULL);
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
// -----------------------------------------------------------------------------
OFixedLine::~OFixedLine()
{
    DBG_DTOR(rpt_OFixedLine,NULL);
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_REFCOUNT( OFixedLine, FixedLineBase )
// --------------------------------------------------------------------------------
uno::Any SAL_CALL OFixedLine::queryInterface( const uno::Type& _rType ) throw (uno::RuntimeException)
{
    uno::Any aReturn = FixedLineBase::queryInterface(_rType);
    if ( !aReturn.hasValue() )
        aReturn = FixedLinePropertySet::queryInterface(_rType);
    if ( !aReturn.hasValue() && OReportControlModel::isInterfaceForbidden(_rType) )
        return aReturn;

    return aReturn.hasValue() ? aReturn : (m_aProps.aComponent.m_xProxy.is() ? m_aProps.aComponent.m_xProxy->queryAggregation(_rType) : aReturn);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::dispose() throw(uno::RuntimeException)
{
    FixedLinePropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}
// -----------------------------------------------------------------------------
::rtl::OUString OFixedLine::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.OFixedLine"));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFixedLine::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > OFixedLine::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aServices(1);
    aServices.getArray()[0] = SERVICE_FIXEDLINE;

    return aServices;
}
//------------------------------------------------------------------------------
uno::Reference< uno::XInterface > OFixedLine::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OFixedLine(xContext));
}

//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OFixedLine::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OFixedLine::supportsService(const ::rtl::OUString& ServiceName) throw( uno::RuntimeException )
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
// -----------------------------------------------------------------------------
// XReportComponent
REPORTCOMPONENT_IMPL3(OFixedLine,m_aProps.aComponent)
REPORTCOMPONENT_NOMASTERDETAIL(OFixedLine)
// -----------------------------------------------------------------------------
::sal_Int16  SAL_CALL OFixedLine::getControlBorder( ) throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setControlBorder( ::sal_Int16 /*_border*/ ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
::sal_Int32 SAL_CALL OFixedLine::getControlBorderColor() throw (beans::UnknownPropertyException,uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setControlBorderColor( ::sal_Int32 /*_bordercolor*/ ) throw (beans::UnknownPropertyException,lang::IllegalArgumentException,uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OFixedLine::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    return FixedLinePropertySet::getPropertySetInfo();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    FixedLinePropertySet::setPropertyValue( aPropertyName, aValue );
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OFixedLine::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return FixedLinePropertySet::getPropertyValue( PropertyName);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FixedLinePropertySet::addPropertyChangeListener( aPropertyName, xListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FixedLinePropertySet::removePropertyChangeListener( aPropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FixedLinePropertySet::addVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FixedLinePropertySet::removeVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
// XReportControlModel
::rtl::OUString SAL_CALL OFixedLine::getDataField() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setDataField( const ::rtl::OUString& /*_datafield*/ ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
::sal_Int32 SAL_CALL OFixedLine::getControlBackground() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setControlBackground( ::sal_Int32 /*_backgroundcolor*/ ) throw (uno::RuntimeException,beans::UnknownPropertyException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OFixedLine::getControlBackgroundTransparent() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setControlBackgroundTransparent( ::sal_Bool /*_controlbackgroundtransparent*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OFixedLine::getPrintWhenGroupChange() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setPrintWhenGroupChange( ::sal_Bool /*_printwhengroupchange*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFixedLine::getConditionalPrintExpression() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setConditionalPrintExpression( const ::rtl::OUString& /*_conditionalprintexpression*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
// -----------------------------------------------------------------------------
// XCloneable
uno::Reference< util::XCloneable > SAL_CALL OFixedLine::createClone(  ) throw (uno::RuntimeException)
{
    uno::Reference< report::XReportComponent> xSource = this;
    uno::Reference< report::XFixedLine> xSet(cloneObject(xSource,m_aProps.aComponent.m_xFactory,SERVICE_FIXEDLINE),uno::UNO_QUERY_THROW);
    return xSet.get();
}
// -----------------------------------------------------------------------------

// XFixedLine
// -----------------------------------------------------------------------------
::sal_Int32 SAL_CALL OFixedLine::getOrientation() throw (beans::UnknownPropertyException,uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_nOrientation;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setOrientation( ::sal_Int32 _orientation ) throw (beans::UnknownPropertyException,uno::RuntimeException)
{
    set(PROPERTY_ORIENTATION,_orientation,m_nOrientation);
}
// -----------------------------------------------------------------------------
drawing::LineStyle SAL_CALL OFixedLine::getLineStyle() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineStyle;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setLineStyle( drawing::LineStyle _linestyle ) throw (uno::RuntimeException)
{
    set(PROPERTY_LINESTYLE,_linestyle,m_LineStyle);
}
// -----------------------------------------------------------------------------
drawing::LineDash SAL_CALL OFixedLine::getLineDash() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineDash;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setLineDash( const drawing::LineDash& _linedash ) throw (uno::RuntimeException)
{
    set(PROPERTY_LINEDASH,_linedash,m_LineDash);
}
// -----------------------------------------------------------------------------
::sal_Int32 SAL_CALL OFixedLine::getLineColor() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineColor;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setLineColor( ::sal_Int32 _linecolor ) throw (uno::RuntimeException)
{
    set(PROPERTY_LINECOLOR,_linecolor,m_LineColor);
}
// -----------------------------------------------------------------------------
::sal_Int16 SAL_CALL OFixedLine::getLineTransparence() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineTransparence;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setLineTransparence( ::sal_Int16 _linetransparence ) throw (uno::RuntimeException)
{
    set(PROPERTY_LINETRANSPARENCE,_linetransparence,m_LineTransparence);
}
// -----------------------------------------------------------------------------
::sal_Int32 SAL_CALL OFixedLine::getLineWidth() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_LineWidth;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setLineWidth( ::sal_Int32 _linewidth ) throw (uno::RuntimeException)
{
    set(PROPERTY_LINEWIDTH,_linewidth,m_LineWidth);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// XChild
uno::Reference< uno::XInterface > SAL_CALL OFixedLine::getParent(  ) throw (uno::RuntimeException)
{
    return OShapeHelper::getParent(this);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setParent( const uno::Reference< uno::XInterface >& Parent ) throw (lang::NoSupportException, uno::RuntimeException)
{
    OShapeHelper::setParent(Parent,this);
}
// -----------------------------------------------------------------------------
uno::Reference< report::XFormatCondition > SAL_CALL OFixedLine::createFormatCondition(  ) throw (uno::Exception, uno::RuntimeException)
{
    return new OFormatCondition(m_aProps.aComponent.m_xContext);
}
// -----------------------------------------------------------------------------
// XContainer
void SAL_CALL OFixedLine::addContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    m_aProps.addContainerListener(xListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    m_aProps.removeContainerListener(xListener);
}
// -----------------------------------------------------------------------------
// XElementAccess
uno::Type SAL_CALL OFixedLine::getElementType(  ) throw (uno::RuntimeException)
{
    return ::getCppuType(static_cast< uno::Reference<report::XFormatCondition>*>(NULL));
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OFixedLine::hasElements(  ) throw (uno::RuntimeException)
{
    return m_aProps.hasElements();
}
// -----------------------------------------------------------------------------
// XIndexContainer
void SAL_CALL OFixedLine::insertByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aProps.insertByIndex(Index,Element);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::removeByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aProps.removeByIndex(Index);
}
// -----------------------------------------------------------------------------
// XIndexReplace
void SAL_CALL OFixedLine::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aProps.replaceByIndex(Index,Element);
}
// -----------------------------------------------------------------------------
// XIndexAccess
::sal_Int32 SAL_CALL OFixedLine::getCount(  ) throw (uno::RuntimeException)
{
    return m_aProps.getCount();
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OFixedLine::getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    return m_aProps.getByIndex( Index );
}
// -----------------------------------------------------------------------------
// XShape
awt::Point SAL_CALL OFixedLine::getPosition(  ) throw (uno::RuntimeException)
{
    return OShapeHelper::getPosition(this);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setPosition( const awt::Point& aPosition ) throw (uno::RuntimeException)
{
    OShapeHelper::setPosition(aPosition,this);
}
// -----------------------------------------------------------------------------
awt::Size SAL_CALL OFixedLine::getSize(  ) throw (uno::RuntimeException)
{
    return OShapeHelper::getSize(this);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFixedLine::setSize( const awt::Size& aSize ) throw (beans::PropertyVetoException, uno::RuntimeException)
{
    if ( (aSize.Width < MIN_WIDTH && m_nOrientation == 1) || (aSize.Height < MIN_HEIGHT && m_nOrientation == 0) )
        throw beans::PropertyVetoException();
    OShapeHelper::setSize(aSize,this);
}
// -----------------------------------------------------------------------------
// XShapeDescriptor
::rtl::OUString SAL_CALL OFixedLine::getShapeType(  ) throw (uno::RuntimeException)
{
   return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ControlShape"));
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFixedLine::getHyperLinkURL() throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    throw beans::UnknownPropertyException();
}
void SAL_CALL OFixedLine::setHyperLinkURL(const ::rtl::OUString & /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    throw beans::UnknownPropertyException();
}
::rtl::OUString SAL_CALL OFixedLine::getHyperLinkTarget() throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    throw beans::UnknownPropertyException();
}
void SAL_CALL OFixedLine::setHyperLinkTarget(const ::rtl::OUString & /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    throw beans::UnknownPropertyException();
}
::rtl::OUString SAL_CALL OFixedLine::getHyperLinkName() throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    throw beans::UnknownPropertyException();
}
void SAL_CALL OFixedLine::setHyperLinkName(const ::rtl::OUString & /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    throw beans::UnknownPropertyException();
}

NO_REPORTCONTROLFORMAT_IMPL(OFixedLine)

::sal_Bool SAL_CALL OFixedLine::getPrintRepeatedValues() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}
void SAL_CALL OFixedLine::setPrintRepeatedValues( ::sal_Bool /*_printrepeatedvalues*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException();
}

// -----------------------------------------------------------------------------
// =============================================================================
} // namespace reportdesign
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
