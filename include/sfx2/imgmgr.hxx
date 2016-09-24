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

#ifndef INCLUDED_SFX2_IMGMGR_HXX
#define INCLUDED_SFX2_IMGMGR_HXX

#include <sal/config.h>
#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>
#include <sfx2/dllapi.h>
#include <vcl/image.hxx>


class ToolBox;
class SfxModule;
class SfxImageManager_Impl;

enum class SfxToolboxFlags
{
    CHANGESYMBOLSET     = 0x01,
    CHANGEOUTSTYLE      = 0x02,
    ALL                 = CHANGESYMBOLSET | CHANGEOUTSTYLE,
};
namespace o3tl
{
    template<> struct typed_flags<SfxToolboxFlags> : is_typed_flags<SfxToolboxFlags, 0x03> {};
}


class SFX2_DLLPUBLIC SfxImageManager
{
    SfxImageManager_Impl* pImp;

public:
    static SfxImageManager*  GetImageManager(SfxModule&);

    SfxImageManager(SfxModule& rModule);
    ~SfxImageManager();

    void            RegisterToolBox( ToolBox *pBox, SfxToolboxFlags nFlags=SfxToolboxFlags::ALL);
    void            ReleaseToolBox( ToolBox *pBox );

    Image           GetImage( sal_uInt16 nId, bool bLarge ) const;
    Image           GetImage( sal_uInt16 nId) const;
    Image           SeekImage( sal_uInt16 nId, bool bLarge ) const;
    Image           SeekImage( sal_uInt16 nId ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
