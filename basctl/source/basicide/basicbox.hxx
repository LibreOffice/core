/*************************************************************************
 *
 *  $RCSfile: basicbox.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-23 12:01:03 $
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

protected:
    virtual void    Select();
    virtual long    PreNotify( NotifyEvent& rNEvt );

    virtual void    SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                            const SfxHint& rHint, const TypeId& rHintType );


public:
                    BasicLibBox( Window* pParent,
                                 const com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
                    ~BasicLibBox();

    void            FillBox( BOOL bSelect = TRUE );
    void            Update( const SfxStringItem* pItem );
    void            NotifyIDE();
};


#endif  // _BASICBOX_HXX

