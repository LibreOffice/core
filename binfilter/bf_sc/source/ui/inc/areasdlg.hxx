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

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef SC_ANYREFDG_HXX
#include <anyrefdg.hxx>
#endif
namespace binfilter {

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

    virtual void	SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual void	AddRefEntry();

    virtual BOOL	IsTableLocked() const;

    virtual void	SetActive();
    virtual void	Deactivate();
    virtual BOOL	Close();

private:
    ListBox			aLbPrintArea;
    FixedLine       aFlPrintArea;
    ScRefEdit		aEdPrintArea;
    ScRefButton		aRbPrintArea;

    ListBox			aLbRepeatRow;
    FixedLine       aFlRepeatRow;
    ScRefEdit		aEdRepeatRow;
    ScRefButton		aRbRepeatRow;

    ListBox			aLbRepeatCol;
    FixedLine       aFlRepeatCol;
    ScRefEdit		aEdRepeatCol;
    ScRefButton		aRbRepeatCol;

    OKButton		aBtnOk;
    CancelButton	aBtnCancel;
    HelpButton		aBtnHelp;

    BOOL			bDlgLostFocus;
    ScRefEdit*		pRefInputEdit;
    ScDocument*		pDoc;
    ScViewData*		pViewData;
    USHORT			nCurTab;

#ifdef _AREASDLG_CXX
private:
    void Impl_Reset();
    BOOL Impl_CheckRefStrings();
    void Impl_FillLists();
    BOOL Impl_GetItem( Edit* pEd, SfxStringItem& rItem );

    // Handler:
    DECL_LINK( Impl_SelectHdl,		ListBox*    );
    DECL_LINK( Impl_ModifyHdl,		ScRefEdit*  );
    DECL_LINK( Impl_BtnHdl,    		PushButton* );
    DECL_LINK( Impl_GetFocusHdl,	Control*	);
#endif
};



} //namespace binfilter
#endif

