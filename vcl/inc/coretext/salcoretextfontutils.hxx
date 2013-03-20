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

#ifndef _VCL_CORETEXT_SALCORETEXTFONTUTILS_HXX
#define _VCL_CORETEXT_SALCORETEXTFONTUTILS_HXX

#include <boost/unordered_map.hpp>

#include <vcl/fontcapabilities.hxx>

#include "coretext/common.h"
#include "outfont.hxx"
#include "impfont.hxx"

class CoreTextPhysicalFontFace : public PhysicalFontFace
{
public:
    CoreTextPhysicalFontFace(const ImplDevFontAttributes&, CTFontRef font);
    virtual ~CoreTextPhysicalFontFace();
    virtual PhysicalFontFace* Clone() const;
    virtual ImplFontEntry* CreateFontInstance( FontSelectPattern& ) const;
    virtual sal_IntPtr GetFontId() const { return (sal_IntPtr)m_CTFontRef;};
    CTFontRef GetCTFont() const { return m_CTFontRef; };
    const ImplFontCharMap* GetImplFontCharMap();
    bool GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities);
    bool HasChar( sal_uInt32 cChar ) const;
    void ReadOs2Table();
    void ReadIosCmapEncoding();
    bool HasCJKSupport();
    bool GetRawFontData( std::vector<unsigned char>& rBuffer, bool* pJustCFF ) const;

private:
    void DetermineCJKSupport_OS2(CFDataRef rOS2Table);
    void DetermineCJKSupport_cmap(CFDataRef rCmapTable);
    CTFontRef m_CTFontRef;
    mutable const ImplFontCharMap* m_pCharMap;
    mutable vcl::FontCapabilities m_aFontCapabilities;
    mutable bool m_bHasOs2Table;
    mutable bool m_bOs2TableRead;
    mutable bool m_bCmapTableRead; // true if cmap encoding of Mac font is read
    mutable bool m_bHasCJKSupport; // #i78970# CJK fonts need extra leading
    mutable bool m_bFontCapabilitiesRead;
};

/* This class has the responsibility of assembling a list of CoreText
   fonts available on the system and enabling access to that list.
 */
class SystemFontList
{
public:
    SystemFontList();
    ~SystemFontList();

    void AnnounceFonts( ImplDevFontList& ) const;
    CoreTextPhysicalFontFace* GetFontDataFromRef( CTFontRef ) const;

private:
    typedef boost::unordered_map<CTFontRef,CoreTextPhysicalFontFace*> CoreTextFontContainer;
    CoreTextFontContainer m_aFontContainer;

    void InitGlyphFallbacks();
};

#endif  // _VCL_CORETEXT_SALCORETEXTFONTUTILS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
