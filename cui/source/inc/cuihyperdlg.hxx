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
private :
    SvxHpLinkDlg *pParent;

    SfxStatusForwarder aRdOnlyForwarder;

public :
    SvxHlinkCtrl( sal_uInt16 nId, SfxBindings & rBindings, SvxHpLinkDlg* pDlg);

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                const SfxPoolItem* pState ) SAL_OVERRIDE;
};



/*************************************************************************
|*
|* Hyperlink-Dialog
|*
\************************************************************************/

class SvxHpLinkDlg : public IconChoiceDialog
{
private:
    SvxHlinkCtrl        maCtrl;         ///< Controler
    SfxBindings*        mpBindings;
    SfxItemSet*         mpItemSet;

    sal_Bool            mbGrabFocus : 1;
    sal_Bool            mbReadOnly  : 1;
    sal_Bool            mbIsHTMLDoc : 1;

    DECL_LINK (ClickApplyHdl_Impl, void * );
    DECL_LINK (ClickCloseHdl_Impl, void * );

protected:
    virtual bool            Close() SAL_OVERRIDE;
    virtual void            Move() SAL_OVERRIDE;

public:
    SvxHpLinkDlg (Window* pParent, SfxBindings* pBindings );
    virtual ~SvxHpLinkDlg ();

    virtual void            PageCreated( sal_uInt16 nId, IconChoicePage& rPage ) SAL_OVERRIDE;

    sal_uInt16                  SetPage( SvxHyperlinkItem* pItem );
    void                    SetReadOnlyMode( sal_Bool bReadOnly = sal_False );
    inline sal_Bool     IsHTMLDoc() const { return mbIsHTMLDoc; }

    inline SfxBindings*     GetBindings() const { return mpBindings; };
    inline SfxDispatcher*   GetDispatcher() const { return mpBindings->GetDispatcher(); }
};


#endif // INCLUDED_CUI_SOURCE_INC_CUIHYPERDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
