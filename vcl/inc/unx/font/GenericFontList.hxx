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

#include <sal/config.h>

#include <unx/font/GenericFontFace.hxx>

#include <o3tl/sorted_vector.hxx>
#include <rtl/ref.hxx>
#include <vcl/dllapi.h>

#include <fontattributes.hxx>

#include <memory>
#include <string_view>
#include <unordered_map>

namespace vcl::font
{
class PhysicalFontCollection;
}

int GetDefaultAntiAliasPrio();

 /**
  * The GenericFontList is the list of the fonts we know about.
  *
  * It enumerates them from fontconfig and maps a font id to the GenericFontFace
  * for it. The faces it hands to a PhysicalFontCollection are the very same
  * objects, shared by every collection.
  **/
class VCL_DLLPUBLIC GenericFontList final
{
public:
    GenericFontList(const GenericFontList&) = delete;
    GenericFontList& operator=(const GenericFontList&) = delete;

    SAL_DLLPRIVATE ~GenericFontList();

    static GenericFontList& get();

    bool                    AddFontFile(std::u16string_view rFileUrl, const OUString& rFontName);
    void                    RemoveFontFile(std::u16string_view rFileUrl);

    const GenericFontFace* FindFontFace(const OString& rFileName, int nFaceNum,
                                         int nVariationNum) const;

    SAL_DLLPRIVATE void     AnnounceFonts( vcl::font::PhysicalFontCollection* ) const;

private:
    // to access the constructor
    friend class GenericUnixSalData;
    SAL_DLLPRIVATE explicit GenericFontList();

    SAL_DLLPRIVATE void Init();

    SAL_DLLPRIVATE void AddFontFace(const FontAttributes& rDFA, const OString& rFileName,
                                    int nFaceNum, int nVariationNum);

    typedef std::unordered_map<sal_IntPtr, rtl::Reference<GenericFontFace>> FontFaceList;

    sal_IntPtr              m_nNextFontId = 1;
    FontFaceList            m_aFontFaceList;
    std::unordered_map<OString, o3tl::sorted_vector<sal_IntPtr>> m_aFontFileToFontId;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
