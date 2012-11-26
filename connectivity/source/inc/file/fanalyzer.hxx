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



#ifndef _CONNECTIVITY_FILE_FANALYZER_HXX_
#define _CONNECTIVITY_FILE_FANALYZER_HXX_

#include "file/fcomp.hxx"
#include "file/filedllapi.hxx"
#include <tools/solar.h>

namespace connectivity
{
    namespace file
    {
        class OConnection;
        class OOO_DLLPUBLIC_FILE OSQLAnalyzer
        {
            typedef ::std::list<OEvaluateSet*>      OEvaluateSetList;
            typedef ::std::pair< ::vos::ORef<OPredicateCompiler>,::vos::ORef<OPredicateInterpreter> > TPredicates;

            ::std::vector< TPredicates >        m_aSelectionEvaluations;
            ::vos::ORef<OPredicateCompiler>     m_aCompiler;
            ::vos::ORef<OPredicateInterpreter>  m_aInterpreter;
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
            void describeParam(::vos::ORef<OSQLColumns> rParameterColumns); // genauere Beschreibung der Parameter
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

