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
#ifndef _CONNECTIVITY_FILE_FCOMP_HXX_
#define _CONNECTIVITY_FILE_FCOMP_HXX_

#include "file/fcode.hxx"
#include "file/filedllapi.hxx"
#include <list>

namespace connectivity
{
    class OSQLParseNode;
    namespace file
    {
        class OCode;
        class OOperand;
        class OSQLAnalyzer;
        typedef::std::vector<OCode*> OCodeList;

        class OPredicateCompiler : public ::salhelper::SimpleReferenceObject
        {
            friend class OPredicateInterpreter;
            friend class OSQLAnalyzer;

            OCodeList                               m_aCodeList;
            OFileColumns                            m_orgColumns; // in filecurs this are the filecolumns
            OSQLAnalyzer*                           m_pAnalyzer;
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> m_xIndexes;
            sal_Int32                               m_nParamCounter;
            sal_Bool                                m_bORCondition;
        public:
            OPredicateCompiler(OSQLAnalyzer* pAnalyzer);

            virtual ~OPredicateCompiler();

            inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW(())
                { return ::rtl_allocateMemory( nSize ); }
            inline static void * SAL_CALL operator new( size_t /*nSize*/,void* _pHint ) SAL_THROW(())
                { return _pHint; }
            inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW(())
                { ::rtl_freeMemory( pMem ); }
            inline static void SAL_CALL operator delete( void * /*pMem*/,void* /*_pHint*/ ) SAL_THROW(())
                {  }
            void dispose();

            void start(connectivity::OSQLParseNode* pSQLParseNode);
            OOperand* execute(connectivity::OSQLParseNode* pPredicateNode);

            void Clean();
            sal_Bool isClean() const {return m_aCodeList.empty();}
            sal_Bool hasCode() const {return !isClean();}
            sal_Bool hasORCondition() const {return m_bORCondition;}
            void     setOrigColumns(const OFileColumns& rCols) { m_orgColumns = rCols; }
            const OFileColumns getOrigColumns() const { return m_orgColumns; }
        protected:
            OOperand* execute_COMPARE(connectivity::OSQLParseNode* pPredicateNode) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OOperand* execute_LIKE(connectivity::OSQLParseNode* pPredicateNode) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OOperand* execute_BETWEEN(connectivity::OSQLParseNode* pPredicateNode) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OOperand* execute_ISNULL(connectivity::OSQLParseNode* pPredicateNode) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OOperand* execute_Operand(connectivity::OSQLParseNode* pPredicateNode) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OOperand* execute_Fold(OSQLParseNode* pPredicateNode) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OOperand* executeFunction(OSQLParseNode* pPredicateNode) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };


        class OPredicateInterpreter :
            public ::salhelper::SimpleReferenceObject
        {
            OCodeStack          m_aStack;
            ::rtl::Reference<OPredicateCompiler> m_rCompiler;

        public:
            OPredicateInterpreter(const ::rtl::Reference<OPredicateCompiler>& rComp) : m_rCompiler(rComp){}
            virtual ~OPredicateInterpreter();

            sal_Bool    evaluate(OCodeList& rCodeList);
            void        evaluateSelection(OCodeList& rCodeList,ORowSetValueDecoratorRef& _rVal);

            inline sal_Bool start()
            {
                return evaluate(m_rCompiler->m_aCodeList);
            }

            inline void startSelection(ORowSetValueDecoratorRef& _rVal)
            {
                evaluateSelection(m_rCompiler->m_aCodeList,_rVal);
            }


        };
    }
}
#endif // _CONNECTIVITY_FILE_FCOMP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
