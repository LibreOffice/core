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
#ifndef _XMLFILTERTABPAGEBASIC_HXX_
#define _XMLFILTERTABPAGEBASIC_HXX_

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <svtools/svmedit.hxx>

class Window;
class ResMgr;
class filter_info_impl;

class XMLFilterTabPageBasic : public TabPage
{
public:
    XMLFilterTabPageBasic( Window* pParent, ResMgr& rResMgr );
    virtual ~XMLFilterTabPageBasic();

    bool FillInfo( filter_info_impl* pInfo );
    void SetInfo(const filter_info_impl* pInfo);

    static rtl::OUString decodeComment( const rtl::OUString& rComment );
    static rtl::OUString encodeComment( const rtl::OUString& rComment );

    FixedText		maFTFilterName;
    Edit			maEDFilterName;

    FixedText		maFTApplication;
    ComboBox		maCBApplication;

    FixedText		maFTInterfaceName;
    Edit			maEDInterfaceName;

    FixedText		maFTExtension;
    Edit			maEDExtension;
    FixedText		maFTDescription;
    MultiLineEdit	maEDDescription;
};

#endif
