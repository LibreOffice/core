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


#ifndef _CONNECTIVITY_MORK_COLUMNALIAS_HXX_
#define _CONNECTIVITY_MORK_COLUMNALIAS_HXX_

#include <unotools/confignode.hxx>

#include <osl/mutex.hxx>
#include <vector>
#include <boost/unordered_map.hpp>

namespace connectivity
{
    namespace mork
    {
        class OColumnAlias
        {
        public:
            struct AliasEntry
            {
                ::rtl::OString  programmaticAsciiName;
                size_t columnPosition;

                AliasEntry()
                    :programmaticAsciiName()
                    ,columnPosition( 0 )
                {
                }
                AliasEntry( const sal_Char* _programmaticAsciiName, size_t _columnPosition )
                    :programmaticAsciiName( _programmaticAsciiName )
                    ,columnPosition( _columnPosition )
                {
                }
            };
            typedef ::boost::unordered_map< ::rtl::OUString, AliasEntry, ::rtl::OUStringHash > AliasMap;

        private:
            AliasMap    m_aAliasMap;

        public:
            OColumnAlias( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & );

            inline bool hasAlias( const ::rtl::OUString& _rAlias ) const
            {
                return m_aAliasMap.find( _rAlias ) != m_aAliasMap.end();
            }
            ::rtl::OString getProgrammaticNameOrFallbackToUTF8Alias( const ::rtl::OUString& _rAlias ) const;

            inline AliasMap::const_iterator begin() const { return m_aAliasMap.begin(); }
            inline AliasMap::const_iterator end() const { return m_aAliasMap.end(); }

            bool isColumnSearchable( const ::rtl::OUString _alias ) const;

        private:
            void initialize( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );
        };
    }
}
#endif // _CONNECTIVITY_MORK_COLUMNALIAS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
