/*************************************************************************
 *
 *  $RCSfile: commands.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2002-07-02 07:22:42 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

/*-************************************************************************************************************//**
    @short          These values describe some command line arguments of our office.
                    We need it e.g. to detect running mode of it - plugin, headless ...
*//*-*************************************************************************************************************/

#define COMMAND_PLUGIN                      DECLARE_ASCII("-plugin"     )   /// office is plugged into a browser
#define COMMAND_HEADLESS                    DECLARE_ASCII("-headless"   )   /// office runs in headless mode for scripting
#define COMMAND_INVISIBLE                   DECLARE_ASCII("-invisible"  )   /// office runs in invisible mode which supress splash screen and first empry document
#define COMMAND_SERVER                      DECLARE_ASCII("-server"     )   /// office runs as server

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
