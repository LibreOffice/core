/*************************************************************************
 *
 *  $RCSfile: xmlfiltertabpagexslt.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 14:19:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _XMLFILTERTABPAGEXSLT_HXX_
#define _XMLFILTERTABPAGEXSLT_HXX_

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif

#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif

#ifndef _SVTOOLS_INETTBC_HXX
#include <svtools/inettbc.hxx>
#endif

class Window;
class ResMgr;
class filter_info_impl;

class XMLFilterTabPageXSLT : public TabPage
{
public:
    XMLFilterTabPageXSLT( Window* pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    virtual ~XMLFilterTabPageXSLT();

    bool FillInfo( filter_info_impl* pInfo );
    void SetInfo(const filter_info_impl* pInfo);

    DECL_LINK( ClickBrowseHdl_Impl, PushButton * );

    FixedText       maFTDocType;
    Edit            maEDDocType;

    FixedText       maFTDTDSchema;
    SvtURLBox       maEDDTDSchema;
    PushButton      maPBDTDSchemaBrowse;

    FixedText       maFTExportXSLT;
    SvtURLBox       maEDExportXSLT;
    PushButton      maPBExprotXSLT;

    FixedText       maFTImportXSLT;
    SvtURLBox       maEDImportXSLT;
    PushButton      maPBImportXSLT;

    FixedText       maFTImportTemplate;
    SvtURLBox       maEDImportTemplate;
    PushButton      maPBImportTemplate;

private:
    void SetURL( SvtURLBox& rURLBox, const rtl::OUString& rURL );
    rtl::OUString GetURL( SvtURLBox& rURLBox );

    ::rtl::OUString sHTTPSchema;
    ::rtl::OUString sSHTTPSchema;
    ::rtl::OUString sFILESchema;
    ::rtl::OUString sFTPSchema;
    ::rtl::OUString sInstPath;
};

#endif
