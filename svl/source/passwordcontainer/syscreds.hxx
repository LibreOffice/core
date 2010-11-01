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

#ifndef INCLUDED_SVTOOLS_SYSCREDS_HXX
#define INCLUDED_SVTOOLS_SYSCREDS_HXX

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
