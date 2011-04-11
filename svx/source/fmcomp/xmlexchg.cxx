/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "svx/xmlexchg.hxx"
#include <sot/formats.hxx>
#include <sot/exchange.hxx>
#include <tools/debug.hxx>

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
