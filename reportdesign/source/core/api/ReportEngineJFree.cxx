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
#include <ReportEngineJFree.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/storagehelper.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryAnalyzer.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdb/CommandType.hpp>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XJob.hpp>

#include <unotools/useroptions.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/sharedunocomponent.hxx>

#include <Tools.hxx>
#include <strings.hxx>
#include <strings.hrc>
#include <core_resource.hxx>

#include <connectivity/CommonTools.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/docfilt.hxx>

namespace reportdesign
{

    using namespace com::sun::star;
    using namespace comphelper;


OReportEngineJFree::OReportEngineJFree( const uno::Reference< uno::XComponentContext >& context)
:ReportEngineBase(m_aMutex)
,ReportEnginePropertySet(context,IMPLEMENTS_PROPERTY_SET,uno::Sequence< OUString >())
,m_xContext(context)
,m_nMaxRows(0)
{
}

// TODO: VirtualFunctionFinder: This is virtual function!

OReportEngineJFree::~OReportEngineJFree()
{
}

IMPLEMENT_FORWARD_XINTERFACE2(OReportEngineJFree,ReportEngineBase,ReportEnginePropertySet)

void SAL_CALL OReportEngineJFree::dispose()
{
    ReportEnginePropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
    m_xActiveConnection.clear();
}

OUString OReportEngineJFree::getImplementationName_Static(  )
{
    return OUString("com.sun.star.comp.report.OReportEngineJFree");
}


OUString SAL_CALL OReportEngineJFree::getImplementationName(  )
{
    return getImplementationName_Static();
}

uno::Sequence< OUString > OReportEngineJFree::getSupportedServiceNames_Static(  )
{
    uno::Sequence< OUString > aServices { "com.sun.star.report.ReportEngine" };

    return aServices;
}

uno::Reference< uno::XInterface > OReportEngineJFree::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OReportEngineJFree(xContext));
}


uno::Sequence< OUString > SAL_CALL OReportEngineJFree::getSupportedServiceNames(  )
{
    return getSupportedServiceNames_Static();
}

sal_Bool SAL_CALL OReportEngineJFree::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XReportEngine
    // Attributes
uno::Reference< report::XReportDefinition > SAL_CALL OReportEngineJFree::getReportDefinition()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xReport;
}

void SAL_CALL OReportEngineJFree::setReportDefinition( const uno::Reference< report::XReportDefinition >& _report )
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

uno::Reference< task::XStatusIndicator > SAL_CALL OReportEngineJFree::getStatusIndicator()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_StatusIndicator;
}

void SAL_CALL OReportEngineJFree::setStatusIndicator( const uno::Reference< task::XStatusIndicator >& _statusindicator )
{
    set(PROPERTY_STATUSINDICATOR,_statusindicator,m_StatusIndicator);
}

OUString OReportEngineJFree::getNewOutputName()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportEngineBase::rBHelper.bDisposed);
    if ( !m_xReport.is() || !m_xActiveConnection.is() )
        throw lang::IllegalArgumentException();

    static const char s_sMediaType[] = "MediaType";

    MimeConfigurationHelper aConfighelper(m_xContext);
    const OUString sMimeType = m_xReport->getMimeType();
    std::shared_ptr<const SfxFilter> pFilter = SfxFilter::GetDefaultFilter( aConfighelper.GetDocServiceNameFromMediaType(sMimeType) );
    OUString sExt(".rpt");
    if ( pFilter )
        sExt = ::comphelper::string::stripStart(pFilter->GetDefaultExtension(), '*');

    uno::Reference< embed::XStorage > xTemp = OStorageHelper::GetTemporaryStorage(/*sFileTemp,embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE,*/ m_xContext);
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
    aConvertedProperties[nPos].Name = "InputStorage";
    aConvertedProperties[nPos++].Value <<= xTemp;
    aConvertedProperties[nPos].Name = "OutputStorage";

    OUString sFileURL;
    OUString sName = m_xReport->getCaption();
    if ( sName.isEmpty() )
        sName = m_xReport->getName();
    {
        ::utl::TempFile aTestFile(sName, false, &sExt);
        if ( !aTestFile.IsValid() )
        {
            sName = RptResId(RID_STR_REPORT);
            ::utl::TempFile aFile(sName, false, &sExt);
            sFileURL = aFile.GetURL();
        }
        else
            sFileURL = aTestFile.GetURL();
    }

    uno::Reference< embed::XStorage > xOut = OStorageHelper::GetStorageFromURL(sFileURL,embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE, m_xContext);
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
    OUStringBuffer sAuthor(aUserOpts.GetFirstName());
    sAuthor.append(" ");
    sAuthor.append(aUserOpts.GetLastName());
    aConvertedProperties[nPos].Name = "Author";
    aConvertedProperties[nPos++].Value <<= sAuthor.makeStringAndClear();

    aConvertedProperties[nPos].Name = "Title";
    aConvertedProperties[nPos++].Value <<= m_xReport->getCaption();

    OUString sOutputName;

    // create job factory and initialize
    const OUString sReportEngineServiceName = ::dbtools::getDefaultReportEngineServiceName(m_xContext);
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

    return sOutputName;
}

