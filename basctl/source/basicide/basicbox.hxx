/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

