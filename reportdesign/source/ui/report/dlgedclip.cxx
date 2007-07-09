/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedclip.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:33 $
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

#ifndef _REPORT_RPTUICLIP_HXX
#include "dlgedclip.hxx"
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPE_HPP_
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPEFACTORY_HPP_
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#endif

namespace rptui
{

using namespace comphelper;
using namespace ::com::sun::star;

//============================================================================
// OReportExchange
//============================================================================
//----------------------------------------------------------------------------
OReportExchange::OReportExchange(const TSectionElements& _rCopyElements )
: m_aCopyElements(_rCopyElements)
{
}
//--------------------------------------------------------------------
sal_uInt32 OReportExchange::getDescriptorFormatId()
{
    static sal_uInt32 s_nFormat = (sal_uInt32)-1;
    if ((sal_uInt32)-1 == s_nFormat)
    {
        s_nFormat = SotExchange::RegisterFormatName(String::CreateFromAscii("application/x-openoffice;windows_formatname=\"report.ReportObjectsTransfer\""));
        OSL_ENSURE((sal_uInt32)-1 != s_nFormat, "OReportExchange::getDescriptorFormatId: bad exchange id!");
    }
    return s_nFormat;
}
//--------------------------------------------------------------------
void OReportExchange::AddSupportedFormats()
{
    AddFormat(getDescriptorFormatId());
}
//--------------------------------------------------------------------
sal_Bool OReportExchange::GetData( const datatransfer::DataFlavor& _rFlavor )
{
    const sal_uInt32 nFormatId = SotExchange::GetFormat(_rFlavor);
    return (nFormatId == getDescriptorFormatId()) ?
        SetAny( uno::Any(m_aCopyElements), _rFlavor )
        : sal_False;
}
// -----------------------------------------------------------------------------
sal_Bool OReportExchange::canExtract(const DataFlavorExVector& _rFlavor)
{
    return IsFormatSupported(_rFlavor,getDescriptorFormatId());
}
// -----------------------------------------------------------------------------
OReportExchange::TSectionElements OReportExchange::extractCopies(const TransferableDataHelper& _rData)
{
    sal_Int32 nKnownFormatId = getDescriptorFormatId();
    if ( _rData.HasFormat( nKnownFormatId ) )
    {
        // extract the any from the transferable
        datatransfer::DataFlavor aFlavor;
#if OSL_DEBUG_LEVEL > 0
        sal_Bool bSuccess =
#endif
        SotExchange::GetFormatDataFlavor(nKnownFormatId, aFlavor);
        OSL_ENSURE(bSuccess, "OReportExchange::extractCopies: invalid data format (no flavor)!");

        uno::Any aDescriptor = _rData.GetAny(aFlavor);

        TSectionElements aCopies;
#if OSL_DEBUG_LEVEL > 0
        bSuccess =
#endif
        aDescriptor >>= aCopies;
        OSL_ENSURE(bSuccess, "OReportExchange::extractCopies: invalid clipboard format!");

        // build the real descriptor
        return aCopies;
    }

    return TSectionElements();
}
//============================================================================
} // rptui
//============================================================================
