/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FORMULA_SOURCE_UI_DLG_FUNCPAGE_HXX
#define INCLUDED_FORMULA_SOURCE_UI_DLG_FUNCPAGE_HXX

#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>

#include <vcl/tabctrl.hxx>
#include <vcl/button.hxx>
#include <svtools/treelistbox.hxx>

#include <vector>

namespace formula
{

class IFunctionDescription;
class IFunctionManager;
class IFunctionCategory;


class FormulaListBox : public ListBox
{
protected:

    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;

public:
                    FormulaListBox( vcl::Window* pParent, WinBits nBits );

};


typedef const IFunctionDescription* TFunctionDesc;

class FuncPage : public TabPage
{
private:
    Link<FuncPage&,void>     aDoubleClickLink;
    Link<FuncPage&,void>     aSelectionLink;
    VclPtr<ListBox>          m_pLbCategory;
    VclPtr<FormulaListBox>   m_pLbFunction;
    VclPtr<Edit>             m_plbFunctionSearchString;
    const IFunctionManager*  m_pFunctionManager;

    ::std::vector< TFunctionDesc >  aLRUList;
    OString    m_aHelpId;


    void impl_addFunctions(const IFunctionCategory* _pCategory);
                    DECL_LINK( SelHdl, ListBox&, void );
                    DECL_LINK(DblClkHdl, ListBox&, void);
                    DECL_LINK(ModifyHdl, Edit&, void);

protected:

    void            UpdateFunctionList(const OUString&);


public:

                    FuncPage( vcl::Window* pParent,const IFunctionManager* _pFunctionManager);
    virtual         ~FuncPage() override;
    virtual void    dispose() override;

    void            SetCategory(sal_Int32  nCat);
    void            SetFunction(sal_Int32  nFunc);
    void            SetFocus();
    sal_Int32       GetCategory();
    sal_Int32       GetFunction();
    sal_Int32       GetFunctionEntryCount();

    sal_Int32       GetFuncPos(const IFunctionDescription* _pDesc);
    const IFunctionDescription* GetFuncDesc( sal_Int32  nPos ) const;
    OUString        GetSelFunctionName() const;

    void            SetDoubleClickHdl( const Link<FuncPage&,void>& rLink ) { aDoubleClickLink = rLink; }

    void            SetSelectHdl( const Link<FuncPage&,void>& rLink ) { aSelectionLink = rLink; }

};

} // formula
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
