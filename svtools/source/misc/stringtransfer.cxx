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
#include "precompiled_svtools.hxx"
#include <svtools/stringtransfer.hxx>

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

