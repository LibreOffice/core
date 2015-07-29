/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_UCB_SOURCE_UCP_CMIS_CMIS_URL_HXX
#define INCLUDED_UCB_SOURCE_UCP_CMIS_CMIS_URL_HXX

#include <map>
#include <string>

#include <rtl/ustring.hxx>
#include <tools/urlobj.hxx>

namespace cmis
{
    class URL
    {
        private:
            OUString m_sBindingUrl;
            OUString m_sRepositoryId;
            OUString m_sPath;
            OUString m_sId;
            OUString m_sUser;
            OUString m_sPass;

        public:
            explicit URL( OUString const & urlStr );

            const OUString& getObjectPath() const { return m_sPath; }
            const OUString& getObjectId() const { return m_sId; }
            const OUString& getBindingUrl() const { return m_sBindingUrl; }
            const OUString& getRepositoryId() const { return m_sRepositoryId; }
            const OUString& getUsername() const { return m_sUser; }
            const OUString& getPassword() const { return m_sPass; }
            void setObjectPath( const OUString& sPath );
            void setObjectId( const OUString& sId );
            void setUsername( const OUString& sUser );

            OUString asString( );
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
