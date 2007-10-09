/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basicbox.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:21:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
//
#ifndef _BASICBOX_HXX
#define _BASICBOX_HXX

#include "doceventnotifier.hxx"

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVHEADER_HXX
#include <svheader.hxx>
#endif

#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif


class LibBoxControl: public SfxToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();

                        LibBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
                        ~LibBoxControl();

    virtual void        StateChanged( USHORT nSID, SfxItemState eState,
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
    BOOL            bIgnoreSelect;
    BOOL            bFillBox;
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

                        LanguageBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
                        ~LanguageBoxControl();

    virtual void        StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
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

