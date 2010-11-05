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
            @return <sal_True/> if the extraction was successfull, i.e. if the clipboard contained a string content
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

