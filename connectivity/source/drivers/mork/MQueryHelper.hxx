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

#ifndef _CONNECTIVITY_MORK_QUERYHELPER_HXX_
#define _CONNECTIVITY_MORK_QUERYHELPER_HXX_

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <comphelper/stl_types.hxx>
#include <osl/thread.hxx>
#include <connectivity/FValue.hxx>

#include <boost/unordered_map.hpp>

namespace connectivity
{
    namespace mork
    {
        class OConnection;
        class MQueryHelperResultEntry
        {
        private:
            typedef ::boost::unordered_map< ::rtl::OString, ::rtl::OUString, ::rtl::OStringHash >  FieldMap;

            mutable ::osl::Mutex    m_aMutex;
            FieldMap                m_Fields;

        public:
            MQueryHelperResultEntry();
            ~MQueryHelperResultEntry();

            void            insert( const rtl::OString &key, rtl::OUString &value );
            rtl::OUString   getValue( const rtl::OString &key ) const;
            void            setValue( const rtl::OString &key, const rtl::OUString & rValue);
        };

        class MQueryHelper
        {
        private:
            typedef std::vector< MQueryHelperResultEntry* > resultsArray;

            mutable ::osl::Mutex        m_aMutex;
            ::osl::Condition    m_aCondition;
            resultsArray        m_aResults;
            sal_uInt32          m_nIndex;
            sal_Bool            m_bHasMore;
            sal_Bool            m_bAtEnd;
            void            append(MQueryHelperResultEntry* resEnt );
            void            clear_results();
            OColumnAlias        m_rColumnAlias;

/*
            void            clearResultOrComplete();
            void            notifyResultOrComplete();
            sal_Bool        waitForResultOrComplete( );
            void            getCardValues(nsIAbCard  *card,sal_uInt32 rowIndex=0);
*/

        public:
                                       MQueryHelper(const OColumnAlias& _ca);
            virtual                    ~MQueryHelper();

            void                       reset();
            MQueryHelperResultEntry*   next();
            MQueryHelperResultEntry*   getByIndex( sal_uInt32 nRow );
            sal_Bool                   isError() const;
            sal_Bool                   queryComplete() const;
            sal_Int32                  getResultCount() const;
            sal_Bool                   checkRowAvailable( sal_Int32 nDBRow );
            sal_Bool getRowValue( ORowSetValue& rValue, sal_Int32 nDBRow,const rtl::OUString& aDBColumnName, sal_Int32 nType );
            sal_Int32 executeQuery(OConnection* xConnection);
        };
    }
}

#endif // _CONNECTIVITY_MORK_QUERYHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
