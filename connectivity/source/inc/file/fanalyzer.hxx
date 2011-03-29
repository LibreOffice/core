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

#ifndef _CONNECTIVITY_FILE_FANALYZER_HXX_
#define _CONNECTIVITY_FILE_FANALYZER_HXX_

#include "file/fcomp.hxx"
#include "file/filedllapi.hxx"

namespace connectivity
{
    namespace file
    {
        class OConnection;
        class OOO_DLLPUBLIC_FILE OSQLAnalyzer
        {
            typedef ::std::list<OEvaluateSet*>      OEvaluateSetList;
            typedef ::std::pair< ::rtl::Reference<OPredicateCompiler>,::rtl::Reference<OPredicateInterpreter> > TPredicates;

            ::std::vector< TPredicates >        m_aSelectionEvaluations;
            ::rtl::Reference<OPredicateCompiler>        m_aCompiler;
            ::rtl::Reference<OPredicateInterpreter> m_aInterpreter;
            OConnection*                        m_pConnection;

            mutable sal_Bool                    m_bHasSelectionCode;
            mutable sal_Bool                    m_bSelectionFirstTime;

            void bindRow(OCodeList& rCodeList,const OValueRefRow& _pRow,OEvaluateSetList& _rEvaluateSetList);

        public:
            OSQLAnalyzer(OConnection* _pConnection);
            virtual ~OSQLAnalyzer();
            inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
                { return ::rtl_allocateMemory( nSize ); }
            inline static void * SAL_CALL operator new( size_t /*nSize*/,void* _pHint ) SAL_THROW( () )
                { return _pHint; }
            inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
                { ::rtl_freeMemory( pMem ); }
            inline static void SAL_CALL operator delete( void * /*pMem*/,void* /*_pHint*/ ) SAL_THROW( () )
                {  }

            OConnection* getConnection() const { return m_pConnection; }
            void describeParam(::rtl::Reference<OSQLColumns> rParameterColumns); // genauere Beschreibung der Parameter
            ::std::vector<sal_Int32>* bindEvaluationRow(OValueRefRow& _pRow);                   // Anbinden einer Ergebniszeile an die Restrictions
            /** bind the select columns if they contain a function which needs a row value
                @param  _pRow   the result row
            */
            void bindSelectRow(const OValueRefRow& _pRow);

            /** binds the row to parameter for the restrictions
                @param  _pRow   the parameter row
            */
            void bindParameterRow(OValueRefRow& _pRow);

            void setIndexes(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xIndexes);

            void dispose();
            void start(OSQLParseNode* pSQLParseNode);
            void clean();
            virtual sal_Bool hasRestriction() const;
            virtual sal_Bool hasFunctions() const;
            inline sal_Bool evaluateRestriction()   { return m_aInterpreter->start(); }
            void setSelectionEvaluationResult(OValueRefRow& _pRow,const ::std::vector<sal_Int32>& _rColumnMapping);
            void setOrigColumns(const OFileColumns& rCols);
            virtual OOperandAttr* createOperandAttr(sal_Int32 _nPos,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xCol,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xIndexes=NULL);
        };
    }
}
#endif // _CONNECTIVITY_FILE_FANALYZER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
