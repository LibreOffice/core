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

#ifndef _SVX_BKWND_HYPERLINK_HXX
#define _SVX_BKWND_HYPERLINK_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <svtools/svtreebx.hxx>

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
    sal_Bool RefreshFromDoc( ::rtl::OUString aURL );

    SvLBoxEntry* FindEntry ( String aStrName );
    void ClearTree();
    int FillTree( ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > xLinks, SvLBoxEntry* pParentEntry =NULL );

    virtual void Move ();

    DECL_LINK (ClickApplyHdl_Impl, void * );
    DECL_LINK (ClickCloseHdl_Impl, void * );

public:
    SvxHlinkDlgMarkWnd (SvxHyperlinkTabPageBase *pParent);
    ~SvxHlinkDlgMarkWnd();

    sal_Bool MoveTo ( Point aNewPos );
    void RefreshTree ( String aStrURL );
    void SelectEntry ( String aStrMark );

    sal_Bool ConnectToDialog( sal_Bool bDoit = sal_True );

    sal_uInt16 SetError( sal_uInt16 nError);
};


#endif  // _SVX_BKWND_HYPERLINK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
