/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <svpm.h>

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


