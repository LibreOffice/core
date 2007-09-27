/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: glossary.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:01:45 $
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
#ifndef _GLOSSARY_HXX
#define _GLOSSARY_HXX

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef _MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTENTENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XELEMENTACCESS_HPP_
#include <com/sun/star/container/XElementAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

#include <actctrl.hxx>

class SwGlossaryHdl;
class SwNewGlosNameDlg;
class SwWrtShell;
class SfxViewFrame;
class PopupMenu;
class Menu;

const short RET_EDIT = 100;

//------------------------------------------------------------------

class SwGlTreeListBox : public SvTreeListBox
{
    const String    sReadonly;

    SvLBoxEntry*  pDragEntry;

    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rContainer,
                                            SvLBoxEntry* );
    virtual sal_Bool    NotifyAcceptDrop( SvLBoxEntry* );

    virtual sal_Bool    NotifyMoving(   SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    ULONG&        rNewChildPos
                                );
    virtual sal_Bool    NotifyCopying(  SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    ULONG&        rNewChildPos);
public:
    SwGlTreeListBox(Window* pParent, const ResId& rResId);

    virtual void    RequestHelp( const HelpEvent& rHEvt );
    void            Clear();
};

//------------------------------------------------------------------
class SwOneExampleFrame;
class SwGlossaryDlg : public SvxStandardDialog
{
    friend class SwNewGlosNameDlg;
    friend class SwGlTreeListBox;

    CheckBox        aInsertTipCB;
    FixedText       aNameLbl;
    Edit            aNameED;
    FixedText       aShortNameLbl;
    NoSpaceEdit     aShortNameEdit;
    SwGlTreeListBox aCategoryBox;
    FixedLine       aRelativeFL;
    CheckBox        aFileRelCB;
    CheckBox        aNetRelCB;
    Window          aExampleWIN;
    Window          aExampleDummyWIN;
    CheckBox        aShowExampleCB;
    OKButton        aInsertBtn;
    CancelButton    aCloseBtn;
    HelpButton      aHelpBtn;
    MenuButton      aEditBtn;
    PushButton      aBibBtn;
    PushButton      aPathBtn;

    String          sReadonlyPath;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        _xAutoText;
    SwOneExampleFrame*  pExampleFrame;

    PopupMenu*      pMenu;
    SwGlossaryHdl*  pGlossaryHdl;

    String          sResumeGroup;
    String          sResumeShortName;
    BOOL            bResume;


    const sal_Bool      bSelection : 1;
    sal_Bool            bReadOnly : 1;
    sal_Bool            bIsOld : 1;
    sal_Bool            bIsDocReadOnly:1;

    SwWrtShell*     pSh;

    void EnableShortName(sal_Bool bOn = sal_True);

    DECL_LINK( NameModify, Edit * );
    DECL_LINK( NameDoubleClick, SvTreeListBox * );
    DECL_LINK( GrpSelect, SvTreeListBox * );
    DECL_LINK( MenuHdl, Menu * );
    DECL_LINK( EnableHdl, Menu * );
    DECL_LINK( BibHdl, Button * );
    DECL_LINK( EditHdl, Button * );
    DECL_LINK( PathHdl, Button * );
    DECL_LINK( CheckBoxHdl, CheckBox * );
    DECL_LINK( ShowPreviewHdl, CheckBox * );
    DECL_LINK( PreviewLoadedHdl, void * );


    virtual void    Apply();
    void            Init();
    SvLBoxEntry*    DoesBlockExist(const String& sBlock, const String& rShort);
    void            ShowAutoText(const String& rGroup, const String& rShortName);
    void            ResumeShowAutoText();

    BOOL            GetResumeData(String& rGroup, String& rShortName)
                        {rGroup = sResumeGroup; rShortName = sResumeShortName; return bResume;}
    void            SetResumeData(const String& rGroup, const String& rShortName)
                        {sResumeGroup = rGroup; sResumeShortName = rShortName; bResume = TRUE;}
    void            ResetResumeData() {bResume = FALSE;}
public:
    SwGlossaryDlg(SfxViewFrame* pViewFrame, SwGlossaryHdl* pGlosHdl, SwWrtShell *pWrtShell);
    ~SwGlossaryDlg();
    String          GetCurrGrpName() const;
    inline String   GetCurrLongName() const;
    inline String   GetCurrShortName() const;
    static String   GetCurrGroup();
    static void     SetActGroup(const String& rNewGroup);
    static String   GetExtension();
};

inline String SwGlossaryDlg::GetCurrLongName() const
{
    return aNameED.GetText();
}
inline String SwGlossaryDlg::GetCurrShortName() const
{
    return aShortNameEdit.GetText();
}


#endif

