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
//
#ifndef _BASICBOX_HXX
#define _BASICBOX_HXX

#include "doceventnotifier.hxx"
#include <svl/stritem.hxx>
#include <svheader.hxx>
#include <sfx2/tbxctrl.hxx>
#include <vcl/lstbox.hxx>


class LibBoxControl: public SfxToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();

                        LibBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
                        ~LibBoxControl();

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );
};

/** base class for list boxes which need to update their content according to the list
    of open documents
*/
class DocListenerBox    :public ListBox
                        ,public ::basctl::DocumentEventListener
{
protected:
    DocListenerBox( Window* pParent );
    ~DocListenerBox();

protected:
    virtual void    FillBox() = 0;

private:
    // DocumentEventListener
    virtual void onDocumentCreated( const ScriptDocument& _rDocument );
    virtual void onDocumentOpened( const ScriptDocument& _rDocument );
    virtual void onDocumentSave( const ScriptDocument& _rDocument );
    virtual void onDocumentSaveDone( const ScriptDocument& _rDocument );
    virtual void onDocumentSaveAs( const ScriptDocument& _rDocument );
    virtual void onDocumentSaveAsDone( const ScriptDocument& _rDocument );
    virtual void onDocumentClosed( const ScriptDocument& _rDocument );
    virtual void onDocumentTitleChanged( const ScriptDocument& _rDocument );
    virtual void onDocumentModeChanged( const ScriptDocument& _rDocument );

private:
    ::basctl::DocumentEventNotifier m_aNotifier;
};

//

class BasicLibBox : public DocListenerBox
{
private:
    String          aCurText;
    sal_Bool            bIgnoreSelect;
    sal_Bool            bFillBox;
    com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;

    void            ReleaseFocus();
    void            InsertEntries( const ScriptDocument& rDocument, LibraryLocation eLocation );

    void            ClearBox();
    void            NotifyIDE();

    // DocListenerBox
    virtual void    FillBox();

protected:
    virtual void    Select();
    virtual long    PreNotify( NotifyEvent& rNEvt );

public:
                    BasicLibBox( Window* pParent,
                                 const com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
                    ~BasicLibBox();

    using           Window::Update;
    void            Update( const SfxStringItem* pItem );
};

class LanguageBoxControl: public SfxToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();

                        LanguageBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
                        ~LanguageBoxControl();

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );
};

class BasicLanguageBox : public DocListenerBox
{
private:
    String          m_sNotLocalizedStr;
    String          m_sDefaultLanguageStr;
    String          m_sCurrentText;

    bool            m_bIgnoreSelect;

    void            ClearBox();
    void            SetLanguage();

    // DocListenerBox
    virtual void    FillBox();

protected:
    virtual void    Select();
    virtual long    PreNotify( NotifyEvent& rNEvt );

public:
    BasicLanguageBox( Window* pParent );
    ~BasicLanguageBox();

    using           Window::Update;
    void            Update( const SfxStringItem* pItem );
};

#endif  // _BASICBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
