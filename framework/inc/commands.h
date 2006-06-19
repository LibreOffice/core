/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commands.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:48:00 $
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

#ifndef __FRAMEWORK_COMMANDS_H_
#define __FRAMEWORK_COMMANDS_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
