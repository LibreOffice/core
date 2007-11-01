/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sqlscan.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 14:51:21 $
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
#ifndef _CONNECTIVITY_SQLSCAN_HXX
#define _CONNECTIVITY_SQLSCAN_HXX

#include <stdarg.h>

#ifndef CONNECTIVITY_IPARSECONTEXT_HXX
#include "connectivity/IParseContext.hxx"
#endif

namespace connectivity
{
    //==========================================================================
    //= OSQLScanner
    //==========================================================================
    /** Scanner for SQL92
    */
    class OSQLScanner
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
