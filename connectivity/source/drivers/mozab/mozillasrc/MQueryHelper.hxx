/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

#include <boost/unordered_map.hpp>

namespace connectivity
{
    namespace mozab
    {
        class MQueryHelperResultEntry
        {
        private:
            typedef ::boost::unordered_map< OString, OUString, OStringHash >  FieldMap;

            mutable ::osl::Mutex    m_aMutex;
            FieldMap                m_Fields;
            nsCOMPtr<nsIAbCard>     m_Card;
            sal_Int32               m_RowStates;

        public:
            MQueryHelperResultEntry();
            ~MQueryHelperResultEntry();

            void            insert( const OString &key, OUString &value );
            OUString   getValue( const OString &key ) const;
            void            setValue( const OString &key, const OUString & rValue);

            void setCard(nsIAbCard *card);
            nsIAbCard *getCard();
            sal_Bool setRowStates(sal_Int32 state){m_RowStates = state; return sal_True;};
            sal_Int32 getRowStates() const { return m_RowStates;};
        };

        class MQueryHelper : public nsIAbDirectoryQueryResultListener
        {
        private:
            typedef std::vector< MQueryHelperResultEntry* > resultsArray;

            mutable ::osl::Mutex        m_aMutex;
            ::osl::Condition    m_aCondition;
            resultsArray        m_aResults;
            sal_uInt32          m_nIndex;
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
            void            getCardValues(nsIAbCard  *card,sal_uInt32 rowIndex=0);
#if OSL_DEBUG_LEVEL > 0
            oslThreadIdentifier m_oThreadID;
#endif

        public:
            NS_DECL_ISUPPORTS
            NS_DECL_NSIABDIRECTORYQUERYRESULTLISTENER

                                            MQueryHelper();
            virtual                         ~MQueryHelper();

            void                            reset();

            MQueryHelperResultEntry*   next( );

            MQueryHelperResultEntry*   getByIndex( sal_uInt32 nRow );

            const ErrorDescriptor&     getError() const { return m_aError; }

            sal_Bool                   isError() const;

            sal_Bool                   queryComplete() const;

            sal_Bool                   waitForQueryComplete(  );
            sal_Bool                   waitForRow( sal_Int32 rowNum );

            sal_Int32                  getResultCount() const;
            sal_uInt32                 getRealCount() const;
            sal_Int32                  createNewCard(); //return Row count number
            sal_Bool                   resyncRow(sal_uInt32 rowIndex);

            void                       notifyQueryError() ;
            sal_Bool                   setCardValues(const sal_uInt32 rowIndex);
            sal_Int32                  commitCard(const sal_uInt32 rowIndex, nsIAbDirectory * directory);
            sal_Int32                  deleteCard(const sal_uInt32 rowIndex, nsIAbDirectory * directory);
        };
    }
}
#endif // _CONNECTIVITY_MAB_QUERYHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
