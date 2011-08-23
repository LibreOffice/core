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

#include <bf_svx/pageitem.hxx>

#include <bf_svtools/stdctrl.hxx>

#include <vcl/group.hxx>

#include "scitems.hxx"          // wegen enum SvxNumType

#include <vcl/timer.hxx>

#include <vcl/virdev.hxx>

#include <popmenu.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <cppuhelper/weakref.hxx>
namespace binfilter {

//===================================================================

class ScHeaderEditEngine;
class ScPatternAttr;
class EditView;
class EditTextObject;
class SvxFieldItem;
class ScAccessibleEditObject;

enum ScEditWindowLocation
{
    Left,
    Center,
    Right
};

class ScEditWindow : public Control
{
public:
            ScEditWindow( Window* pParent, const ResId& rResId, ScEditWindowLocation eLoc );
            ~ScEditWindow();

    void 			SetFont( const ScPatternAttr& rPattern );
    void 			SetText( const EditTextObject& rTextObject );
    EditTextObject*	CreateTextObject();
    void			SetCharAttriutes();

    void			InsertField( const SvxFieldItem& rFld );

    void			SetNumType(SvxNumType eNumType);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

protected:
    virtual void	Paint( const Rectangle& rRec );
    virtual void	MouseMove( const MouseEvent& rMEvt );
    virtual void	MouseButtonDown( const MouseEvent& rMEvt );
    virtual void	MouseButtonUp( const MouseEvent& rMEvt );
    virtual void	KeyInput( const KeyEvent& rKEvt );
    virtual void	Command( const CommandEvent& rCEvt );
    virtual void	GetFocus();
    virtual void    LoseFocus();

private:
    ScHeaderEditEngine*	pEdEngine;
    EditView*			pEdView;
    ScEditWindowLocation eLocation;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > xAcc;
    ScAccessibleEditObject* pAcc;
};

//===================================================================
class ScExtIButton : public ImageButton
{
private:

    Timer			aTimer;
    ScPopupMenu*	pPopupMenu;
    Link			aMLink;
    USHORT			nSelected;

    DECL_LINK( TimerHdl, Timer*);

//	void			DrawArrow();

protected:

    virtual void	MouseButtonDown( const MouseEvent& rMEvt );
    virtual void	MouseButtonUp( const MouseEvent& rMEvt);
    virtual void	Click();

    virtual void	StartPopup();

public:

    ScExtIButton(Window* pParent, const ResId& rResId );

    void			SetPopupMenu(ScPopupMenu* pPopUp);

    USHORT			GetSelected();

    void            SetMenuHdl( const Link& rLink ) { aMLink = rLink; }
    const Link&     GetMenuHdl() const { return aMLink; }

    virtual long	PreNotify( NotifyEvent& rNEvt );
};


//===================================================================

class ScHFEditPage : public SfxTabPage
{
public:
    virtual	BOOL	FillItemSet	( SfxItemSet& rCoreSet );
    virtual	void	Reset		( const SfxItemSet& rCoreSet );

    void			SetNumType(SvxNumType eNumType);

protected:
                ScHFEditPage( Window*			pParent,
                              USHORT			nResId,
                              const SfxItemSet&	rCoreSet,
                              USHORT			nWhich );
    virtual		~ScHFEditPage();

private:
    FixedText		aFtLeft;
    ScEditWindow	aWndLeft;
    FixedText		aFtCenter;
    ScEditWindow	aWndCenter;
    FixedText		aFtRight;
    ScEditWindow	aWndRight;
    ImageButton		aBtnText;
    ScExtIButton	aBtnFile;
    ImageButton		aBtnTable;
    ImageButton		aBtnPage;
    ImageButton		aBtnLastPage;
    ImageButton		aBtnDate;
    ImageButton		aBtnTime;
    FixedLine		aFlInfo;
    FixedInfo		aFtInfo;
    ScPopupMenu		aPopUpFile;

    USHORT			nWhich;
    String			aCmdArr[6];

private:
#ifdef _TPHFEDIT_CXX
    void FillCmdArr();
    DECL_LINK( ClickHdl, ImageButton* );
    DECL_LINK( MenuHdl, ScExtIButton* );
#endif
};

//===================================================================

class ScRightHeaderEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*	Create( Window* pParent, const SfxItemSet& rCoreSet );
    static USHORT*		GetRanges();

private:
    ScRightHeaderEditPage( Window* pParent, const SfxItemSet& rSet );
};

//===================================================================

class ScLeftHeaderEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*	Create( Window* pParent, const SfxItemSet& rCoreSet );
    static USHORT*		GetRanges();

private:
    ScLeftHeaderEditPage( Window* pParent, const SfxItemSet& rSet );
};

//===================================================================

class ScRightFooterEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*	Create( Window* pParent, const SfxItemSet& rCoreSet );
    static USHORT*		GetRanges();

private:
    ScRightFooterEditPage( Window* pParent, const SfxItemSet& rSet );
};

//===================================================================

class ScLeftFooterEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*	Create( Window* pParent, const SfxItemSet& rCoreSet );
    static USHORT*		GetRanges();

private:
    ScLeftFooterEditPage( Window* pParent, const SfxItemSet& rSet );
};



} //namespace binfilter
#endif // SC_TPHFEDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
