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

#ifndef _SVX_BKWND_HYPERLINK_HXX
#define _SVX_BKWND_HYPERLINK_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <svtools/treelistbox.hxx>

#include "hlmarkwn_def.hxx"
class SvxHyperlinkTabPageBase;

//########################################################################
//#                                                                      #
//# Tree-Window                                                          #
//#                                                                      #
//########################################################################

class SvxHlinkDlgMarkWnd;

class SvxHlmarkTreeLBox : public SvTreeListBox
{
private:
    SvxHlinkDlgMarkWnd* mpParentWnd;

public:
    SvxHlmarkTreeLBox( Window* pParent, const ResId& rResId );

    virtual void Paint( const Rectangle& rRect );
};

//########################################################################
//#                                                                      #
//# Window-Class                                                         #
//#                                                                      #
//########################################################################

class SvxHlinkDlgMarkWnd : public ModalDialog //FloatingWindow
{
private:
    friend class SvxHlmarkTreeLBox;

    PushButton      maBtApply;
    PushButton      maBtClose;
    SvxHlmarkTreeLBox maLbTree;

    sal_Bool            mbUserMoved;

    SvxHyperlinkTabPageBase* mpParent;

    String          maStrLastURL;

    sal_uInt16          mnError;

protected:
    sal_Bool RefreshFromDoc( OUString aURL );
    void RestoreLastSelection();

    SvTreeListEntry* FindEntry ( String aStrName );
    void ClearTree();
    int FillTree( ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > xLinks, SvTreeListEntry* pParentEntry =NULL );

    virtual void Move ();

    DECL_LINK (ClickApplyHdl_Impl, void * );
    DECL_LINK (ClickCloseHdl_Impl, void * );

public:
    SvxHlinkDlgMarkWnd (SvxHyperlinkTabPageBase *pParent);
    ~SvxHlinkDlgMarkWnd();

    sal_Bool MoveTo ( Point aNewPos );
    void RefreshTree ( String aStrURL );
    bool SelectEntry(String aStrMark);

    sal_Bool ConnectToDialog( sal_Bool bDoit = sal_True );

    sal_uInt16 SetError( sal_uInt16 nError);
};


#endif  // _SVX_BKWND_HYPERLINK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
