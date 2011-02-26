/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
        virtual void output(sal_Int32) { OSL_FAIL("Internal error in sdblex.l: output not possible"); }
        virtual void ECHO(void) { OSL_FAIL("Internal error in sdblex.l: ECHO not possible"); }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
