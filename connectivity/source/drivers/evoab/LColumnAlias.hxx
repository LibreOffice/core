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



#ifndef _CONNECTIVITY_EVOAB_LCOLUMNALIAS_HXX_
#define _CONNECTIVITY_EVOAB_LCOLUMNALIAS_HXX_

#include <unotools/confignode.hxx>

#include <osl/mutex.hxx>
#include <vector>
#include <map>

namespace connectivity
{
    namespace evoab
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
            NOTES,
            END
        } ProgrammaticName;

        class OColumnAlias
        {
            /**
             * m_Alias holds aliases for the evolution addressbook
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
                ::std::vector< ::rtl::OUString> m_aHeadLineNames;
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
                OColumnAlias(void);
                ~OColumnAlias(void);
        };
    }
}
#endif // _CONNECTIVITY_EVOAB_LCOLUMNALIAS_HXX_
