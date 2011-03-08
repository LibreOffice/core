/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SFXDOCINF_HXX
#define _SFXDOCINF_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
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
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> i_xDocProps,
    SotStorage* i_pStorage );

/** save document meta-data to OLE stream

    @param  i_xDocProps     Document meta-data
    @param  i_pStorage      OLE Storage
    @param  i_pThumb        Thumbnail: preview bitmap
    @param  i_pGuid         Blob: Guid blob ("_PID_GUID")
    @param  i_pHyperlinks   Blob: Hyperlink blob ("_PID_HLINKS")
 */
bool SFX2_DLLPUBLIC SaveOlePropertySet(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> i_xDocProps,
    SotStorage* i_pStorage,
    const ::com::sun::star::uno::Sequence<sal_uInt8> * i_pThumb = 0,
    const ::com::sun::star::uno::Sequence<sal_uInt8> * i_pGuid = 0,
    const ::com::sun::star::uno::Sequence<sal_uInt8> * i_pHyperlinks = 0);


::com::sun::star::uno::Sequence<sal_uInt8> SFX2_DLLPUBLIC convertMetaFile(
    GDIMetaFile* i_pThumb);

} // namespace sfx2

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
