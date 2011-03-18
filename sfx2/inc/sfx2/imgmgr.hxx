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

#ifndef _SFXIMGMGR_HXX
#define _SFXIMGMGR_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <sal/types.h>
#include <tools/gen.hxx>
#include <vcl/image.hxx>

class ToolBox;
class SfxModule;
class SfxImageManager_Impl;
class SFX2_DLLPUBLIC SfxImageManager
{
    SfxImageManager_Impl* pImp;

public:
    static SfxImageManager*  GetImageManager( SfxModule* );

                    SfxImageManager( SfxModule* pModule = 0 );
                    ~SfxImageManager();

    void            RegisterToolBox( ToolBox *pBox, sal_uInt16 nFlags=0xFFFF);
    void            ReleaseToolBox( ToolBox *pBox );

                    // get images from resources
    void            SetImages( ToolBox& rToolBox );
    void            SetImages( ToolBox& rToolBox, bool bLarge );
    void            SetImagesForceSize( ToolBox& rToolBox, bool bLarge );

    Image           GetImage( sal_uInt16 nId, bool bLarge ) const;
    Image           GetImage( sal_uInt16 nId) const;
    Image           SeekImage( sal_uInt16 nId, bool bLarge ) const;
    Image           SeekImage( sal_uInt16 nId ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
