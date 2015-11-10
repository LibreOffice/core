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
#ifndef INCLUDED_SFX2_DOCINF_HXX
#define INCLUDED_SFX2_DOCINF_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/uno/Reference.hxx>


class GDIMetaFile;
class SotStorage;

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
} } }


namespace sfx2 {

/** load document meta-data from OLE stream

    @param  i_xDocProps     Document meta-data
    @param  i_pStorage      OLE Storage
 */
sal_uInt32 SFX2_DLLPUBLIC LoadOlePropertySet(
    css::uno::Reference< css::document::XDocumentProperties> i_xDocProps,
    SotStorage* i_pStorage );

/** save document meta-data to OLE stream

    @param  i_xDocProps     Document meta-data
    @param  i_pStorage      OLE Storage
    @param  i_pThumb        Thumbnail: preview bitmap
    @param  i_pGuid         Blob: Guid blob ("_PID_GUID")
    @param  i_pHyperlinks   Blob: Hyperlink blob ("_PID_HLINKS")
 */
bool SFX2_DLLPUBLIC SaveOlePropertySet(
    css::uno::Reference< css::document::XDocumentProperties> i_xDocProps,
    SotStorage* i_pStorage,
    const css::uno::Sequence<sal_uInt8> * i_pThumb = nullptr,
    const css::uno::Sequence<sal_uInt8> * i_pGuid = nullptr,
    const css::uno::Sequence<sal_uInt8> * i_pHyperlinks = nullptr);


css::uno::Sequence<sal_uInt8> SFX2_DLLPUBLIC convertMetaFile(
    GDIMetaFile* i_pThumb);

} // namespace sfx2

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
