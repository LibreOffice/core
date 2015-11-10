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
#include "Section.hxx"
#include <comphelper/enumhelper.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/report/XReportComponent.hpp>
#include <com/sun/star/report/ForceNewPage.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "corestrings.hrc"
#include "core_resource.hxx"
#include "core_resource.hrc"
#include <tools/debug.hxx>
#include "Tools.hxx"
#include "RptModel.hxx"
#include "RptPage.hxx"
#include "ReportDefinition.hxx"
#include "Shape.hxx"
#include <svx/unoshape.hxx>
#include <vcl/svapp.hxx>
#include "RptObject.hxx"
#include "ReportDrawPage.hxx"
#include <comphelper/property.hxx>

namespace reportdesign
{

    using namespace com::sun::star;
    using namespace comphelper;


uno::Sequence< OUString> lcl_getGroupAbsent()
{
    OUString pProps[] = {
                OUString(PROPERTY_CANGROW)
                ,OUString(PROPERTY_CANSHRINK)
        };

    return uno::Sequence< OUString >(pProps,sizeof(pProps)/sizeof(pProps[0]));
}


uno::Sequence< OUString> lcl_getAbsent(bool _bPageSection)
{
    if ( _bPageSection )
    {
        OUString pProps[] = {
                OUString(PROPERTY_FORCENEWPAGE)
                ,OUString(PROPERTY_NEWROWORCOL)
                ,OUString(PROPERTY_KEEPTOGETHER)
                ,OUString(PROPERTY_CANGROW)
                ,OUString(PROPERTY_CANSHRINK)
                ,OUString(PROPERTY_REPEATSECTION)
        };
        return uno::Sequence< OUString >(pProps,sizeof(pProps)/sizeof(pProps[0]));
    }

    OUString pProps[] = {
                OUString(PROPERTY_CANGROW)
                ,OUString(PROPERTY_CANSHRINK)
                ,OUString(PROPERTY_REPEATSECTION)
        };

    return uno::Sequence< OUString >(pProps,sizeof(pProps)/sizeof(pProps[0]));
}

uno::Reference<report::XSection> OSection::createOSection(
    const uno::Reference< report::XReportDefinition >& xParentDef,
    const uno::Reference< uno::XComponentContext >& context,
    bool const bPageSection)
{
    OSection *const pNew =
        new OSection(xParentDef, nullptr, context, lcl_getAbsent(bPageSection));
    pNew->init();
    return pNew;
}

uno::Reference<report::XSection> OSection::createOSection(
    const uno::Reference< report::XGroup >& xParentGroup,
    const uno::Reference< uno::XComponentContext >& context,
    bool const)
{
    OSection *const pNew =
        new OSection(nullptr, xParentGroup, context, lcl_getGroupAbsent());
    pNew->init();
    return pNew;
}


OSection::OSection(const uno::Reference< report::XReportDefinition >& xParentDef
                   ,const uno::Reference< report::XGroup >& xParentGroup
                   ,const uno::Reference< uno::XComponentContext >& context
                   ,uno::Sequence< OUString> const& rStrings)
:SectionBase(m_aMutex)
,SectionPropertySet(context,SectionPropertySet::IMPLEMENTS_PROPERTY_SET,rStrings)
,m_aContainerListeners(m_aMutex)
,m_xContext(context)
,m_xGroup(xParentGroup)
,m_xReportDefinition(xParentDef)
,m_nHeight(3000)
,m_nBackgroundColor(COL_TRANSPARENT)
,m_nForceNewPage(report::ForceNewPage::NONE)
,m_nNewRowOrCol(report::ForceNewPage::NONE)
,m_bKeepTogether(false)
,m_bRepeatSection(false)
,m_bVisible(true)
,m_bBacktransparent(true)
,m_bInRemoveNotify(false)
,m_bInInsertNotify(false)
{
}

// TODO: VirtualFunctionFinder: This is virtual function!

OSection::~OSection()
{
}

//IMPLEMENT_FORWARD_XINTERFACE2(OSection,SectionBase,SectionPropertySet)
IMPLEMENT_FORWARD_REFCOUNT( OSection, SectionBase )

uno::Any SAL_CALL OSection::queryInterface( const uno::Type& _rType ) throw (uno::RuntimeException, std::exception)
{
    uno::Any aReturn = SectionBase::queryInterface(_rType);
    if ( !aReturn.hasValue() )
        aReturn = SectionPropertySet::queryInterface(_rType);

    if ( !aReturn.hasValue() && OReportControlModel::isInterfaceForbidden(_rType) )
        return aReturn;

    return aReturn;
}


void SAL_CALL OSection::dispose() throw(uno::RuntimeException, std::exception)
{
    OSL_ENSURE(!rBHelper.bDisposed,"Already disposed!");
    SectionPropertySet::dispose();
    uno::Reference<lang::XComponent> const xPageComponent(m_xDrawPage,
            uno::UNO_QUERY);
    if (xPageComponent.is())
    {
        xPageComponent->dispose();
    }
    cppu::WeakComponentImplHelperBase::dispose();

}

// TODO: VirtualFunctionFinder: This is virtual function!

void SAL_CALL OSection::disposing()
{
    lang::EventObject aDisposeEvent( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aContainerListeners.disposeAndClear( aDisposeEvent );
    m_xContext.clear();
}

OUString SAL_CALL OSection::getImplementationName(  ) throw(uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.report.Section");
}

uno::Sequence< OUString> OSection::getSupportedServiceNames_Static() throw( uno::RuntimeException )
{
    uno::Sequence< OUString> aSupported(1);
    aSupported.getArray()[0] = SERVICE_SECTION;
    return aSupported;
}

uno::Sequence< OUString> SAL_CALL OSection::getSupportedServiceNames() throw(uno::RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

sal_Bool SAL_CALL OSection::supportsService( const OUString& _rServiceName ) throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

void OSection::init()
{
    SolarMutexGuard g; // lock while manipulating SdrModel
    uno::Reference< report::XReportDefinition> xReport = getReportDefinition();
    std::shared_ptr<rptui::OReportModel> pModel = OReportDefinition::getSdrModel(xReport);
    assert(pModel && "No model set at the report definition!");
    if ( pModel )
    {
        uno::Reference<report::XSection> const xSection(this);
        SdrPage & rSdrPage(*pModel->createNewPage(xSection));
        m_xDrawPage.set(rSdrPage.getUnoPage(), uno::UNO_QUERY_THROW);
        m_xDrawPage_ShapeGrouper.set(m_xDrawPage, uno::UNO_QUERY_THROW);
        // apparently we may also get OReportDrawPage which doesn't support this
        m_xDrawPage_FormSupplier.set(m_xDrawPage, uno::UNO_QUERY);
        m_xDrawPage_Tunnel.set(m_xDrawPage, uno::UNO_QUERY_THROW);
        // fdo#53872: now also exchange the XDrawPage in the SdrPage so that
        // rSdrPage.getUnoPage returns this
        rSdrPage.SetUnoPage(this);
        // createNewPage _should_ have stored away 2 uno::References to this,
        // so our ref count cannot be 1 here, so this isn't destroyed here
        assert(m_refCount > 1);
    }
}

// XSection

sal_Bool SAL_CALL OSection::getVisible() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_bVisible;
}

void SAL_CALL OSection::setVisible( sal_Bool _visible ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_VISIBLE,_visible,m_bVisible);
}

OUString SAL_CALL OSection::getName() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_sName;
}

