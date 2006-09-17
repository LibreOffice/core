/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: statusbarontroller.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:02:43 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "statusbarontroller.hxx"
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif

extern "C" void SAL_CALL createRegistryInfo_OStatusbarController()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OStatusbarController> aAutoRegistration;
}
namespace dbaui
{
    using namespace svt;
    using namespace com::sun::star::uno;
    using namespace com::sun::star::beans;
    using namespace com::sun::star::lang;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::util;

    IMPLEMENT_SERVICE_INFO1_STATIC(OStatusbarController,"com.sun.star.sdb.ApplicationStatusbarController","com.sun.star.frame.StatusbarController")
    IMPLEMENT_FORWARD_XINTERFACE2(OStatusbarController,StatusbarController,OStatusbarController_BASE)
}
