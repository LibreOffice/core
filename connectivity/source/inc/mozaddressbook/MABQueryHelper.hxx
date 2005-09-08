/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MABQueryHelper.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:30:49 $
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

#ifndef _CONNECTIVITY_MAB_QUERYHELPER_HXX_
#define _CONNECTIVITY_MAB_QUERYHELPER_HXX_

#include <MABNSInclude.hxx>

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif


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

