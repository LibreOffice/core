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
#include <com/sun/star/beans/PropertyValue.hpp>
#include "ReportEngineJFree.hxx"
#include <comphelper/enumhelper.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/storagehelper.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/property.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryAnalyzer.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdb/CommandType.hpp>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XJob.hpp>

#include <tools/debug.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/sharedunocomponent.hxx>

#include "Tools.hxx"
#include "corestrings.hrc"
#include "core_resource.hrc"
#include "core_resource.hxx"

#include <connectivity/CommonTools.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/docfilt.hxx>
// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;

DBG_NAME( rpt_OReportEngineJFree )
// -----------------------------------------------------------------------------
OReportEngineJFree::OReportEngineJFree( const uno::Reference< uno::XComponentContext >& context)
:ReportEngineBase(m_aMutex)
,ReportEnginePropertySet(context,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),uno::Sequence< ::rtl::OUString >())
,m_xContext(context)
,m_nMaxRows(0)
{
    DBG_CTOR( rpt_OReportEngineJFree,NULL);
}
// -----------------------------------------------------------------------------
// TODO: VirtualFunctionFinder: This is virtual function!
//
OReportEngineJFree::~OReportEngineJFree()
{
    DBG_DTOR( rpt_OReportEngineJFree,NULL);
}
//--------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2(OReportEngineJFree,ReportEngineBase,ReportEnginePropertySet)
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::dispose() throw(uno::RuntimeException)
{
    ReportEnginePropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
    m_xActiveConnection.clear();
}
// -----------------------------------------------------------------------------
::rtl::OUString OReportEngineJFree::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.OReportEngineJFree"));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OReportEngineJFree::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > OReportEngineJFree::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aServices(1);
    aServices.getArray()[0] = SERVICE_REPORTENGINE;

    return aServices;
}
//------------------------------------------------------------------------------
uno::Reference< uno::XInterface > OReportEngineJFree::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OReportEngineJFree(xContext));
}

//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OReportEngineJFree::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OReportEngineJFree::supportsService(const ::rtl::OUString& ServiceName) throw( uno::RuntimeException )
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
// -----------------------------------------------------------------------------
// XReportEngine
    // Attributes
