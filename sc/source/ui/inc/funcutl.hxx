/*************************************************************************
 *
 *  $RCSfile: funcutl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_FUNCUTL_HXX
#define SC_FUNCUTL_HXX

#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"     // ScRefButton
#endif


//============================================================================
// class ValWnd

class ValWnd : public Window
{
public:
            ValWnd( Window* pParent, const ResId& rId );

    void    SetValue( const String& rStrVal );

protected:
    virtual void    Paint( const Rectangle& rRect );

private:
    String      aStrValue;
    Rectangle   aRectOut;
};


//============================================================================
// class ScEditBox

class ScEditBox : public Control
{
private:

    MultiLineEdit*  pMEdit;
    Link            aSelChangedLink;
    Selection       aOldSel;
    BOOL            bMouseFlag;
                    DECL_LINK( ChangedHdl, ScEditBox* );

protected:

    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual void    SelectionChanged();
    virtual void    Resize();
    virtual void    GetFocus();


public:
                    ScEditBox( Window* pParent,
                                WinBits nWinStyle = WB_LEFT | WB_BORDER );
                    ScEditBox( Window* pParent, const ResId& rResId );

                    ~ScEditBox();

    MultiLineEdit*  GetEdit() {return pMEdit;}

    void            SetSelChangedHdl( const Link& rLink ) { aSelChangedLink = rLink; }
    const Link&     GetSelChangedHdl() const { return aSelChangedLink; }
};



//============================================================================
// class ArgEdit

class ArgEdit : public ScRefEdit
{
public:
            ArgEdit( Window* pParent, WinBits nWinStyle = WB_LEFT | WB_BORDER );
            ArgEdit( Window* pParent, const ResId& rResId );

    void    Init( ArgEdit* pPrevEdit, ArgEdit* pNextEdit,
                  ScrollBar& rArgSlider, USHORT nArgCount );

protected:
    virtual void    KeyInput( const KeyEvent& rKEvt );

private:
    ArgEdit*    pEdPrev;
    ArgEdit*    pEdNext;
    ScrollBar*  pSlider;
    USHORT      nArgs;
};


//============================================================================
// class ArgInput

class ArgInput
{
private:

    Link            aFxClickLink;
    Link            aRefClickLink;
    Link            aFxFocusLink;
    Link            aRefFocusLink;
    Link            aEdFocusLink;
    Link            aEdModifyLink;

    FixedText*      pFtArg;
    ImageButton*    pBtnFx;
    ArgEdit*        pEdArg;
    ScRefButton*    pRefBtn;

    DECL_LINK(  FxBtnClickHdl, ImageButton* );
    DECL_LINK(  RefBtnClickHdl,ScRefButton* );
    DECL_LINK(  FxBtnFocusHdl, ImageButton* );
    DECL_LINK(  RefBtnFocusHdl,ScRefButton* );
    DECL_LINK(  EdFocusHdl, ArgEdit* );
    DECL_LINK(  EdModifyHdl,ArgEdit* );

protected:

    virtual void    FxClick();
    virtual void    RefClick();
    virtual void    FxFocus();
    virtual void    RefFocus();
    virtual void    EdFocus();
    virtual void    EdModify();

public:

    ArgInput();

    void        InitArgInput (  FixedText*      pftArg,
                                ImageButton*    pbtnFx,
                                ArgEdit*        pedArg,
                                ScRefButton*    prefBtn);

    void        SetArgName(const String &aArg);
    String      GetArgName();
    void        SetArgNameFont(const Font&);

    void        SetArgVal(const String &aVal);
    String      GetArgVal();

    void        SetArgSelection (const Selection& rSel );
    void        ReplaceSelOfArg (const String& rStr );

    Selection   GetArgSelection();


    ArgEdit*    GetArgEdPtr() {return pEdArg;}


    void            SetFxClickHdl( const Link& rLink ) { aFxClickLink = rLink; }
    const Link&     GetFxClickHdl() const { return aFxClickLink; }

    void            SetRefClickHdl( const Link& rLink ) { aRefClickLink = rLink; }
    const Link&     GetRefClickHdl() const { return aRefClickLink; }

    void            SetFxFocusHdl( const Link& rLink ) { aFxFocusLink = rLink; }
    const Link&     GetFxFocusHdl() const { return aFxFocusLink; }

    void            SetRefFocusHdl( const Link& rLink ) { aRefFocusLink = rLink; }
    const Link&     GetRefFocusHdl() const { return aRefFocusLink; }

    void            SetEdFocusHdl( const Link& rLink ) { aEdFocusLink = rLink; }
    const Link&     GetEdFocusHdl() const { return aEdFocusLink; }

    void            SetEdModifyHdl( const Link& rLink ) { aEdModifyLink = rLink; }
    const Link&     GetEdModifyHdl() const { return aEdModifyLink; }

    void Hide();
    void Show();

};


#endif
