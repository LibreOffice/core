/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlfiltertabpagebasic.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:21:41 $
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
#ifndef _XMLFILTERTABPAGEBASIC_HXX_
#define _XMLFILTERTABPAGEBASIC_HXX_

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

#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif

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

    FixedText       maFTFilterName;
    Edit            maEDFilterName;

    FixedText       maFTApplication;
    ComboBox        maCBApplication;

    FixedText       maFTInterfaceName;
    Edit            maEDInterfaceName;

    FixedText       maFTExtension;
    Edit            maEDExtension;
    FixedText       maFTDescription;
    MultiLineEdit   maEDDescription;
};

#endif
