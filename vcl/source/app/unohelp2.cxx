/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vcl/unohelp2.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>


using namespace ::com::sun::star;

namespace vcl { namespace unohelper {

    TextDataObject::TextDataObject( const OUString& rText ) : maText( rText )
    {
    }

    TextDataObject::~TextDataObject()
    {
    }

    void TextDataObject::CopyStringTo( const OUString& rContent,
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
        uno::Any aRet = ::cppu::queryInterface( rType, (static_cast< datatransfer::XTransferable* >(this)) );
        return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
    }

    // ::com::sun::star::datatransfer::XTransferable
    uno::Any TextDataObject::getTransferData( const datatransfer::DataFlavor& rFlavor ) throw(datatransfer::UnsupportedFlavorException, io::IOException, uno::RuntimeException)
    {
        uno::Any aAny;

        sal_uLong nT = SotExchange::GetFormat( rFlavor );
        if ( nT == SOT_FORMAT_STRING )
        {
            aAny <<= (OUString)GetString();
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
        sal_uLong nT = SotExchange::GetFormat( rFlavor );
        return ( nT == SOT_FORMAT_STRING );
    }

}}  // namespace vcl::unohelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
