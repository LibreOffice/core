/*************************************************************************
 *
 *  $RCSfile: mutex.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: as $ $Date: 2001-06-11 10:19:23 $
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

#ifndef __FRAMEWORK_MACROS_DEBUG_MUTEX_HXX_
#define __FRAMEWORK_MACROS_DEBUG_MUTEX_HXX_

//*****************************************************************************************************************
//  special macros for mutex handling
//*****************************************************************************************************************

#ifdef  ENABLE_MUTEXDEBUG

    //_____________________________________________________________________________________________________________
    //  includes
    //_____________________________________________________________________________________________________________

    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif

    /*_____________________________________________________________________________________________________________
        LOGFILE_MUTEX

        For follow macros we need a special log file. If user forget to specify anyone, we must do it for him!
    _____________________________________________________________________________________________________________*/

    #ifndef LOGFILE_MUTEX
        #define LOGFILE_MUTEX   "mutex.log"
    #endif

    /*_____________________________________________________________________________________________________________
        LOG_LOCKTYPE( _EFALLBACK, _ECURRENT )

        Write informations about current set lock type for whole framework project to special file.
    _____________________________________________________________________________________________________________*/

    #define LOG_LOCKTYPE( _EFALLBACK, _ECURRENT )                                                               \
                /* new scope to prevent us against multiple definitions of variables ... */                     \
                {                                                                                               \
                    ::rtl::OStringBuffer _sBuffer( 256 );                                                       \
                    _sBuffer.append( "Set framework lock type to fallback: \"" );                               \
                    switch( _EFALLBACK )                                                                        \
                    {                                                                                           \
                        case E_NOTHING      :   _sBuffer.append( "E_NOTHING"    );                              \
                                                break;                                                          \
                        case E_OWNMUTEX     :   _sBuffer.append( "E_OWNMUTEX"   );                              \
                                                break;                                                          \
                        case E_SOLARMUTEX   :   _sBuffer.append( "E_SOLARMUTEX" );                              \
                                                break;                                                          \
                        case E_FAIRRWLOCK   :   _sBuffer.append( "E_FAIRRWLOCK" );                              \
                                                break;                                                          \
                    }                                                                                           \
                    _sBuffer.append( "\"\n" );                                                                  \
                    if( _EFALLBACK != _ECURRENT )                                                               \
                    {                                                                                           \
                        _sBuffer.append( "... environment overwrite framework lock type with: \"" );            \
                        switch( _ECURRENT )                                                                     \
                        {                                                                                       \
                            case E_NOTHING      :   _sBuffer.append( "E_NOTHING"    );                          \
                                                    break;                                                      \
                            case E_OWNMUTEX     :   _sBuffer.append( "E_OWNMUTEX"   );                          \
                                                    break;                                                      \
                            case E_SOLARMUTEX   :   _sBuffer.append( "E_SOLARMUTEX" );                          \
                                                    break;                                                      \
                            case E_FAIRRWLOCK   :   _sBuffer.append( "E_FAIRRWLOCK" );                          \
                                                    break;                                                      \
                        }                                                                                       \
                        _sBuffer.append( "\"\n" );                                                              \
                    }                                                                                           \
                    else                                                                                        \
                    {                                                                                           \
                        _sBuffer.append( "... use fallback, because user don't set another value!\n" );         \
                    }                                                                                           \
                    WRITE_LOGFILE( LOGFILE_MUTEX, _sBuffer.makeStringAndClear() )                               \
                }

#else   // #ifdef ENABLE_MUTEXDEBUG

    /*_____________________________________________________________________________________________________________
        If right testmode is'nt set - implements these macro with normal functionality!
        We need the guard but not the log mechanism.
    _____________________________________________________________________________________________________________*/

    #undef  LOGFILE_MUTEX
    #define LOG_LOCKTYPE( _EFALLBACK, _ECURRENT )

#endif  // #ifdef ENABLE_MUTEXDEBUG

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_MUTEX_HXX_