void SAL_CALL OSection::setName( const OUString& _name ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_NAME,_name,m_sName);
}

::sal_uInt32 SAL_CALL OSection::getHeight() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_nHeight;
}

void SAL_CALL OSection::setHeight( ::sal_uInt32 _height ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_HEIGHT,_height,m_nHeight);
}

::sal_Int32 SAL_CALL OSection::getBackColor() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_bBacktransparent ? COL_TRANSPARENT : m_nBackgroundColor;
}

void SAL_CALL OSection::setBackColor( ::sal_Int32 _backgroundcolor ) throw (uno::RuntimeException, std::exception)
{
    bool bTransparent = _backgroundcolor == static_cast<sal_Int32>(COL_TRANSPARENT);
    setBackTransparent(bTransparent);
    if ( !bTransparent )
        set(PROPERTY_BACKCOLOR,_backgroundcolor,m_nBackgroundColor);
}

sal_Bool SAL_CALL OSection::getBackTransparent() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_bBacktransparent;
}

void SAL_CALL OSection::setBackTransparent( sal_Bool _backtransparent ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_BACKTRANSPARENT,_backtransparent,m_bBacktransparent);
    if ( _backtransparent )
        set(PROPERTY_BACKCOLOR,static_cast<sal_Int32>(COL_TRANSPARENT),m_nBackgroundColor);
}

OUString SAL_CALL OSection::getConditionalPrintExpression() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_sConditionalPrintExpression;
}

void SAL_CALL OSection::setConditionalPrintExpression( const OUString& _conditionalprintexpression ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_CONDITIONALPRINTEXPRESSION,_conditionalprintexpression,m_sConditionalPrintExpression);
}

void OSection::checkNotPageHeaderFooter()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    uno::Reference< report::XReportDefinition > xRet = m_xReportDefinition;
    if ( xRet.is() )
    {
        if ( xRet->getPageHeaderOn() && xRet->getPageHeader() == *this )
            throw beans::UnknownPropertyException();
        if ( xRet->getPageFooterOn() && xRet->getPageFooter() == *this )
            throw beans::UnknownPropertyException();
    }
}

