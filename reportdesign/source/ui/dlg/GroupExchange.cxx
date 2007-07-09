/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GroupExchange.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:29 $
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
#include "precompiled_reportdesign.hxx"

#ifndef RPTUI_GROUP_EXCHANGE_HXX
#include "GroupExchange.hxx"
#endif

#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif

namespace rptui
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    sal_uInt32 OGroupExchange::getReportGroupId()
    {
        static sal_uInt32 s_nReportFormat = (sal_uInt32)-1;
        if ( (sal_uInt32)-1 == s_nReportFormat )
        {
            s_nReportFormat = SotExchange::RegisterFormatName(String::CreateFromAscii("application/x-openoffice;windows_formatname=\"reportdesign.GroupFormat\"" ));
            OSL_ENSURE((sal_uInt32)-1 != s_nReportFormat, "Bad exchange id!");
        }
        return s_nReportFormat;
    }
    OGroupExchange::OGroupExchange(const uno::Sequence< uno::Any >& _aGroupRow)
        : m_aGroupRow(_aGroupRow)
    {
    }
    // -----------------------------------------------------------------------------
    void OGroupExchange::AddSupportedFormats()
    {
        if ( m_aGroupRow.getLength() )
        {
            AddFormat(OGroupExchange::getReportGroupId());
        }
    }
    // -----------------------------------------------------------------------------
    sal_Bool OGroupExchange::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
    {
        ULONG nFormat = SotExchange::GetFormat(rFlavor);
        if(nFormat == OGroupExchange::getReportGroupId() )
        {
            return SetAny(uno::makeAny(m_aGroupRow),rFlavor);
        }
        return sal_False;
    }
    // -----------------------------------------------------------------------------
    void OGroupExchange::ObjectReleased()
    {
        m_aGroupRow.realloc(0);
    }
    // -----------------------------------------------------------------------------
}
