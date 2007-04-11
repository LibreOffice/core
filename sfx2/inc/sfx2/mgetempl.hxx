/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mgetempl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:23:25 $
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
#ifndef _SFX_MGETEMPL_HXX
#define _SFX_MGETEMPL_HXX

#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SV_MEDIT_HXX
#include <svtools/svmedit.hxx>
#endif

#include <sfx2/tabdlg.hxx>

/* erwartet:
    SID_TEMPLATE_NAME   :   In: StringItem, Name der Vorlage
    SID_TEMPLATE_FAMILY :   In: Familie der Vorlage
*/

class SfxStyleFamilies;
class SfxStyleFamilyItem;
class SfxStyleSheetBase;

#ifdef _SFXDLG_HXX
#define FixedInfo FixedText
#endif

class SfxManageStyleSheetPage : public SfxTabPage
{
    FixedText aNameFt;
    Edit aNameEd;
    CheckBox    aAutoCB;

    FixedText aFollowFt;
    ListBox aFollowLb;

    FixedText aBaseFt;
    ListBox aBaseLb;

    FixedText aFilterFt;
    ListBox aFilterLb;

    FixedInfo aDescFt;
    MultiLineEdit aDescED;
    FixedLine aDescGb;

    SfxStyleSheetBase *pStyle;
    SfxStyleFamilies *pFamilies;
    const SfxStyleFamilyItem *pItem;
    String aBuf;
    BOOL bModified;

        // initiale Daten des Styles
    String aName;
    String aFollow;
    String aParent;
    USHORT nFlags;

private:
friend class SfxStyleDialog;

//#if 0 // _SOLAR__PRIVATE
    DECL_LINK( GetFocusHdl, Edit * );
    DECL_LINK( LoseFocusHdl, Edit * );

    void    UpdateName_Impl(ListBox *, const String &rNew);
    void    SetDescriptionText_Impl();
//#endif

    SfxManageStyleSheetPage(Window *pParent, const SfxItemSet &rAttrSet );
    ~SfxManageStyleSheetPage();

    static SfxTabPage*  Create(Window *pParent, const SfxItemSet &rAttrSet );

protected:
    virtual BOOL        FillItemSet(SfxItemSet &);
    virtual void        Reset(const SfxItemSet &);

    using TabPage::ActivatePage;
        virtual void        ActivatePage(const SfxItemSet &);
        using TabPage::DeactivatePage;
    virtual int     DeactivatePage(SfxItemSet * = 0);
};

#ifdef FixedInfo
#undef FixedInfo
#endif

#endif

