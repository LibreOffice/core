/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <cpo/uno/Sequence.hxx>

#if !defined WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objidl.h>

/*------------------------------------------------------------------------
    input:
    aMetaFilePict - a sequence of bytes containing a METAFILEPICT struct
------------------------------------------------------------------------*/
cpo::uno::Sequence<sal_Int8> WinMFPictToOOMFPict(cpo::uno::Sequence<sal_Int8>& aMetaFilePict);
cpo::uno::Sequence<sal_Int8> WinENHMFPictToOOMFPict(HENHMETAFILE hEnhMetaFile);

/*------------------------------------------------------------------------
    input:
    aByteStream - a sequence of bytes containing a LibreOffice metafile
                  picture with a leading METAFILEHEADER
------------------------------------------------------------------------*/
HMETAFILEPICT OOMFPictToWinMFPict(cpo::uno::Sequence<sal_Int8> const& aOOMetaFilePict);
HENHMETAFILE OOMFPictToWinENHMFPict(cpo::uno::Sequence<sal_Int8> const& aOOMetaFilePict);

/*------------------------------------------------------------------------
    input:
    aWinDIB - sequence of bytes containing a windows device independent
              bitmap
------------------------------------------------------------------------*/
cpo::uno::Sequence<sal_Int8> WinDIBToOOBMP(const cpo::uno::Sequence<sal_Int8>& aWinDIB);

/*------------------------------------------------------------------------
    input:
    HBITMAP - a windows bitmap handle
------------------------------------------------------------------------*/
cpo::uno::Sequence<sal_Int8> WinBITMAPToOOBMP(HBITMAP);

/*------------------------------------------------------------------------
    input:
    aOOBmp - sequence of bytes containing a LibreOffice bitmap
    May contain CF_DIBV5 or CF_DIB, but removing the BITMAPFILEHEADER
    is always the same size
------------------------------------------------------------------------*/
cpo::uno::Sequence<sal_Int8> OOBmpToWinDIB(cpo::uno::Sequence<sal_Int8>& aOOBmp);

/*------------------------------------------------------------------------
    input:
    aTextHtml - a sequence of text/html which will be converted to the
    HTML Format; the HTML Format has header before the real html data
    the Format is described in the MSDN Library under HTML Clipboard
    Format
------------------------------------------------------------------------*/
cpo::uno::Sequence<sal_Int8> TextHtmlToHTMLFormat(cpo::uno::Sequence<sal_Int8> const& aTextHtml);

/**
    Return a FileList in which Windows Shell Links (lnk) are resolved.
    If for whatever reason a resolution is not possible leave the
    original lnk file.
*/
cpo::uno::Sequence<sal_Int8> CF_HDROPToFileList(HGLOBAL hGlobal);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
