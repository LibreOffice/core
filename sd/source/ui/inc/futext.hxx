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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FUTEXT_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FUTEXT_HXX

#include <editeng/editdata.hxx>
#include "fuconstr.hxx"
#include <svx/svdotext.hxx>

struct StyleRequestData;
class SdrTextObj;
class FontList;
class OutlinerView;

namespace sd {

/**
 * Base class for text functions
 */
class FuText
    : public FuConstruct
{
public:
    TYPEINFO_OVERRIDE();

    static rtl::Reference<FuPoor> Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );
    virtual void DoExecute( SfxRequest& rReq ) override;

    virtual bool KeyInput(const KeyEvent& rKEvt) override;
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool Command(const CommandEvent& rCEvt) override;
    virtual bool RequestHelp(const HelpEvent& rHEvt) override;
    virtual void ReceiveRequest(SfxRequest& rReq) override;
    virtual void DoubleClick(const MouseEvent& rMEvt) override;

    virtual void Activate() override;           ///< activates the function
    virtual void Deactivate() override;         ///< deactivates the function

    void    SetInEditMode(const MouseEvent& rMEvt, bool bQuickDrag);
    bool    DeleteDefaultText();
    SdrTextObj* GetTextObj() { return static_cast< SdrTextObj* >( mxTextObj.get() ); }

    virtual SdrObject* CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle) override;

    /** is called when the current function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if a active function was aborted
    */
    virtual bool cancel() override;

    static void ChangeFontSize( bool, OutlinerView*, const FontList*, ::sd::View* );

protected:
    FuText (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

private:
    virtual void disposing() override;

    SdrObjectWeakRef    mxTextObj;
    bool                bFirstObjCreated;
    bool                bJustEndedEdit;

    SfxRequest&         rRequest;

    void ImpSetAttributesForNewTextObject(SdrTextObj* pTxtObj);
    void ImpSetAttributesFitToSize(SdrTextObj* pTxtObj);
    void ImpSetAttributesFitToSizeVertical(SdrTextObj* pTxtObj);
    void ImpSetAttributesFitCommon(SdrTextObj* pTxtObj);
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
