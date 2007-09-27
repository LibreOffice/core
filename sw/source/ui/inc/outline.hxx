/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outline.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:06:55 $
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
#ifndef _OUTLINE_HXX
#define _OUTLINE_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif


#ifndef _SV_MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#include "swtypes.hxx"      //fuer MAXLEVEL
#ifndef _NUMPREVW_HXX
#include <numprevw.hxx>
#endif
#ifndef _NUMBERINGTYPELISTBOX_HXX
#include <numberingtypelistbox.hxx>
#endif

class SwWrtShell;
class SwNumRule;
class SwChapterNumRules;

/* -----------------07.07.98 13:38-------------------
 *
 * --------------------------------------------------*/
class SwOutlineTabDialog : public SfxTabDialog
{
    static     USHORT    nNumLevel;

    String              aNullStr;
    String              aCollNames[MAXLEVEL];
    PopupMenu           aFormMenu;

    SwWrtShell&         rWrtSh;
    SwNumRule*          pNumRule;
    SwChapterNumRules*  pChapterNumRules;

    BOOL                bModified : 1;

    protected:
    DECL_LINK( CancelHdl, Button * );
    DECL_LINK( FormHdl, Button * );
    DECL_LINK( MenuSelectHdl, Menu * );

        virtual void    PageCreated(USHORT nPageId, SfxTabPage& rPage);
        virtual short   Ok();

    public:
        SwOutlineTabDialog(Window* pParent,
                    const SfxItemSet* pSwItemSet,
                    SwWrtShell &);
        ~SwOutlineTabDialog();

    SwNumRule*          GetNumRule() {return pNumRule;}
    USHORT              GetLevel(const String &rFmtName) const;
    String*             GetCollNames() {return aCollNames;}

    static USHORT       GetActNumLevel() {return nNumLevel;}
    static void         SetActNumLevel(USHORT nSet) {nNumLevel = nSet;}
};
/* -----------------07.07.98 13:47-------------------
 *
 * --------------------------------------------------*/
class SwOutlineSettingsTabPage : public SfxTabPage
{
    ListBox         aLevelLB;
    FixedLine       aLevelFL;

    FixedText       aCollLbl;
    ListBox         aCollBox;
    FixedText       aNumberLbl;
    SwNumberingTypeListBox  aNumberBox;
    FixedText       aCharFmtFT;
    ListBox         aCharFmtLB;
    FixedText       aAllLevelFT;
    NumericField    aAllLevelNF;
    FixedText       aDelim;
    FixedText       aPrefixFT;
    Edit            aPrefixED;
    FixedText       aSuffixFT;
    Edit            aSuffixED;
    FixedText       aStartLbl;
    NumericField    aStartEdit;
    FixedLine        aNumberFL;
    NumberingPreview aPreviewWIN;

    String              aNoFmtName;
    String              aSaveCollNames[MAXLEVEL];
    SwWrtShell*         pSh;
    SwNumRule*          pNumRule;
    String*             pCollNames;
    USHORT              nActLevel;

    DECL_LINK( LevelHdl, ListBox * );
    DECL_LINK( ToggleComplete, NumericField * );
    DECL_LINK( CollSelect, ListBox * );
    DECL_LINK( CollSelectGetFocus, ListBox * );
    DECL_LINK( NumberSelect, SwNumberingTypeListBox * );
    DECL_LINK( DelimModify, Edit * );
    DECL_LINK( StartModified, NumericField * );
    DECL_LINK( CharFmtHdl, ListBox * );

    void    Update();

    void    SetModified(){aPreviewWIN.Invalidate();}
    void    CheckForStartValue_Impl(sal_uInt16 nNumberingType);

public:
    SwOutlineSettingsTabPage(Window* pParent, const SfxItemSet& rSet);
    ~SwOutlineSettingsTabPage();

    void SetWrtShell(SwWrtShell* pShell);

    using TabPage::ActivatePage;
    virtual void        ActivatePage(const SfxItemSet& rSet);
    using TabPage::DeactivatePage;
    virtual int         DeactivatePage(SfxItemSet *pSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
};
#endif
