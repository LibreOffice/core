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

#ifndef SC_AREASDLG_HXX
#define SC_AREASDLG_HXX

#include "address.hxx"

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <anyrefdg.hxx>

class ScDocument;
class ScViewData;
class ScRangeUtil;
class ScRangeItem;


//============================================================================

class ScPrintAreasDlg : public ScAnyRefDlg
{
public:
                    ScPrintAreasDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent );
                    ~ScPrintAreasDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual void    AddRefEntry();

    virtual sal_Bool    IsTableLocked() const;

    virtual void    SetActive();
    virtual void    Deactivate();
    virtual sal_Bool    Close();

private:
    ListBox         aLbPrintArea;
    FixedLine       aFlPrintArea;
    formula::RefEdit        aEdPrintArea;
    formula::RefButton      aRbPrintArea;

    ListBox         aLbRepeatRow;
    FixedLine       aFlRepeatRow;
    formula::RefEdit        aEdRepeatRow;
    formula::RefButton      aRbRepeatRow;

    ListBox         aLbRepeatCol;
    FixedLine       aFlRepeatCol;
    formula::RefEdit        aEdRepeatCol;
    formula::RefButton      aRbRepeatCol;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    sal_Bool            bDlgLostFocus;
    formula::RefEdit*       pRefInputEdit;
    ScDocument*     pDoc;
    ScViewData*     pViewData;
    SCTAB           nCurTab;

#ifdef _AREASDLG_CXX
private:
    void Impl_Reset();
    sal_Bool Impl_CheckRefStrings();
    void Impl_FillLists();
    sal_Bool Impl_GetItem( Edit* pEd, SfxStringItem& rItem );

    // Handler:
    DECL_LINK( Impl_SelectHdl,      ListBox*    );
    DECL_LINK( Impl_ModifyHdl,      formula::RefEdit*  );
    DECL_LINK( Impl_BtnHdl,         PushButton* );
    DECL_LINK( Impl_GetFocusHdl,    Control*    );
#endif
};



#endif