::sal_Int16 SAL_CALL OSection::getForceNewPage() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    checkNotPageHeaderFooter();
    return m_nForceNewPage;
}

void SAL_CALL OSection::setForceNewPage( ::sal_Int16 _forcenewpage ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    if ( _forcenewpage < report::ForceNewPage::NONE || _forcenewpage > report::ForceNewPage::BEFORE_AFTER_SECTION )
        throwIllegallArgumentException("css::report::ForceNewPage"
                        ,*this
                        ,1
                        ,m_xContext);
    checkNotPageHeaderFooter();
    set(PROPERTY_FORCENEWPAGE,_forcenewpage,m_nForceNewPage);
}

::sal_Int16 SAL_CALL OSection::getNewRowOrCol() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkNotPageHeaderFooter();
    return m_nNewRowOrCol;
}

void SAL_CALL OSection::setNewRowOrCol( ::sal_Int16 _newroworcol ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    if ( _newroworcol < report::ForceNewPage::NONE || _newroworcol > report::ForceNewPage::BEFORE_AFTER_SECTION )
        throwIllegallArgumentException("css::report::ForceNewPage"
                        ,*this
                        ,1
                        ,m_xContext);
    checkNotPageHeaderFooter();

    set(PROPERTY_NEWROWORCOL,_newroworcol,m_nNewRowOrCol);
}

sal_Bool SAL_CALL OSection::getKeepTogether() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkNotPageHeaderFooter();
    return m_bKeepTogether;
}

void SAL_CALL OSection::setKeepTogether( sal_Bool _keeptogether ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        checkNotPageHeaderFooter();
    }

    set(PROPERTY_KEEPTOGETHER,_keeptogether,m_bKeepTogether);
}

sal_Bool SAL_CALL OSection::getCanGrow() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException(); ///TODO: unsupported at the moment
}

void SAL_CALL OSection::setCanGrow( sal_Bool /*_cangrow*/ ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException(); ///TODO: unsupported at the moment
}

sal_Bool SAL_CALL OSection::getCanShrink() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException(); ///TODO: unsupported at the moment
}

void SAL_CALL OSection::setCanShrink( sal_Bool /*_canshrink*/ ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException(); ///TODO: unsupported at the moment
}

sal_Bool SAL_CALL OSection::getRepeatSection() throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    uno::Reference< report::XGroup > xGroup = m_xGroup;
    if ( !xGroup.is() )
        throw beans::UnknownPropertyException();
    return m_bRepeatSection;
}

void SAL_CALL OSection::setRepeatSection( sal_Bool _repeatsection ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        uno::Reference< report::XGroup > xGroup = m_xGroup;
        if ( !xGroup.is() )
            throw beans::UnknownPropertyException();
    }
    set(PROPERTY_REPEATSECTION,_repeatsection,m_bRepeatSection);
}

uno::Reference< report::XGroup > SAL_CALL OSection::getGroup() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xGroup;
}

uno::Reference< report::XReportDefinition > SAL_CALL OSection::getReportDefinition() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    uno::Reference< report::XReportDefinition > xRet = m_xReportDefinition;
    uno::Reference< report::XGroup > xGroup = m_xGroup;
    if ( !xRet.is() && xGroup.is() )
    {
        uno::Reference< report::XGroups> xGroups(xGroup->getGroups());
        if ( xGroups.is() )
            xRet = xGroups->getReportDefinition();
    }

    return xRet;
}

// XChild
uno::Reference< uno::XInterface > SAL_CALL OSection::getParent(  ) throw (uno::RuntimeException, std::exception)
{
    uno::Reference< uno::XInterface > xRet;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        xRet = m_xReportDefinition;
        if ( !xRet.is() )
            xRet = m_xGroup;
    }
    return  xRet;
}

void SAL_CALL OSection::setParent( const uno::Reference< uno::XInterface >& /*Parent*/ ) throw (lang::NoSupportException, uno::RuntimeException, std::exception)
{
    throw lang::NoSupportException();
}

// XContainer
void SAL_CALL OSection::addContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException, std::exception)
{
    m_aContainerListeners.addInterface(xListener);
}

void SAL_CALL OSection::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException, std::exception)
{
    m_aContainerListeners.removeInterface(xListener);
}

// XElementAccess
uno::Type SAL_CALL OSection::getElementType(  ) throw (uno::RuntimeException, std::exception)
{
    return cppu::UnoType<report::XReportComponent>::get();
}

sal_Bool SAL_CALL OSection::hasElements(  ) throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xDrawPage.is() && m_xDrawPage->hasElements();
}

// XIndexAccess
::sal_Int32 SAL_CALL OSection::getCount(  ) throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xDrawPage.is() ? m_xDrawPage->getCount() : 0;
}

