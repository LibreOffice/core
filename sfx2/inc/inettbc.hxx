/*************************************************************************
 *
 *  $RCSfile: inettbc.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:23 $
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

#ifndef _SFX_INETTBC_HXX
#define _SFX_INETTBC_HXX

// includes *****************************************************************
#include <tools/string.hxx>
#include <tools/urlobj.hxx>

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

struct SfxPickEntry_Impl;
class SfxURLBox : public ComboBox
{
friend class SfxMatchContext_Impl;
friend class SfxURLBox_Impl;
    Link                            aOpenHdl;
    String                          aBaseURL;
    INetProtocol                    eSmartProtocol;
    SfxMatchContext_Impl*           pCtx;
    SfxURLBox_Impl*                 pImp;
    BOOL                            bAutoCompleteMode;
    BOOL                            bOnlyDirectories;
    BOOL                            bModified;
    BOOL                            bTryAutoComplete: 1,
                                    bCtrlClick: 1;

    BOOL                            ProcessKey( const KeyCode& rCode );
    void                            TryAutoComplete( BOOL bForward, BOOL bForce );
    void                            UpdatePicklistForSmartProtocol_Impl();
    DECL_LINK(                      AutoCompleteHdl_Impl, void* );

protected:
    virtual long                    Notify( NotifyEvent& rNEvt );
    virtual void                    Select();
    virtual void                    Modify();
    virtual BOOL                    QueryDrop( DropEvent &rEvt );
    virtual BOOL                    Drop( const DropEvent &rEvt );
    virtual long                    PreNotify( NotifyEvent& rNEvt );

public:
                                    SfxURLBox( Window* pParent, INetProtocol eSmart = INET_PROT_NOT_VALID );

    void                            OpenURL( SfxPickEntry_Impl* pEntry, const String& rName, BOOL nMod ) const;
    void                            SetBaseURL( const String& rURL ) { aBaseURL = rURL; }
    const String&                   GetBaseURL() const { return aBaseURL; }
    void                            SetOpenHdl( const Link& rLink ) { aOpenHdl = rLink; }
    const Link&                     GetOpenHdl() const { return aOpenHdl; }
    void                            SetOnlyDirectories( BOOL bDir = TRUE );
    INetProtocol                    GetSmartProtocol() const { return eSmartProtocol; }
    void                            SetSmartProtocol( INetProtocol eProt );
    BOOL                            IsCtrlOpen()
                                    { return bCtrlClick; }
};

#if __PRIVATE

#include "tbxctrl.hxx"
class SfxURLToolBoxControl_Impl : public SfxToolBoxControl
{
private:
    SfxStatusForwarder      aURLForwarder;
    SfxURLBox*              GetURLBox() const;
    DECL_LINK(              OpenHdl, void* );
    DECL_LINK(              SelectHdl, void* );
public:

                            SFX_DECL_TOOLBOX_CONTROL();

                            SfxURLToolBoxControl_Impl( USHORT nId,
                                                    ToolBox& rBox,
                                                    SfxBindings& rBindings );

    virtual Window*         CreateItemWindow( Window* pParent );
    virtual void            StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
};

class SfxCancelToolBoxControl_Impl : public SfxToolBoxControl
{
public:

                            SFX_DECL_TOOLBOX_CONTROL();

                            SfxCancelToolBoxControl_Impl(
                                            USHORT nId,
                                            ToolBox& rBox,
                                            SfxBindings& rBindings );

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
};

#endif

#endif

