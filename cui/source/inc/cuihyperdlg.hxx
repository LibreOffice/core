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

#ifndef _CUI_TAB_HYPERLINK_HXX
#define _CUI_TAB_HYPERLINK_HXX

// include ---------------------------------------------------------------

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

    SfxStatusForwarder aOnlineForwarder;
    SfxStatusForwarder aRdOnlyForwarder;

public :
    SvxHlinkCtrl( sal_uInt16 nId, SfxBindings & rBindings, SvxHpLinkDlg* pDlg);

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                const SfxPoolItem* pState );
};



/*************************************************************************
|*
|* Hyperlink-Dialog
|*
\************************************************************************/

class SvxHpLinkDlg : public IconChoiceDialog
{
private:
    SvxHlinkCtrl        maCtrl;         // Controler
    SfxBindings*        mpBindings;
    SfxItemSet*         mpItemSet;

    sal_Bool            mbDummy1    : 1;
    sal_Bool            mbDummy2    : 1;
    sal_Bool            mbDummy3    : 1;
    sal_Bool            mbDummy4    : 1;
    sal_Bool            mbDummy5    : 1;
    sal_Bool            mbGrabFocus : 1;
    sal_Bool            mbReadOnly  : 1;
    sal_Bool            mbIsHTMLDoc : 1;

    void*               mpDummy1;
    void*               mpDummy2;

    DECL_LINK (ClickApplyHdl_Impl, void * );
    DECL_LINK (ClickCloseHdl_Impl, void * );

protected:
    virtual sal_Bool            Close();
    virtual void            Move();

//    virtual long          PreNotify( NotifyEvent& rNEvt );
public:
    SvxHpLinkDlg (Window* pParent, SfxBindings* pBindings );
    ~SvxHpLinkDlg ();

    virtual void            PageCreated( sal_uInt16 nId, IconChoicePage& rPage );

    sal_uInt16                  SetPage( SvxHyperlinkItem* pItem );
    void                    EnableInetBrowse( sal_Bool bEnable = sal_True );
    void                    SetReadOnlyMode( sal_Bool bReadOnly = sal_False );
    inline sal_Bool     IsHTMLDoc() const { return mbIsHTMLDoc; }

    inline SfxBindings*     GetBindings() const { return mpBindings; };
    inline SfxDispatcher*   GetDispatcher() const { return mpBindings->GetDispatcher(); }
};


#endif // _CUI_TAB_HYPERLINK_HXX
