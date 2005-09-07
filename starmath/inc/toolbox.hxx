/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: toolbox.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:00:46 $
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

#include "dialog.hrc"

#define NUM_TBX_CATEGORIES  9

class SmToolBoxWindow : public SfxFloatingWindow
{

protected:
    ToolBox     aToolBoxCat;
    FixedLine   aToolBoxCat_Delim;  // to visualy seperate the catalog part
    ToolBox    *pToolBoxCmd;
    ToolBox    *vToolBoxCategories[NUM_TBX_CATEGORIES];
    ImageList  *aImageLists [NUM_TBX_CATEGORIES + 1];   /* regular */
    ImageList  *aImageListsH[NUM_TBX_CATEGORIES + 1];   /* high contrast */
    USHORT      nActiveCategoryRID;

    virtual BOOL    Close();
    virtual void    GetFocus();

    void            ApplyImageLists( USHORT nCategoryRID );

    DECL_LINK( CategoryClickHdl, ToolBox* );
    DECL_LINK( CmdSelectHdl, ToolBox* );

    const ImageList * GetImageList( USHORT nResId, BOOL bHighContrast );

public:
    SmToolBoxWindow(SfxBindings    *pBindings,
                    SfxChildWindow *pChildWindow,
                    Window         *pParent);
    ~SmToolBoxWindow();

    // Window
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent &rEvt );

    void        AdjustPosSize( BOOL bSetPos );
    void        SetCategory(USHORT nCategory);
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

