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

// header for class SfxItemSet
#include <svl/itemset.hxx>
#include <boost/scoped_ptr.hpp>

namespace sd {

class DrawViewShell;

class FuFormatPaintBrush : public FuText
{
public:
    TYPEINFO_OVERRIDE();

    static rtl::Reference<FuPoor> Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    virtual sal_Bool MouseMove(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt) SAL_OVERRIDE;

    virtual void Activate() SAL_OVERRIDE;
    virtual void Deactivate() SAL_OVERRIDE;

    static void GetMenuState( DrawViewShell& rDrawViewShell, SfxItemSet &rSet );
    static bool CanCopyThisType( sal_uInt32 nObjectInventor, sal_uInt16 nObjectIdentifier );

private:
    FuFormatPaintBrush ( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq);

    void DoExecute( SfxRequest& rReq ) SAL_OVERRIDE;

    bool HasContentForThisType( sal_uInt32 nObjectInventor, sal_uInt16 nObjectIdentifier ) const;
    void Paste( bool, bool );

    void implcancel();

    ::boost::shared_ptr<SfxItemSet> mpItemSet;
    bool   mbPermanent;
    bool   mbOldIsQuickTextEditMode;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
