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

#ifndef INCLUDED_SC_SOURCE_UI_INC_FUNCPAGE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_FUNCPAGE_HXX

#include "funcutl.hxx"
#include "global.hxx"
#include <svtools/stdctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>

#include <vcl/tabctrl.hxx>
#include "parawin.hxx"
#include <svtools/treelistbox.hxx>
#include "compiler.hxx"

class ScFuncDesc;

#define LRU_MAX 10

class ScListBox : public ListBox
{
protected:

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual bool    PreNotify( NotifyEvent& rNEvt );

public:
                    ScListBox( vcl::Window* pParent, const ResId& rResId );
};

class ScFuncPage : public TabPage
{
private:

    Link<>          aSelectionLink;
    FixedText       aFtCategory;
    ListBox         aLbCategory;
    FixedText       aFtFunction;
    ScListBox       aLbFunction;
    ImageButton     aIBFunction;

    const ScFuncDesc*   aLRUList[LRU_MAX];

                    DECL_LINK_TYPED( SelHdl, ListBox&, void );
                    DECL_LINK_TYPED( DblClkHdl, ListBox&, void );

protected:

    void            UpdateFunctionList();
    void            InitLRUList();

public:

                      ScFuncPage( vcl::Window* pParent);

    void              SetCategory(sal_uInt16 nCat);
    void              SetFunction(sal_uInt16 nFunc);
    void              SetFocus();
    sal_uInt16        GetCategory();
    sal_uInt16        GetFunction();
    sal_uInt16        GetFunctionEntryCount();

    sal_uInt16        GetFuncPos(const ScFuncDesc*);
    const ScFuncDesc* GetFuncDesc( sal_uInt16 nPos ) const;
    OUString          GetSelFunctionName() const;

    void              SetSelectHdl( const Link<>& rLink ) { aSelectionLink = rLink; }
    const Link<>&     GetSelectHdl() const { return aSelectionLink; }

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
