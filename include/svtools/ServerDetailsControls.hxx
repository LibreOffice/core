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
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>

class DetailsContainer
{
    protected:
        Link<DetailsContainer*,void> m_aChangeHdl;
        VclPtr<VclGrid>        m_pDetailsGrid;
        VclPtr<VclHBox>        m_pHostBox;
        VclPtr<Edit>           m_pEDHost;
        VclPtr<FixedText>      m_pFTHost;
        VclPtr<NumericField>   m_pEDPort;
        VclPtr<FixedText>      m_pFTPort;
        VclPtr<Edit>           m_pEDRoot;
        VclPtr<FixedText>      m_pFTRoot;
        bool                   m_bIsActive;

    public:
        DetailsContainer( VclBuilderContainer* pBuilder );
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

        virtual void setActive( bool bActive = true );

    protected:
        void notifyChange( );
        DECL_LINK ( ValueChangeHdl, void * );
};

class HostDetailsContainer : public DetailsContainer
{
    private:
        sal_uInt16 m_nDefaultPort;
        OUString m_sScheme;
        OUString m_sHost;

    public:
        HostDetailsContainer( VclBuilderContainer* pBuilder, sal_uInt16 nPort, const OUString& sScheme );
        virtual ~HostDetailsContainer( ) { };

        virtual void show( bool bShow = true ) SAL_OVERRIDE;
        virtual INetURLObject getUrl( ) SAL_OVERRIDE;
        virtual bool setUrl( const INetURLObject& rUrl ) SAL_OVERRIDE;

    protected:
        void setScheme( const OUString& sScheme ) { m_sScheme = sScheme; }

        /** Verifies that the schement split from the URL can be handled by
            the container and set the proper controls accordingly if needed.
          */
        virtual bool verifyScheme( const OUString& rScheme );
};

class DavDetailsContainer : public HostDetailsContainer
{
    private:
        VclPtr<CheckBox>   m_pCBDavs;

    public:
        DavDetailsContainer( VclBuilderContainer* pBuilder );
        virtual ~DavDetailsContainer( ) { };

        virtual void show( bool bShow = true ) SAL_OVERRIDE;

    protected:
        virtual bool verifyScheme( const OUString& rScheme ) SAL_OVERRIDE;

    private:
        DECL_LINK_TYPED( ToggledDavsHdl, CheckBox&, void );
};

class SmbDetailsContainer : public DetailsContainer
{
    private:
        VclPtr<Edit>           m_pEDShare;
        VclPtr<FixedText>      m_pFTShare;

    public:
        SmbDetailsContainer( VclBuilderContainer* pBuilder );
        virtual ~SmbDetailsContainer( ) { };

        virtual INetURLObject getUrl( ) SAL_OVERRIDE;
        virtual bool setUrl( const INetURLObject& rUrl ) SAL_OVERRIDE;
        virtual void show( bool bShow = true ) SAL_OVERRIDE;
};

class CmisDetailsContainer : public DetailsContainer
{
    private:
        OUString m_sUsername;
        OUString m_sPassword;
        com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > m_xCmdEnv;
        std::vector< OUString > m_aRepoIds;
        OUString m_sRepoId;
        OUString m_sBinding;

        VclPtr<VclHBox>    m_pRepositoryBox;
        VclPtr<FixedText>  m_pFTRepository;
        VclPtr<ListBox>    m_pLBRepository;
        VclPtr<Button>     m_pBTRepoRefresh;

    public:
        CmisDetailsContainer( VclBuilderContainer* pBuilder, OUString const & sBinding );
        virtual ~CmisDetailsContainer( ) { };

        virtual void show( bool bShow = true ) SAL_OVERRIDE;
        virtual INetURLObject getUrl( ) SAL_OVERRIDE;
        virtual bool setUrl( const INetURLObject& rUrl ) SAL_OVERRIDE;
        virtual void setUsername( const OUString& rUsername ) SAL_OVERRIDE;
        virtual void setPassword( const OUString& rPass ) SAL_OVERRIDE;

    private:
        void selectRepository( );
        DECL_LINK_TYPED ( RefreshReposHdl, Button*, void );
        DECL_LINK ( SelectRepoHdl, void * );
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
