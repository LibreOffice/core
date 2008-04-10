/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MABQuery.hxx,v $
 * $Revision: 1.8 $
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


#ifndef _CONNECTIVITY_MAB_QUERY_HXX_
#define _CONNECTIVITY_MAB_QUERY_HXX_

#ifndef _CONNECTIVITY_FILE_QUERY_HXX_
#include "file/FTable.hxx"
#endif
#include <tools/date.hxx>

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

