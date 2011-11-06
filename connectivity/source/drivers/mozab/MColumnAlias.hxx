/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




#ifndef _CONNECTIVITY_MAB_COLUMNALIAS_HXX_
#define _CONNECTIVITY_MAB_COLUMNALIAS_HXX_

#include <unotools/confignode.hxx>

#include <osl/mutex.hxx>
#include <vector>
#include <hash_map>

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
            typedef ::std::hash_map< ::rtl::OUString, AliasEntry, ::rtl::OUStringHash > AliasMap;

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
