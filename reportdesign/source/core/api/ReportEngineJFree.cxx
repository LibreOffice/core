/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportEngineJFree.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:44:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <com/sun/star/beans/PropertyValue.hpp>
#include "ReportEngineJFree.hxx"

#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _COMPHELPER_DOCUMENTCONSTANTS_HXX
#include <comphelper/documentconstants.hxx>
#endif
#ifndef _COMPHELPER_STORAGEHELPER_HXX
#include <comphelper/storagehelper.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDEXECUTION_HPP_
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#endif
#include <com/sun/star/sdb/XSingleSelectQueryAnalyzer.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <connectivity/statementcomposer.hxx>

#include <com/sun/star/task/XInteractionHandler.hpp>
#ifndef _COM_SUN_STAR_TASK_XJOB_HPP_
#include <com/sun/star/task/XJob.hpp>
#endif
#ifndef REPORTDESIGN_SHARED_CORESTRINGS_HRC
#include "corestrings.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef UNOTOOLS_INC_SHAREDUNOCOMPONENT_HXX
#include <unotools/sharedunocomponent.hxx>
#endif
#ifndef _COMPHELPER_MIMECONFIGHELPER_HXX_
#include <comphelper/mimeconfighelper.hxx>
#endif
#ifndef REPORTDESIGN_TOOLS_HXX
#include "Tools.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
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
{
    DBG_CTOR( rpt_OReportEngineJFree,NULL);
}
// -----------------------------------------------------------------------------
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
    ::comphelper::disposeComponent(m_xRowSet);
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
            ::comphelper::disposeComponent(m_xRowSet);
            m_xRowSet.set(m_xContext->getServiceManager()->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.RowSet"))
                ,m_xContext),uno::UNO_QUERY);
        }
        setRowSetProperties();
    }
    l.notify();
}
// -----------------------------------------------------------------------------
void OReportEngineJFree::setRowSetProperties()
{
    if ( m_xReport.is() && m_xRowSet.is() )
    {
        uno::Reference< beans::XPropertySet> xRowSetProp(m_xRowSet,uno::UNO_QUERY);
        xRowSetProp->setPropertyValue(PROPERTY_COMMAND,m_xReport->getPropertyValue(PROPERTY_COMMAND));
        xRowSetProp->setPropertyValue(PROPERTY_COMMANDTYPE,m_xReport->getPropertyValue(PROPERTY_COMMANDTYPE));
        xRowSetProp->setPropertyValue(PROPERTY_FILTER,m_xReport->getPropertyValue(PROPERTY_FILTER));
    }
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
::rtl::OUString OReportEngineJFree::transform()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("content.xml")); /// TODO has to be changed into the real name for the report transformation
}
// -----------------------------------------------------------------------------
::rtl::OUString OReportEngineJFree::getNewOutputName()
{
    ::rtl::OUString sOutputName;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(ReportEngineBase::rBHelper.bDisposed);
        if ( !m_xReport.is() || !m_xRowSet.is() || !m_xActiveConnection.is() )
            throw lang::IllegalArgumentException();

        try
        {
            uno::Reference< embed::XStorage > xTemp = OStorageHelper::GetTemporaryStorage(/*sFileTemp,embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE,*/uno::Reference< lang::XMultiServiceFactory >(m_xContext->getServiceManager(),uno::UNO_QUERY));
            utl::DisposableComponent aTemp(xTemp);
            uno::Sequence< beans::PropertyValue > aEmpty;
            uno::Reference< beans::XPropertySet> xStorageProp(xTemp,uno::UNO_QUERY);
            if ( xStorageProp.is() )
            {
                static const ::rtl::OUString sPropName(RTL_CONSTASCII_USTRINGPARAM("MediaType"));
                xStorageProp->setPropertyValue( sPropName, uno::makeAny(m_xReport->getMimeType()));
            }
            m_xReport->storeToStorage(xTemp,aEmpty); // store to temp file because it may contain information which aren't in the database yet.

            uno::Sequence< beans::NamedValue > aConvertedProperties(5/*6*/);
            sal_Int32 nPos = 0;

            //aConvertedProperties[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Key"));
            //::rtl::OUString sKey;// = StorageContainer::registerStorage(m_xReport);
            //aConvertedProperties[nPos++].Value <<= sKey;

            aConvertedProperties[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InputStorage"));
            aConvertedProperties[nPos++].Value <<= xTemp;

            aConvertedProperties[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OutputStorage"));

            //! TODO: has to be changed into a temp storage later on
            const static String s_sExt = String::CreateFromAscii(".rpt");
            String sName = m_xReport->getName();
            ::utl::TempFile aFile(sName,sal_False,&s_sExt);
            uno::Reference< embed::XStorage > xOut = OStorageHelper::GetStorageFromURL(aFile.GetURL(),embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE,uno::Reference< lang::XMultiServiceFactory >(m_xContext->getServiceManager(),uno::UNO_QUERY));
            utl::DisposableComponent aOut(xOut);
            xStorageProp.set(xOut,uno::UNO_QUERY);
            if ( xStorageProp.is() )
            {
                static const ::rtl::OUString sPropName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType"));
                xStorageProp->setPropertyValue( sPropName, uno::makeAny(m_xReport->getMimeType()));
            }
            /*::rtl::OUString sStyles(RTL_CONSTASCII_USTRINGPARAM("styles.xml"));
            xTemp->copyElementTo(sStyles,xOut,sStyles);*/

            aConvertedProperties[nPos++].Value <<= xOut;

            aConvertedProperties[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("mimetype"));
            aConvertedProperties[nPos++].Value <<= m_xReport->getMimeType();

            aConvertedProperties[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RowSet"));
            aConvertedProperties[nPos++].Value <<= m_xRowSet;

            // create job factory and initialize
            const ::rtl::OUString sReportEngineServiceName = ::dbtools::getDefaultReportEngineServiceName(uno::Reference< lang::XMultiServiceFactory >(m_xContext->getServiceManager(),uno::UNO_QUERY_THROW));
            uno::Reference<task::XJob> xJob(m_xContext->getServiceManager()->createInstanceWithContext(sReportEngineServiceName,m_xContext),uno::UNO_QUERY_THROW);
            if ( m_xReport->getCommand().getLength() )
            {
                uno::Reference< beans::XPropertySet> xRowSetProp(m_xRowSet,uno::UNO_QUERY_THROW);
                xRowSetProp->setPropertyValue(PROPERTY_ACTIVECONNECTION,uno::makeAny(m_xActiveConnection));

                ::rtl::OUString sOrder = getOrderStatement();
                if (sOrder.getLength() > 0)
                {
                    sal_Int32 nCommandType = 0;
                    xRowSetProp->getPropertyValue(PROPERTY_COMMANDTYPE) >>= nCommandType;
                    if (nCommandType != sdb::CommandType::TABLE)
                    {
                        rtl::OUString sOldCommand;
                        xRowSetProp->getPropertyValue(PROPERTY_COMMAND) >>= sOldCommand;
                        dbtools::StatementComposer aComposer(m_xActiveConnection, sOldCommand, nCommandType, sal_True );

                        uno::Reference< sdb::XSingleSelectQueryComposer > xComposer( aComposer.getComposer() );
                        if ( xComposer.is() )
                        {
                            rtl::OUString sCurrentSQL = xComposer->getQuery();
                            // Magic here, read the nice documentation out of the IDL.
                            xComposer->setQuery(sCurrentSQL);
                            rtl::OUString sOldOrder = xComposer->getOrder();
                            if (sOldOrder.getLength() > 0)
                            {
                                sOrder += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
                                sOrder += sOldOrder;
                                xComposer->setOrder(rtl::OUString());
                                rtl::OUString sQuery = xComposer->getQuery();
                                xRowSetProp->setPropertyValue(PROPERTY_COMMAND, uno::makeAny(sQuery));
                                xRowSetProp->setPropertyValue(PROPERTY_COMMANDTYPE, uno::makeAny(sdb::CommandType::COMMAND));
                            }
                        }
                    }
                }
                // we have to set the order everytime, at least we clear the old one.
                xRowSetProp->setPropertyValue(PROPERTY_ORDER,uno::makeAny(sOrder));
                xRowSetProp->setPropertyValue(PROPERTY_APPLYFILTER,uno::makeAny(m_xReport->getFilter().getLength() != 0));

                uno::Reference<sdb::XCompletedExecution> xExecute( m_xRowSet, uno::UNO_QUERY );
                if ( xExecute.is() )
                {
                    uno::Reference<task::XInteractionHandler> xHandler(
                             m_xContext->getServiceManager()->createInstanceWithContext(
                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.InteractionHandler"))
                             ,m_xContext),
                                        uno::UNO_QUERY);
                    xExecute->executeWithCompletion(xHandler);
                }
                else
                    m_xRowSet->execute();
                xJob->execute(aConvertedProperties);
            }

            if ( xStorageProp.is() )
                sOutputName = aFile.GetURL();

            uno::Reference<embed::XTransactedObject> xTransact(xOut,uno::UNO_QUERY);
            if ( sOutputName.getLength() && xTransact.is() )
                xTransact->commit();

            if ( !sOutputName.getLength() )
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
    uno::Reference< frame::XModel > xModel;
    ::rtl::OUString sOutputName = getNewOutputName();
    if ( sOutputName.getLength() )
    {
        uno::Reference< lang::XMultiServiceFactory > xFac(m_xContext->getServiceManager(),uno::UNO_QUERY);
        ::comphelper::MimeConfigurationHelper aHelper(xFac);
        ::rtl::OUString sServiceName = aHelper.GetDocServiceNameFromMediaType(m_xReport->getMimeType());
        xModel.set(m_xContext->getServiceManager()->createInstanceWithContext(sServiceName,m_xContext),uno::UNO_QUERY_THROW);
        uno::Sequence< beans::PropertyValue > aArguments;
        xModel->attachResource(sOutputName,aArguments);
    }
    return xModel;
}
// -----------------------------------------------------------------------------
uno::Reference< frame::XModel > SAL_CALL OReportEngineJFree::createDocumentAlive( const uno::Reference< frame::XFrame >& _frame ) throw (lang::DisposedException, lang::IllegalArgumentException, uno::Exception, uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel;
    ::rtl::OUString sOutputName = getNewOutputName(); // starts implicite the report generator
    if ( sOutputName.getLength() )
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
            ::rtl::OUString sTarget = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_blank"));
            sal_Int32 nFrameSearchFlag = frame::FrameSearchFlag::TASKS | frame::FrameSearchFlag::CREATE;
            uno::Reference< frame::XFrame> xFrame = uno::Reference< frame::XFrame>(xFrameLoad,uno::UNO_QUERY)->findFrame(sTarget,nFrameSearchFlag);
            xFrameLoad.set( xFrame,uno::UNO_QUERY);
        }

        if ( xFrameLoad.is() )
        {
            uno::Sequence < beans::PropertyValue > aArgs( 2 );
            sal_Int32 nLen = 0;
            aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AsTemplate"));
            aArgs[nLen++].Value <<= sal_False;

            aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
            aArgs[nLen++].Value <<= sal_True;

            uno::Reference< lang::XMultiServiceFactory > xFac(m_xContext->getServiceManager(),uno::UNO_QUERY);
            ::comphelper::MimeConfigurationHelper aHelper(xFac);
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
::rtl::OUString OReportEngineJFree::getOrderStatement() const
{
    OSL_ENSURE(m_xActiveConnection.is(),"OReportEngineJFree::getOrderStatement: The ActiveConnection can not be NULL here!");
    OSL_ENSURE(m_xReport.is(),"OReportEngineJFree::getOrderStatement: The ReportDefinition can not be NULL here!");

    // retrieve columns of the object we're bound to
    uno::Reference< lang::XComponent > xHoldAlive;
    ::dbtools::SQLExceptionInfo aErrorInfo;
    uno::Reference< container::XNameAccess> xColumns( ::dbtools::getFieldsByCommandDescriptor(
        m_xActiveConnection, m_xReport->getCommandType(), m_xReport->getCommand(), xHoldAlive, & aErrorInfo ) );
    if ( aErrorInfo.isValid() )
        aErrorInfo.doThrow();
    if ( !xColumns.is() )
    {
        DBG_WARNING( "OReportEngineJFree::getOrderStatement: could not retrieve the columns for the ORDER statement!" );
        return ::rtl::OUString();
    }

    // set order for groups
    ::rtl::OUStringBuffer aOrder;

    const ::rtl::OUString sQuote = m_xActiveConnection->getMetaData()->getIdentifierQuoteString();

    uno::Reference< report::XGroups> xGroups = m_xReport->getGroups();
    const sal_Int32 nCount = xGroups->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i )
    {
        uno::Reference< report::XGroup> xGroup(xGroups->getByIndex(i),uno::UNO_QUERY);
        ::rtl::OUString sExpression = xGroup->getExpression();
        if ( xColumns->hasByName(sExpression) )
            sExpression = ::dbtools::quoteName( sQuote, sExpression );
        sExpression = sExpression.trim(); // Trim away white spaces
        if (sExpression.getLength() > 0)
        {
            aOrder.append( sExpression );
            if (aOrder.getLength() > 0)
                aOrder.appendAscii( " " );
            if ( !xGroup->getSortAscending() )
                aOrder.appendAscii( "DESC" );
            if ( (i+1) < nCount )
                aOrder.appendAscii( "," );
        }
    }

    return aOrder.makeStringAndClear();
}
// =============================================================================
} // namespace reportdesign
// =============================================================================