// Methods
uno::Reference< frame::XModel > SAL_CALL OReportEngineJFree::createDocumentModel( )
{
    return createDocumentAlive(nullptr,true);
}

uno::Reference< frame::XModel > SAL_CALL OReportEngineJFree::createDocumentAlive( const uno::Reference< frame::XFrame >& _frame )
{
    return createDocumentAlive(_frame,false);
}

uno::Reference< frame::XModel > OReportEngineJFree::createDocumentAlive( const uno::Reference< frame::XFrame >& _frame,bool _bHidden )
{
    uno::Reference< frame::XModel > xModel;
    OUString sOutputName = getNewOutputName(); // starts implicitly the report generator
    if ( !sOutputName.isEmpty() )
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(ReportEngineBase::rBHelper.bDisposed);
        uno::Reference<frame::XComponentLoader> xFrameLoad(_frame,uno::UNO_QUERY);
        if ( !xFrameLoad.is() )
        {
            // if there is no frame given, find the right
            xFrameLoad.set( frame::Desktop::create(m_xContext), uno::UNO_QUERY);
            sal_Int32 const nFrameSearchFlag = frame::FrameSearchFlag::TASKS | frame::FrameSearchFlag::CREATE;
            uno::Reference< frame::XFrame> xFrame = uno::Reference< frame::XFrame>(xFrameLoad,uno::UNO_QUERY)->findFrame("_blank",nFrameSearchFlag);
            xFrameLoad.set( xFrame,uno::UNO_QUERY);
        }

        if ( xFrameLoad.is() )
        {
            uno::Sequence < beans::PropertyValue > aArgs( _bHidden ? 3 : 2 );
            sal_Int32 nLen = 0;
            aArgs[nLen].Name = "AsTemplate";
            aArgs[nLen++].Value <<= false;

            aArgs[nLen].Name = "ReadOnly";
            aArgs[nLen++].Value <<= true;

            if ( _bHidden )
            {
                aArgs[nLen].Name = "Hidden";
                aArgs[nLen++].Value <<= true;
            }

            xModel.set( xFrameLoad->loadComponentFromURL(
                sOutputName,
                OUString(), // empty frame name
                0,
                aArgs
                ),uno::UNO_QUERY);
        }
    }
    return xModel;
}

util::URL SAL_CALL OReportEngineJFree::createDocument( )
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

void SAL_CALL OReportEngineJFree::interrupt(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportEngineBase::rBHelper.bDisposed);
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL OReportEngineJFree::getPropertySetInfo(  )
{
    return ReportEnginePropertySet::getPropertySetInfo();
}

void SAL_CALL OReportEngineJFree::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    ReportEnginePropertySet::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL OReportEngineJFree::getPropertyValue( const OUString& PropertyName )
{
    return ReportEnginePropertySet::getPropertyValue( PropertyName);
}

void SAL_CALL OReportEngineJFree::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
{
    ReportEnginePropertySet::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL OReportEngineJFree::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener )
{
    ReportEnginePropertySet::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL OReportEngineJFree::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
{
    ReportEnginePropertySet::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL OReportEngineJFree::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
{
    ReportEnginePropertySet::removeVetoableChangeListener( PropertyName, aListener );
}

uno::Reference< sdbc::XConnection > SAL_CALL OReportEngineJFree::getActiveConnection()
{
    return m_xActiveConnection;
}

void SAL_CALL OReportEngineJFree::setActiveConnection( const uno::Reference< sdbc::XConnection >& _activeconnection )
{
    if ( !_activeconnection.is() )
        throw lang::IllegalArgumentException();
    set(PROPERTY_ACTIVECONNECTION,_activeconnection,m_xActiveConnection);
}

::sal_Int32 SAL_CALL OReportEngineJFree::getMaxRows()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_nMaxRows;
}

void SAL_CALL OReportEngineJFree::setMaxRows( ::sal_Int32 MaxRows )
{
    set(PROPERTY_MAXROWS,MaxRows,m_nMaxRows);
}

} // namespace reportdesign


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
