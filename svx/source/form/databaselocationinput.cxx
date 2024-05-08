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


#include <svx/databaselocationinput.hxx>
#include <svx/dialmgr.hxx>

#include <svx/strings.hrc>

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svl/filenotation.hxx>
#include <svtools/inettbc.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <unotools/confignode.hxx>
#include <unotools/ucbhelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

namespace svx
{
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;

    namespace TemplateDescription = ::com::sun::star::ui::dialogs::TemplateDescription;

    class DatabaseLocationInputController_Impl
    {
    public:
        DatabaseLocationInputController_Impl(
            const Reference<XComponentContext>&     _rContext,
            SvtURLBox& _rLocationInput,
            weld::Button& _rBrowseButton,
            weld::Window& _rDialog
        );

        bool     prepareCommit();
        void     setURL( const OUString& _rURL );
        OUString getURL() const;

    private:
        void     impl_initFilterProperties_nothrow();
        void     impl_onBrowseButtonClicked();
        OUString impl_getCurrentURL() const;

        DECL_LINK( OnButtonAction, weld::Button&, void );

    private:
        const Reference<XComponentContext>      m_xContext;
        SvtURLBox& m_rLocationInput;
        weld::Window& m_rDialog;
        Sequence< OUString >             m_aFilterExtensions;
        OUString                         m_sFilterUIName;
        bool                                    m_bNeedExistenceCheck;
    };

    DatabaseLocationInputController_Impl::DatabaseLocationInputController_Impl(const Reference<XComponentContext>& _rContext,
            SvtURLBox& _rLocationInput, weld::Button& _rBrowseButton, weld::Window& _rDialog)
        :m_xContext( _rContext )
        ,m_rLocationInput( _rLocationInput )
        ,m_rDialog( _rDialog )
        ,m_bNeedExistenceCheck( true )
    {
        impl_initFilterProperties_nothrow();

        // forward the allowed extensions to the input control
        OUStringBuffer aExtensionList;
        for (auto const& extension : m_aFilterExtensions)
        {
            aExtensionList.append( extension + ";" );
        }
        m_rLocationInput.SetFilter( aExtensionList.makeStringAndClear() );
        _rBrowseButton.connect_clicked(LINK(this, DatabaseLocationInputController_Impl, OnButtonAction));
    }

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
                std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(m_rLocationInput.getWidget(),
                                                               VclMessageType::Question, VclButtonsType::YesNo,
                                                               SvxResId(RID_STR_ALREADYEXISTOVERWRITE)));
                if (xQueryBox->run() != RET_YES)
                    return false;
            }
        }

        return true;
    }

    void DatabaseLocationInputController_Impl::setURL( const OUString& _rURL )
    {
        ::svt::OFileNotation aTransformer( _rURL );
        m_rLocationInput.set_entry_text( aTransformer.get( ::svt::OFileNotation::N_SYSTEM ) );
    }

    OUString DatabaseLocationInputController_Impl::getURL() const
    {
        return impl_getCurrentURL();
    }

    void DatabaseLocationInputController_Impl::impl_initFilterProperties_nothrow()
    {
        try
        {
            // get the name of the default filter for database documents
            ::utl::OConfigurationTreeRoot aConfig(
                ::utl::OConfigurationTreeRoot::createWithComponentContext(
                    m_xContext,
                    u"/org.openoffice.Setup/Office/Factories/com.sun.star.sdb.OfficeDatabaseDocument"_ustr
            ) );
            OUString sDatabaseFilter;
            OSL_VERIFY( aConfig.getNodeValue( u"ooSetupFactoryActualFilter"_ustr ) >>= sDatabaseFilter );

            // get the type this filter is responsible for
            Reference< XNameAccess > xFilterFactory(
                m_xContext->getServiceManager()->createInstanceWithContext(u"com.sun.star.document.FilterFactory"_ustr, m_xContext),
                UNO_QUERY_THROW );
            ::comphelper::NamedValueCollection aFilterProperties( xFilterFactory->getByName( sDatabaseFilter ) );
            OUString sDocumentType = aFilterProperties.getOrDefault( u"Type"_ustr, OUString() );

            // get the extension(s) for this type
            Reference< XNameAccess > xTypeDetection(
                m_xContext->getServiceManager()->createInstanceWithContext(u"com.sun.star.document.TypeDetection"_ustr, m_xContext),
                UNO_QUERY_THROW );

            ::comphelper::NamedValueCollection aTypeProperties( xTypeDetection->getByName( sDocumentType ) );
            m_aFilterExtensions = aTypeProperties.getOrDefault( u"Extensions"_ustr, m_aFilterExtensions );
            m_sFilterUIName = aTypeProperties.getOrDefault( u"UIName"_ustr, m_sFilterUIName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }

        // ensure we have at least one extension
        OSL_ENSURE( m_aFilterExtensions.hasElements(),
            "DatabaseLocationInputController_Impl::impl_initFilterProperties_nothrow: unable to determine the file extension(s)!" );
        if ( !m_aFilterExtensions.hasElements() )
        {
            m_aFilterExtensions = { u"*.odb"_ustr };
        }
    }

    IMPL_LINK_NOARG(DatabaseLocationInputController_Impl, OnButtonAction, weld::Button&, void)
    {
        impl_onBrowseButtonClicked();
    }

    OUString DatabaseLocationInputController_Impl::impl_getCurrentURL() const
    {
        OUString sCurrentFile( m_rLocationInput.get_active_text() );
        if ( !sCurrentFile.isEmpty() )
        {
            ::svt::OFileNotation aCurrentFile( sCurrentFile );
            sCurrentFile = aCurrentFile.get( ::svt::OFileNotation::N_URL );
        }
        return sCurrentFile;
    }

    void DatabaseLocationInputController_Impl::impl_onBrowseButtonClicked()
    {
        ::sfx2::FileDialogHelper aFileDlg(
            TemplateDescription::FILESAVE_AUTOEXTENSION,
            FileDialogFlags::NONE,
            &m_rDialog
        );
        aFileDlg.SetDisplayDirectory( impl_getCurrentURL() );

        aFileDlg.AddFilter( m_sFilterUIName, "*." + m_aFilterExtensions[0] );
        aFileDlg.SetCurrentFilter( m_sFilterUIName );

        if ( aFileDlg.Execute() == ERRCODE_NONE )
        {
            INetURLObject aURL( aFileDlg.GetPath() );
            if( aURL.GetProtocol() != INetProtocol::NotValid )
            {
                ::svt::OFileNotation aFileNotation( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
                m_rLocationInput.set_entry_text(aFileNotation.get(::svt::OFileNotation::N_SYSTEM));
                m_rLocationInput.trigger_changed();
                // the dialog already checked for the file's existence, so we don't need to, again
                m_bNeedExistenceCheck = false;
            }
        }
    }

    DatabaseLocationInputController::DatabaseLocationInputController( const Reference<XComponentContext>& _rContext,
            SvtURLBox& _rLocationInput, weld::Button& _rBrowseButton, weld::Window& _rDialog )
        :m_pImpl( new DatabaseLocationInputController_Impl( _rContext, _rLocationInput, _rBrowseButton, _rDialog ) )
    {
    }

    DatabaseLocationInputController::~DatabaseLocationInputController()
    {
    }

    bool DatabaseLocationInputController::prepareCommit()
    {
        return m_pImpl->prepareCommit();
    }

    void DatabaseLocationInputController::setURL( const OUString& _rURL )
    {
        m_pImpl->setURL( _rURL );
    }

    OUString DatabaseLocationInputController::getURL() const
    {
        return m_pImpl->getURL();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
