/*************************************************************************
 *
 *  $RCSfile: mutex.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: as $ $Date: 2001-05-02 13:00:41 $
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

    #ifndef __FRAMEWORK_THREADHELP_IRWLOCK_H_
    #include <threadhelp/irwlock.h>
    #endif

    #ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
    #include <threadhelp/readguard.hxx>
    #endif

    #ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
    #include <threadhelp/writeguard.hxx>
    #endif

    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif

    #ifndef _RTL_USTRING_
    #include <rtl/ustring>
    #endif

    #ifndef _OSL_MUTEX_HXX_
    #include <osl/mutex.hxx>
    #endif

    /*_____________________________________________________________________________________________________________
        LOGFILE_MUTEX

        For follow macros we need a special log file. If user forget to specify anyone, we must do it for him!
    _____________________________________________________________________________________________________________*/

    #ifndef LOGFILE_MUTEX
        #define LOGFILE_MUTEX   \
                    "mutex.log"
    #endif

    //_____________________________________________________________________________________________________________
    //  debug guard!
    //_____________________________________________________________________________________________________________

    class dbgGuard
    {
        public:

            dbgGuard::dbgGuard( ::osl::Mutex* pMutex, const char* sMethod )
                :   m_pMutex    ( pMutex    )
                ,   m_sMethod   ( sMethod   )
            {
                ::rtl::OStringBuffer sBuffer(256);
                if( m_pMutex->tryToAcquire() == sal_False )
                {
                    sBuffer.append( "\""                            );
                    sBuffer.append( m_sMethod                       );
                    sBuffer.append( "\" block at acquire( pMutex="  );
                    sBuffer.append( (sal_Int32)(m_pMutex)           );
                    sBuffer.append( " this="                        );
                    sBuffer.append( (sal_Int32)(this)               );
                    sBuffer.append( " )\n"                          );
                    WRITE_LOGFILE( LOGFILE_MUTEX, sBuffer.makeStringAndClear() );
                    m_pMutex->acquire();
                }

                sBuffer.append( "\""                        );
                sBuffer.append( m_sMethod                   );
                sBuffer.append( "\" acquire( pMutex=");
                sBuffer.append( (sal_Int32)(m_pMutex)       );
                sBuffer.append( " this="                    );
                sBuffer.append( (sal_Int32)(this)           );
                sBuffer.append( " )\n"                      );
                WRITE_LOGFILE( LOGFILE_MUTEX, sBuffer.makeStringAndClear() );
            }

            dbgGuard( ::osl::Mutex& rMutex, const char* sMethod )
                :   m_pMutex    ( &rMutex   )
                ,   m_sMethod   ( sMethod   )
            {
                ::rtl::OStringBuffer sBuffer(256);
                if( m_pMutex->tryToAcquire() == sal_False )
                {
                    sBuffer.append( "\""                            );
                    sBuffer.append( m_sMethod                       );
                    sBuffer.append( "\" block at acquire( pMutex="  );
                    sBuffer.append( (sal_Int32)(m_pMutex)           );
                    sBuffer.append( " this="                        );
                    sBuffer.append( (sal_Int32)(this)               );
                    sBuffer.append( " )\n"                          );
                    WRITE_LOGFILE( LOGFILE_MUTEX, sBuffer.makeStringAndClear() );
                    m_pMutex->acquire();
                }

                sBuffer.append( "\""                        );
                sBuffer.append( m_sMethod                   );
                sBuffer.append( "\" finish acquire( pMutex=");
                sBuffer.append( (sal_Int32)(m_pMutex)       );
                sBuffer.append( " this="                    );
                sBuffer.append( (sal_Int32)(this)           );
                sBuffer.append( " )\n"                      );
                WRITE_LOGFILE( LOGFILE_MUTEX, sBuffer.makeStringAndClear() );
            }

            inline ~dbgGuard()
            {
                clear();
            }

            inline void clear()
            {
                if( m_pMutex != NULL )
                {
                    m_pMutex->release();

                    ::rtl::OStringBuffer sBuffer(256);
                    sBuffer.append( "\""                    );
                    sBuffer.append( m_sMethod               );
                    sBuffer.append( "\" release( pMutex="   );
                    sBuffer.append( (sal_Int32)(m_pMutex)   );
                    sBuffer.append( " this="                );
                    sBuffer.append( (sal_Int32)(this)       );
                    sBuffer.append( " )\n"                  );
                    WRITE_LOGFILE( LOGFILE_MUTEX, sBuffer.makeStringAndClear() );

                    m_pMutex = NULL;
                }
            }

        protected:
            ::osl::Mutex*   m_pMutex    ;
            ::rtl::OString  m_sMethod   ;
    };

    /*_____________________________________________________________________________________________________________
        LOCK_MUTEX( AGUARD, AMUTEX, SMETHOD )

        These macro define a clearable guard and log information about result of this mutex aquire.
        If you will see a "block statement" without a "successfull statement" you know ...
        these can be a deadlock or something else is wrong!
    _____________________________________________________________________________________________________________*/

    #define LOCK_MUTEX( AGUARD, AMUTEX, SMETHOD )                                                               \
                dbgGuard AGUARD( AMUTEX, SMETHOD );

    /*_____________________________________________________________________________________________________________
        LOCK_GLOBALMUTEX( AGUARD, SMETHOD )

        These macro define a clearable guard and log information about result of this mutex aquire.
        As mutex we use the osl global mutex automaticly.
        If you will see a "block statement" without a "successfull statement" you know ...
        these can be a deadlock or something else is wrong!
    _____________________________________________________________________________________________________________*/

    #define LOCK_GLOBALMUTEX( AGUARD, SMETHOD )                                                                 \
                dbgGuard AGUARD( ::osl::Mutex::getGlobalMutex(), SMETHOD );

    /*_____________________________________________________________________________________________________________
        UNLOCK_MUTEX( AGUARD, SMETHOD )

        Use this macro to unlock any locked mutex in same scope.
        We will log some information for you.
    _____________________________________________________________________________________________________________*/

    #define UNLOCK_MUTEX( AGUARD, SMETHOD )                                                                     \
                AGUARD.clear();

#else   // #ifdef ENABLE_MUTEXDEBUG

    /*_____________________________________________________________________________________________________________
        If right testmode is'nt set - implements these macro with normal functionality!
        We need the guard but not the log mechanism.
    _____________________________________________________________________________________________________________*/

    #undef  LOGFILE_MUTEX

    #define LOCK_MUTEX( AGUARD, AMUTEX, SMETHOD )                                                               \
                ::osl::ClearableMutexGuard AGUARD( AMUTEX );

    #define LOCK_GLOBALMUTEX( AGUARD, SMETHOD )                                                                 \
                ::osl::ClearableMutexGuard AGUARD( ::osl::Mutex::getGlobalMutex() );

    #define UNLOCK_MUTEX( AGUARD, SMETHOD )                                                                     \
                AGUARD.clear();

#endif  // #ifdef ENABLE_MUTEXDEBUG

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_MUTEX_HXX_
