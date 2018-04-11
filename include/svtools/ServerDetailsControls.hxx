/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SVTOOLS_SERVERDETAILSCONTROLS_HXX
#define INCLUDED_SVTOOLS_SERVERDETAILSCONTROLS_HXX

#include <map>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/task/PasswordContainer.hpp>
#include <com/sun/star/task/XPasswordContainer2.hpp>

#include <tools/urlobj.hxx>
#include <vcl/builder.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/weld.hxx>

class DetailsContainer
{
    protected:
        Link<DetailsContainer*,void> m_aChangeHdl;
        std::unique_ptr<weld::Widget> m_xDetailsGrid;
        std::unique_ptr<weld::Widget> m_xHostBox;
        std::unique_ptr<weld::Entry> m_xEDHost;
        std::unique_ptr<weld::Label> m_xFTHost;
        std::unique_ptr<weld::SpinButton> m_xEDPort;
        std::unique_ptr<weld::Label> m_xFTPort;
        std::unique_ptr<weld::Entry> m_xEDRoot;
        std::unique_ptr<weld::Label> m_xFTRoot;

    public:
        DetailsContainer( weld::Builder* pBuilder );
        virtual ~DetailsContainer( );

        void setChangeHdl( const Link<DetailsContainer*,void>& rLink ) { m_aChangeHdl = rLink; }

        virtual void show( bool bShow = true );
        virtual INetURLObject getUrl( );

        /** Try to split the URL in the controls of that container.

            \param sUrl the URL to split
            \return true if the split worked, false otherwise.
         */
        virtual bool setUrl( const INetURLObject& rUrl );

        virtual void setUsername( const OUString& /*rUsername*/ ) { };
        virtual void setPassword( const OUString& ) { };

        virtual bool enableUserCredentials( ) { return true; };

    protected:
        void notifyChange( );
        DECL_LINK(ValueChangeHdl, weld::Entry&, void);
        DECL_LINK(FormatPortHdl, weld::SpinButton&, void);
};

class HostDetailsContainer : public DetailsContainer
{
    private:
        sal_uInt16 m_nDefaultPort;
        OUString m_sScheme;
        OUString m_sHost;

    public:
        HostDetailsContainer( weld::Builder* pBuilder, sal_uInt16 nPort, const OUString& sScheme );

        virtual void show( bool bShow = true ) override;
        virtual INetURLObject getUrl( ) override;
        virtual bool setUrl( const INetURLObject& rUrl ) override;

    protected:
        void setScheme( const OUString& sScheme ) { m_sScheme = sScheme; }

        /** Verifies that the scheme split from the URL can be handled by
            the container and set the proper controls accordingly if needed.
          */
        virtual bool verifyScheme( const OUString& rScheme );
};

class DavDetailsContainer : public HostDetailsContainer
{
    private:
        std::unique_ptr<weld::CheckButton> m_xCBDavs;

    public:
        DavDetailsContainer(weld::Builder* pBuilder);

        virtual void show( bool bShow = true ) override;
        virtual bool enableUserCredentials( ) override { return false; };

    protected:
        virtual bool verifyScheme( const OUString& rScheme ) override;

    private:
        DECL_LINK(ToggledDavsHdl, weld::ToggleButton&, void);
};

class SmbDetailsContainer : public DetailsContainer
{
    private:
        std::unique_ptr<weld::Entry> m_xEDShare;
        std::unique_ptr<weld::Label> m_xFTShare;

    public:
        SmbDetailsContainer(weld::Builder* pBuilder);

        virtual INetURLObject getUrl( ) override;
        virtual bool setUrl( const INetURLObject& rUrl ) override;
        virtual void show( bool bShow = true ) override;
};

class CmisDetailsContainer : public DetailsContainer
{
    private:
        OUString m_sUsername;
        OUString m_sPassword;
        css::uno::Reference< css::ucb::XCommandEnvironment > m_xCmdEnv;
        std::vector< OUString > m_aRepoIds;
        OUString m_sRepoId;
        OUString m_sBinding;
        css::uno::Reference< css::awt::XWindow > m_xParentDialog;

        std::unique_ptr<weld::Widget> m_xRepositoryBox;
        std::unique_ptr<weld::Label> m_xFTRepository;
        std::unique_ptr<weld::ComboBoxText> m_xLBRepository;
        std::unique_ptr<weld::Button> m_xBTRepoRefresh;

    public:
        CmisDetailsContainer(weld::Builder* pBuilder, weld::Dialog* pParentDialog, OUString const & sBinding);

        virtual void show( bool bShow = true ) override;
        virtual INetURLObject getUrl( ) override;
        virtual bool setUrl( const INetURLObject& rUrl ) override;
        virtual void setUsername( const OUString& rUsername ) override;
        virtual void setPassword( const OUString& rPass ) override;

    private:
        void selectRepository( );
        DECL_LINK ( RefreshReposHdl, weld::Button&, void );
        DECL_LINK ( SelectRepoHdl, weld::ComboBoxText&, void );
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
