/*************************************************************************
 *
 *  $RCSfile: MABQuery.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: wvd $ $Date: 2001-07-19 14:48:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _CONNECTIVITY_MAB_QUERY_HXX_
#define _CONNECTIVITY_MAB_QUERY_HXX_

#ifndef _CONNECTIVITY_FILE_QUERY_HXX_
#include "file/FTable.hxx"
#endif
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif

#ifndef _CONNECTIVITY_MAB_COLUMNALIAS_HXX_
#include "MABColumnAlias.hxx"
#endif

namespace connectivity
{
    namespace mozaddressbook
    {

        class OMozabQueryHelper;

        //class OMozabQuery : public nsIAbDirectoryQueryResultListener
        class OMozabQuery
        {
            /*
             * A query resultset with a maximum limit of
             * m_nMaxNrOfReturns return items, is created from
             * the following SQL statement:
             *
             * -------------------------------------------------
             * SELECT m_aAttributes FROM m_aAddressbook
             * WHERE  m_aMatchItems SQL_OPR m_aMatchValue
             * -------------------------------------------------
             *
             * We are\are not, depending on boolean m_bQuerySubDirs,
             * interested in querying the sub-directories of the
             * addressbook directory, if any.
             *
             * SQL_OPR:
             * m_aSqlOppr contains the SQL operations for every
             * attribute in m_aAttributes.
             * This member must be initialised together with
             * m_aAttributes.
             *
             * The SQL operations defined for 'SQL_OPR' are:
             * matchExists         = 0,
             * matchDoesNotExist   = 1,
             * matchContains       = 2,
             * matchDoesNotContain = 3,
             * matchIs             = 4,
             * matchIsNot          = 5,
             * matchBeginsWith     = 6,
             * matchEndsWith       = 7,
             * matchSoundsLike     = 8,
             * matchRegExp         = 9.
             * There must be mapping to one of these values.
             *
             * The following members MUST be initialised before
             * a query is executed:
             * m_Attributes, m_aMapAttrOppr, m_aAddressbook,
             * m_aMatchItems and m_aMatchValue.
             *
             * m_bQuerySubDirs and m_nMaxNrReturns are set to a
             * default value in the constructor which can be
             * overridden. If (element of) m_aSqlOppr is not set,
             * the default SQL operation is 'matchIs'.
             *
             */
        public:
             typedef enum {
                 matchExists         = 0,
                 matchDoesNotExist   = 1,
                 matchContains       = 2,
                 matchDoesNotContain = 3,
                 matchIs             = 4,
                 matchIsNot          = 5,
                 matchBeginsWith     = 6,
                 matchEndsWith       = 7,
                 matchSoundsLike     = 8,
                 matchRegExp         = 9
            } eSqlOppr;

        private:
            OMozabQueryHelper              *m_aQueryHelper;
            ::std::vector< ::rtl::OUString> m_aAttributes;
            ::rtl::OUString                 m_aAddressbook;
            ::std::vector< ::rtl::OUString> m_aMatchItems;
            ::std::vector< ::rtl::OUString> m_aMatchValues;
            sal_Int32                       m_nMaxNrOfReturns;
            sal_Bool                        m_bQuerySubDirs;
            ::std::vector<eSqlOppr>         m_aSqlOppr;
            ::std::map< ::rtl::OUString,
                                    ::rtl::OUString>    m_aColumnAliasMap;
        protected:
            ::osl::Mutex                    m_aMutex;
        public:
            /*
             * - Contains accessors to the members of this class.
             * - executeQuery() initiates a non-blocking query.
             */
            void                            setAttributes( ::std::vector< ::rtl::OUString>&);
            const ::std::vector< ::rtl::OUString> &getAttributes(void) const;
            void                            setAddressbook( ::rtl::OUString&);
            ::rtl::OUString                 getAddressbook(void) const;
            void                            setMatchItems( ::std::vector< ::rtl::OUString>&);
            const ::std::vector< ::rtl::OUString> &getMatchItems(void) const;
            void                            setMatchValues( ::std::vector< ::rtl::OUString>&);
            const ::std::vector< ::rtl::OUString> &getMatchValues(void) const;
            void                            setMaxNrOfReturns( const sal_Int32);
            sal_Int32                       getMaxNrOfReturns(void) const;
            void                            setQuerySubDirs( sal_Bool&);
            sal_Bool                        getQuerySubDirs(void) const;
            void                            setSqlOppr( ::std::vector< eSqlOppr >&);
            const ::std::vector< eSqlOppr >       &getSqlOppr(void) const;
            sal_Int32                       executeQuery(sal_Bool _bIsOutlookExpress);

            sal_Int32                       getRowCount( void );

            sal_Int32                       getRealRowCount( void );

            sal_Bool                        queryComplete( void );

            sal_Bool                        checkRowAvailable( sal_Int32 nDBRow );

            void                            getRowValue( connectivity::ORowSetValue& rValue,
                                                         sal_Int32 nDBRow, rtl::OUString& aDBColumnName,
                                                         sal_Int32 nType );

        public:
            OMozabQuery();
            OMozabQuery(const ::std::map< ::rtl::OUString, ::rtl::OUString> &);
            virtual ~OMozabQuery();
        };
    }
}

#endif // _CONNECTIVITY_MAB_QUERY_HXX_

