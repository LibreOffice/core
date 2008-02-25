/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_shared.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-25 16:47:59 $
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

#if !defined INCLUDED_DP_GUI_SHARED_HXX
#define INCLUDED_DP_GUI_SHARED_HXX

#include "unotools/configmgr.hxx"
#include "rtl/instance.hxx"
#include "tools/resmgr.hxx"


namespace css = ::com::sun::star;

namespace dp_gui {

struct DeploymentGuiResMgr :
    public ::rtl::StaticWithInit< ResMgr *, DeploymentGuiResMgr > {
        ResMgr * operator () () {
            return ResMgr::CreateResMgr( "deploymentgui" );
    }
};

struct BrandName : public ::rtl::StaticWithInit<const ::rtl::OUString, BrandName> {
    const ::rtl::OUString operator () () {
        return ::utl::ConfigManager::GetDirectConfigProperty(
            ::utl::ConfigManager::PRODUCTNAME ).get< ::rtl::OUString >();
    }
};

class DpGuiResId : public ResId
{
public:
    DpGuiResId( USHORT nId ):ResId( nId, *DeploymentGuiResMgr::get() ) {}
};

} // namespace dp_gui

#endif
