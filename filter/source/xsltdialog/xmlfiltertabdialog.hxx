/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlfiltertabdialog.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:20:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _XMLFILTERTABDIALOG_HXX_
#define _XMLFILTERTABDIALOG_HXX_

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif

class Window;
class ResMgr;

class filter_info_impl;
class XMLFilterTabPageBasic;
class XMLFilterTabPageXSLT;

class XMLFilterTabDialog: public TabDialog
{
public:
    XMLFilterTabDialog( Window *pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF, const filter_info_impl* pInfo );
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

    ResMgr& mrResMgr;

    const filter_info_impl* mpOldInfo;
    filter_info_impl* mpNewInfo;

    TabControl      maTabCtrl;
    OKButton        maOKBtn;
    CancelButton    maCancelBtn;
    HelpButton      maHelpBtn;

    XMLFilterTabPageBasic*  mpBasicPage;
    XMLFilterTabPageXSLT* mpXSLTPage;
};


#endif
