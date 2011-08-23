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
#ifndef _XMLFILTERTABDIALOG_HXX_
#define _XMLFILTERTABDIALOG_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <sfx2/tabdlg.hxx>

class Window;
class ResMgr;

class filter_info_impl;
class XMLFilterTabPageBasic;
class XMLFilterTabPageXSLT;

class XMLFilterTabDialog: public TabDialog
{
public:
    XMLFilterTabDialog(	Window *pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF, const filter_info_impl* pInfo );
    virtual ~XMLFilterTabDialog();

    ResMgr& getResMgr() { return mrResMgr; }

    bool onOk();

    filter_info_impl* getNewFilterInfo() const;

private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;

    DECL_LINK( ActivatePageHdl, TabControl * );
    DECL_LINK( DeactivatePageHdl, TabControl * );
    DECL_LINK( OkHdl, Button * );
    DECL_LINK( CancelHdl, Button * );

    ResMgr&	mrResMgr;

    const filter_info_impl* mpOldInfo;
    filter_info_impl* mpNewInfo;

    TabControl		maTabCtrl;
    OKButton		maOKBtn;
    CancelButton	maCancelBtn;
    HelpButton		maHelpBtn;

    XMLFilterTabPageBasic*	mpBasicPage;
    XMLFilterTabPageXSLT* mpXSLTPage;
};


#endif
