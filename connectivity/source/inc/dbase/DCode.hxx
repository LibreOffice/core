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
            TYPEINFO();
        };
    }
}

#endif // CONNECTIVITY_DBASE_DCODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
