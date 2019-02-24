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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FCOMP_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FCOMP_HXX

#include <file/fcode.hxx>
#include <file/filedllapi.hxx>
#include <list>

namespace connectivity
{
    class OSQLParseNode;
    namespace file
    {
        class OCode;
        class OOperand;
        class OSQLAnalyzer;
        typedef std::vector<std::unique_ptr<OCode>> OCodeList;

        class OPredicateCompiler final : public ::salhelper::SimpleReferenceObject
        {
            friend class OPredicateInterpreter;
            friend class OSQLAnalyzer;

            OCodeList                               m_aCodeList;
            css::uno::Reference< css::container::XNameAccess> m_orgColumns; // in filecurs this are the filecolumns
            OSQLAnalyzer*                           m_pAnalyzer;
            sal_Int32                               m_nParamCounter;
        public:
            OPredicateCompiler(OSQLAnalyzer* pAnalyzer);

            virtual ~OPredicateCompiler() override;

            void dispose();

            void start(connectivity::OSQLParseNode const * pSQLParseNode);
            OOperand* execute(connectivity::OSQLParseNode const * pPredicateNode);

            void Clean();
            bool isClean() const {return m_aCodeList.empty();}
            bool hasCode() const {return !isClean();}
            void  setOrigColumns(const css::uno::Reference< css::container::XNameAccess>& rCols) { m_orgColumns = rCols; }
            const css::uno::Reference< css::container::XNameAccess>& getOrigColumns() const { return m_orgColumns; }
        private:
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void execute_COMPARE(connectivity::OSQLParseNode const * pPredicateNode);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void execute_LIKE(connectivity::OSQLParseNode const * pPredicateNode);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void execute_BETWEEN(connectivity::OSQLParseNode const * pPredicateNode);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void execute_ISNULL(connectivity::OSQLParseNode const * pPredicateNode);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            OOperand* execute_Operand(connectivity::OSQLParseNode const * pPredicateNode);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void execute_Fold(OSQLParseNode const * pPredicateNode);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void executeFunction(OSQLParseNode const * pPredicateNode);
        };


        class OPredicateInterpreter :
            public ::salhelper::SimpleReferenceObject
        {
            OCodeStack          m_aStack;
            ::rtl::Reference<OPredicateCompiler> m_rCompiler;

        public:
            OPredicateInterpreter(const ::rtl::Reference<OPredicateCompiler>& rComp) : m_rCompiler(rComp){}
            virtual ~OPredicateInterpreter() override;

            bool        evaluate(OCodeList& rCodeList);
            void        evaluateSelection(OCodeList& rCodeList, ORowSetValueDecoratorRef const & _rVal);

            bool start()
            {
                return evaluate(m_rCompiler->m_aCodeList);
            }

            void startSelection(ORowSetValueDecoratorRef const & _rVal)
            {
                evaluateSelection(m_rCompiler->m_aCodeList,_rVal);
            }


        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FCOMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
