/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: log_services.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 14:58:24 $
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
#include "precompiled_extensions.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef EXTENSIONS_LOG_MODULE_HXX
#include "log_module.hxx"
#endif

//........................................................................
namespace logging
{
//........................................................................

    //--------------------------------------------------------------------
    extern void createRegistryInfo_LoggerPool();
    extern void createRegistryInfo_FileHandler();
    extern void createRegistryInfo_ConsoleHandler();
    extern void createRegistryInfo_PlainTextFormatter();

    static void initializeModule()
    {
        static bool bInitialized( false );
        if ( !bInitialized )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !bInitialized )
            {
                createRegistryInfo_LoggerPool();
                createRegistryInfo_FileHandler();
                createRegistryInfo_ConsoleHandler();
                createRegistryInfo_PlainTextFormatter();
            }
        }
    }

//........................................................................
} // namespace logging
//........................................................................

IMPLEMENT_COMPONENT_LIBRARY_API( ::logging::LogModule, ::logging::initializeModule )

