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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TPHFEDIT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TPHFEDIT_HXX

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
            ScEditWindow( vcl::Window* pParent,  WinBits nBits , ScEditWindowLocation eLoc );
            virtual ~ScEditWindow();
    virtual void dispose() override;

    using Control::SetFont;
    void            SetFont( const ScPatternAttr& rPattern );
    using Control::SetText;
    void            SetText( const EditTextObject& rTextObject );
    EditTextObject* CreateTextObject();
    void            SetCharAttributes();

    void            InsertField( const SvxFieldItem& rFld );

    void            SetNumType(SvxNumType eNumType);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() override;

    ScHeaderEditEngine*  GetEditEngine() const { return pEdEngine; }
    void SetObjectSelectHdl( const Link<ScEditWindow&,void>& aLink) { aObjectSelectLink = aLink; }

    void SetLocation(ScEditWindowLocation eLoc) { eLocation = eLoc; }
protected:
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;
    virtual void    Resize() override;

private:
    ScHeaderEditEngine* pEdEngine;
    EditView*           pEdView;
    ScEditWindowLocation eLocation;
    bool mbRTL;

    com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > xAcc;
    ScAccessibleEditObject* pAcc;

    Link<ScEditWindow&,void> aObjectSelectLink;
};

class SC_DLLPUBLIC ScExtIButton : public ImageButton
{
private:

    Idle            aIdle;
    PopupMenu*      pPopupMenu;
    Link<ScExtIButton&,void> aMLink;
    sal_uInt16      nSelected;
    OString         aSelectedIdent;

                    DECL_DLLPRIVATE_LINK_TYPED( TimerHdl, Idle*, void );

protected:

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt) override;
    virtual void    Click() override;

    void            StartPopup();

public:

    ScExtIButton(vcl::Window* pParent, WinBits nBits );

    void            SetPopupMenu(PopupMenu* pPopUp);

    sal_uInt16      GetSelected() const { return nSelected;}
    OString         GetSelectedIdent() const { return aSelectedIdent;}

    void            SetMenuHdl( const Link<ScExtIButton&,void>& rLink ) { aMLink = rLink; }

    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
};

#endif // INCLUDED_SC_SOURCE_UI_INC_TPHFEDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
