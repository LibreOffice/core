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
#ifndef _SFXPREVIEW_HXX
#define _SFXPREVIEW_HXX


#include <sfx2/doctempl.hxx>
#include <sfx2/basedlgs.hxx>

class SfxObjectShell;
class SfxFrameWindow;

class SfxFrame;
class GDIMetaFile;

class SfxPreviewBase_Impl : public Window
{
protected:
    ::boost::shared_ptr<GDIMetaFile> pMetaFile;
public:
                    SfxPreviewBase_Impl( Window* pParent,
                                         const ResId& rResId );
                    SfxPreviewBase_Impl( Window* pParent );
                    ~SfxPreviewBase_Impl(  );
    void            SetObjectShell( SfxObjectShell* pObj );
    void            SetGDIFile( ::boost::shared_ptr<GDIMetaFile> pFile );
    virtual void    Resize();
};



class SfxPreviewWin_Impl: public SfxPreviewBase_Impl
{
protected:
    virtual void    Paint( const Rectangle& rRect );
public:
                    SfxPreviewWin_Impl( Window* pParent,
                                    const ResId& rResId )
                    : SfxPreviewBase_Impl( pParent, rResId ){};
                    SfxPreviewWin_Impl( Window* pParent )
                    : SfxPreviewBase_Impl( pParent ){};
    static void     ImpPaint(
        const Rectangle& rRect, GDIMetaFile* pFile, Window* pWindow );
    static SfxFrameWindow*
                    PreviewFactory( SfxFrame* pFrame, const String& rName );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