uno::Reference< report::XReportDefinition > SAL_CALL OReportEngineJFree::getReportDefinition() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xReport;
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::setReportDefinition( const uno::Reference< report::XReportDefinition >& _report ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    if ( !_report.is() )
        throw lang::IllegalArgumentException();
    BoundListeners l;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if ( m_xReport != _report )
        {
            prepareSet(PROPERTY_REPORTDEFINITION, uno::makeAny(m_xReport), uno::makeAny(_report), &l);
            m_xReport = _report;
        }
    }
    l.notify();
}
// -----------------------------------------------------------------------------
uno::Reference< task::XStatusIndicator > SAL_CALL OReportEngineJFree::getStatusIndicator() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_StatusIndicator;
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::setStatusIndicator( const uno::Reference< task::XStatusIndicator >& _statusindicator ) throw (uno::RuntimeException)
{
    set(PROPERTY_STATUSINDICATOR,_statusindicator,m_StatusIndicator);
}
// -----------------------------------------------------------------------------
::rtl::OUString OReportEngineJFree::getNewOutputName()
{
    ::rtl::OUString sOutputName;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(ReportEngineBase::rBHelper.bDisposed);
        if ( !m_xReport.is() || !m_xActiveConnection.is() )
            throw lang::IllegalArgumentException();

        static const ::rtl::OUString s_sMediaType(RTL_CONSTASCII_USTRINGPARAM("MediaType"));
        try
        {
            const uno::Reference< lang::XMultiServiceFactory > xFactory(m_xContext->getServiceManager(),uno::UNO_QUERY_THROW);
            MimeConfigurationHelper aConfighelper(xFactory);
            const ::rtl::OUString sMimeType = m_xReport->getMimeType();
            const SfxFilter* pFilter = SfxFilter::GetDefaultFilter( aConfighelper.GetDocServiceNameFromMediaType(sMimeType) );
            String sExt;
            if ( pFilter )
                sExt = ::comphelper::string::stripStart(pFilter->GetDefaultExtension(), '*');
            else
                sExt = rtl::OUString(".rpt");

            uno::Reference< embed::XStorage > xTemp = OStorageHelper::GetTemporaryStorage(/*sFileTemp,embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE,*/uno::Reference< lang::XMultiServiceFactory >(m_xContext->getServiceManager(),uno::UNO_QUERY));
            utl::DisposableComponent aTemp(xTemp);
            uno::Sequence< beans::PropertyValue > aEmpty;
            uno::Reference< beans::XPropertySet> xStorageProp(xTemp,uno::UNO_QUERY);
            if ( xStorageProp.is() )
            {
                xStorageProp->setPropertyValue( s_sMediaType, uno::makeAny(sMimeType));
            }
            m_xReport->storeToStorage(xTemp,aEmpty); // store to temp file because it may contain information which isn't in the database yet.

            uno::Sequence< beans::NamedValue > aConvertedProperties(8);
            sal_Int32 nPos = 0;
            aConvertedProperties[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InputStorage"));
            aConvertedProperties[nPos++].Value <<= xTemp;
            aConvertedProperties[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OutputStorage"));

            ::rtl::OUString sFileURL;
            String sName = m_xReport->getCaption();
            if ( !sName.Len() )
                sName = m_xReport->getName();
            {
                ::utl::TempFile aTestFile(sName,sal_False,&sExt);
                if ( !aTestFile.IsValid() )
                {
                    sName = RPT_RESSTRING(RID_STR_REPORT,m_xContext->getServiceManager());
                    ::utl::TempFile aFile(sName,sal_False,&sExt);
                    sFileURL = aFile.GetURL();
                }
                else
                    sFileURL = aTestFile.GetURL();
            }

            uno::Reference< embed::XStorage > xOut = OStorageHelper::GetStorageFromURL(sFileURL,embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE,uno::Reference< lang::XMultiServiceFactory >(m_xContext->getServiceManager(),uno::UNO_QUERY));
            utl::DisposableComponent aOut(xOut);
            xStorageProp.set(xOut,uno::UNO_QUERY);
            if ( xStorageProp.is() )
            {
                xStorageProp->setPropertyValue( s_sMediaType, uno::makeAny(sMimeType));
            }

            aConvertedProperties[nPos++].Value <<= xOut;

            aConvertedProperties[nPos].Name = PROPERTY_REPORTDEFINITION;
            aConvertedProperties[nPos++].Value <<= m_xReport;

            aConvertedProperties[nPos].Name = PROPERTY_ACTIVECONNECTION;
            aConvertedProperties[nPos++].Value <<= m_xActiveConnection;

            aConvertedProperties[nPos].Name = PROPERTY_MAXROWS;
            aConvertedProperties[nPos++].Value <<= m_nMaxRows;

            // some meta data
            SvtUserOptions aUserOpts;
            ::rtl::OUStringBuffer sAuthor(aUserOpts.GetFirstName());
            sAuthor.appendAscii(" ");
            sAuthor.append(aUserOpts.GetLastName());
            static const ::rtl::OUString s_sAuthor(RTL_CONSTASCII_USTRINGPARAM("Author"));
            aConvertedProperties[nPos].Name = s_sAuthor;
            aConvertedProperties[nPos++].Value <<= sAuthor.makeStringAndClear();

            static const ::rtl::OUString s_sTitle(RTL_CONSTASCII_USTRINGPARAM("Title"));
            aConvertedProperties[nPos].Name = s_sTitle;
            aConvertedProperties[nPos++].Value <<= m_xReport->getCaption();

            // create job factory and initialize
            const ::rtl::OUString sReportEngineServiceName = ::dbtools::getDefaultReportEngineServiceName(xFactory);
            uno::Reference<task::XJob> xJob(m_xContext->getServiceManager()->createInstanceWithContext(sReportEngineServiceName,m_xContext),uno::UNO_QUERY_THROW);
            if ( !m_xReport->getCommand().isEmpty() )
            {
                xJob->execute(aConvertedProperties);
                if ( xStorageProp.is() )
                {
                    sOutputName = sFileURL;
                }
            }

            uno::Reference<embed::XTransactedObject> xTransact(xOut,uno::UNO_QUERY);
            if ( !sOutputName.isEmpty() && xTransact.is() )
                xTransact->commit();

            if ( sOutputName.isEmpty() )
                throw lang::IllegalArgumentException();
        }
        catch(const uno::Exception& e)
        {
            (void)e; // helper to know what e contains
            throw;
        }
    }
    return sOutputName;
}
// -----------------------------------------------------------------------------
// Methods
uno::Reference< frame::XModel > SAL_CALL OReportEngineJFree::createDocumentModel( ) throw (lang::DisposedException, lang::IllegalArgumentException, uno::Exception, uno::RuntimeException)
{
    return createDocumentAlive(NULL,true);
}
// -----------------------------------------------------------------------------
uno::Reference< frame::XModel > SAL_CALL OReportEngineJFree::createDocumentAlive( const uno::Reference< frame::XFrame >& _frame ) throw (lang::DisposedException, lang::IllegalArgumentException, uno::Exception, uno::RuntimeException)
{
    return createDocumentAlive(_frame,false);
}
// -----------------------------------------------------------------------------
uno::Reference< frame::XModel > SAL_CALL OReportEngineJFree::createDocumentAlive( const uno::Reference< frame::XFrame >& _frame,bool _bHidden ) throw (lang::DisposedException, lang::IllegalArgumentException, uno::Exception, uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel;
    ::rtl::OUString sOutputName = getNewOutputName(); // starts implicite the report generator
    if ( !sOutputName.isEmpty() )
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(ReportEngineBase::rBHelper.bDisposed);
        uno::Reference<frame::XComponentLoader> xFrameLoad(_frame,uno::UNO_QUERY);
        if ( !xFrameLoad.is() )
        {
            // if there is no frame given, find the right
            xFrameLoad.set( m_xContext->getServiceManager()->createInstanceWithContext(
                                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"))
                                                    ,m_xContext)
                                                    ,uno::UNO_QUERY);
            ::rtl::OUString sTarget(RTL_CONSTASCII_USTRINGPARAM("_blank"));
            sal_Int32 nFrameSearchFlag = frame::FrameSearchFlag::TASKS | frame::FrameSearchFlag::CREATE;
            uno::Reference< frame::XFrame> xFrame = uno::Reference< frame::XFrame>(xFrameLoad,uno::UNO_QUERY)->findFrame(sTarget,nFrameSearchFlag);
            xFrameLoad.set( xFrame,uno::UNO_QUERY);
        }

        if ( xFrameLoad.is() )
        {
            uno::Sequence < beans::PropertyValue > aArgs( _bHidden ? 3 : 2 );
            sal_Int32 nLen = 0;
            aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AsTemplate"));
            aArgs[nLen++].Value <<= sal_False;

            aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
            aArgs[nLen++].Value <<= sal_True;

            if ( _bHidden )
            {
                aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Hidden"));
                aArgs[nLen++].Value <<= sal_True;
            }

            uno::Reference< lang::XMultiServiceFactory > xFac(m_xContext->getServiceManager(),uno::UNO_QUERY);
            xModel.set( xFrameLoad->loadComponentFromURL(
                sOutputName,
                ::rtl::OUString(), // empty frame name
                0,
                aArgs
                ),uno::UNO_QUERY);
        }
    }
    return xModel;
}
// -----------------------------------------------------------------------------
util::URL SAL_CALL OReportEngineJFree::createDocument( ) throw (lang::DisposedException, lang::IllegalArgumentException, uno::Exception, uno::RuntimeException)
{
    util::URL aRet;
    uno::Reference< frame::XModel > xModel = createDocumentModel();
    if ( xModel.is() )
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(ReportEngineBase::rBHelper.bDisposed);
    }
    return aRet;
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::interrupt(  ) throw (lang::DisposedException, uno::Exception, uno::RuntimeException)
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(ReportEngineBase::rBHelper.bDisposed);
    }
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OReportEngineJFree::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    return ReportEnginePropertySet::getPropertySetInfo();
}
// -------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    ReportEnginePropertySet::setPropertyValue( aPropertyName, aValue );
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OReportEngineJFree::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return ReportEnginePropertySet::getPropertyValue( PropertyName);
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ReportEnginePropertySet::addPropertyChangeListener( aPropertyName, xListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ReportEnginePropertySet::removePropertyChangeListener( aPropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ReportEnginePropertySet::addVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ReportEnginePropertySet::removeVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
uno::Reference< sdbc::XConnection > SAL_CALL OReportEngineJFree::getActiveConnection() throw (uno::RuntimeException)
{
    return m_xActiveConnection;
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::setActiveConnection( const uno::Reference< sdbc::XConnection >& _activeconnection ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    if ( !_activeconnection.is() )
        throw lang::IllegalArgumentException();
    set(PROPERTY_ACTIVECONNECTION,_activeconnection,m_xActiveConnection);
}
// -----------------------------------------------------------------------------
::sal_Int32 SAL_CALL OReportEngineJFree::getMaxRows() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_nMaxRows;
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::setMaxRows( ::sal_Int32 _MaxRows ) throw (uno::RuntimeException)
{
    set(PROPERTY_MAXROWS,_MaxRows,m_nMaxRows);
}
// =============================================================================
} // namespace reportdesign
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
