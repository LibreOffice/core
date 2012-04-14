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

#ifndef _VCL_IOS_CORETEXT_SALCORETEXTFONTUTILS_HXX
#define _VCL_IOS_CORETEXT_SALCORETEXTFONTUTILS_HXX

class ImplCoreTextFontData;
class ImplDevFontList;

#include <boost/unordered_map.hpp>

#include <vcl/fontcapabilities.hxx>

#include "outfont.hxx"
#include "impfont.hxx"

class ImplCoreTextFontData : public ImplFontData
{
public:
    ImplCoreTextFontData(const ImplDevFontAttributes&, CTFontRef font);
    virtual ~ImplCoreTextFontData();
    virtual ImplFontData* Clone() const;
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
    ImplCoreTextFontData* GetFontDataFromRef( CTFontRef ) const;

private:
    typedef boost::unordered_map<CTFontRef,ImplCoreTextFontData*> CoreTextFontContainer;
    CoreTextFontContainer m_aFontContainer;

    void InitGlyphFallbacks();
};

#endif  // _VCL_IOS_CORETEXT_SALCORETEXTFONTUTILS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
