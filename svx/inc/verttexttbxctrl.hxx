/*************************************************************************
 *
 *  $RCSfile: verttexttbxctrl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:59:53 $
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
#ifndef _SVX_VERT_TEXT_TBXCTRL_HXX
#define _SVX_VERT_TEXT_TBXCTRL_HXX

#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif
/* -----------------------------27.04.01 15:38--------------------------------
    control to remove/insert cjk settings dependent vertical text toolbox item
 ---------------------------------------------------------------------------*/
class SvxVertCTLTextTbxCtrl : public SfxToolBoxControl
{
    sal_Bool bCheckVertical; //determines whether vertical mode or CTL mode has to be checked
public:
    SvxVertCTLTextTbxCtrl( USHORT nId, ToolBox& rTbx, SfxBindings& );
    ~SvxVertCTLTextTbxCtrl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    void    SetVert(sal_Bool bSet) {bCheckVertical = bSet;}

};
/* -----------------------------12.09.2002 11:50------------------------------

 ---------------------------------------------------------------------------*/
class SvxCTLTextTbxCtrl : public SvxVertCTLTextTbxCtrl
{
    SfxStatusForwarder  aStateForwarder;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxCTLTextTbxCtrl(USHORT nId, ToolBox& rTbx, SfxBindings& rBind) :
        SvxVertCTLTextTbxCtrl( nId, rTbx, rBind ),
        aStateForwarder( SID_CTLFONT_STATE, *this )
        {SetVert(FALSE);};

};
/* -----------------------------12.09.2002 11:50------------------------------

 ---------------------------------------------------------------------------*/
class SvxVertTextTbxCtrl : public SvxVertCTLTextTbxCtrl
{
    SfxStatusForwarder  aStateForwarder;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxVertTextTbxCtrl(USHORT nId, ToolBox& rTbx, SfxBindings& rBind) :
        SvxVertCTLTextTbxCtrl( nId, rTbx, rBind ),
        aStateForwarder( SID_VERTICALTEXT_STATE, *this )
        {SetVert(TRUE);};
};

#endif
