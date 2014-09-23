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

#ifndef INCLUDED_SD_SOURCE_UI_INC_DOCPREV_HXX
#define INCLUDED_SD_SOURCE_UI_INC_DOCPREV_HXX

#include <com/sun/star/presentation/FadeEffect.hpp>
#include <vcl/window.hxx>
#include <tools/gen.hxx>
#include <tools/wintypes.hxx>

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
    bool            bInEffect;
    Link            aClickHdl;
    SfxObjectShell* mpObj;
    sal_uInt16      mnShowPage;
    Color           maDocumentColor;
    rtl::Reference< sd::SlideShow > mxSlideShow;

    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual Size    GetOptimalSize() const SAL_OVERRIDE;
    static void     CalcSizeAndPos( GDIMetaFile* pFile, Size& rSize, Point& rPoint );
    void            ImpPaint( GDIMetaFile* pFile, OutputDevice* pVDev );

    static const int FRAME;

    svtools::ColorConfig maColorConfig;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) SAL_OVERRIDE;

    void updateViewSettings();

public:
                    SdDocPreviewWin( vcl::Window* pParent, const WinBits nStyle );
                    virtual ~SdDocPreviewWin();
    void            SetObjectShell( SfxObjectShell* pObj, sal_uInt16 nShowPage = 0 );
    virtual void    Resize() SAL_OVERRIDE;
    void            startPreview();

    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    void            SetClickHdl( const Link& rLink ) { aClickHdl = rLink; }
    const Link&     GetClickHdl() const { return aClickHdl; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
