/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "Section.hxx"
#include <comphelper/enumhelper.hxx>
#include <connectivity/dbtools.hxx>
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
#include "RptObject.hxx"
#include "ReportDrawPage.hxx"
#include <comphelper/property.hxx>
// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;
DBG_NAME( rpt_OSection )
// -----------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString> lcl_getGroupAbsent()
{
    ::rtl::OUString pProps[] = {
                PROPERTY_CANGROW
                ,PROPERTY_CANSHRINK
        };

    return uno::Sequence< ::rtl::OUString >(pProps,sizeof(pProps)/sizeof(pProps[0]));
}

// -----------------------------------------------------------------------------
OSection::OSection( const uno::Reference< report::XGroup >& _xParent
                   ,const uno::Reference< uno::XComponentContext >& context
                   ,bool /*_bPageSection*/)
:SectionBase(m_aMutex)
,SectionPropertySet(context,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),lcl_getGroupAbsent())
,m_aContainerListeners(m_aMutex)
,m_xContext(context)
,m_xGroup(_xParent)
,m_nHeight(3000)
,m_nBackgroundColor(COL_TRANSPARENT)
,m_nForceNewPage(report::ForceNewPage::NONE)
,m_nNewRowOrCol(report::ForceNewPage::NONE)
,m_bKeepTogether(sal_False)
,m_bCanGrow(sal_False)
,m_bCanShrink(sal_False)
,m_bRepeatSection(sal_False)
,m_bVisible(sal_True)
,m_bBacktransparent(sal_True)
,m_bInRemoveNotify(false)
,m_bInInsertNotify(false)
{
    DBG_CTOR( rpt_OSection,NULL);
    init();
}
// -----------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString> lcl_getAbsent(bool _bPageSection)
{
    if ( _bPageSection )
    {
        ::rtl::OUString pProps[] = {
                PROPERTY_FORCENEWPAGE
                ,PROPERTY_NEWROWORCOL
                ,PROPERTY_KEEPTOGETHER
                ,PROPERTY_CANGROW
                ,PROPERTY_CANSHRINK
                ,PROPERTY_REPEATSECTION
        };
        return uno::Sequence< ::rtl::OUString >(pProps,sizeof(pProps)/sizeof(pProps[0]));
    }

    ::rtl::OUString pProps[] = {
                PROPERTY_CANGROW
                ,PROPERTY_CANSHRINK
                ,PROPERTY_REPEATSECTION
        };

    return uno::Sequence< ::rtl::OUString >(pProps,sizeof(pProps)/sizeof(pProps[0]));
}
// -----------------------------------------------------------------------------
OSection::OSection(const uno::Reference< report::XReportDefinition >& _xParent
                   ,const uno::Reference< uno::XComponentContext >& context
                   ,bool _bPageSection)
:SectionBase(m_aMutex)
,SectionPropertySet(context,SectionPropertySet::IMPLEMENTS_PROPERTY_SET,lcl_getAbsent(_bPageSection))
,m_aContainerListeners(m_aMutex)
,m_xContext(context)
,m_xReportDefinition(_xParent)
,m_nHeight(3000)
,m_nBackgroundColor(COL_TRANSPARENT)
,m_nForceNewPage(report::ForceNewPage::NONE)
,m_nNewRowOrCol(report::ForceNewPage::NONE)
,m_bKeepTogether(sal_False)
,m_bCanGrow(sal_False)
,m_bCanShrink(sal_False)
,m_bRepeatSection(sal_False)
,m_bVisible(sal_True)
,m_bBacktransparent(sal_True)
,m_bInRemoveNotify(false)
,m_bInInsertNotify(false)
{
    DBG_CTOR( rpt_OSection,NULL);
    init();
}
//--------------------------------------------------------------------------
// TODO: VirtualFunctionFinder: This is virtual function!
//
OSection::~OSection()
{
    DBG_DTOR( rpt_OSection,NULL);
}
//--------------------------------------------------------------------------
//IMPLEMENT_FORWARD_XINTERFACE2(OSection,SectionBase,SectionPropertySet)
IMPLEMENT_FORWARD_REFCOUNT( OSection, SectionBase )
// --------------------------------------------------------------------------------
uno::Any SAL_CALL OSection::queryInterface( const uno::Type& _rType ) throw (uno::RuntimeException)
{
    uno::Any aReturn = SectionBase::queryInterface(_rType);
    if ( !aReturn.hasValue() )
        aReturn = SectionPropertySet::queryInterface(_rType);

    if ( !aReturn.hasValue() && OReportControlModel::isInterfaceForbidden(_rType) )
        return aReturn;

    return aReturn;
}

