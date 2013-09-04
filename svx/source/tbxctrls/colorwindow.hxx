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

//========================================================================
// class SvxColorWindow_Impl --------------------------------------------------
//========================================================================
#ifndef __SVX_COLORWINDOW_HXX_
#define __SVX_COLORWINDOW_HXX_

#include <sfx2/tbxctrl.hxx>
#include <svtools/valueset.hxx>
#include <svl/lstner.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <svx/SvxColorValueSet.hxx>

//========================================================================
// class SvxColorWindow_Impl --------------------------------------------------
//========================================================================

class SvxColorWindow_Impl : public SfxPopupWindow
{
    using FloatingWindow::StateChanged;

private:
    const sal_uInt16 theSlotId;
    SvxColorValueSet aColorSet;
    OUString  maCommand;
    const Color    mLastColor;

    DECL_LINK( SelectHdl, void * );

protected:
    virtual void    Resize();
    virtual sal_Bool    Close();

public:
    SvxColorWindow_Impl( const OUString& rCommand,
                         sal_uInt16 nSlotId,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                         const OUString& rWndTitle,
                         Window* pParentWindow,
                         const Color rLastColor = COL_AUTO);
    ~SvxColorWindow_Impl();
    void                StartSelection();

    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );

    virtual SfxPopupWindow* Clone() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
