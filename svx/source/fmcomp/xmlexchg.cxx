/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


