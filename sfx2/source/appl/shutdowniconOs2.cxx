/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shutdowniconOs2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 13:14:10 $
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


#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif

#include <svtools/dynamicmenuoptions.hxx>

#include "shutdownicon.hxx"

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOB_HPP_
#include <com/sun/star/task/XJob.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif


using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::osl;

//
// This ObjectID must match the one created in WarpIN scripts!!
//
#define QUICKSTART_OBJID    "OOB2_QUICKSTART_2B"

bool ShutdownIcon::IsQuickstarterInstalled()
{
    HOBJECT hObject;
    // Check quickstart icon presence
    hObject = WinQueryObject( "<" QUICKSTART_OBJID ">");
    if (hObject)
        return true;
    // object not found, quickstart not available
    return false;
}

void ShutdownIcon::SetAutostartOs2( bool bActivate )
{
    HOBJECT hObject;

    if( bActivate && IsQuickstarterInstalled() )
    {
        // place quickstart shadow in the startup folder
        hObject = WinCreateObject( "WPShadow", "dummy",
                              "OBJECTID=<" QUICKSTART_OBJID "_SHW>;SHADOWID=<" QUICKSTART_OBJID ">;",
                              "<WP_START>",
                              CO_UPDATEIFEXISTS);
    }
    else
    {
        // remove quickstart shadow from the startup folder
        hObject = WinQueryObject( "<" QUICKSTART_OBJID "_SHW>");
        if (hObject)
            WinDestroyObject( hObject);
    }
}

bool ShutdownIcon::GetAutostartOs2( )
{
    // check for quickstart shadow in the startup folder
    if (WinQueryObject( "<" QUICKSTART_OBJID "_SHW>"))
        return true;
    else
        return false;
}


