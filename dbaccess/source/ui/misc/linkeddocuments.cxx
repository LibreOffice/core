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

#include <core_resource.hxx>
#include <linkeddocuments.hxx>
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>
#include <unotools/confignode.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/OpenCommandArgument.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <comphelper/types.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <svl/filenotation.hxx>
#include <browserids.hxx>
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#include <comphelper/mimeconfighelper.hxx>
#include <utility>
#include <vcl/weld.hxx>

#include <cppuhelper/exc_hlp.hxx>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/io/WrongFormatException.hpp>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb::application;
    using namespace ::com::sun::star::task;

    namespace
    {
        Sequence< sal_Int8 > lcl_GetSequenceClassID( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                                                    sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                                                    sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 )
        {
            Sequence< sal_Int8 > aResult{ /* [ 0] */ static_cast<sal_Int8>(n1 >> 24),
                                          /* [ 1] */ static_cast<sal_Int8>(( n1 << 8 ) >> 24),
                                          /* [ 2] */ static_cast<sal_Int8>(( n1 << 16 ) >> 24),
                                          /* [ 3] */ static_cast<sal_Int8>(( n1 << 24 ) >> 24),
                                          /* [ 4] */ static_cast<sal_Int8>(n2 >> 8),
                                          /* [ 5] */ static_cast<sal_Int8>(( n2 << 8 ) >> 8),
                                          /* [ 6] */ static_cast<sal_Int8>(n3 >> 8),
                                          /* [ 7] */ static_cast<sal_Int8>(( n3 << 8 ) >> 8),
                                          /* [ 8] */ static_cast<sal_Int8>(b8),
                                          /* [ 9] */ static_cast<sal_Int8>(b9),
                                          /* [10] */ static_cast<sal_Int8>(b10),
                                          /* [11] */ static_cast<sal_Int8>(b11),
                                          /* [12] */ static_cast<sal_Int8>(b12),
                                          /* [13] */ static_cast<sal_Int8>(b13),
                                          /* [14] */ static_cast<sal_Int8>(b14),
                                          /* [15] */ static_cast<sal_Int8>(b15) };
            return aResult;
        }
    }

    // OLinkedDocumentsAccess
    OLinkedDocumentsAccess::OLinkedDocumentsAccess( weld::Window* pDialogParent, const Reference< XDatabaseDocumentUI >& i_rDocumentUI,
        const Reference< XComponentContext >& _rxContext, const Reference< XNameAccess >& _rxContainer,
        const Reference< XConnection>& _xConnection, OUString _sDataSourceName )
        :m_xContext(_rxContext)
        ,m_xDocumentContainer(_rxContainer)
        ,m_xConnection(_xConnection)
        ,m_xDocumentUI( i_rDocumentUI )
        ,m_pDialogParent(pDialogParent)
        ,m_sDataSourceName(std::move(_sDataSourceName))
    {
        OSL_ENSURE(m_xContext.is(), "OLinkedDocumentsAccess::OLinkedDocumentsAccess: invalid service factory!");
        assert(m_pDialogParent && "OLinkedDocumentsAccess::OLinkedDocumentsAccess: really need a dialog parent!");
    }
    OLinkedDocumentsAccess::~OLinkedDocumentsAccess()
    {
    }
    Reference< XComponent> OLinkedDocumentsAccess::impl_open( const OUString& _rLinkName, Reference< XComponent >& _xDefinition,
        ElementOpenMode _eOpenMode, const ::comphelper::NamedValueCollection& _rAdditionalArgs )
    {
        Reference< XComponent> xRet;
        OSL_ENSURE(m_xDocumentContainer.is(), "OLinkedDocumentsAccess::OLinkedDocumentsAccess: invalid document container!");
        Reference< XComponentLoader > xComponentLoader(m_xDocumentContainer,UNO_QUERY);
        if ( !xComponentLoader.is() )
            return xRet;

        weld::WaitObject aWaitCursor(m_pDialogParent);

        ::comphelper::NamedValueCollection aArguments;
        OUString sOpenMode;
        switch ( _eOpenMode )
        {
            case ElementOpenMode::Normal:
                sOpenMode = "open";
                break;

            case ElementOpenMode::Mail:
                aArguments.put( "Hidden", true );
                [[fallthrough]];

            case ElementOpenMode::Design:
                sOpenMode = "openDesign";
                break;

            default:
                OSL_FAIL( "OLinkedDocumentsAccess::implOpen: invalid open mode!" );
                break;
        }
        aArguments.put( "OpenMode", sOpenMode );

        aArguments.put( PROPERTY_ACTIVE_CONNECTION, m_xConnection );

        Reference<XHierarchicalNameContainer> xHier(m_xDocumentContainer,UNO_QUERY);
        if ( xHier.is() && xHier->hasByHierarchicalName(_rLinkName) )
        {
            _xDefinition.set(xHier->getByHierarchicalName(_rLinkName),UNO_QUERY);
        }

        aArguments.merge( _rAdditionalArgs, true );

        xRet = xComponentLoader->loadComponentFromURL( _rLinkName, OUString(), 0, aArguments.getPropertyValues() );

        return xRet;
    }
    void OLinkedDocumentsAccess::impl_newWithPilot( const char* _pWizardService,
        const sal_Int32 _nCommandType, const OUString& _rObjectName )
    {
        try
        {
            ::comphelper::NamedValueCollection aArgs;
            aArgs.put( "DataSourceName", m_sDataSourceName );

            if ( m_xConnection.is() )
                aArgs.put( "ActiveConnection", m_xConnection );

            if ( !_rObjectName.isEmpty() && ( _nCommandType != -1 ) )
            {
                aArgs.put( "CommandType", _nCommandType );
                aArgs.put( "Command", _rObjectName );
            }

            aArgs.put( "DocumentUI", m_xDocumentUI );

            Reference< XJobExecutor > xWizard;
            {
                weld::WaitObject aWaitCursor(m_pDialogParent);
                xWizard.set( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                    OUString::createFromAscii( _pWizardService ),
                    aArgs.getWrappedPropertyValues(),
                    m_xContext
                    ), UNO_QUERY_THROW );
            }

            xWizard->trigger( "start" );
            ::comphelper::disposeComponent( xWizard );
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }
    void OLinkedDocumentsAccess::newFormWithPilot( const sal_Int32 _nCommandType,const OUString& _rObjectName )
    {
        impl_newWithPilot( "com.sun.star.wizards.form.CallFormWizard", _nCommandType, _rObjectName );
    }

    void OLinkedDocumentsAccess::newReportWithPilot( const sal_Int32 _nCommandType, const OUString& _rObjectName )
    {
        impl_newWithPilot( "com.sun.star.wizards.report.CallReportWizard", _nCommandType, _rObjectName );
    }
    void OLinkedDocumentsAccess::newTableWithPilot()
    {
        impl_newWithPilot( "com.sun.star.wizards.table.CallTableWizard", -1, OUString() );
    }
    void OLinkedDocumentsAccess::newQueryWithPilot()
    {
        impl_newWithPilot( "com.sun.star.wizards.query.CallQueryWizard", -1, OUString() );
    }
    Reference< XComponent > OLinkedDocumentsAccess::newDocument( sal_Int32 i_nActionID,
        const ::comphelper::NamedValueCollection& i_rCreationArgs, Reference< XComponent >& o_rDefinition )
    {
        OSL_ENSURE(m_xDocumentContainer.is(), "OLinkedDocumentsAccess::newDocument: invalid document container!");
        // determine the class ID to use for the new document
        Sequence<sal_Int8> aClassId;
        if  (   !i_rCreationArgs.has( "ClassID" )
            &&  !i_rCreationArgs.has( "MediaType" )
            &&  !i_rCreationArgs.has( "DocumentServiceName" )
            )
        {
            switch ( i_nActionID )
            {
                case ID_FORM_NEW_TEXT:
                    aClassId = lcl_GetSequenceClassID(SO3_SW_CLASSID);
                    OSL_ENSURE(aClassId == comphelper::MimeConfigurationHelper::GetSequenceClassID(SO3_SW_CLASSID),"Not equal");
                    break;

                case ID_FORM_NEW_CALC:
                    aClassId = lcl_GetSequenceClassID(SO3_SC_CLASSID);
                    break;

                case ID_FORM_NEW_IMPRESS:
                    aClassId = lcl_GetSequenceClassID(SO3_SIMPRESS_CLASSID);
                    break;

                case ID_REPORT_NEW_TEXT:
                    aClassId = comphelper::MimeConfigurationHelper::GetSequenceClassID(SO3_RPT_CLASSID_90);
                    break;

                default:
                    OSL_FAIL( "OLinkedDocumentsAccess::newDocument: please use newFormWithPilot!" );
                    return Reference< XComponent >();

            }
        }

        // load the document as template
        Reference< XComponent > xNewDocument;
        try
        {   // get the desktop object

            Reference<XMultiServiceFactory> xORB(m_xDocumentContainer,UNO_QUERY);
            if ( xORB.is() )
            {
                ::comphelper::NamedValueCollection aCreationArgs( i_rCreationArgs );
                if ( aClassId.hasElements() )
                    aCreationArgs.put( "ClassID", aClassId );
                aCreationArgs.put( PROPERTY_ACTIVE_CONNECTION, m_xConnection );

                // separate values which are real creation args from args relevant for opening the doc
                ::comphelper::NamedValueCollection aCommandArgs;
                if ( aCreationArgs.has( "Hidden" ) )
                {
                    aCommandArgs.put( "Hidden", aCreationArgs.get( "Hidden" ) );
                    aCreationArgs.remove( "Hidden" );
                }

                Reference< XCommandProcessor > xContent( xORB->createInstanceWithArguments(
                        SERVICE_SDB_DOCUMENTDEFINITION,
                        aCreationArgs.getWrappedPropertyValues()
                    ),
                    UNO_QUERY_THROW
                );
                o_rDefinition.set( xContent, UNO_QUERY );

                // put the OpenMode into the OpenArgs
                OpenCommandArgument aOpenModeArg;
                aOpenModeArg.Mode = OpenMode::DOCUMENT;
                aCommandArgs.put( "OpenMode", aOpenModeArg );

                Command aCommand;
                aCommand.Name = "openDesign";
                aCommand.Argument <<= aCommandArgs.getPropertyValues();
                weld::WaitObject aWaitCursor(m_pDialogParent);
                xNewDocument.set( xContent->execute( aCommand, xContent->createCommandIdentifier(), nullptr ), UNO_QUERY );
            }
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }

        return xNewDocument;
    }

    Reference< XComponent > OLinkedDocumentsAccess::open( const OUString& _rLinkName, Reference< XComponent >& _xDefinition,
        ElementOpenMode _eOpenMode, const ::comphelper::NamedValueCollection& _rAdditionalArgs )
    {
        dbtools::SQLExceptionInfo aInfo;
        Reference< XComponent > xRet;
        try
        {
            xRet = impl_open( _rLinkName, _xDefinition, _eOpenMode, _rAdditionalArgs );
            if ( !xRet.is() )
            {
                OUString sMessage = DBA_RES(STR_COULDNOTOPEN_LINKEDDOC);
                sMessage = sMessage.replaceFirst("$file$",_rLinkName);

                css::sdbc::SQLException aSQLException(sMessage, {}, {}, 0, {});
                aInfo = dbtools::SQLExceptionInfo(aSQLException);
            }
        }
        catch(const css::io::WrongFormatException &e)
        {
            css::sdbc::SQLException aSQLException(e.Message, e.Context, {}, 0, {});
            aInfo = dbtools::SQLExceptionInfo(aSQLException);

            // more like a hack, insert an empty message
            OUString sText( DBA_RES( RID_STR_EXTENSION_NOT_PRESENT ) );
            sText = sText.replaceFirst("$file$",_rLinkName);
            aInfo.prepend(sText);

            OUString sMessage = DBA_RES(STR_COULDNOTOPEN_LINKEDDOC);
            sMessage = sMessage.replaceFirst("$file$",_rLinkName);
            aInfo.prepend(sMessage);
        }
        catch(const Exception& e)
        {
            Any aAny = ::cppu::getCaughtException();
            css::sdbc::SQLException a;
            if ( !(aAny >>= a) || (a.ErrorCode != dbtools::ParameterInteractionCancelled) )
            {
                css::sdbc::SQLException aSQLException(e.Message, e.Context, {}, 0, {});
                aInfo = dbtools::SQLExceptionInfo(aSQLException);

                // more like a hack, insert an empty message
                aInfo.prepend(" \n");

                OUString sMessage = DBA_RES(STR_COULDNOTOPEN_LINKEDDOC);
                sMessage = sMessage.replaceFirst("$file$",_rLinkName);
                aInfo.prepend(sMessage);
            }
        }
        if (aInfo.isValid())
        {
            showError(aInfo, m_pDialogParent->GetXWindow(), m_xContext);
        }
        return xRet;
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
