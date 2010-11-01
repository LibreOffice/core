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


#include <unotools/moduleoptions.hxx>

#include <unotools/dynamicmenuoptions.hxx>

#include "shutdownicon.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/beans/NamedValue.hpp>


using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::osl;

//
// This ObjectID must match the one created in WarpIN scripts!!
//
#define QUICKSTART_OBJID    "OO2_QUICKSTART"

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
