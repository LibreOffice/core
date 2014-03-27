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

#ifndef SC_TPHFEDIT_HXX
#define SC_TPHFEDIT_HXX

#include <sfx2/tabdlg.hxx>
#include <svx/pageitem.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/timer.hxx>
#include <vcl/virdev.hxx>
#include "scdllapi.h"
#include "scitems.hxx"
#include "popmenu.hxx"
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <cppuhelper/weakref.hxx>



class ScHeaderEditEngine;
class ScPatternAttr;
class EditView;
class EditTextObject;
class SvxFieldItem;
class ScAccessibleEditObject;
class ScEditWindow;

SC_DLLPUBLIC ScEditWindow* GetScEditWindow ();

enum ScEditWindowLocation
{
    Left,
    Center,
    Right
};

class SC_DLLPUBLIC ScEditWindow : public Control
{
public:
            ScEditWindow( Window* pParent,  WinBits nBits , ScEditWindowLocation eLoc );
            ~ScEditWindow();

    using Control::SetFont;
    void            SetFont( const ScPatternAttr& rPattern );
    using Control::SetText;
    void            SetText( const EditTextObject& rTextObject );
    EditTextObject* CreateTextObject();
    void            SetCharAttriutes();

    void            InsertField( const SvxFieldItem& rFld );

    void            SetNumType(SvxNumType eNumType);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() SAL_OVERRIDE;

    ScHeaderEditEngine*  GetEditEngine() const { return pEdEngine; }
    void SetObjectSelectHdl( const Link& aLink) { aObjectSelectLink = aLink; }

    void SetLocation(ScEditWindowLocation eLoc) { eLocation = eLoc; }
protected:
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual void    GetFocus() SAL_OVERRIDE;
    virtual void    LoseFocus() SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;

private:
    ScHeaderEditEngine* pEdEngine;
    EditView*           pEdView;
    ScEditWindowLocation eLocation;
    bool mbRTL;

    com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > xAcc;
    ScAccessibleEditObject* pAcc;

    Link                aObjectSelectLink;
};


class SC_DLLPUBLIC ScExtIButton : public ImageButton
{
private:

    Timer           aTimer;
    PopupMenu*      pPopupMenu;
    Link            aMLink;
    sal_uInt16      nSelected;
    OString         aSelectedIdent;

    SAL_DLLPRIVATE  DECL_LINK( TimerHdl, void*);

protected:

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void    Click() SAL_OVERRIDE;

    virtual void    StartPopup();

public:

    ScExtIButton(Window* pParent, WinBits nBits );

    void            SetPopupMenu(PopupMenu* pPopUp);

    sal_uInt16      GetSelected() const;
    OString         GetSelectedIdent() const;

    void            SetMenuHdl( const Link& rLink ) { aMLink = rLink; }
    const Link&     GetMenuHdl() const { return aMLink; }

    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
};

#endif // SC_TPHFEDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