uno::Any SAL_CALL OSection::getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xDrawPage.is() ? m_xDrawPage->getByIndex(Index) : uno::Any();
}

// XEnumerationAccess
uno::Reference< container::XEnumeration > SAL_CALL OSection::createEnumeration(  ) throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return new ::comphelper::OEnumerationByIndex(static_cast<XSection*>(this));
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL OSection::getPropertySetInfo(  ) throw(uno::RuntimeException, std::exception)
{
    return SectionPropertySet::getPropertySetInfo();
}

void SAL_CALL OSection::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SectionPropertySet::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL OSection::getPropertyValue( const OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    return SectionPropertySet::getPropertyValue( PropertyName);
}

void SAL_CALL OSection::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SectionPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL OSection::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SectionPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL OSection::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SectionPropertySet::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL OSection::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SectionPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL OSection::add( const uno::Reference< drawing::XShape >& xShape ) throw (uno::RuntimeException, std::exception)
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        m_bInInsertNotify = true;
        OSL_ENSURE(m_xDrawPage.is(),"No DrawPage!");
        m_xDrawPage->add(xShape);
        m_bInInsertNotify = false;
    }
    notifyElementAdded(xShape);
}

void SAL_CALL OSection::remove( const uno::Reference< drawing::XShape >& xShape ) throw (uno::RuntimeException, std::exception)
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        m_bInRemoveNotify = true;
        OSL_ENSURE(m_xDrawPage.is(),"No DrawPage!");
        m_xDrawPage->remove(xShape);
        m_bInRemoveNotify = false;
    }
    notifyElementRemoved(xShape);
}

// XShapeGrouper
uno::Reference< drawing::XShapeGroup > SAL_CALL
OSection::group(uno::Reference< drawing::XShapes > const& xShapes)
    throw (uno::RuntimeException, std::exception)
{
    // no lock because m_xDrawPage_ShapeGrouper is const
    return (m_xDrawPage_ShapeGrouper.is())
        ? m_xDrawPage_ShapeGrouper->group(xShapes)
        : nullptr;
}
void SAL_CALL
OSection::ungroup(uno::Reference<drawing::XShapeGroup> const& xGroup)
    throw (uno::RuntimeException, std::exception)
{
    // no lock because m_xDrawPage_ShapeGrouper is const
    if (m_xDrawPage_ShapeGrouper.is()) {
        m_xDrawPage_ShapeGrouper->ungroup(xGroup);
    }
}

// XFormsSupplier
uno::Reference<container::XNameContainer> SAL_CALL OSection::getForms()
    throw (uno::RuntimeException, std::exception)
{
    // no lock because m_xDrawPage_FormSupplier is const
    return (m_xDrawPage_FormSupplier.is())
        ? m_xDrawPage_FormSupplier->getForms()
        : nullptr;
}
// XFormsSupplier2
sal_Bool SAL_CALL OSection::hasForms() throw (uno::RuntimeException, std::exception)
{
    // no lock because m_xDrawPage_FormSupplier is const
    return (m_xDrawPage_FormSupplier.is())
        && m_xDrawPage_FormSupplier->hasForms();
}


// css::lang::XUnoTunnel

sal_Int64 OSection::getSomething( const uno::Sequence< sal_Int8 > & rId ) throw (uno::RuntimeException, std::exception)
{
    if (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);
    return (m_xDrawPage_Tunnel.is()) ? m_xDrawPage_Tunnel->getSomething(rId) : 0;
}


OSection* OSection::getImplementation( const uno::Reference< uno::XInterface >& _rxComponent )
{
    OSection* pContent( nullptr );

    uno::Reference< lang::XUnoTunnel > xUnoTunnel( _rxComponent, uno::UNO_QUERY );
    if ( xUnoTunnel.is() )
        pContent = reinterpret_cast< OSection* >( xUnoTunnel->getSomething( getUnoTunnelImplementationId() ) );

    return pContent;
}

uno::Sequence< sal_Int8 > OSection::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = nullptr;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

void OSection::notifyElementAdded(const uno::Reference< drawing::XShape >& xShape )
{
    if ( !m_bInInsertNotify )
    {
        container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::Any(), uno::makeAny(xShape), uno::Any());
        m_aContainerListeners.notifyEach(&container::XContainerListener::elementInserted,aEvent);
    }
}

void OSection::notifyElementRemoved(const uno::Reference< drawing::XShape >& xShape)
{
    if ( !m_bInRemoveNotify )
    {
        // notify our container listeners
        container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::Any(), uno::makeAny(xShape), uno::Any());
        m_aContainerListeners.notifyEach(&container::XContainerListener::elementRemoved,aEvent);
    }
}

} // namespace reportdesign


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
