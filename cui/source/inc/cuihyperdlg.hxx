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

#ifndef INCLUDED_CUI_SOURCE_INC_CUIHYPERDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_CUIHYPERDLG_HXX

#include <svx/hlnkitem.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/image.hxx>

#include "iconcdlg.hxx"

/*************************************************************************
|*
|* Hyperlink-Dialog
|*
\************************************************************************/

class SvxHpLinkDlg;
class SvxHlinkCtrl : public SfxControllerItem
{
private:
    VclPtr<SvxHpLinkDlg> pParent;

    SfxStatusForwarder aRdOnlyForwarder;

public:
    SvxHlinkCtrl( sal_uInt16 nId, SfxBindings & rBindings, SvxHpLinkDlg* pDlg);
    virtual void dispose() override;

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                const SfxPoolItem* pState ) override;
};



/*************************************************************************
|*
|* Hyperlink-Dialog
|*
\************************************************************************/

class SvxHpLinkDlg : public IconChoiceDialog
{
private:
    SvxHlinkCtrl        maCtrl;         ///< Controller
    SfxBindings*        mpBindings;
    SfxItemSet*         mpItemSet;

    bool            mbGrabFocus : 1;
    bool            mbReadOnly  : 1;
    bool            mbIsHTMLDoc : 1;

    DECL_LINK_TYPED (ClickOkHdl_Impl, Button *, void );
    DECL_LINK_TYPED (ClickApplyHdl_Impl, Button *, void );
    DECL_LINK_TYPED (ClickCloseHdl_Impl, Button *, void );

protected:
    virtual bool            Close() override;
    virtual void            Move() override;
    void Apply();

public:
    SvxHpLinkDlg (vcl::Window* pParent, SfxBindings* pBindings );
    virtual ~SvxHpLinkDlg ();
    virtual void dispose() override;

    virtual void            PageCreated( sal_uInt16 nId, IconChoicePage& rPage ) override;

    sal_uInt16              SetPage( SvxHyperlinkItem* pItem );
    void                    SetReadOnlyMode( bool bReadOnly );
    inline bool             IsHTMLDoc() const { return mbIsHTMLDoc; }

    inline SfxBindings*     GetBindings() const { return mpBindings; };
    inline SfxDispatcher*   GetDispatcher() const { return mpBindings->GetDispatcher(); }
};


#endif // INCLUDED_CUI_SOURCE_INC_CUIHYPERDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
