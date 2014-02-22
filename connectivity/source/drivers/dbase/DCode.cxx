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

OOperandAttr* OFILEAnalyzer::createOperandAttr(sal_Int32 _nPos,
                                               const Reference< XPropertySet>& _xCol,
                                               const Reference< XNameAccess>& _xIndexes)
{
    return new OFILEOperandAttr((sal_uInt16)_nPos,_xCol,_xIndexes);
}


OFILEOperandAttr::OFILEOperandAttr(sal_uInt16 _nPos,
                                   const Reference< XPropertySet>& _xColumn,
                                   const Reference< XNameAccess>& _xIndexes)
    : OOperandAttr(_nPos,_xColumn)
{
    if(_xIndexes.is())
    {
        OUString sName;
        Reference<XPropertySetInfo> xColInfo = _xColumn->getPropertySetInfo();
        Reference<XPropertySet> xIndex;

        Sequence< OUString> aSeq = _xIndexes->getElementNames();
        const OUString* pBegin = aSeq.getConstArray();
        const OUString* pEnd   = pBegin + aSeq.getLength();
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

sal_Bool OFILEOperandAttr::isIndexed() const
{
    return m_xIndex.is();
}

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
