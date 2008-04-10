/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MABQueryHelper.hxx,v $
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

#ifndef _CONNECTIVITY_MAB_QUERYHELPER_HXX_
#define _CONNECTIVITY_MAB_QUERYHELPER_HXX_

#include <MABNSInclude.hxx>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <comphelper/stl_types.hxx>


namespace connectivity
{
    namespace mozaddressbook
    {
        class OMozabQueryHelperResultEntry
        {
        private:
            mutable ::osl::Mutex        m_aMutex;

            DECLARE_STL_USTRINGACCESS_MAP(::rtl::OUString,fieldMap);

            fieldMap    m_Fields;

        public:
            OMozabQueryHelperResultEntry();
            ~OMozabQueryHelperResultEntry();

            void insert( rtl::OUString &key, rtl::OUString &value );
            rtl::OUString getValue( const rtl::OUString &key ) const;
        };

        class OMozabQueryHelper : public nsIAbDirectoryQueryResultListener
        {
        private:
            typedef std::vector< OMozabQueryHelperResultEntry* > resultsArray;

            mutable ::osl::Mutex        m_aMutex;
            ::osl::Condition    m_aCondition;
            resultsArray        m_aResults;
            sal_Int32           m_nIndex;
            sal_Bool            m_bHasMore;
            sal_Bool            m_bAtEnd;
            sal_Bool            m_bQueryComplete;

            void            append(OMozabQueryHelperResultEntry* resEnt );

            void            clear_results();

            void            clearResultOrComplete();
            void            notifyResultOrComplete();
            void            waitForResultOrComplete();


        public:

            NS_DECL_ISUPPORTS
            NS_DECL_NSIABDIRECTORYQUERYRESULTLISTENER

                                            OMozabQueryHelper();

                                            ~OMozabQueryHelper();

            void                            reset();

            void                            rewind();

            OMozabQueryHelperResultEntry*   next();

            OMozabQueryHelperResultEntry*   getByIndex( sal_Int32 nRow );

            sal_Bool                        hasMore() const;

            sal_Bool                        atEnd() const;

            sal_Bool                        queryComplete() const;

            void                            waitForRow( sal_Int32 rowNum );

            sal_Int32                       getResultCount() const;

            sal_Int32                       getRealCount() const;

            void                            notifyQueryError() ;

        };
    }
}
#endif // _CONNECTIVITY_MAB_QUERYHELPER_HXX_

