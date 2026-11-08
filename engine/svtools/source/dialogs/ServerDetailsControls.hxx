/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SVTOOLS_SERVERDETAILSCONTROLS_HXX
#define INCLUDED_SVTOOLS_SERVERDETAILSCONTROLS_HXX

#include <tools/link.hxx>
#include <tools/urlobj.hxx>
#include <vcl/weld.hxx>

namespace weld {
    class Entry;
    class SpinButton;
}

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

        virtual void set_visible( bool bShow );
        virtual INetURLObject getUrl( );

        /** Try to split the URL in the controls of that container.

            \param sUrl the URL to split
            \return true if the split worked, false otherwise.
         */
        virtual bool setUrl( const INetURLObject& rUrl );

        virtual bool enableUserCredentials( ) { return true; };

    protected:
        void notifyChange( );
        DECL_LINK(ValueChangeHdl, weld::Entry&, void);
        DECL_STATIC_LINK(DetailsContainer, FormatPortHdl, sal_Int64, OUString);
};

class HostDetailsContainer : public DetailsContainer
{
    private:
        sal_uInt16 m_nDefaultPort;
        OUString m_sScheme;
        OUString m_sHost;

    public:
        HostDetailsContainer(PlaceEditDialog* pDialog, sal_uInt16 nPort, OUString sScheme);

        virtual void set_visible( bool bShow ) override;
        virtual INetURLObject getUrl( ) override;
        virtual bool setUrl( const INetURLObject& rUrl ) override;

    protected:
        void setScheme( const OUString& sScheme ) { m_sScheme = sScheme; }

        /** Verifies that the scheme split from the URL can be handled by
            the container and set the proper controls accordingly if needed.
          */
        virtual bool verifyScheme( const OUString& rScheme );
};

class DavDetailsContainer final : public HostDetailsContainer
{
    public:
        DavDetailsContainer(PlaceEditDialog* pDialog);

        virtual void set_visible( bool bShow ) override;
        virtual bool enableUserCredentials( ) override { return false; };

    private:
        virtual bool verifyScheme( const OUString& rScheme ) override;

        DECL_LINK(ToggledDavsHdl, weld::Toggleable&, void);
};

class SmbDetailsContainer final : public DetailsContainer
{
    private:
        OUString m_sHost;

    public:
        SmbDetailsContainer(PlaceEditDialog* pDialog);

        virtual INetURLObject getUrl( ) override;
        virtual bool setUrl( const INetURLObject& rUrl ) override;
        virtual void set_visible( bool bShow ) override;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
