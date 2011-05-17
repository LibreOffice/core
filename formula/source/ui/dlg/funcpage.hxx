/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef FORMULA_FUNCPAGE_HXX
#define FORMULA_FUNCPAGE_HXX

#include <svtools/stdctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>

#include <vcl/tabctrl.hxx>
#include <vcl/button.hxx>
#include <svtools/svtreebx.hxx>

#include <boost/shared_ptr.hpp>
#include <vector>
#include "ModuleHelper.hxx"
//============================================================================
namespace formula
{

class IFunctionDescription;
class IFunctionManager;
class IFunctionCategory;

//============================================================================
class FormulaListBox : public ListBox
{
protected:

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

public:
                    FormulaListBox( Window* pParent, WinBits nWinStyle = WB_BORDER );

                    FormulaListBox( Window* pParent, const ResId& rResId );

};



typedef const IFunctionDescription* TFunctionDesc;
//============================================================================
class FuncPage : public TabPage
{
private:
    OModuleClient   m_aModuleClient;
    Link            aDoubleClickLink;
    Link            aSelectionLink;
    FixedText       aFtCategory;
    ListBox         aLbCategory;
    FixedText       aFtFunction;
    FormulaListBox  aLbFunction;
    const IFunctionManager*
                    m_pFunctionManager;

    ::std::vector< TFunctionDesc >  aLRUList;
    rtl::OString    m_aHelpId;


    void impl_addFunctions(const IFunctionCategory* _pCategory);
                    DECL_LINK( SelHdl, ListBox* );
                    DECL_LINK( DblClkHdl, ListBox* );

protected:

    void            UpdateFunctionList();
    void            InitLRUList();


public:

                    FuncPage( Window* pParent,const IFunctionManager* _pFunctionManager);

    void            SetCategory(sal_uInt16 nCat);
    void            SetFunction(sal_uInt16 nFunc);
    void            SetFocus();
    sal_uInt16          GetCategory();
    sal_uInt16          GetFunction();
    sal_uInt16          GetFunctionEntryCount();

    sal_uInt16          GetFuncPos(const IFunctionDescription* _pDesc);
    const IFunctionDescription* GetFuncDesc( sal_uInt16 nPos ) const;
    String          GetSelFunctionName() const;

    void            SetDoubleClickHdl( const Link& rLink ) { aDoubleClickLink = rLink; }
    const Link&     GetDoubleClickHdl() const { return aDoubleClickLink; }

    void            SetSelectHdl( const Link& rLink ) { aSelectionLink = rLink; }
    const Link&     GetSelectHdl() const { return aSelectionLink; }

};

} // formula
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
