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

#include <sal/config.h>

#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <eeobj.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <editeng/editeng.hxx>
#include <svl/itempool.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <osl/mutex.hxx>

using namespace ::com::sun::star;


EditDataObject::EditDataObject()
{
}

EditDataObject::~EditDataObject()
{
}

// uno::XInterface
uno::Any EditDataObject::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = ::cppu::queryInterface( rType, (static_cast< datatransfer::XTransferable* >(this)) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// datatransfer::XTransferable
uno::Any EditDataObject::getTransferData( const datatransfer::DataFlavor& rFlavor )
{
    uno::Any aAny;

    SotClipboardFormatId nT = SotExchange::GetFormat( rFlavor );
    if ( nT == SotClipboardFormatId::STRING )
    {
        aAny <<= GetString();
    }
    else if ( ( nT == SotClipboardFormatId::EDITENGINE ) || ( nT == SotClipboardFormatId::RTF ) || ( nT == SotClipboardFormatId::RICHTEXT ) )
    {
        // No RTF on demand any more:
        // 1) Was not working, because I had to flush() the clipboard immediately anyway
        // 2) Don't have the old pool defaults and the StyleSheetPool here.

        SvMemoryStream* pStream = ( nT == SotClipboardFormatId::EDITENGINE ) ? &GetStream() : &GetRTFStream();
        pStream->Seek( STREAM_SEEK_TO_END );
        sal_Int32 nLen = pStream->Tell();
        if (nLen < 0) { abort(); }
        pStream->Seek(0);

        uno::Sequence< sal_Int8 > aSeq( nLen );
        memcpy( aSeq.getArray(), pStream->GetData(), nLen );
        aAny <<= aSeq;
    }
    else
    {
        datatransfer::UnsupportedFlavorException aException;
        throw( aException );
    }

    return aAny;
}

uno::Sequence< datatransfer::DataFlavor > EditDataObject::getTransferDataFlavors(  )
{
    uno::Sequence< datatransfer::DataFlavor > aDataFlavors(4);
    SotExchange::GetFormatDataFlavor( SotClipboardFormatId::EDITENGINE, aDataFlavors.getArray()[0] );
    SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aDataFlavors.getArray()[1] );
    SotExchange::GetFormatDataFlavor( SotClipboardFormatId::RTF, aDataFlavors.getArray()[2] );
    SotExchange::GetFormatDataFlavor( SotClipboardFormatId::RICHTEXT, aDataFlavors.getArray()[3] );

    return aDataFlavors;
}

sal_Bool EditDataObject::isDataFlavorSupported( const datatransfer::DataFlavor& rFlavor )
{
    bool bSupported = false;

    SotClipboardFormatId nT = SotExchange::GetFormat( rFlavor );
    if ( ( nT == SotClipboardFormatId::STRING ) || ( nT == SotClipboardFormatId::RTF ) || ( nT == SotClipboardFormatId::RICHTEXT )
        || ( nT == SotClipboardFormatId::EDITENGINE ) )
        bSupported = true;

    return bSupported;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
