/*************************************************************************
 *
 *  $RCSfile: DCode.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-05-23 09:13:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
            ODbaseIndex* pIndex = (ODbaseIndex*)xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId());
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


