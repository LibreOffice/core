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


#include "svx/databaselocationinput.hxx"
#include "svx/dialmgr.hxx"

#include "svx/fmresids.hrc"

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svtools/urlcontrol.hxx>
#include <svl/filenotation.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/confignode.hxx>
#include <unotools/ucbhelper.hxx>
#include <vcl/button.hxx>
#include <vcl/msgbox.hxx>

//........................................................................
namespace svx
{
//........................................................................

    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;

    namespace TemplateDescription = ::com::sun::star::ui::dialogs::TemplateDescription;

    //====================================================================
    //= DatabaseLocationInputController_Impl
    //====================================================================
    class DatabaseLocationInputController_Impl
    {
    public:
        DatabaseLocationInputController_Impl(
            const Reference<XComponentContext>&     _rContext,
            ::svt::OFileURLControl&                 _rLocationInput,
            PushButton&                             _rBrowseButton
        );
        ~DatabaseLocationInputController_Impl();

        bool     prepareCommit();
        void     setURL( const OUString& _rURL );
        OUString getURL() const;

    private:
        void     impl_initFilterProperties_nothrow();
        void     impl_onBrowseButtonClicked();
        void     impl_onLocationModified();
        OUString impl_getCurrentURL() const;

        DECL_LINK( OnControlAction, VclWindowEvent* );

    private:
        const Reference<XComponentContext>      m_xContext;
        ::svt::OFileURLControl&                 m_rLocationInput;
        PushButton&                             m_rBrowseButton;
        Sequence< OUString >             m_aFilterExtensions;
        OUString                         m_sFilterUIName;
        bool                                    m_bNeedExistenceCheck;
    };

    //--------------------------------------------------------------------
    DatabaseLocationInputController_Impl::DatabaseLocationInputController_Impl( const Reference<XComponentContext>& _rContext,
            ::svt::OFileURLControl& _rLocationInput, PushButton& _rBrowseButton )
        :m_xContext( _rContext )
        ,m_rLocationInput( _rLocationInput )
        ,m_rBrowseButton( _rBrowseButton )
        ,m_aFilterExtensions()
        ,m_sFilterUIName()
        ,m_bNeedExistenceCheck( true )
    {
        impl_initFilterProperties_nothrow();

        // forward the allowed extensions to the input control
        OUStringBuffer aExtensionList;
        for (   const OUString* pExtension = m_aFilterExtensions.getConstArray();
                pExtension != m_aFilterExtensions.getConstArray() + m_aFilterExtensions.getLength();
                ++pExtension
            )
        {
            aExtensionList.append( *pExtension );
            aExtensionList.append( (sal_Unicode)';' );
        }
        m_rLocationInput.SetFilter( aExtensionList.makeStringAndClear() );

        m_rBrowseButton.AddEventListener( LINK( this, DatabaseLocationInputController_Impl, OnControlAction ) );
        m_rLocationInput.AddEventListener( LINK( this, DatabaseLocationInputController_Impl, OnControlAction ) );
    }

    //--------------------------------------------------------------------
    DatabaseLocationInputController_Impl::~DatabaseLocationInputController_Impl()
    {
        m_rBrowseButton.RemoveEventListener( LINK( this, DatabaseLocationInputController_Impl, OnControlAction ) );
        m_rLocationInput.RemoveEventListener( LINK( this, DatabaseLocationInputController_Impl, OnControlAction ) );
    }

    //--------------------------------------------------------------------
    bool DatabaseLocationInputController_Impl::prepareCommit()
    {
        OUString sURL( impl_getCurrentURL() );
        if ( sURL.isEmpty() )
            return false;

        // check if the name exists
        if ( m_bNeedExistenceCheck )
        {
            if ( ::utl::UCBContentHelper::Exists( sURL ) )
            {
                QueryBox aBox( m_rLocationInput.GetSystemWindow(), WB_YES_NO, SVX_RESSTR(RID_STR_ALREADYEXISTOVERWRITE) );
                if ( aBox.Execute() != RET_YES )
                    return false;
            }
        }

        return true;
    }

    //--------------------------------------------------------------------
    void DatabaseLocationInputController_Impl::setURL( const OUString& _rURL )
    {
        ::svt::OFileNotation aTransformer( _rURL );
        m_rLocationInput.SetText( aTransformer.get( ::svt::OFileNotation::N_SYSTEM ) );
    }

    //--------------------------------------------------------------------
    OUString DatabaseLocationInputController_Impl::getURL() const
    {
        return impl_getCurrentURL();
    }

