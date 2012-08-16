/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "scitems.hxx"          // due to enum SvxNumType
#include "popmenu.hxx"
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <cppuhelper/weakref.hxx>

//===================================================================

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
            ScEditWindow( Window* pParent, const ResId& rResId, ScEditWindowLocation eLoc );
            ~ScEditWindow();

    using Control::SetFont;
    void            SetFont( const ScPatternAttr& rPattern );
    using Control::SetText;
    void            SetText( const EditTextObject& rTextObject );
    EditTextObject* CreateTextObject();
    void            SetCharAttriutes();

    void            InsertField( const SvxFieldItem& rFld );

    void            SetNumType(SvxNumType eNumType);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    inline ScHeaderEditEngine*  GetEditEngine() const {return pEdEngine;}
protected:
    virtual void    Paint( const Rectangle& rRec );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    GetFocus();
    virtual void    LoseFocus();

private:
    ScHeaderEditEngine* pEdEngine;
    EditView*           pEdView;
    ScEditWindowLocation eLocation;
    bool mbRTL;

    com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > xAcc;
    ScAccessibleEditObject* pAcc;
};

//===================================================================
class SC_DLLPUBLIC ScExtIButton : public ImageButton
{
private:

    Timer           aTimer;
    ScPopupMenu*    pPopupMenu;
    Link            aMLink;
    sal_uInt16          nSelected;

    SC_DLLPRIVATE  DECL_LINK( TimerHdl, void*);

protected:

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt);
    virtual void    Click();

    virtual void    StartPopup();

public:

    ScExtIButton(Window* pParent, const ResId& rResId );

    void            SetPopupMenu(ScPopupMenu* pPopUp);

    sal_uInt16          GetSelected();

    void            SetMenuHdl( const Link& rLink ) { aMLink = rLink; }
    const Link&     GetMenuHdl() const { return aMLink; }

    virtual long    PreNotify( NotifyEvent& rNEvt );
};

#endif // SC_TPHFEDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
