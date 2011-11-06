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


#ifndef _CONNECTIVITY_SQLSCAN_HXX
#define _CONNECTIVITY_SQLSCAN_HXX

#include <stdarg.h>
#include "connectivity/IParseContext.hxx"
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    //==========================================================================
    //= OSQLScanner
    //==========================================================================
    /** Scanner for SQL92
    */
    class OOO_DLLPUBLIC_DBTOOLS OSQLScanner
    {
        const IParseContext*    m_pContext;                 // context for parse, knows all international stuff
        ::rtl::OString          m_sStatement;               // statement to parse
        ::rtl::OUString         m_sErrorMessage;

        sal_Int32               m_nCurrentPos;             // next position to read from the statement
        sal_Bool                m_bInternational;          // do we have a statement which may uses
        sal_Int32               m_nRule;                   // rule to be set

    public:
        OSQLScanner();
        virtual ~OSQLScanner();

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t,void* _pHint ) SAL_THROW( () )
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void *,void* ) SAL_THROW( () )
            {  }

        virtual sal_Int32 SQLyygetc(void);
        virtual void SQLyyerror(char *fmt);
        virtual void output(sal_Int32) { OSL_ASSERT("Internal error in sdblex.l: output not possible"); }
        virtual void ECHO(void) { OSL_ASSERT("Internal error in sdblex.l: ECHO not possible"); }
        virtual IParseContext::InternationalKeyCode getInternationalTokenID(const char* sToken) const;

        // setting the new information before scanning
        void prepareScan(const ::rtl::OUString & rNewStatement, const IParseContext* pContext, sal_Bool bInternational);
        const ::rtl::OUString& getErrorMessage() const {return m_sErrorMessage;}
        ::rtl::OString getStatement() const { return m_sStatement; }

        sal_Int32 SQLlex();
        // set this as scanner for flex
        void setScanner(sal_Bool _bNull=sal_False);
        // rules settings
        void SetRule(sal_Int32 nRule) {m_nRule = nRule;}
        sal_Int32   GetCurrentRule() const;
        sal_Int32   GetGERRule() const;
        sal_Int32   GetENGRule() const;
        sal_Int32   GetSQLRule() const;
        sal_Int32   GetDATERule() const;
        sal_Int32   GetSTRINGRule() const;
        inline sal_Int32 GetCurrentPos() const { return m_nCurrentPos; }
    };
}

#endif
