/*************************************************************************
 *
 *  $RCSfile: RowSetDrop.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:05:33 $
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

#ifndef DBAUI_TOKENWRITER_HXX
#include "TokenWriter.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCOLUMNLOCATE_HPP_
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _DBU_MISC_HRC_
#include "dbu_misc.hrc"
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif

#include <functional>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::lang;
//  using namespace ::com::sun::star::sdbcx;

// export data
ORowSetImportExport::ORowSetImportExport(   Window* _pParent,
                                            const Reference< XResultSetUpdate >& _xResultSetUpdate,
                                            const ::svx::ODataAccessDescriptor& _aDataDescriptor,
                                            const Reference< XMultiServiceFactory >& _rM,
                                            const String& rExchange
                                            )
                                            : ODatabaseImportExport(_aDataDescriptor,_rM,NULL,rExchange)
                                            ,m_xTargetResultSetUpdate(_xResultSetUpdate)
                                            ,m_xTargetRowUpdate(_xResultSetUpdate,UNO_QUERY)
                                            ,m_pParent(_pParent)
                                            ,m_bAlreadyAsked(sal_False)
{
    OSL_ENSURE(_pParent,"Window can't be null!");
}
// -----------------------------------------------------------------------------
void ORowSetImportExport::initialize()
{
    ODatabaseImportExport::initialize();
    // do namemapping
    Reference<XColumnLocate> xColumnLocate(m_xResultSet,UNO_QUERY);
    OSL_ENSURE(xColumnLocate.is(),"The rowset normally should support this");

    m_xTargetResultSetMetaData = Reference<XResultSetMetaDataSupplier>(m_xTargetResultSetUpdate,UNO_QUERY)->getMetaData();
    if(!m_xTargetResultSetMetaData.is() || !xColumnLocate.is())
        return;

    sal_Int32 nCount = m_xTargetResultSetMetaData->getColumnCount();
    m_aColumnMapping.reserve(nCount);
    m_aColumnTypes.reserve(nCount);
    for (sal_Int32 i = 1;i <= nCount; ++i)
    {
        sal_Int32 nPos = -1; // -1 means column is autoincrement or doesn't exists
        if(!m_xTargetResultSetMetaData->isAutoIncrement(i))
        {
            try
            {
                ::rtl::OUString sColumnName = m_xTargetResultSetMetaData->getColumnName(i);
                nPos = xColumnLocate->findColumn(sColumnName);
            }
            catch(const SQLException&)
            {
                if(m_xTargetResultSetMetaData->isNullable(i))
                    nPos = 0; // column doesn't exists but we could set it to null
            }
        }

        m_aColumnMapping.push_back(nPos);
        if(nPos > 0)
            m_aColumnTypes.push_back(m_xResultSetMetaData->getColumnType(nPos));
        else
            m_aColumnTypes.push_back(DataType::OTHER);
    }
}
// -----------------------------------------------------------------------------
BOOL ORowSetImportExport::Write()
{
    return TRUE;
}
// -----------------------------------------------------------------------------
BOOL ORowSetImportExport::Read()
{
    // check if there is any column to copy
    if(::std::find_if(m_aColumnMapping.begin(),m_aColumnMapping.end(),
                        ::std::bind2nd(::std::greater<sal_Int32>(),0)) == m_aColumnMapping.end())
        return FALSE;
    sal_Int32 nCurrentRow = 0;
    sal_Int32 nRowFilterIndex = 0;
    sal_Int32 kk=0;
    sal_Bool bContinue = sal_True;
    if(m_aSelection.getLength())
    {
        const Any* pBegin = m_aSelection.getConstArray();
        const Any* pEnd   = pBegin + m_aSelection.getLength();
        for(;pBegin != pEnd && bContinue;++pBegin)
        {
            sal_Int32 nPos = -1;
            *pBegin >>= nPos;
            OSL_ENSURE(nPos != -1,"Invalid posiotion!");
            bContinue = (m_xResultSet.is() && m_xResultSet->absolute(nPos) && insertNewRow());
        }
    }
    else
    {
        Reference<XPropertySet> xProp(m_xResultSet,UNO_QUERY);
        sal_Int32 nRowCount = 0;
        if ( xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_ISROWCOUNTFINAL) )
        {
            sal_Bool bFinal = sal_False;
            xProp->getPropertyValue(PROPERTY_ISROWCOUNTFINAL) >>= bFinal;
            if ( !bFinal )
                m_xResultSet->afterLast();
            xProp->getPropertyValue(PROPERTY_ROWCOUNT) >>= nRowCount;
        }
        if ( !nRowCount )
        {
            m_xResultSet->afterLast();
            nRowCount = m_xResultSet->getRow();
        }
        OSL_ENSURE(nRowCount,"RowCount is 0!");
        m_xResultSet->beforeFirst();
        while(m_xResultSet.is() && m_xResultSet->next() && bContinue && nRowCount )
        {
            --nRowCount;
            ++nCurrentRow;
            if(!m_pRowMarker || m_pRowMarker[nRowFilterIndex] == nCurrentRow)
            {
                ++nRowFilterIndex;
                bContinue = insertNewRow();
            }
        }
    }
    return TRUE;
}
// -----------------------------------------------------------------------------
sal_Bool ORowSetImportExport::insertNewRow()
{
    try
    {
        m_xTargetResultSetUpdate->moveToInsertRow();
        sal_Int32 i = 1;
        for (::std::vector<sal_Int32>::iterator aIter = m_aColumnMapping.begin(); aIter != m_aColumnMapping.end() ;++aIter,++i )
        {
            if(*aIter > 0)
            {
                Any aValue;
                switch(m_aColumnTypes[i-1])
                {
                    case DataType::CHAR:
                    case DataType::VARCHAR:
                        aValue <<= m_xRow->getString(*aIter);
                        break;
                    case DataType::DECIMAL:
                    case DataType::NUMERIC:
                        aValue <<= m_xRow->getDouble(*aIter);
                        break;
                    case DataType::BIGINT:
                        aValue <<= m_xRow->getLong(*aIter);
                        break;
                    case DataType::FLOAT:
                        aValue <<= m_xRow->getFloat(*aIter);
                        break;
                    case DataType::DOUBLE:
                        aValue <<= m_xRow->getDouble(*aIter);
                        break;
                    case DataType::LONGVARCHAR:
                        aValue <<= m_xRow->getString(*aIter);
                        break;
                    case DataType::LONGVARBINARY:
                        aValue <<= m_xRow->getBytes(*aIter);
                        break;
                    case DataType::DATE:
                        aValue <<= m_xRow->getDate(*aIter);
                        break;
                    case DataType::TIME:
                        aValue <<= m_xRow->getTime(*aIter);
                        break;
                    case DataType::TIMESTAMP:
                        aValue <<= m_xRow->getTimestamp(*aIter);
                        break;
                    case DataType::BIT:
                        aValue <<= m_xRow->getBoolean(*aIter);
                        break;
                    case DataType::TINYINT:
                        aValue <<= m_xRow->getByte(*aIter);
                        break;
                    case DataType::SMALLINT:
                        aValue <<= m_xRow->getShort(*aIter);
                        break;
                    case DataType::INTEGER:
                        aValue <<= m_xRow->getInt(*aIter);
                        break;
                    case DataType::REAL:
                        aValue <<= m_xRow->getDouble(*aIter);
                        break;
                    case DataType::BINARY:
                    case DataType::VARBINARY:
                        aValue <<= m_xRow->getBytes(*aIter);
                        break;
                    default:
                        OSL_ENSURE(0,"Unknown type");
                }
                if(m_xRow->wasNull())
                    m_xTargetRowUpdate->updateNull(i);
                else
                    m_xTargetRowUpdate->updateObject(i,aValue);
            }
            else if(*aIter == 0)//now we have know that we to set this column to null
                m_xTargetRowUpdate->updateNull(i);
        }
        m_xTargetResultSetUpdate->insertRow();
    }
    catch(const SQLException&)
    {
        if(!m_bAlreadyAsked)
        {
            String sAskIfContinue = String(ModuleRes(STR_ERROR_OCCURED_WHILE_COPYING));
            String sTitle = String(ModuleRes(STR_STAT_WARNING));
            OSQLMessageBox aDlg(m_pParent,sTitle,sAskIfContinue,WB_YES_NO|WB_DEF_YES,OSQLMessageBox::Warning);
            if(aDlg.Execute() == RET_YES)
                m_bAlreadyAsked = sal_True;
            else
                return sal_False;
        }
    }
    return sal_True;
}
// -----------------------------------------------------------------------------



