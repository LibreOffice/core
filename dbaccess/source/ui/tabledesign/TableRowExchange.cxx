/*************************************************************************
 *
 *  $RCSfile: TableRowExchange.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-22 07:49:01 $
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
#ifndef DBAUI_TABLEROW_EXCHANGE_HXX
#include "TableRowExchange.hxx"
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef DBAUI_TABLEROW_HXX
#include "TableRow.hxx"
#endif

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    OTableRowExchange::OTableRowExchange(const ::std::vector<OTableRow*>& _rvTableRow)
        : m_vTableRow(_rvTableRow)
    {
    }
    // -----------------------------------------------------------------------------
    sal_Bool OTableRowExchange::WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
    {
        if(nUserObjectId == SOT_FORMATSTR_ID_SBA_TABED)
        {
            ::std::vector<OTableRow*>* pRows = reinterpret_cast< ::std::vector<OTableRow*>* >(pUserObject);
            if(pRows)
            {
                (*rxOStm) << (sal_Int32)pRows->size(); // first stream the size
                ::std::vector<OTableRow*>::const_iterator aIter = pRows->begin();
                for(;aIter != pRows->end();++aIter)
                    (*rxOStm) << *(*aIter);
                return sal_True;
            }
        }
        return sal_False;
    }
    // -----------------------------------------------------------------------------
    void OTableRowExchange::AddSupportedFormats()
    {
        if(m_vTableRow.size())
            AddFormat(SOT_FORMATSTR_ID_SBA_TABED);
    }
    // -----------------------------------------------------------------------------
    sal_Bool OTableRowExchange::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
    {
        ULONG nFormat = SotExchange::GetFormat(rFlavor);
        if(nFormat == SOT_FORMATSTR_ID_SBA_TABED)
            return SetObject(&m_vTableRow,SOT_FORMATSTR_ID_SBA_TABED,rFlavor);
        return sal_False;
    }
    // -----------------------------------------------------------------------------
    void OTableRowExchange::ObjectReleased()
    {
        ::std::vector<OTableRow*>::iterator aIter = m_vTableRow.begin();
        for(;aIter != m_vTableRow.end();++aIter)
            delete *aIter;
        m_vTableRow.clear();
    }
    // -----------------------------------------------------------------------------
}