// -----------------------------------------------------------------------------
void SAL_CALL OSection::dispose() throw(uno::RuntimeException)
{
    OSL_ENSURE(!rBHelper.bDisposed,"Already disposed!");
    SectionPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();

}
// -----------------------------------------------------------------------------
// TODO: VirtualFunctionFinder: This is virtual function!
//
void SAL_CALL OSection::disposing()
{
    lang::EventObject aDisposeEvent( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aContainerListeners.disposeAndClear( aDisposeEvent );
    m_xContext.clear();
}
//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSection::getImplementationName(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.Section"));
}
//------------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString> OSection::getSupportedServiceNames_Static(void) throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString> aSupported(1);
    aSupported.getArray()[0] = SERVICE_SECTION;
    return aSupported;
}
//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString> SAL_CALL OSection::getSupportedServiceNames() throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OSection::supportsService( const ::rtl::OUString& _rServiceName ) throw(uno::RuntimeException)
{
    return ::comphelper::existsValue(_rServiceName,getSupportedServiceNames_Static());
}
// -----------------------------------------------------------------------------
void OSection::init()
{
    uno::Reference< report::XReportDefinition> xReport = getReportDefinition();
    ::boost::shared_ptr<rptui::OReportModel> pModel = OReportDefinition::getSdrModel(xReport);
    assert(pModel); //"No model set at the report definition!"
    if ( pModel )
    {
        uno::Reference<report::XSection> const xSection(this);
        m_xDrawPage.set(pModel->createNewPage(xSection)->getUnoPage(),
                uno::UNO_QUERY_THROW);
        m_xDrawPage_ShapeGrouper.set(m_xDrawPage, uno::UNO_QUERY_THROW);
        // apparently we may also get OReportDrawPage which doesn't support this
        m_xDrawPage_FormSupplier.set(m_xDrawPage, uno::UNO_QUERY);
        // createNewPage _should_ have stored away 2 uno::References to this,
        // so our ref count cannot be 1 here, so this isn't destroyed here
        assert(m_refCount > 1);
    }
}
// -----------------------------------------------------------------------------
// XSection
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OSection::getVisible() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_bVisible;
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::setVisible( ::sal_Bool _visible ) throw (uno::RuntimeException)
{
    set(PROPERTY_VISIBLE,_visible,m_bVisible);
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSection::getName() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_sName;
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::setName( const ::rtl::OUString& _name ) throw (uno::RuntimeException)
{
    set(PROPERTY_NAME,_name,m_sName);
}
// -----------------------------------------------------------------------------
::sal_uInt32 SAL_CALL OSection::getHeight() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_nHeight;
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::setHeight( ::sal_uInt32 _height ) throw (uno::RuntimeException)
{
    set(PROPERTY_HEIGHT,_height,m_nHeight);
}
// -----------------------------------------------------------------------------
::sal_Int32 SAL_CALL OSection::getBackColor() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_bBacktransparent ? COL_TRANSPARENT : m_nBackgroundColor;
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::setBackColor( ::sal_Int32 _backgroundcolor ) throw (uno::RuntimeException)
{
    sal_Bool bTransparent = _backgroundcolor == static_cast<sal_Int32>(COL_TRANSPARENT);
    setBackTransparent(bTransparent);
    if ( !bTransparent )
        set(PROPERTY_BACKCOLOR,_backgroundcolor,m_nBackgroundColor);
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OSection::getBackTransparent() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_bBacktransparent;
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::setBackTransparent( ::sal_Bool _backtransparent ) throw (uno::RuntimeException)
{
    set(PROPERTY_BACKTRANSPARENT,_backtransparent,m_bBacktransparent);
    if ( _backtransparent )
        set(PROPERTY_BACKCOLOR,static_cast<sal_Int32>(COL_TRANSPARENT),m_nBackgroundColor);
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSection::getConditionalPrintExpression() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_sConditionalPrintExpression;
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::setConditionalPrintExpression( const ::rtl::OUString& _conditionalprintexpression ) throw (uno::RuntimeException)
{
    set(PROPERTY_CONDITIONALPRINTEXPRESSION,_conditionalprintexpression,m_sConditionalPrintExpression);
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
::sal_Int16 SAL_CALL OSection::getForceNewPage() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    checkNotPageHeaderFooter();
    return m_nForceNewPage;
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::setForceNewPage( ::sal_Int16 _forcenewpage ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException)
{
    if ( _forcenewpage < report::ForceNewPage::NONE || _forcenewpage > report::ForceNewPage::BEFORE_AFTER_SECTION )
        throwIllegallArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com::sun::star::report::ForceNewPage"))
                        ,*this
                        ,1
                        ,m_xContext);
    checkNotPageHeaderFooter();
    set(PROPERTY_FORCENEWPAGE,_forcenewpage,m_nForceNewPage);
}
// -----------------------------------------------------------------------------
::sal_Int16 SAL_CALL OSection::getNewRowOrCol() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkNotPageHeaderFooter();
    return m_nNewRowOrCol;
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::setNewRowOrCol( ::sal_Int16 _newroworcol ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException)
{
    if ( _newroworcol < report::ForceNewPage::NONE || _newroworcol > report::ForceNewPage::BEFORE_AFTER_SECTION )
        throwIllegallArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com::sun::star::report::ForceNewPage"))
                        ,*this
                        ,1
                        ,m_xContext);
    checkNotPageHeaderFooter();

    set(PROPERTY_NEWROWORCOL,_newroworcol,m_nNewRowOrCol);
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OSection::getKeepTogether() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkNotPageHeaderFooter();
    return m_bKeepTogether;
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::setKeepTogether( ::sal_Bool _keeptogether ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException)
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        checkNotPageHeaderFooter();
    }

    set(PROPERTY_KEEPTOGETHER,_keeptogether,m_bKeepTogether);
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OSection::getCanGrow() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException(); ///TODO: unsupported at the moment
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::setCanGrow( ::sal_Bool /*_cangrow*/ ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException(); ///TODO: unsupported at the moment
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OSection::getCanShrink() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException(); ///TODO: unsupported at the moment
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::setCanShrink( ::sal_Bool /*_canshrink*/ ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException)
{
    throw beans::UnknownPropertyException(); ///TODO: unsupported at the moment
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OSection::getRepeatSection() throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    uno::Reference< report::XGroup > xGroup = m_xGroup;
    if ( !xGroup.is() )
        throw beans::UnknownPropertyException();
    return m_bRepeatSection;
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::setRepeatSection( ::sal_Bool _repeatsection ) throw (lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException)
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        uno::Reference< report::XGroup > xGroup = m_xGroup;
        if ( !xGroup.is() )
            throw beans::UnknownPropertyException();
    }
    set(PROPERTY_REPEATSECTION,_repeatsection,m_bRepeatSection);
}
// -----------------------------------------------------------------------------
uno::Reference< report::XGroup > SAL_CALL OSection::getGroup() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xGroup;
}
// -----------------------------------------------------------------------------
uno::Reference< report::XReportDefinition > SAL_CALL OSection::getReportDefinition() throw (uno::RuntimeException)
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
// -----------------------------------------------------------------------------
const ::std::vector< ::rtl::OUString >& lcl_getControlModelMap()
{
    static ::std::vector< ::rtl::OUString > s_sControlModels;
    if ( s_sControlModels.empty() )
    {
        s_sControlModels.push_back( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FixedText")) );
        s_sControlModels.push_back( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FixedLine")) );
        s_sControlModels.push_back( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ImageControl")) );
        s_sControlModels.push_back( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FormattedField")) );
        s_sControlModels.push_back( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Shape")) );
    }
    return s_sControlModels;

}
// -----------------------------------------------------------------------------
uno::Reference< report::XReportComponent > SAL_CALL OSection::createReportComponent( const ::rtl::OUString& _sReportComponentSpecifier ) throw (uno::Exception, lang::IllegalArgumentException,uno::RuntimeException)
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);
    const ::std::vector< ::rtl::OUString >& aRet = lcl_getControlModelMap();
    ::std::vector< ::rtl::OUString >::const_iterator aFind = ::std::find(aRet.begin(),aRet.end(),_sReportComponentSpecifier);
    if ( aFind == aRet.end() )
        throw lang::IllegalArgumentException();

    uno::Reference< report::XReportComponent > xRet;
    uno::Reference< lang::XMultiServiceFactory> xFac(getReportDefinition(),uno::UNO_QUERY_THROW);
    switch( aFind - aRet.begin()  )
    {
        case 0:
            xRet.set(xFac->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.FixedText"))),uno::UNO_QUERY);
            break;
        case 1:
            xRet.set(xFac->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFixedLineModel"))),uno::UNO_QUERY);
            break;
        case 2:
            xRet.set(xFac->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.DatabaseImageControl"))),uno::UNO_QUERY);
            break;
        case 3:
            xRet.set(xFac->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.FormattedField"))),uno::UNO_QUERY);
            break;
        case 4:
            xRet.set(xFac->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ControlShape"))),uno::UNO_QUERY);
            break;
        default:
            break;
    }
    return xRet;
}
// -----------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OSection::getAvailableReportComponentNames(  ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    const ::std::vector< ::rtl::OUString >& aRet = lcl_getControlModelMap();
    const ::rtl::OUString* pRet = aRet.empty() ? 0 : &aRet[0];
    return uno::Sequence< ::rtl::OUString >(pRet, aRet.size());
}
// -----------------------------------------------------------------------------
// XChild
uno::Reference< uno::XInterface > SAL_CALL OSection::getParent(  ) throw (uno::RuntimeException)
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
// -----------------------------------------------------------------------------
void SAL_CALL OSection::setParent( const uno::Reference< uno::XInterface >& /*Parent*/ ) throw (lang::NoSupportException, uno::RuntimeException)
{
    throw lang::NoSupportException();
}
// -----------------------------------------------------------------------------
// XContainer
void SAL_CALL OSection::addContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    m_aContainerListeners.addInterface(xListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    m_aContainerListeners.removeInterface(xListener);
}
// -----------------------------------------------------------------------------
// XElementAccess
uno::Type SAL_CALL OSection::getElementType(  ) throw (uno::RuntimeException)
{
    return ::getCppuType(static_cast< uno::Reference<report::XReportComponent>*>(NULL));
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OSection::hasElements(  ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xDrawPage.is() ? m_xDrawPage->hasElements() : sal_False;
}
// -----------------------------------------------------------------------------
// XIndexAccess
::sal_Int32 SAL_CALL OSection::getCount(  ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xDrawPage.is() ? m_xDrawPage->getCount() : 0;
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OSection::getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xDrawPage.is() ? m_xDrawPage->getByIndex(Index) : uno::Any();
}
// -----------------------------------------------------------------------------
// XEnumerationAccess
uno::Reference< container::XEnumeration > SAL_CALL OSection::createEnumeration(  ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return new ::comphelper::OEnumerationByIndex(static_cast<XSection*>(this));
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OSection::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    return SectionPropertySet::getPropertySetInfo();
}
// -------------------------------------------------------------------------
void SAL_CALL OSection::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    SectionPropertySet::setPropertyValue( aPropertyName, aValue );
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OSection::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return SectionPropertySet::getPropertyValue( PropertyName);
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    SectionPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    SectionPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    SectionPropertySet::addVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    SectionPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void OSection::lcl_copySection(const uno::Reference< report::XSection>& _xSource,uno::Reference< report::XSection>& _xDest)
{
    if ( _xSource.is() )
    {
        ::comphelper::copyProperties(_xSource.get(),_xDest.get());
        sal_Int32 nCount = _xSource->getCount();
        for(sal_Int32 i = 0;i != nCount;++i)
        {
            uno::Reference<util::XCloneable> xClone(_xSource->getByIndex(i),uno::UNO_QUERY);
            OSL_ENSURE(xClone.is(),"No XCloneable interface found!");
            if ( xClone.is() )
            {
                uno::Reference< drawing::XShape> xShape(xClone->createClone(),uno::UNO_QUERY);
                _xDest->add(xShape);
            }
        }
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OSection::add( const uno::Reference< drawing::XShape >& xShape ) throw (uno::RuntimeException)
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
// -----------------------------------------------------------------------------
void SAL_CALL OSection::remove( const uno::Reference< drawing::XShape >& xShape ) throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    // no lock because m_xDrawPage_ShapeGrouper is const
    return (m_xDrawPage_ShapeGrouper.is())
        ? m_xDrawPage_ShapeGrouper->group(xShapes)
        : 0;
}
void SAL_CALL
OSection::ungroup(uno::Reference<drawing::XShapeGroup> const& xGroup)
    throw (uno::RuntimeException)
{
    // no lock because m_xDrawPage_ShapeGrouper is const
    if (m_xDrawPage_ShapeGrouper.is()) {
        m_xDrawPage_ShapeGrouper->ungroup(xGroup);
    }
}

// XFormsSupplier
uno::Reference<container::XNameContainer> SAL_CALL OSection::getForms()
    throw (uno::RuntimeException)
{
    // no lock because m_xDrawPage_FormSupplier is const
    return (m_xDrawPage_FormSupplier.is())
        ? m_xDrawPage_FormSupplier->getForms()
        : 0;
}
// XFormsSupplier2
sal_Bool SAL_CALL OSection::hasForms() throw (uno::RuntimeException)
{
    // no lock because m_xDrawPage_FormSupplier is const
    return (m_xDrawPage_FormSupplier.is())
        ? m_xDrawPage_FormSupplier->hasForms()
        : 0;
}

// -----------------------------------------------------------------------------
// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OSection::getSomething( const uno::Sequence< sal_Int8 > & rId ) throw (uno::RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);
    return (m_xDrawPage_Tunnel.is()) ? m_xDrawPage_Tunnel->getSomething(rId) : 0;
}

// -----------------------------------------------------------------------------
OSection* OSection::getImplementation( const uno::Reference< uno::XInterface >& _rxComponent )
{
    OSection* pContent( NULL );

    uno::Reference< lang::XUnoTunnel > xUnoTunnel( _rxComponent, uno::UNO_QUERY );
    if ( xUnoTunnel.is() )
        pContent = reinterpret_cast< OSection* >( xUnoTunnel->getSomething( getUnoTunnelImplementationId() ) );

    return pContent;
}
//------------------------------------------------------------------------
uno::Sequence< sal_Int8 > OSection::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
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
// -----------------------------------------------------------------------------
void OSection::notifyElementAdded(const uno::Reference< drawing::XShape >& xShape )
{
    if ( !m_bInInsertNotify )
    {
        container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::Any(), uno::makeAny(xShape), uno::Any());
        m_aContainerListeners.notifyEach(&container::XContainerListener::elementInserted,aEvent);
    }
}
// -----------------------------------------------------------------------------
void OSection::notifyElementRemoved(const uno::Reference< drawing::XShape >& xShape)
{
    if ( !m_bInRemoveNotify )
    {
        // notify our container listeners
        container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::Any(), uno::makeAny(xShape), uno::Any());
        m_aContainerListeners.notifyEach(&container::XContainerListener::elementRemoved,aEvent);
    }
}
// =============================================================================
} // namespace reportdesign
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
