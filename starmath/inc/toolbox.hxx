/*************************************************************************
 *
 *  $RCSfile: toolbox.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:25 $
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
#ifndef TOOLBOX_HXX
#define TOOLBOX_HXX

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif

#ifndef _SMMOD_HXX
#include "smmod.hxx"
#endif
#ifndef CONFIG_HXX
#include "config.hxx"
#endif


#define NUM_TBX_CATEGORIES  9

class SmToolBoxWindow : public SfxFloatingWindow
{

protected:
    ToolBox     aToolBoxCat;
    ToolBox    *pToolBoxCmd;
    ToolBox    *vToolBoxCategories[NUM_TBX_CATEGORIES];
    USHORT      nActiveCategory;

    virtual BOOL Close();

    DECL_LINK( CategoryClickHdl, ToolBox* );
    DECL_LINK( CmdSelectHdl, ToolBox* );

public:
    SmToolBoxWindow(SfxBindings    *pBindings,
                    SfxChildWindow *pChildWindow,
                    Window         *pParent);
    ~SmToolBoxWindow();

    // Window
    virtual void    StateChanged( StateChangedType nStateChange );

    void        AdjustPosition(const Point &rPoint);
    void        SetCategory(USHORT nCategory);
    void        ShowWindows();
};

/**************************************************************************/

class SmToolBoxWrapper : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW(SmToolBoxWrapper);

protected:
    SmToolBoxWrapper(Window *pParentWindow,
                     USHORT, SfxBindings*, SfxChildWinInfo*);
};

#endif

