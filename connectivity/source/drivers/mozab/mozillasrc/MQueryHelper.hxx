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



#ifndef _CONNECTIVITY_MAB_QUERYHELPER_HXX_
#define _CONNECTIVITY_MAB_QUERYHELPER_HXX_

#include <MNSInclude.hxx>
#include "MErrorResource.hxx"
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <comphelper/stl_types.hxx>
#include <osl/thread.hxx>

#include <hash_map>

namespace connectivity
{
    namespace mozab
    {
        class MQueryHelperResultEntry
        {
        private:
            typedef ::std::hash_map< ::rtl::OString, ::rtl::OUString, ::rtl::OStringHash >  FieldMap;

            mutable ::osl::Mutex    m_aMutex;
            FieldMap                m_Fields;
            nsCOMPtr<nsIAbCard>     m_Card;
            sal_Int32               m_RowStates;

        public:
            MQueryHelperResultEntry();
            ~MQueryHelperResultEntry();

            void            insert( const rtl::OString &key, rtl::OUString &value );
            rtl::OUString   getValue( const rtl::OString &key ) const;
            void            setValue( const rtl::OString &key, const rtl::OUString & rValue);

            void setCard(nsIAbCard *card);
            nsIAbCard *getCard();
            sal_Bool setRowStates(sal_Int32 state){m_RowStates = state; return sal_True;};
            sal_Int32 getRowStates()  { return m_RowStates;};
        };

        class MQueryHelper : public nsIAbDirectoryQueryResultListener
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
            ErrorDescriptor     m_aError;

            void            append(MQueryHelperResultEntry* resEnt );

            void            clear_results();

            void            clearResultOrComplete();
            void            notifyResultOrComplete();
            sal_Bool        waitForResultOrComplete( );
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

            const ErrorDescriptor&     getError() const { return m_aError; }

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

