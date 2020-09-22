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

#pragma once

#include <sal/types.h>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "Fetc.hxx"

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <vector>

/*****************************************************************
    a simple container for FORMATECT structures
    instances of this class are not thread-safe
*****************************************************************/

class CFormatEtcContainer
{
public:
    CFormatEtcContainer( );

    // duplicates not allowed
    void addFormatEtc( const CFormatEtc& fetc );

    // removes the specified formatetc
    void removeFormatEtc( const CFormatEtc& fetc );

    // removes the formatetc at pos
    void removeAllFormatEtc( );

    bool hasFormatEtc( const CFormatEtc& fetc ) const;

    bool hasElements( ) const;

    // begin enumeration
    void beginEnumFormatEtc( );

    // copies the specified number of formatetc structures starting
    // at the current enum position
    // the return value is the number of copied elements; if the
    // current enum position is at the end the return value is 0
    sal_uInt32 nextFormatEtc( LPFORMATETC lpFetc, sal_uInt32 aNum = 1 );

    // skips the specified number of elements in the container
    bool skipFormatEtc( sal_uInt32 aNum );

protected:
    typedef std::vector< CFormatEtc > FormatEtcMap_t;

private:
    FormatEtcMap_t           m_FormatMap;
    FormatEtcMap_t::iterator m_EnumIterator;
};

/*****************************************************************
    a helper class which converts data flavors to clipformats,
    creates an appropriate formatetc structures and if possible
    synthesizes clipboard formats if necessary, e.g. if text
    is provided a locale will also be provided;
    the class registers the formatetc within a CFormatEtcContainer

    instances of this class are not thread-safe and multiple
    instances of this class would use the same static variables
    that's why this class should not be used by multiple threads,
    only one thread of a process should use it
*****************************************************************/

// forward
class CDataFormatTranslator;

class CFormatRegistrar
{
public:
    CFormatRegistrar( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                      const CDataFormatTranslator& aDataFormatTranslator );

    void RegisterFormats( const css::uno::Reference< css::datatransfer::XTransferable >& aXTransferable,
                                   CFormatEtcContainer& aFormatEtcContainer );

    bool hasSynthesizedLocale( ) const;
    static LCID getSynthesizedLocale( );
    static sal_uInt32 getRegisteredTextCodePage( );
    css::datatransfer::DataFlavor getRegisteredTextFlavor( ) const;

    static bool isSynthesizeableFormat( const CFormatEtc& aFormatEtc );
    static bool needsToSynthesizeAccompanyFormats( const CFormatEtc& aFormatEtc );

private:
    OUString getCharsetFromDataFlavor( const css::datatransfer::DataFlavor& aFlavor );

    bool hasUnicodeFlavor(
        const css::uno::Reference< css::datatransfer::XTransferable >& aXTransferable ) const;

    static bool findLocaleForTextCodePage( );

    static bool isLocaleOemCodePage( LCID lcid, sal_uInt32 codepage );
    static bool isLocaleAnsiCodePage( LCID lcid, sal_uInt32 codepage );
    static bool isLocaleCodePage( LCID lcid, LCTYPE lctype, sal_uInt32 codepage );

    static BOOL CALLBACK EnumLocalesProc( LPSTR lpLocaleStr );

private:
    const CDataFormatTranslator&             m_DataFormatTranslator;
    bool                                     m_bHasSynthesizedLocale;
    css::datatransfer::DataFlavor            m_RegisteredTextFlavor;

    const css::uno::Reference< css::uno::XComponentContext >  m_xContext;

    static LCID       m_TxtLocale;
    static sal_uInt32 m_TxtCodePage;

private:
    CFormatRegistrar( const CFormatRegistrar& );
    CFormatRegistrar& operator=( const CFormatRegistrar& );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
