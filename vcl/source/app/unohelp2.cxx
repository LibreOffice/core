/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unohelp2.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:50:58 $
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
#include "precompiled_vcl.hxx"

#ifndef _VCL_UNOHELP2_HXX
#include <unohelp2.hxx>
#endif

#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif

#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XFLUSHABLECLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#endif


using namespace ::com::sun::star;

namespace vcl { namespace unohelper {

    TextDataObject::TextDataObject( const String& rText ) : maText( rText )
    {
    }

    TextDataObject::~TextDataObject()
    {
    }

    void TextDataObject::CopyStringTo( const String& rContent,
        const uno::Reference< datatransfer::clipboard::XClipboard >& rxClipboard )
    {
        DBG_ASSERT( rxClipboard.is(), "TextDataObject::CopyStringTo: invalid clipboard!" );
        if ( !rxClipboard.is() )
            return;

        TextDataObject* pDataObj = new TextDataObject( rContent );

        const sal_uInt32 nRef = Application::ReleaseSolarMutex();
        try
        {
            rxClipboard->setContents( pDataObj, NULL );

            uno::Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( rxClipboard, uno::UNO_QUERY );
            if( xFlushableClipboard.is() )
                xFlushableClipboard->flushClipboard();
        }
        catch( const uno::Exception& )
        {
        }
        Application::AcquireSolarMutex( nRef );
    }

    // ::com::sun::star::uno::XInterface
    uno::Any TextDataObject::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
    {
        uno::Any aRet = ::cppu::queryInterface( rType, SAL_STATIC_CAST( datatransfer::XTransferable*, this ) );
        return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
    }

    // ::com::sun::star::datatransfer::XTransferable
    uno::Any TextDataObject::getTransferData( const datatransfer::DataFlavor& rFlavor ) throw(datatransfer::UnsupportedFlavorException, io::IOException, uno::RuntimeException)
    {
        uno::Any aAny;

        ULONG nT = SotExchange::GetFormat( rFlavor );
        if ( nT == SOT_FORMAT_STRING )
        {
            aAny <<= (::rtl::OUString)GetString();
        }
        else
        {
            throw datatransfer::UnsupportedFlavorException();
        }
        return aAny;
    }

    uno::Sequence< datatransfer::DataFlavor > TextDataObject::getTransferDataFlavors(  ) throw(uno::RuntimeException)
    {
        uno::Sequence< datatransfer::DataFlavor > aDataFlavors(1);
        SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aDataFlavors.getArray()[0] );
        return aDataFlavors;
    }

    sal_Bool TextDataObject::isDataFlavorSupported( const datatransfer::DataFlavor& rFlavor ) throw(uno::RuntimeException)
    {
        ULONG nT = SotExchange::GetFormat( rFlavor );
        return ( nT == SOT_FORMAT_STRING );
    }

}}  // namespace vcl::unohelper
