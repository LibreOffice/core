/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LColumnAlias.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:42:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_EVOAB_LCOLUMNALIAS_HXX_
#define _CONNECTIVITY_EVOAB_LCOLUMNALIAS_HXX_

#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif

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
