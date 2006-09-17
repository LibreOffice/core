/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlexchg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:02:29 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SVX_XMLEXCHG_HXX_
#include "xmlexchg.hxx"
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::com::sun::star::datatransfer;

    //====================================================================
    //= OXFormsTransferable
    //====================================================================
    //--------------------------------------------------------------------
    OXFormsTransferable::OXFormsTransferable( const OXFormsDescriptor &rhs ) :
        m_aDescriptor(rhs)
    {
    }
    //--------------------------------------------------------------------
    sal_uInt32 OXFormsTransferable::getDescriptorFormatId()
    {
        static sal_uInt32 s_nFormat = (sal_uInt32)-1;
        if ((sal_uInt32)-1 == s_nFormat)
        {
            s_nFormat = SotExchange::RegisterFormatName( String::CreateFromAscii("application/x-openoffice;windows_formatname=\"???\"") );
            OSL_ENSURE( (sal_uInt32)-1 != s_nFormat, "OXFormsTransferable::getDescriptorFormatId: bad exchange id!" );
        }
        return s_nFormat;
    }
    //--------------------------------------------------------------------
    void OXFormsTransferable::AddSupportedFormats()
    {
        AddFormat( SOT_FORMATSTR_ID_XFORMS );
    }
    //--------------------------------------------------------------------
    sal_Bool OXFormsTransferable::GetData( const DataFlavor& _rFlavor )
    {
        const sal_uInt32 nFormatId = SotExchange::GetFormat( _rFlavor );
        if ( SOT_FORMATSTR_ID_XFORMS == nFormatId )
        {
            return SetString( ::rtl::OUString( String::CreateFromAscii("XForms-Transferable") ), _rFlavor );
        }
        return sal_False;
    }
    //--------------------------------------------------------------------
    const OXFormsDescriptor &OXFormsTransferable::extractDescriptor( const TransferableDataHelper &_rData ) {

        using namespace ::com::sun::star::uno;
        using namespace ::com::sun::star::datatransfer;
        Reference<XTransferable> &transfer = const_cast<Reference<XTransferable> &>(_rData.GetTransferable());
        XTransferable *pInterface = transfer.get();
        OXFormsTransferable *pThis = dynamic_cast<OXFormsTransferable *>(pInterface);
        DBG_ASSERT(pThis,"XTransferable is NOT an OXFormsTransferable???");
        return pThis->m_aDescriptor;
    }


//........................................................................
}   // namespace svx
//........................................................................


