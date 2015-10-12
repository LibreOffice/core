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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FUFORMATPAINTBRUSH_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FUFORMATPAINTBRUSH_HXX

#include "futext.hxx"

#include <svl/itemset.hxx>
#include <memory>

namespace sd {

class DrawViewShell;

class FuFormatPaintBrush : public FuText
{
public:
    TYPEINFO_OVERRIDE();

    static rtl::Reference<FuPoor> Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool KeyInput(const KeyEvent& rKEvt) override;

    virtual void Activate() override;
    virtual void Deactivate() override;

    static void GetMenuState( DrawViewShell& rDrawViewShell, SfxItemSet &rSet );

private:
    FuFormatPaintBrush ( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq);

    void DoExecute( SfxRequest& rReq ) override;

    bool HasContentForThisType( sal_uInt32 nObjectInventor, sal_uInt16 nObjectIdentifier ) const;
    void Paste( bool, bool );

    void implcancel();

    std::shared_ptr<SfxItemSet> mxItemSet;
    bool   mbPermanent;
    bool   mbOldIsQuickTextEditMode;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
