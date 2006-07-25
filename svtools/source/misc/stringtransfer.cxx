/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stringtransfer.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 09:48:31 $
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

#ifndef _SVTOOLS_STRINGTRANSFER_HXX_
#include "stringtransfer.hxx"
#endif

//........................................................................
namespace svt
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::datatransfer;

    //====================================================================
    //= OStringTransferable
    //====================================================================
    //--------------------------------------------------------------------
    OStringTransferable::OStringTransferable(const ::rtl::OUString& _rContent)
        :TransferableHelper()
        ,m_sContent( _rContent )
    {
    }

    //--------------------------------------------------------------------
    void OStringTransferable::AddSupportedFormats()
    {
        AddFormat(SOT_FORMAT_STRING);
    }

    //--------------------------------------------------------------------
    sal_Bool OStringTransferable::GetData( const DataFlavor& _rFlavor )
    {
        sal_uInt32 nFormat = SotExchange::GetFormat( _rFlavor );
        if (SOT_FORMAT_STRING == nFormat)
            return SetString( m_sContent, _rFlavor );

        return sal_False;
    }

    //====================================================================
    //= OStringTransfer
    //====================================================================
    //--------------------------------------------------------------------
    void OStringTransfer::CopyString( const ::rtl::OUString& _rContent, Window* _pWindow )
    {
        OStringTransferable* pTransferable = new OStringTransferable( _rContent );
        Reference< XTransferable > xTransfer = pTransferable;
        pTransferable->CopyToClipboard( _pWindow );
    }

    //--------------------------------------------------------------------
    sal_Bool OStringTransfer::PasteString( ::rtl::OUString& _rContent, Window* _pWindow )
    {
        TransferableDataHelper aClipboardData = TransferableDataHelper::CreateFromSystemClipboard( _pWindow );

        // check for a string format
        const DataFlavorExVector& rFormats = aClipboardData.GetDataFlavorExVector();
        for (   DataFlavorExVector::const_iterator aSearch = rFormats.begin();
                aSearch != rFormats.end();
                ++aSearch
            )
        {
            if (SOT_FORMAT_STRING == aSearch->mnSotId)
            {
                String sContent;
                sal_Bool bSuccess = aClipboardData.GetString( SOT_FORMAT_STRING, sContent );
                _rContent = sContent;
                return bSuccess;
            }
        }

        return sal_False;
    }

    //--------------------------------------------------------------------
    void OStringTransfer::StartStringDrag( const ::rtl::OUString& _rContent, Window* _pWindow, sal_Int8 _nDragSourceActions )
    {
        OStringTransferable* pTransferable = new OStringTransferable( _rContent );
        Reference< XTransferable > xTransfer = pTransferable;
        pTransferable->StartDrag(_pWindow, _nDragSourceActions);
    }

//........................................................................
}   // namespace svt
//........................................................................

