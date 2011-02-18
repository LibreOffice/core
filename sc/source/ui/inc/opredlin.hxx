/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SC_OPREDLIN_HXX
#define _SC_OPREDLIN_HXX

#include <sfx2/tabdlg.hxx>

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <svtools/ctrlbox.hxx>
#include <svx/fntctrl.hxx>
#include <svx/strarray.hxx>

/*-----------------------------------------------------------------------
    Beschreibung: Redlining-Optionen
 -----------------------------------------------------------------------*/

class ScRedlineOptionsTabPage : public SfxTabPage
{
    FixedText           aContentFT;
    ColorListBox        aContentColorLB;
    FixedText           aRemoveFT;
    ColorListBox        aRemoveColorLB;
    FixedText           aInsertFT;
    ColorListBox        aInsertColorLB;
    FixedText           aMoveFT;
    ColorListBox        aMoveColorLB;
    FixedLine           aChangedGB;
    String              aAuthorStr;
    DECL_LINK( ColorHdl, ColorListBox *pColorLB );


public:

    ScRedlineOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
    ~ScRedlineOptionsTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};
#endif


