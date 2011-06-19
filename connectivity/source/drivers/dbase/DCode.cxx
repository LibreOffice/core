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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "dbase/DCode.hxx"
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include "dbase/DIndex.hxx"
#include "dbase/DIndexIter.hxx"


using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

TYPEINIT1(OFILEOperandAttr, OOperandAttr);
// -----------------------------------------------------------------------------
OOperandAttr* OFILEAnalyzer::createOperandAttr(sal_Int32 _nPos,
                                               const Reference< XPropertySet>& _xCol,
                                               const Reference< XNameAccess>& _xIndexes)
{
    return new OFILEOperandAttr((sal_uInt16)_nPos,_xCol,_xIndexes);
}

//------------------------------------------------------------------
OFILEOperandAttr::OFILEOperandAttr(sal_uInt16 _nPos,
                                   const Reference< XPropertySet>& _xColumn,
                                   const Reference< XNameAccess>& _xIndexes)
    : OOperandAttr(_nPos,_xColumn)
{
    if(_xIndexes.is())
    {
        ::rtl::OUString sName;
        Reference<XPropertySetInfo> xColInfo = _xColumn->getPropertySetInfo();
        Reference<XPropertySet> xIndex;

        Sequence< ::rtl::OUString> aSeq = _xIndexes->getElementNames();
        const ::rtl::OUString* pBegin = aSeq.getConstArray();
        const ::rtl::OUString* pEnd   = pBegin + aSeq.getLength();
        for(;pBegin != pEnd;++pBegin)
        {
            _xIndexes->getByName(*pBegin) >>= xIndex;
            if(xIndex.is())
            {
                Reference<XColumnsSupplier> xColsSup(xIndex,UNO_QUERY);
                Reference<XNameAccess> xNameAccess = xColsSup->getColumns();
                _xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= sName;
                if(xNameAccess->hasByName(sName))
                {
                    m_xIndex = xIndex;
                    break;
                }
                else if(xColInfo->hasPropertyByName(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME)))
                {
                    _xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME)) >>= sName;
                    if(xNameAccess->hasByName(sName))
                    {
                        m_xIndex = xIndex;
                        break;
                    }
                }
            }
        }
    }

}
// -------------------------------------------------------------------------
sal_Bool OFILEOperandAttr::isIndexed() const
{
    return m_xIndex.is();
}
//------------------------------------------------------------------
OEvaluateSet* OFILEOperandAttr::preProcess(OBoolOperator* pOp, OOperand* pRight)
{
    OEvaluateSet* pEvaluateSet = NULL;
    if (isIndexed())
    {
        Reference<XUnoTunnel> xTunnel(m_xIndex,UNO_QUERY);
        if(xTunnel.is())
        {
            ODbaseIndex* pIndex = reinterpret_cast< ODbaseIndex* >( xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId()) );
            if(pIndex)
            {
                OIndexIterator* pIter = pIndex->createIterator(pOp,pRight);

                if (pIter)
                {
                    pEvaluateSet = new OEvaluateSet();
                    sal_uIntPtr nRec = pIter->First();
                    while (nRec != NODE_NOTFOUND)
                    {
                        (*pEvaluateSet)[nRec] = nRec;
                        nRec = pIter->Next();
                    }
                }
                delete pIter;
            }
        }
    }
    return pEvaluateSet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
