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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "linkeddocuments.hxx"
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include <unotools/confignode.hxx>
#include "dbustrings.hrc"
#include <comphelper/classids.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/OpenCommandArgument.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include <vcl/msgbox.hxx>
#include <ucbhelper/content.hxx>
#include "dbu_misc.hrc"
#include <svl/filenotation.hxx>
#include "browserids.hxx"
#include <sfx2/new.hxx>
#include "moduledbu.hxx"
// -----------------
// for calling basic
#include <sfx2/app.hxx>
#include <basic/sbx.hxx>
#include <basic/sbuno.hxx>
#include <svtools/ehdl.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#include <vcl/waitobj.hxx>
#include <comphelper/mimeconfighelper.hxx>

#include <cppuhelper/exc_hlp.hxx>
#include <connectivity/dbtools.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/io/WrongFormatException.hpp>
#include "com/sun/star/sdb/RowSetVetoException.hpp"

//......................................................................
namespace dbaui
{
//......................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb::application;
    using namespace ::com::sun::star::task;
    using namespace ::svt;

    namespace
    {
        Sequence< sal_Int8 > lcl_GetSequenceClassID( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                                                    sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                                                    sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 )
        {
            Sequence< sal_Int8 > aResult( 16 );
            aResult[0] = static_cast<sal_Int8>(n1 >> 24);
            aResult[1] = static_cast<sal_Int8>(( n1 << 8 ) >> 24);
            aResult[2] = static_cast<sal_Int8>(( n1 << 16 ) >> 24);
            aResult[3] = static_cast<sal_Int8>(( n1 << 24 ) >> 24);
            aResult[4] = static_cast<sal_Int8>(n2 >> 8);
            aResult[5] = static_cast<sal_Int8>(( n2 << 8 ) >> 8);
            aResult[6] = static_cast<sal_Int8>(n3 >> 8);
            aResult[7] = static_cast<sal_Int8>(( n3 << 8 ) >> 8);
            aResult[8] = b8;
            aResult[9] = b9;
            aResult[10] = b10;
            aResult[11] = b11;
            aResult[12] = b12;
            aResult[13] = b13;
            aResult[14] = b14;
            aResult[15] = b15;

            return aResult;
        }
    }


    //==================================================================
    //= OLinkedDocumentsAccess
    //==================================================================
    DBG_NAME(OLinkedDocumentsAccess)
    //------------------------------------------------------------------
    OLinkedDocumentsAccess::OLinkedDocumentsAccess( Window* _pDialogParent, const Reference< XDatabaseDocumentUI >& i_rDocumentUI,
        const Reference< XMultiServiceFactory >& _rxORB, const Reference< XNameAccess >& _rxContainer,
        const Reference< XConnection>& _xConnection, const ::rtl::OUString& _sDataSourceName )
        :m_xORB(_rxORB)
        ,m_xDocumentContainer(_rxContainer)
        ,m_xConnection(_xConnection)
        ,m_xDocumentUI( i_rDocumentUI )
        ,m_pDialogParent(_pDialogParent)
        ,m_sDataSourceName(_sDataSourceName)
    {
        DBG_CTOR(OLinkedDocumentsAccess,NULL);
        OSL_ENSURE(m_xORB.is(), "OLinkedDocumentsAccess::OLinkedDocumentsAccess: invalid service factory!");
        OSL_ENSURE(m_pDialogParent, "OLinkedDocumentsAccess::OLinkedDocumentsAccess: really need a dialog parent!");
    }
    //------------------------------------------------------------------
    OLinkedDocumentsAccess::~OLinkedDocumentsAccess()
    {
        DBG_DTOR(OLinkedDocumentsAccess,NULL);
    }
    //------------------------------------------------------------------
    Reference< XComponent> OLinkedDocumentsAccess::impl_open( const ::rtl::OUString& _rLinkName, Reference< XComponent >& _xDefinition,
        ElementOpenMode _eOpenMode, const ::comphelper::NamedValueCollection& _rAdditionalArgs )
    {
        Reference< XComponent> xRet;
        OSL_ENSURE(m_xDocumentContainer.is(), "OLinkedDocumentsAccess::OLinkedDocumentsAccess: invalid document container!");
        Reference< XComponentLoader > xComponentLoader(m_xDocumentContainer,UNO_QUERY);
        if ( !xComponentLoader.is() )
            return xRet;

        WaitObject aWaitCursor( m_pDialogParent );

        ::comphelper::NamedValueCollection aArguments;
        ::rtl::OUString sOpenMode;
        switch ( _eOpenMode )
        {
            case E_OPEN_NORMAL:
                sOpenMode = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) );
                break;

            case E_OPEN_FOR_MAIL:
                aArguments.put( "Hidden", true );
                // fall through

