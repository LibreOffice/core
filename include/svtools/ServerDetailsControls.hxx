/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SERVERDETAILSCONTROLS_HXX
#define _SERVERDETAILSCONTROLS_HXX

#include <map>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>

#include <tools/urlobj.hxx>
#include <vcl/builder.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>

class DetailsContainer
{
    private:
        Link m_aChangeHdl;
        VclFrame*       m_pFrame;

    public:
        DetailsContainer( VclBuilderContainer* pBuilder, const OString& rFrame );
        virtual ~DetailsContainer( );

        void setChangeHdl( const Link& rLink ) { m_aChangeHdl = rLink; }

        virtual void show( bool bShow = true );
        virtual INetURLObject getUrl( );

        /** Try to split the URL in the controls of that container.

            \param sUrl the URL to split
            \return true if the split worked, false otherwise.
         */
        virtual bool setUrl( const INetURLObject& rUrl );

        virtual void setUsername( const OUString& /*rUsername*/ ) { };

    protected:
        void notifyChange( );
        DECL_LINK ( ValueChangeHdl, void * );
};

class HostDetailsContainer : public DetailsContainer
{
    private:
        sal_uInt16 m_nDefaultPort;
        OUString m_sScheme;

    protected:
        Edit*           m_pEDHost;
        NumericField*   m_pEDPort;
        Edit*           m_pEDPath;

    public:
        HostDetailsContainer( VclBuilderContainer* pBuilder, sal_uInt16 nPort, OUString sScheme );
        virtual ~HostDetailsContainer( ) { };

        virtual void show( bool bShow = true );
        virtual INetURLObject getUrl( );
        virtual bool setUrl( const INetURLObject& rUrl );

    protected:
        void setScheme( OUString sScheme ) { m_sScheme = sScheme; }

        /** Verifies that the schement split from the URL can be handled by
            the container and set the proper controls accordingly if needed.
          */
        virtual bool verifyScheme( const OUString& rScheme );
};

class DavDetailsContainer : public HostDetailsContainer
{
    private:
        CheckBox*   m_pCBDavs;

    public:
        DavDetailsContainer( VclBuilderContainer* pBuilder );
        ~DavDetailsContainer( ) { };

        virtual void show( bool bShow = true );

    protected:
        virtual bool verifyScheme( const OUString& rScheme );

    private:
        DECL_LINK ( ToggledDavsHdl, CheckBox * pCheckBox );
};

class SmbDetailsContainer : public DetailsContainer
{
    private:
        Edit*           m_pEDHost;
        Edit*           m_pEDShare;
        Edit*           m_pEDPath;

    public:
        SmbDetailsContainer( VclBuilderContainer* pBuilder );
        ~SmbDetailsContainer( ) { };

        virtual INetURLObject getUrl( );
        virtual bool setUrl( const INetURLObject& rUrl );
};

class CmisDetailsContainer : public DetailsContainer
{
    private:
        OUString m_sUsername;
        com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > m_xCmdEnv;
        std::vector< OUString > m_aServerTypesURLs;
        std::vector< OUString > m_aRepoIds;
        OUString m_sRepoId;

        Edit*       m_pEDBinding;
        ListBox*    m_pLBRepository;
        Button*     m_pBTRepoRefresh;
        ListBox*    m_pLBServerType;
        Edit*       m_pEDPath;

    public:
        CmisDetailsContainer( VclBuilderContainer* pBuilder );
        ~CmisDetailsContainer( ) { };

        virtual INetURLObject getUrl( );
        virtual bool setUrl( const INetURLObject& rUrl );
        virtual void setUsername( const OUString& rUsername );

    private:
        void selectRepository( );
        DECL_LINK ( SelectServerTypeHdl, void * );
        DECL_LINK ( RefreshReposHdl, void * );
        DECL_LINK ( SelectRepoHdl, void * );
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
