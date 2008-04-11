/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: commands.h,v $
 * $Revision: 1.5 $
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

#ifndef __FRAMEWORK_COMMANDS_H_
#define __FRAMEWORK_COMMANDS_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include <macros/generic.hxx>
#include <vos/process.hxx>
#include <rtl/ustring.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

/*-************************************************************************************************************//**
    @short          These values describe some command line arguments of our office.
                    We need it e.g. to detect running mode of it - plugin, headless ...
*//*-*************************************************************************************************************/

#define COMMAND_PLUGIN    DECLARE_ASCII("-plugin"     ) /// office is plugged into a browser
#define COMMAND_HEADLESS  DECLARE_ASCII("-headless"   ) /// office runs in headless mode for scripting
#define COMMAND_INVISIBLE DECLARE_ASCII("-invisible"  ) /// office runs in invisible mode which supress splash screen and first empry document
#define COMMAND_SERVER    DECLARE_ASCII("-server"     ) /// office runs as server

/*-************************************************************************************************************//**
    @short          This method parse command line of office and search for any argument.
                    We return true/false for existing/non existing values.
*//*-*************************************************************************************************************/

sal_Bool c_existCommand( const ::rtl::OUString& sSearchCommand )
{
    // Set default return value if search failed or unknown commands are searched.
    sal_Bool            bState          = sal_False                         ;
    ::vos::OStartupInfo aCommandLine                                        ;
    sal_uInt32          nCount          = aCommandLine.getCommandArgCount() ;
    ::rtl::OUString     sCommand                                            ;
    for( sal_uInt32 nCommand=0; nCommand<nCount; ++nCommand )
    {
        if( aCommandLine.getCommandArg( nCommand, sCommand ) == ::vos::OStartupInfo::E_None )
        {
            if( sCommand == sSearchCommand )
            {
                bState = sal_True;
                break;
            }
        }
    }

    return bState;
}

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_TARGETS_H_
