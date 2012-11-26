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


#ifndef CONNECTIVITY_DBASE_DCODE_HXX
#define CONNECTIVITY_DBASE_DCODE_HXX

#include "file/fanalyzer.hxx"
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "file/fcode.hxx"

namespace connectivity
{
    namespace file
    {
        class OConnection;
    }
    namespace dbase
    {
        class OFILEAnalyzer : public file::OSQLAnalyzer
        {
        public:
            OFILEAnalyzer(file::OConnection* _pConnection) : file::OSQLAnalyzer(_pConnection){}
            virtual file::OOperandAttr* createOperandAttr(sal_Int32 _nPos,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xCol,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xIndexes=NULL);
        };

        // Attribute aus einer Ergebniszeile
        class OFILEOperandAttr : public file::OOperandAttr
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> m_xIndex;
        public:
            OFILEOperandAttr(sal_uInt16 _nPos,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xIndexes=NULL);

            virtual sal_Bool isIndexed() const;
            virtual file::OEvaluateSet* preProcess(file::OBoolOperator* pOp, file::OOperand* pRight = 0);
        };
    }
}

#endif // CONNECTIVITY_DBASE_DCODE_HXX

