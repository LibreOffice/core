/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DCode.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:18:55 $
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
#ifndef CONNECTIVITY_DBASE_DCODE_HXX
#include "dbase/DCode.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _CONNECTIVITY_DBASE_INDEX_HXX_
#include "dbase/DIndex.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEXITER_HXX_
#include "dbase/DIndexIter.hxx"
#endif


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
                    ULONG nRec = pIter->First();
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


