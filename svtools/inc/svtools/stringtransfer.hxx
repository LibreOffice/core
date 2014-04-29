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



#ifndef _SVTOOLS_STRINGTRANSFER_HXX_
#define _SVTOOLS_STRINGTRANSFER_HXX_

#include "svtools/svtdllapi.h"
#include <svtools/transfer.hxx>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= OStringTransferable
    //====================================================================
    class SVT_DLLPUBLIC OStringTransferable : public TransferableHelper
    {
    protected:
        ::rtl::OUString     m_sContent;

    public:
        OStringTransferable(const ::rtl::OUString& _rContent);

    protected:
        // TransferableHelper overridables
        virtual void                AddSupportedFormats();
        virtual sal_Bool            GetData( const ::com::sun::star::datatransfer::DataFlavor& _rFlavor );
    };

    //====================================================================
    //= OStringTransfer
    //====================================================================
    class OStringTransfer
    {
    public:
        /** copies the string given into the system clipboard
            <p>Beware: In opposite to the old DragServer::CopyString, this method does not <em>add</em> the string
            to the clipboard, instead the old clipboard content is <em>replaced</em>!</p>
        */
        SVT_DLLPUBLIC static void           CopyString( const ::rtl::OUString& _rContent, Window* _pWindow = NULL );

        /** extracts a string from the system clipboard given
            @return <TRUE/> if the extraction was successful, i.e. if the clipboard contained a string content
        */
        SVT_DLLPUBLIC static sal_Bool       PasteString( ::rtl::OUString& _rContent, Window* _pWindow = NULL );

        /** starts dragging a simple string
        */
        SVT_DLLPUBLIC static void           StartStringDrag( const ::rtl::OUString& _rContent, Window* _pWindow, sal_Int8 _nDragSourceActions );
    };

//........................................................................
}   // namespace svt
//........................................................................

#endif // _SVTOOLS_STRINGTRANSFER_HXX_

