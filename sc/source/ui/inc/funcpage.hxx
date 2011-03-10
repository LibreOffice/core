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

#ifndef SC_FUNCPAGE_HXX
#define SC_FUNCPAGE_HXX

#include "funcutl.hxx"
#include "global.hxx"       // ScAddress
#include <svtools/stdctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>

#ifndef _SVSTDARR_STRINGS

#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>

#endif
#include <vcl/tabctrl.hxx>
#include "parawin.hxx"
#include <svtools/svtreebx.hxx>
#include "compiler.hxx"
#include "cell.hxx"


class ScViewData;
class ScFuncName_Impl;
class ScDocument;
class ScFuncDesc;

//============================================================================

#define LRU_MAX 10

//============================================================================
class ScListBox : public ListBox
{
protected:

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

public:
                    ScListBox( Window* pParent, const ResId& rResId );
};




//============================================================================
class ScFuncPage : public TabPage
{
private:

    Link            aDoubleClickLink;
    Link            aSelectionLink;
    FixedText       aFtCategory;
    ListBox         aLbCategory;
    FixedText       aFtFunction;
    ScListBox       aLbFunction;
    ImageButton     aIBFunction;

    const ScFuncDesc*   aLRUList[LRU_MAX];


                    DECL_LINK( SelHdl, ListBox* );
                    DECL_LINK( DblClkHdl, ListBox* );

protected:

    void            UpdateFunctionList();
    void            InitLRUList();


public:

                    ScFuncPage( Window* pParent);

    void            SetCategory(sal_uInt16 nCat);
    void            SetFunction(sal_uInt16 nFunc);
    void            SetFocus();
    sal_uInt16          GetCategory();
    sal_uInt16          GetFunction();
    sal_uInt16          GetFunctionEntryCount();

    sal_uInt16          GetFuncPos(const ScFuncDesc*);
    const ScFuncDesc*   GetFuncDesc( sal_uInt16 nPos ) const;
    String          GetSelFunctionName() const;

    void            SetDoubleClickHdl( const Link& rLink ) { aDoubleClickLink = rLink; }
    const Link&     GetDoubleClickHdl() const { return aDoubleClickLink; }

    void            SetSelectHdl( const Link& rLink ) { aSelectionLink = rLink; }
    const Link&     GetSelectHdl() const { return aSelectionLink; }

};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
