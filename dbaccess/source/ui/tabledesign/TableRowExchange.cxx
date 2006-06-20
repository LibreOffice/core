/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableRowExchange.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:33:40 $
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
    OTableRowExchange::OTableRowExchange(const ::std::vector< ::boost::shared_ptr<OTableRow> >& _rvTableRow)
        : m_vTableRow(_rvTableRow)
    {
    }
    // -----------------------------------------------------------------------------
    sal_Bool OTableRowExchange::WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& /*rFlavor*/ )
    {
        if(nUserObjectId == SOT_FORMATSTR_ID_SBA_TABED)
        {
            ::std::vector< ::boost::shared_ptr<OTableRow> >* pRows = reinterpret_cast< ::std::vector< ::boost::shared_ptr<OTableRow> >* >(pUserObject);
            if(pRows)
            {
                (*rxOStm) << (sal_Int32)pRows->size(); // first stream the size
                ::std::vector< ::boost::shared_ptr<OTableRow> >::const_iterator aIter = pRows->begin();
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
        if ( !m_vTableRow.empty() )
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
        m_vTableRow.clear();
    }
    // -----------------------------------------------------------------------------
}








