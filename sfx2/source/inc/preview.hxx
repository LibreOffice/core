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
#ifndef INCLUDED_SFX2_SOURCE_INC_PREVIEW_HXX
#define INCLUDED_SFX2_SOURCE_INC_PREVIEW_HXX


#include <sfx2/doctempl.hxx>
#include <sfx2/basedlgs.hxx>

class SfxObjectShell;
class GDIMetaFile;

class SfxPreviewBase_Impl : public vcl::Window
{
protected:
    ::boost::shared_ptr<GDIMetaFile> pMetaFile;
public:
    SfxPreviewBase_Impl(vcl::Window* pParent, WinBits nStyle);
    void            SetObjectShell( SfxObjectShell* pObj );
    virtual void    Resize() SAL_OVERRIDE;
    virtual Size    GetOptimalSize() const SAL_OVERRIDE;
};

class SfxPreviewWin_Impl: public SfxPreviewBase_Impl
{
protected:
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
public:
    SfxPreviewWin_Impl(vcl::Window* pParent, WinBits nStyle)
        : SfxPreviewBase_Impl(pParent, nStyle)
    {
    }
    static void     ImpPaint(
        const Rectangle& rRect, GDIMetaFile* pFile, vcl::Window* pWindow );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
