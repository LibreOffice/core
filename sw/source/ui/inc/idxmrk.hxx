/*************************************************************************
 *
 *  $RCSfile: idxmrk.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:40 $
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
#ifndef _IDXMRK_HXX
#define _IDXMRK_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _SVX_STDDLG_HXX
#include <svx/stddlg.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif
#ifndef _TOXE_HXX
#include "toxe.hxx"
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
class SwWrtShell;
class SwTOXMgr;

/*--------------------------------------------------------------------
     Beschreibung:  Markierung fuer Verzeichniseintrag einfuegen
 --------------------------------------------------------------------*/
class SwIndexMarkFloatDlg;
class SwIndexMarkModalDlg;
class SwIndexMarkDlg : public Window //SvxStandardDialog
{
    friend class SwIndexMarkFloatDlg;
    friend class SwIndexMarkModalDlg;
    FixedText       aTypeFT;
    ListBox         aTypeDCB;
    ImageButton     aNewBT;
    FixedText       aEntryFT;
    Edit            aEntryED;
    FixedText       aKeyFT;
    ComboBox        aKeyDCB;
    FixedText       aKey2FT;
    ComboBox        aKey2DCB;
    FixedText       aLevelFT;
    NumericField    aLevelED;
     CheckBox       aMainEntryCB;
     CheckBox       aApplyToAllCB;
     CheckBox       aSearchCaseSensitiveCB;
     CheckBox       aSearchCaseWordOnlyCB;

     GroupBox       aIndexGB;

    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;
    PushButton      aDelBT;
    //PushButton        aNewBT;

    ImageButton     aPrevSameBT;
    ImageButton     aNextSameBT;
    ImageButton     aPrevBT;
    ImageButton     aNextBT;

    String          aOrgStr;
    sal_Bool            bDel;
    sal_Bool            bNewMark;
    sal_Bool            bSelected;

    SwTOXMgr*       pTOXMgr;
    SwWrtShell*     pSh;

    void            Apply();
    void            InitControls();
    void            InsertMark();
    void            UpdateMark();

    DECL_LINK( InsertHdl, Button * );
    DECL_LINK( CloseHdl, Button * );
    DECL_LINK( DelHdl, Button * );
    DECL_LINK( NextHdl, Button * );
    DECL_LINK( NextSameHdl, Button * );
    DECL_LINK( PrevHdl, Button * );
    DECL_LINK( PrevSameHdl, Button * );
    DECL_LINK( ModifyHdl, ListBox* pBox = 0 );
    DECL_LINK( KeyDCBModifyHdl, ComboBox * );
    DECL_LINK( NewUserIdxHdl, Button*);
    DECL_LINK( SearchTypeHdl, CheckBox*);

    void            UpdateKeyBoxes();

    void            UpdateDialog();
    void            InsertUpdate();

    virtual void    Activate();

public:

    SwIndexMarkDlg( Window *pParent,
                       sal_Bool bNewDlg,
                    const ResId& rResId);


    ~SwIndexMarkDlg();

    void    ReInitDlg(SwWrtShell& rWrtShell);
    sal_Bool    IsTOXType(const String& rName)
                {return LISTBOX_ENTRY_NOTFOUND != aTypeDCB.GetEntryPos(rName);}
};
/* -----------------06.10.99 10:11-------------------

 --------------------------------------------------*/
class SwIndexMarkFloatDlg : public SfxModelessDialog
{
    SwIndexMarkDlg      aDlg;
    virtual void    Activate();
    public:
        SwIndexMarkFloatDlg(    SfxBindings* pBindings,
                                   SfxChildWindow* pChild,
                                   Window *pParent,
                                   sal_Bool bNew=sal_True);

        SwIndexMarkDlg&     GetDlg() {return aDlg;}
};
/* -----------------06.10.99 10:33-------------------

 --------------------------------------------------*/
class SwIndexMarkModalDlg : public SvxStandardDialog
{
    SwIndexMarkDlg      aDlg;
public:
    SwIndexMarkModalDlg(Window *pParent, SwWrtShell& rSh);

    SwIndexMarkDlg&     GetDlg() {return aDlg;}
    virtual void        Apply();
};

/* -----------------07.09.99 08:02-------------------

 --------------------------------------------------*/
class SwInsertIdxMarkWrapper : public SfxChildWindow
{
protected:
    SwInsertIdxMarkWrapper( Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SwInsertIdxMarkWrapper);

public:
    void    ReInitDlg(SwWrtShell& rWrtShell)
        {((SwIndexMarkFloatDlg*)pWindow)->GetDlg().ReInitDlg(rWrtShell);}

};

/* -----------------15.09.99 08:39-------------------

 --------------------------------------------------*/
class SwAuthMarkModalDlg;
class SwAuthMarkDlg : public Window
{
    static sal_Bool     bIsFromComponent;

    friend class SwAuthMarkModalDlg;
    RadioButton     aFromComponentRB;
    RadioButton     aFromDocContentRB;
    FixedText       aEntryFT;
    Edit            aEntryED;
    ListBox         aEntryLB;
    FixedText       aAuthorFT;
    FixedInfo       aAuthorFI;
    FixedText       aTitleFT;
    FixedInfo       aTitleFI;

    GroupBox        aEntryGB;

    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;
    PushButton      aCreateEntryPB;
    PushButton      aEditEntryPB;

    String          sChangeST;
    sal_Bool            bNewEntry;
    sal_Bool            bBibAccessInitialized;

    SwWrtShell*     pSh;

    String          m_sColumnTitles[AUTH_FIELD_END];
    String          m_sFields[AUTH_FIELD_END];

    String          m_sCreatedEntry[AUTH_FIELD_END];

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    xBibAccess;

    DECL_LINK(InsertHdl, PushButton*);
    DECL_LINK(CloseHdl, PushButton*);
    DECL_LINK(CreateEntryHdl, PushButton*);
    DECL_LINK(CompEntryHdl, ListBox*);
    DECL_LINK(ChangeSourceHdl, RadioButton*);
    DECL_LINK(IsEntryAllowedHdl, Edit*);

    void InitControls();
public:

    SwAuthMarkDlg( Window *pParent,
                    const ResId& rResId,
                       sal_Bool bNew=sal_True);
    ~SwAuthMarkDlg();

    void    ReInitDlg(SwWrtShell& rWrtShell);
};
/* -----------------07.09.99 08:02-------------------

 --------------------------------------------------*/
class SwInsertAuthMarkWrapper : public SfxChildWindow
{
protected:
    SwInsertAuthMarkWrapper(    Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SwInsertAuthMarkWrapper);

public:
    void    ReInitDlg(SwWrtShell& rWrtShell);
};
/* -----------------06.10.99 10:11-------------------

 --------------------------------------------------*/
class SwAuthMarkFloatDlg : public SfxModelessDialog
{
    SwAuthMarkDlg       aDlg;
    virtual void    Activate();
    public:
        SwAuthMarkFloatDlg(     SfxBindings* pBindings,
                                   SfxChildWindow* pChild,
                                   Window *pParent,
                                   sal_Bool bNew=sal_True);

        SwAuthMarkDlg&      GetDlg() {return aDlg;}
};
/* -----------------06.10.99 10:33-------------------

 --------------------------------------------------*/
class SwAuthMarkModalDlg : public SvxStandardDialog
{
    SwAuthMarkDlg       aDlg;
public:
    SwAuthMarkModalDlg(Window *pParent, SwWrtShell& rSh);

    SwAuthMarkDlg&      GetDlg() {return aDlg;}
    virtual void        Apply();
};


#endif // _IDXMRK_HXX

