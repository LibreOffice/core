/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: opredlin.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:41:35 $
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
#ifndef _SC_OPREDLIN_HXX
#define _SC_OPREDLIN_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

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

#ifndef _CTRLBOX_HXX //autogen
#include <svtools/ctrlbox.hxx>
#endif

#ifndef _SVX_FNTCTRL_HXX //autogen
#include <svx/fntctrl.hxx>
#endif

#ifndef _SVX_STRARRAY_HXX //autogen
#include <svx/strarray.hxx>
#endif

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

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};
#endif


