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


#ifndef _CONNECTIVITY_MAB_COLUMNALIAS_HXX_
#define _CONNECTIVITY_MAB_COLUMNALIAS_HXX_

#include <unotools/confignode.hxx>

#include <osl/mutex.hxx>
#include <vector>
#include <boost/unordered_map.hpp>

namespace connectivity
{
    namespace mozab
    {
        class OColumnAlias
        {
        public:
            struct AliasEntry
            {
                ::rtl::OString  programmaticAsciiName;
                sal_Int32       columnPosition;

                AliasEntry()
                    :programmaticAsciiName()
                    ,columnPosition( 0 )
                {
                }
                AliasEntry( const sal_Char* _programmaticAsciiName, sal_Int32 _columnPosition )
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
#endif // _CONNECTIVITY_MAB_COLUMNALIAS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
