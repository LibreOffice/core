/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MABColumnAlias.hxx,v $
 * $Revision: 1.3 $
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
    namespace mozaddressbook
    {
        typedef enum {
            FIRSTNAME = 0,
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

        class OMozabColumnAlias
        {
            /**
             * m_Alias holds aliases for the mozilla addressbook
              * column names. This member gets initialised during
                         * creation of the connection to the driver.
             * m_aAlias initialises m_aAliasMap which then can be
                 * used to find the corresponding programmatic name
             * when an alias is used as a query attribute. Mozilla
             * expects programmatic names from its clients.
               *
             * m_aAlias:   vector of aliases used to initialise m_aAliasMap.
             * m_AliasMap: map of {alias, programmaticname} pairs.
             *
             */
            private:
                ::std::vector< ::rtl::OUString> m_aAlias;
                ::std::map< ::rtl::OUString, ::rtl::OUString> m_aAliasMap;
            protected:
                ::osl::Mutex  m_aMutex;
            public:
                                void initialise(void);
                const ::std::vector< ::rtl::OUString> & getAlias(void) const;
                const ::std::map< ::rtl::OUString, ::rtl::OUString> & getAliasMap(void) const;
                void setAlias(const ::com::sun::star::uno::Reference<
                                                    ::com::sun::star::lang::XMultiServiceFactory > &);
            private:
                void setAliasMap(void);
            public:
                OMozabColumnAlias(void);
                ~OMozabColumnAlias(void);
        };
    }
}
#endif // _CONNECTIVITY_MAB_COLUMNALIAS_HXX_
