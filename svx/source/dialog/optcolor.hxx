/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optcolor.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:20:54 $
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
#ifndef _SVX_OPTCOLOR_HXX
#define _SVX_OPTCOLOR_HXX

// include ---------------------------------------------------------------

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

/* -----------------------------25.03.2002 10:40------------------------------

 ---------------------------------------------------------------------------*/
namespace svtools {class EditableColorConfig;}
class ColorConfigCtrl_Impl;
class AbstractSvxNameDialog; //CHINA001 class SvxNameDialog;
class SvxColorOptionsTabPage : public SfxTabPage
{
    using SfxTabPage::DeactivatePage;

    FixedLine               aColorSchemeFL;
    FixedText               aColorSchemeFT;
    ListBox                 aColorSchemeLB;
    PushButton              aSaveSchemePB;
    PushButton              aDeleteSchemePB;

    FixedLine               aCustomColorsFL;

    BOOL                    bFillItemSetCalled;

    svtools::EditableColorConfig* pColorConfig;
    ColorConfigCtrl_Impl*   pColorConfigCT;

    DECL_LINK(SchemeChangedHdl_Impl, ListBox*);
    DECL_LINK(SaveDeleteHdl_Impl, PushButton*);
    DECL_LINK(CheckNameHdl_Impl, AbstractSvxNameDialog*);
    void UpdateColorConfig();

public:
    SvxColorOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxColorOptionsTabPage(  );

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual int         DeactivatePage( SfxItemSet* pSet );
    virtual void        FillUserData();

};

#endif

