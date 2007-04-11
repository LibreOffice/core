/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stringtransfer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:35:22 $
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
#define _SVTOOLS_STRINGTRANSFER_HXX_

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

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
            @return <TRUE/> if the extraction was successfull, i.e. if the clipboard contained a string content
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

