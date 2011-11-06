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
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------



#include <svl/urlbmk.hxx>

#include "lnktrans.hxx"
#include "scmod.hxx"

using namespace com::sun::star;

// -----------------------------------------------------------------------

ScLinkTransferObj::ScLinkTransferObj()
{
}

ScLinkTransferObj::~ScLinkTransferObj()
{
}

void ScLinkTransferObj::SetLinkURL( const String& rURL, const String& rText )
{
    aLinkURL = rURL;
    aLinkText = rText;
}

void ScLinkTransferObj::AddSupportedFormats()
{
    if ( aLinkURL.Len() )
    {
        //  TransferableHelper::SetINetBookmark formats

        AddFormat( SOT_FORMATSTR_ID_SOLK );
        AddFormat( SOT_FORMAT_STRING );
        AddFormat( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR );
        AddFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
        AddFormat( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR );
        AddFormat( SOT_FORMATSTR_ID_FILECONTENT );
    }
}

sal_Bool ScLinkTransferObj::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    sal_Bool bOK = sal_False;
    if ( aLinkURL.Len() )
    {
        INetBookmark aBmk( aLinkURL, aLinkText );
        bOK = SetINetBookmark( aBmk, rFlavor );
    }
    return bOK;
}

void ScLinkTransferObj::ObjectReleased()
{
    TransferableHelper::ObjectReleased();
}

void ScLinkTransferObj::DragFinished( sal_Int8 nDropAction )
{
    ScModule* pScMod = SC_MOD();
    pScMod->ResetDragObject();

    TransferableHelper::DragFinished( nDropAction );
}