            case E_OPEN_DESIGN:
                sOpenMode = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "openDesign" ) );
                break;

            default:
                OSL_FAIL( "OLinkedDocumentsAccess::implOpen: invalid open mode!" );
                break;
        }
        aArguments.put( "OpenMode", sOpenMode );

        aArguments.put( (::rtl::OUString)PROPERTY_ACTIVE_CONNECTION, m_xConnection );
        try
        {
            Reference<XHierarchicalNameContainer> xHier(m_xDocumentContainer,UNO_QUERY);
            if ( xHier.is() && xHier->hasByHierarchicalName(_rLinkName) )
            {
                _xDefinition.set(xHier->getByHierarchicalName(_rLinkName),UNO_QUERY);
            }

            aArguments.merge( _rAdditionalArgs, true );

            xRet = xComponentLoader->loadComponentFromURL( _rLinkName, ::rtl::OUString(), 0, aArguments.getPropertyValues() );
        }
        catch(Exception& e)
        {
            (void)e;
            throw;
        }

        return xRet;
    }
    //------------------------------------------------------------------
    void OLinkedDocumentsAccess::impl_newWithPilot( const char* _pWizardService,
        const sal_Int32 _nCommandType, const ::rtl::OUString& _rObjectName )
    {
        try
        {
            ::comphelper::NamedValueCollection aArgs;
            aArgs.put( "DataSourceName", m_sDataSourceName );

            if ( m_xConnection.is() )
                aArgs.put( "ActiveConnection", m_xConnection );

            if ( _rObjectName.getLength() && ( _nCommandType != -1 ) )
            {
                aArgs.put( "CommandType", _nCommandType );
                aArgs.put( "Command", _rObjectName );
            }

            aArgs.put( "DocumentUI", m_xDocumentUI );

            Reference< XJobExecutor > xWizard;
            {
                WaitObject aWaitCursor( m_pDialogParent );
                xWizard.set( m_xORB->createInstanceWithArguments(
                    ::rtl::OUString::createFromAscii( _pWizardService ),
                    aArgs.getWrappedPropertyValues()
                    ), UNO_QUERY_THROW );
            }

            xWizard->trigger( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "start" ) ) );
            ::comphelper::disposeComponent( xWizard );
        }
        catch(const Exception& e)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    //------------------------------------------------------------------
    void OLinkedDocumentsAccess::newFormWithPilot( const sal_Int32 _nCommandType,const ::rtl::OUString& _rObjectName )
    {
        impl_newWithPilot( "com.sun.star.wizards.form.CallFormWizard", _nCommandType, _rObjectName );
    }

    //------------------------------------------------------------------
    void OLinkedDocumentsAccess::newReportWithPilot( const sal_Int32 _nCommandType, const ::rtl::OUString& _rObjectName )
    {
        impl_newWithPilot( "com.sun.star.wizards.report.CallReportWizard", _nCommandType, _rObjectName );
    }
    //------------------------------------------------------------------
    void OLinkedDocumentsAccess::newTableWithPilot()
    {
        impl_newWithPilot( "com.sun.star.wizards.table.CallTableWizard", -1, ::rtl::OUString() );
    }
    //------------------------------------------------------------------
    void OLinkedDocumentsAccess::newQueryWithPilot()
    {
        impl_newWithPilot( "com.sun.star.wizards.query.CallQueryWizard", -1, ::rtl::OUString() );
    }
    //------------------------------------------------------------------
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
                if ( aClassId.getLength() )
                    aCreationArgs.put( "ClassID", aClassId );
                aCreationArgs.put( (::rtl::OUString)PROPERTY_ACTIVE_CONNECTION, m_xConnection );

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
                aCommand.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "openDesign" ) );
                aCommand.Argument <<= aCommandArgs.getPropertyValues();
                WaitObject aWaitCursor( m_pDialogParent );
                xNewDocument.set( xContent->execute( aCommand, xContent->createCommandIdentifier(), NULL ), UNO_QUERY );
            }
        }
        catch(const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return xNewDocument;
    }

    //------------------------------------------------------------------
    Reference< XComponent > OLinkedDocumentsAccess::open( const ::rtl::OUString& _rLinkName, Reference< XComponent >& _xDefinition,
        ElementOpenMode _eOpenMode, const ::comphelper::NamedValueCollection& _rAdditionalArgs )
    {
        dbtools::SQLExceptionInfo aInfo;
        Reference< XComponent > xRet;
        try
        {
            xRet = impl_open( _rLinkName, _xDefinition, _eOpenMode, _rAdditionalArgs );
            if ( !xRet.is() )
            {
                String sMessage = String(ModuleRes(STR_COULDNOTOPEN_LINKEDDOC));
                sMessage.SearchAndReplaceAscii("$file$",_rLinkName);

                com::sun::star::sdbc::SQLException aSQLException;
                aSQLException.Message = sMessage;
                aInfo = dbtools::SQLExceptionInfo(aSQLException);
            }
            return xRet;
        }
        catch (com::sun::star::io::WrongFormatException &e)
        {
            com::sun::star::sdbc::SQLException aSQLException;
            aSQLException.Message = e.Message;
            aSQLException.Context = e.Context;
            aInfo = dbtools::SQLExceptionInfo(aSQLException);

            // more like a hack, insert an empty message
            String sText( ModuleRes( RID_STR_EXTENSION_NOT_PRESENT ) );
            sText.SearchAndReplaceAscii("$file$",_rLinkName);
            aInfo.prepend(sText);

            String sMessage = String(ModuleRes(STR_COULDNOTOPEN_LINKEDDOC));
            sMessage.SearchAndReplaceAscii("$file$",_rLinkName);
            aInfo.prepend(sMessage);
        }
        catch(Exception& e)
        {
            Any aAny = ::cppu::getCaughtException();
            com::sun::star::sdbc::SQLException a;
            if ( !(aAny >>= a) || (a.ErrorCode != dbtools::ParameterInteractionCancelled) )
            {
                com::sun::star::sdbc::SQLException aSQLException;
                aSQLException.Message = e.Message;
                aSQLException.Context = e.Context;
                aInfo = dbtools::SQLExceptionInfo(aSQLException);

                // more like a hack, insert an empty message
                aInfo.prepend(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" \n")));

                String sMessage = String(ModuleRes(STR_COULDNOTOPEN_LINKEDDOC));
                sMessage.SearchAndReplaceAscii("$file$",_rLinkName);
                aInfo.prepend(sMessage);
            }
        }
        if (aInfo.isValid())
        {
            showError(aInfo, VCLUnoHelper::GetInterface(m_pDialogParent), m_xORB );
        }
        return xRet;
    }


//......................................................................
}   // namespace dbaui
//......................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
