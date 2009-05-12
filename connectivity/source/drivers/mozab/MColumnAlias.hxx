/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MColumnAlias.hxx,v $
 * $Revision: 1.4 $
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
#include <map>

namespace connectivity
{
    namespace mozab
    {
        class OColumnAlias
        {
        public:
            typedef enum {
                BEGIN = 0,

                FIRSTNAME = BEGIN,
                LASTNAME,
                DISPLAYNAME,
                NICKNAME,
                PRIMARYEMAIL,
                SECONDEMAIL,
                PREFERMAILFORMAT,
                WORKPHONE,
                HOMEPHONE,
                FAXNUMBER,
                PAGERNUMBER,
                CELLULARNUMBER,
                HOMEADDRESS,
                HOMEADDRESS2,
                HOMECITY,
                HOMESTATE,
                HOMEZIPCODE,
                HOMECOUNTRY,
                WORKADDRESS,
                WORKADDRESS2,
                WORKCITY,
                WORKSTATE,
                WORKZIPCODE,
                WORKCOUNTRY,
                JOBTITLE,
                DEPARTMENT,
                COMPANY,
                WEBPAGE1,
                WEBPAGE2,
                BIRTHYEAR,
                BIRTHMONTH,
                BIRTHDAY,
                CUSTOM1,
                CUSTOM2,
                CUSTOM3,
                CUSTOM4,
                NOTES,

                END
            } ProgrammaticName;

            struct AliasDescription
            {
                ::rtl::OUString     sProgrammaticName;
                ProgrammaticName    eProgrammaticNameIndex;

                AliasDescription()
                    :eProgrammaticNameIndex( END )
                {
                }

                AliasDescription( const ::rtl::OUString& _rName, ProgrammaticName _eIndex )
                    :sProgrammaticName( _rName ), eProgrammaticNameIndex( _eIndex )
                {
                }
            };

            typedef ::std::map< ::rtl::OUString, AliasDescription > AliasMap;

        private:
            AliasMap    m_aAliasMap;

        protected:
            ::osl::Mutex  m_aMutex;

        public:
            OColumnAlias(
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & );

            ProgrammaticName getProgrammaticNameIndex( const ::rtl::OUString& _rAliasName ) const;
            inline bool hasAlias( const ::rtl::OUString& _rAlias ) const
            {
                return m_aAliasMap.find( _rAlias ) != m_aAliasMap.end();
            }
            ::rtl::OUString getProgrammaticNameOrFallbackToAlias( const ::rtl::OUString& _rAlias ) const;

            inline AliasMap::const_iterator begin() const { return m_aAliasMap.begin(); }
            inline AliasMap::const_iterator end() const { return m_aAliasMap.end(); }

        private:
            void initialize( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );
        };
    }
}
#endif // _CONNECTIVITY_MAB_COLUMNALIAS_HXX_
