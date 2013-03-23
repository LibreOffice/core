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

#pragma once
#if 1

#include <set>
#include <memory>
#include "osl/mutex.hxx"
#include "rtl/ustring.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "unotools/configitem.hxx"

class SysCredentialsConfig;

class SysCredentialsConfigItem : public utl::ConfigItem
{
    public:
        SysCredentialsConfigItem( SysCredentialsConfig * pOwner );
        //virtual ~SysCredentialsConfigItem();

        virtual void Notify(
            const com::sun::star::uno::Sequence< rtl::OUString > &
                seqPropertyNames );
        virtual void Commit();

        com::sun::star::uno::Sequence< rtl::OUString >
        getSystemCredentialsURLs();

        void setSystemCredentialsURLs(
            const com::sun::star::uno::Sequence< rtl::OUString > &
                seqURLList );

        //bool isSystemCredentialsURL( const rtl::OUString & rURL ) const;

private:
        ::osl::Mutex m_aMutex;
        bool m_bInited;
        com::sun::star::uno::Sequence< rtl::OUString > m_seqURLs;
        SysCredentialsConfig * m_pOwner;
};

typedef std::set< rtl::OUString > StringSet;

class SysCredentialsConfig
{
    public:
        SysCredentialsConfig();

        rtl::OUString find( rtl::OUString const & rURL );
        void add( rtl::OUString const & rURL, bool bPersistent );
        void remove( rtl::OUString const & rURL );
        com::sun::star::uno::Sequence< rtl::OUString > list( bool bOnlyPersistent );

        void persistentConfigChanged();

    private:
        void initCfg();
        void writeCfg();

        ::osl::Mutex m_aMutex;
        StringSet    m_aMemContainer;
        StringSet    m_aCfgContainer;
        SysCredentialsConfigItem m_aConfigItem;
        bool         m_bCfgInited;
};

#endif // INCLUDED_SVTOOLS_SYSCREDS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
