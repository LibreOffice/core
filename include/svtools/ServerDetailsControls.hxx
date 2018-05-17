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
#include <vcl/weld.hxx>

class PlaceEditDialog;

class DetailsContainer
{
    protected:
        PlaceEditDialog* m_pDialog;
        Link<DetailsContainer*,void> m_aChangeHdl;

    public:
        DetailsContainer(PlaceEditDialog* pDialog);
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
        HostDetailsContainer(PlaceEditDialog* pDialog, sal_uInt16 nPort, const OUString& sScheme);

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
    public:
        DavDetailsContainer(PlaceEditDialog* pDialog);

        virtual void show( bool bShow = true ) override;
        virtual bool enableUserCredentials( ) override { return false; };

    protected:
        virtual bool verifyScheme( const OUString& rScheme ) override;

    private:
        DECL_LINK(ToggledDavsHdl, weld::ToggleButton&, void);
};

class SmbDetailsContainer : public DetailsContainer
{
    public:
        SmbDetailsContainer(PlaceEditDialog* pDialog);

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

    public:
        CmisDetailsContainer(PlaceEditDialog* pDialog, OUString const & sBinding);

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
