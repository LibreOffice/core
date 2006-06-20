/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MQueryHelper.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:52:27 $
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

#include <MNSInclude.hxx>

#ifndef CONNECITIVITY_MOZAB_ERROR_RESOURCE_HXX
#include "MErrorResource.hxx"
#endif

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
#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif

namespace connectivity
{
    namespace mozab
    {
        class MQueryHelperResultEntry
        {
        private:
            mutable ::osl::Mutex        m_aMutex;

            DECLARE_STL_USTRINGACCESS_MAP(::rtl::OUString,fieldMap);

            fieldMap    m_Fields;
            nsCOMPtr<nsIAbCard> m_Card;
            sal_Int32   m_RowStates;
        public:
            MQueryHelperResultEntry();
            ~MQueryHelperResultEntry();

            void insert( const rtl::OUString &key, rtl::OUString &value );
            rtl::OUString getValue( const rtl::OUString &key ) const;
            rtl::OUString setValue( const rtl::OUString &key, const rtl::OUString & rValue);

            void setCard(nsIAbCard *card);
            nsIAbCard *getCard();
            sal_Bool setRowStates(sal_Int32 state){m_RowStates = state; return sal_True;};
            sal_Int32 getRowStates()  { return m_RowStates;};
        };

        class MQueryHelper : public nsIAbDirectoryQueryResultListener, public ErrorResourceAccess
        {
        private:
            typedef std::vector< MQueryHelperResultEntry* > resultsArray;

            mutable ::osl::Mutex        m_aMutex;
            ::osl::Condition    m_aCondition;
            resultsArray        m_aResults;
            sal_Int32           m_nIndex;
            sal_Bool            m_bHasMore;
            sal_Bool            m_bAtEnd;
            sal_Bool            m_bErrorCondition;
            sal_Bool            m_bQueryComplete;

            void            append(MQueryHelperResultEntry* resEnt );

            void            clear_results();

            void            clearResultOrComplete();
            void            notifyResultOrComplete();
            sal_Bool        waitForResultOrComplete( );
            void            addCardAttributeAndValue(const ::rtl::OUString& sName, nsXPIDLString sValue,MQueryHelperResultEntry *resEntry);
            void            getCardAttributeAndValue(const ::rtl::OUString& sName, ::rtl::OUString &ouValue, MQueryHelperResultEntry *resEntry) ;
            void            getCardValues(nsIAbCard  *card,sal_Int32 rowIndex=0);
#if OSL_DEBUG_LEVEL > 0
            oslThreadIdentifier m_oThreadID;
#endif

        public:

            NS_DECL_ISUPPORTS
            NS_DECL_NSIABDIRECTORYQUERYRESULTLISTENER

                                            MQueryHelper();

            virtual                         ~MQueryHelper();

            void                            reset();

            void                            rewind();

            MQueryHelperResultEntry*   next( );

            MQueryHelperResultEntry*   getByIndex( sal_uInt32 nRow );

            sal_Bool                   isError() const;

            sal_Bool                   hasMore() const;

            sal_Bool                   atEnd() const;

            sal_Bool                   queryComplete() const;

            sal_Bool                   waitForQueryComplete(  );

            sal_Bool                   waitForRow( sal_Int32 rowNum );

            sal_Int32                  getResultCount() const;

            sal_uInt32                 getRealCount() const;
            sal_Int32                  createNewCard(); //return Row count number
            sal_Bool                   resyncRow(sal_Int32 rowIndex);

            void                       notifyQueryError() ;
            sal_Bool                   setCardValues(const sal_Int32 rowIndex);
            sal_Int32                  commitCard(const sal_Int32 rowIndex, nsIAbDirectory * directory);
            sal_Int32                  deleteCard(const sal_Int32 rowIndex, nsIAbDirectory * directory);
        };
    }
}
#endif // _CONNECTIVITY_MAB_QUERYHELPER_HXX_

