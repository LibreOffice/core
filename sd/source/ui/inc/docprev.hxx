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

#ifndef _SD_DOCPREV_HXX_
#define _SD_DOCPREV_HXX_

#include <com/sun/star/presentation/FadeEffect.hpp>
#include <vcl/window.hxx>
#include <tools/gen.hxx>

#include <rtl/ref.hxx>

#include <svl/lstner.hxx>
#include <svtools/colorcfg.hxx>
#include "fadedef.h"
#include "sddllapi.h"

namespace sd {
    class SlideShow;
}

class GDIMetaFile;

class SD_DLLPUBLIC SdDocPreviewWin : public Control, public SfxListener
{
protected:
    GDIMetaFile*    pMetaFile;
    sal_Bool            bInEffect;
    Link            aClickHdl;
    SfxObjectShell* mpObj;
    sal_uInt16      mnShowPage;
    Color           maDocumentColor;
    rtl::Reference< sd::SlideShow > mxSlideShow;

    virtual void    Paint( const Rectangle& rRect );
    static void     CalcSizeAndPos( GDIMetaFile* pFile, Size& rSize, Point& rPoint );
    void            ImpPaint( GDIMetaFile* pFile, OutputDevice* pVDev );

    static const int FRAME;

    svtools::ColorConfig maColorConfig;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    void updateViewSettings();

public:
                    SdDocPreviewWin( Window* pParent, const ResId& rResId );
                    ~SdDocPreviewWin();
    void            SetObjectShell( SfxObjectShell* pObj, sal_uInt16 nShowPage = 0 );
    virtual void    Resize();
    void            startPreview();

    virtual long    Notify( NotifyEvent& rNEvt );

    void            SetClickHdl( const Link& rLink ) { aClickHdl = rLink; }
    const Link&     GetClickHdl() const { return aClickHdl; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
