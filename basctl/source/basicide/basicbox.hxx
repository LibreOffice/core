/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basicbox.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-01-02 15:49:02 $
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

//

class BasicLibBox : public ListBox, public SfxListener
{
private:
    String          aCurText;
    BOOL            bIgnoreSelect;
    BOOL            bFillBox;
    com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;

    void            ReleaseFocus();
    void            InsertEntries( SfxObjectShell* pShell, LibraryLocation eLocation );

    void            FillBox( BOOL bSelect = TRUE );
    void            ClearBox();
    void            NotifyIDE();

protected:
    virtual void    Select();
    virtual long    PreNotify( NotifyEvent& rNEvt );

    virtual void    SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                            const SfxHint& rHint, const TypeId& rHintType );


public:
                    BasicLibBox( Window* pParent,
                                 const com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
                    ~BasicLibBox();

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

class BasicLanguageBox : public ListBox, public SfxListener
{
private:
    String          m_sNotLocalizedStr;
    String          m_sDefaultLanguageStr;
    String          m_sCurrentText;

    bool            m_bIgnoreSelect;

    void            FillBox();
    void            ClearBox();
    void            SetLanguage();

protected:
    virtual void    Select();
    virtual long    PreNotify( NotifyEvent& rNEvt );

    virtual void    SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                const SfxHint& rHint, const TypeId& rHintType );

public:
    BasicLanguageBox( Window* pParent );
    ~BasicLanguageBox();

    void            Update( const SfxStringItem* pItem );
};

#endif  // _BASICBOX_HXX

