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

#ifndef INCLUDED_BASCTL_SOURCE_BASICIDE_BASICBOX_HXX
#define INCLUDED_BASCTL_SOURCE_BASICIDE_BASICBOX_HXX

#include <svl/stritem.hxx>
#include <sfx2/tbxctrl.hxx>
#include <vcl/lstbox.hxx>

namespace basctl
{

class LibBoxControl: public SfxToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();

                        LibBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                      const SfxPoolItem* pState ) SAL_OVERRIDE;
    virtual VclPtr<vcl::Window> CreateItemWindow( vcl::Window *pParent ) SAL_OVERRIDE;
};

/** base class for list boxes which need to update their content according to the list
    of open documents
*/
class DocListenerBox    :public ListBox
                        ,public DocumentEventListener
{
protected:
    DocListenerBox( vcl::Window* pParent );
    virtual ~DocListenerBox();
    virtual void dispose() SAL_OVERRIDE;

protected:
    virtual void    FillBox() = 0;

private:
    // DocumentEventListener
    virtual void onDocumentCreated( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentOpened( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentSave( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentSaveDone( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentSaveAs( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentSaveAsDone( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentClosed( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentTitleChanged( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentModeChanged( const ScriptDocument& _rDocument ) SAL_OVERRIDE;

private:
    DocumentEventNotifier m_aNotifier;
};



class LibBox : public DocListenerBox
{
private:
    OUString        aCurText;
    bool            bIgnoreSelect;
    bool            bFillBox;
    com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;

    static void     ReleaseFocus();
    void            InsertEntries( const ScriptDocument& rDocument, LibraryLocation eLocation );

    void            ClearBox();
    void            NotifyIDE();

    // DocListenerBox
    virtual void    FillBox() SAL_OVERRIDE;

protected:
    virtual void    Select() SAL_OVERRIDE;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

public:
                    LibBox( vcl::Window* pParent,
                                 const com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
    virtual         ~LibBox();
    virtual void    dispose() SAL_OVERRIDE;

    using           Window::Update;
    void            Update( const SfxStringItem* pItem );
};

class LanguageBoxControl: public SfxToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();

                        LanguageBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) SAL_OVERRIDE;
    virtual VclPtr<vcl::Window> CreateItemWindow( vcl::Window *pParent ) SAL_OVERRIDE;
};

class LanguageBox : public DocListenerBox
{
private:
    OUString        m_sNotLocalizedStr;
    OUString        m_sDefaultLanguageStr;
    OUString        m_sCurrentText;

    bool            m_bIgnoreSelect;

    void            ClearBox();
    void            SetLanguage();

    // DocListenerBox
    virtual void    FillBox() SAL_OVERRIDE;

protected:
    virtual void    Select() SAL_OVERRIDE;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

public:
    LanguageBox( vcl::Window* pParent );
    virtual ~LanguageBox();
    virtual void    dispose() SAL_OVERRIDE;

    using           Window::Update;
    void            Update( const SfxStringItem* pItem );
};

}

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_BASICBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