    //--------------------------------------------------------------------
    void DatabaseLocationInputController_Impl::impl_initFilterProperties_nothrow()
    {
        try
        {
            // get the name of the default filter for database documents
            ::utl::OConfigurationTreeRoot aConfig(
                ::utl::OConfigurationTreeRoot::createWithComponentContext(
                    m_xContext,
                    OUString( "/org.openoffice.Setup/Office/Factories/com.sun.star.sdb.OfficeDatabaseDocument" )
            ) );
            OUString sDatabaseFilter;
            OSL_VERIFY( aConfig.getNodeValue( "ooSetupFactoryActualFilter" ) >>= sDatabaseFilter );

            // get the type this filter is responsible for
            Reference< XNameAccess > xFilterFactory(
                m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.FilterFactory", m_xContext),
                UNO_QUERY_THROW );
            ::comphelper::NamedValueCollection aFilterProperties( xFilterFactory->getByName( sDatabaseFilter ) );
            OUString sDocumentType = aFilterProperties.getOrDefault( "Type", OUString() );

            // get the extension(s) for this type
            Reference< XNameAccess > xTypeDetection(
                m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", m_xContext),
                UNO_QUERY_THROW );

            ::comphelper::NamedValueCollection aTypeProperties( xTypeDetection->getByName( sDocumentType ) );
            m_aFilterExtensions = aTypeProperties.getOrDefault( "Extensions", m_aFilterExtensions );
            m_sFilterUIName = aTypeProperties.getOrDefault( "UIName", m_sFilterUIName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        // ensure we have at least one extension
        OSL_ENSURE( m_aFilterExtensions.getLength(),
            "DatabaseLocationInputController_Impl::impl_initFilterProperties_nothrow: unable to determine the file extension(s)!" );
        if ( m_aFilterExtensions.getLength() == 0 )
        {
            m_aFilterExtensions.realloc(1);
            m_aFilterExtensions[0] = OUString( "*.odb" );
        }
    }

    // -----------------------------------------------------------------------------
    IMPL_LINK( DatabaseLocationInputController_Impl, OnControlAction, VclWindowEvent*, _pEvent )
    {
        if  (   ( _pEvent->GetWindow() == &m_rBrowseButton )
            &&  ( _pEvent->GetId() == VCLEVENT_BUTTON_CLICK )
            )
        {
            impl_onBrowseButtonClicked();
        }

        if  (   ( _pEvent->GetWindow() == &m_rLocationInput )
            &&  ( _pEvent->GetId() == VCLEVENT_EDIT_MODIFY )
            )
        {
            impl_onLocationModified();
        }

        return 0L;
    }

    // -----------------------------------------------------------------------------
    OUString DatabaseLocationInputController_Impl::impl_getCurrentURL() const
    {
        OUString sCurrentFile( m_rLocationInput.GetText() );
        if ( !sCurrentFile.isEmpty() )
        {
            ::svt::OFileNotation aCurrentFile( sCurrentFile );
            sCurrentFile = aCurrentFile.get( ::svt::OFileNotation::N_URL );
        }
        return sCurrentFile;
    }

    // -----------------------------------------------------------------------------
    void DatabaseLocationInputController_Impl::impl_onBrowseButtonClicked()
    {
        ::sfx2::FileDialogHelper aFileDlg(
            TemplateDescription::FILESAVE_AUTOEXTENSION,
            0,
            m_rLocationInput.GetSystemWindow()
        );
        aFileDlg.SetDisplayDirectory( impl_getCurrentURL() );

        aFileDlg.AddFilter( m_sFilterUIName, OUStringBuffer().appendAscii( "*." ).append( m_aFilterExtensions[0] ).makeStringAndClear() );
        aFileDlg.SetCurrentFilter( m_sFilterUIName );

        if ( aFileDlg.Execute() == ERRCODE_NONE )
        {
            INetURLObject aURL( aFileDlg.GetPath() );
            if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
            {
                ::svt::OFileNotation aFileNotation( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
                m_rLocationInput.SetText( aFileNotation.get( ::svt::OFileNotation::N_SYSTEM ) );
                m_rLocationInput.GetModifyHdl().Call( &m_rLocationInput );
                // the dialog already checked for the file's existence, so we don't need to, again
                m_bNeedExistenceCheck = false;
            }
        }
    }

    // -----------------------------------------------------------------------------
    void DatabaseLocationInputController_Impl::impl_onLocationModified()
    {
        m_bNeedExistenceCheck = true;
    }

    //====================================================================
    //= DatabaseLocationInputController
    //====================================================================
    //--------------------------------------------------------------------
    DatabaseLocationInputController::DatabaseLocationInputController( const Reference<XComponentContext>& _rContext,
            ::svt::OFileURLControl& _rLocationInput, PushButton& _rBrowseButton )
        :m_pImpl( new DatabaseLocationInputController_Impl( _rContext, _rLocationInput, _rBrowseButton ) )
    {
    }

    //--------------------------------------------------------------------
    DatabaseLocationInputController::~DatabaseLocationInputController()
    {
    }

    //--------------------------------------------------------------------
    bool DatabaseLocationInputController::prepareCommit()
    {
        return m_pImpl->prepareCommit();
    }

    //--------------------------------------------------------------------
    void DatabaseLocationInputController::setURL( const OUString& _rURL )
    {
        m_pImpl->setURL( _rURL );
    }

    //--------------------------------------------------------------------
    OUString DatabaseLocationInputController::getURL() const
    {
        return m_pImpl->getURL();
    }

//........................................................................
} // namespace svx
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
