/*************************************************************************
 *
 *  $RCSfile: outline.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2001-03-02 14:08:37 $
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

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
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
class SwTxtFmtColl;
class SwNumRule;
class SwChapterNumRules;

/* -----------------07.07.98 13:38-------------------
 *
 * --------------------------------------------------*/
class SwOutlineTabDialog : public SfxTabDialog
{
    String              aNullStr;
    String              aCollNames[MAXLEVEL];
    PopupMenu           aFormMenu;

    SwWrtShell&         rWrtSh;
    SwNumRule*          pNumRule;
    SwChapterNumRules*  pChapterNumRules;

    USHORT              nNumLevel;
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
    USHORT              GetActNumLevel() {return nNumLevel;}
    void                SetActNumLevel(USHORT nSet) {nNumLevel = nSet;}
};
/* -----------------07.07.98 13:47-------------------
 *
 * --------------------------------------------------*/
class SwOutlineSettingsTabPage : public SfxTabPage
{
    ListBox         aLevelLB;
    GroupBox        aLevelGB;

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
    GroupBox        aNumberGrp;
    GroupBox        aPreviewGB;
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


public:
    SwOutlineSettingsTabPage(Window* pParent, const SfxItemSet& rSet);
    ~SwOutlineSettingsTabPage();

    void SetWrtShell(SwWrtShell* pShell);

    virtual void        ActivatePage(const SfxItemSet& rSet);
    virtual int         DeactivatePage(SfxItemSet *pSet);
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
};
#endif
