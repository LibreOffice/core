/*************************************************************************
 *
 *  $RCSfile: memorymeasure.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-06-05 10:27:24 $
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

#ifndef __FRAMEWORK_MACROS_DEBUG_MEMORYMEASURE_HXX_
#define __FRAMEWORK_MACROS_DEBUG_MEMORYMEASURE_HXX_

//*************************************************************************************************************
//  special macros for time measures
//  1) LOGFILE_MEMORYMEASURE                  used it to define log file for this operations (default will be set automaticly)
//  2) MAKE_MEMORY_SNAPSHOT                   make snapshot of currently set memory informations of OS
//  3) LOG_MEMORYMEASURE                      write measured time to logfile
//*************************************************************************************************************

#ifdef ENABLE_MEMORYMEASURE

    #if !defined( WIN ) && !defined( WNT )
        #error "Macros to measure memory access not available under platforms different from windows!"
    #endif

    //_________________________________________________________________________________________________________________
    //  includes
    //_________________________________________________________________________________________________________________

    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif

    #ifndef __SGI_STL_VECTOR
    #include <vector>
    #endif

    /*_____________________________________________________________________________________________________________
        LOGFILE_MEMORYMEASURE

        For follow macros we need a special log file. If user forget to specify anyone, we must do it for him!
    _____________________________________________________________________________________________________________*/

    #ifndef LOGFILE_MEMORYMEASURE
        #define LOGFILE_MEMORYMEASURE "memorymeasure.log"
    #endif

    /*_____________________________________________________________________________________________________________
        class MemoryMeasure

        We use this baseclass to collect all snapshots in one object and analyze this information at one point.
        Macros of this file are used to enable using of this class by special compile-parameter only!
    _____________________________________________________________________________________________________________*/

    class _DBGMemoryMeasure
    {
        //---------------------------------------------------------------------------------------------------------
        private:
            struct _MemoryInfo
            {
                MEMORYSTATUS        aStatus     ;
                ::rtl::OString      sComment    ;
            };

        //---------------------------------------------------------------------------------------------------------
        public:
            //_____________________________________________________________________________________________________
            inline _DBGMemoryMeasure()
            {
            }

            //_____________________________________________________________________________________________________
            // clear used container!
            inline ~_DBGMemoryMeasure()
            {
                ::std::vector< _MemoryInfo >().swap( m_lSnapshots );
            }

            //_____________________________________________________________________________________________________
            inline void makeSnapshot( const ::rtl::OString& sComment )
            {
                _MemoryInfo aInfo;
                aInfo.sComment = sComment;
                GlobalMemoryStatus    ( &(aInfo.aStatus) );
                m_lSnapshots.push_back( aInfo            );
            }

            //_____________________________________________________________________________________________________
            inline ::rtl::OString getLog()
            {
                ::rtl::OStringBuffer sBuffer( 10000 );

                if( m_lSnapshots.size() > 0 )
                {
                    // Write informations to return buffer
                    ::std::vector< _MemoryInfo >::const_iterator pItem1;
                    ::std::vector< _MemoryInfo >::const_iterator pItem2;

                    pItem1 = m_lSnapshots.begin();
                    pItem2 = pItem1;
                    ++pItem2;

                    while( pItem1!=m_lSnapshots.end() )
                    {
                        sBuffer.append( "snap [ "                                   );
                        sBuffer.append( pItem1->sComment                            );
                        sBuffer.append( " ]\n\tavail phys\t=\t"                     );
                        sBuffer.append( (sal_Int32)pItem1->aStatus.dwAvailPhys      );
                        sBuffer.append( "\n\tavail page\t=\t"                       );
                        sBuffer.append( (sal_Int32)pItem1->aStatus.dwAvailPageFile  );
                        sBuffer.append( "\n\tavail virt\t=\t"                       );
                        sBuffer.append( (sal_Int32)pItem1->aStatus.dwAvailVirtual   );
                        sBuffer.append( "\n\tdifference\t=\t[ "                     );

                        if( pItem1 == m_lSnapshots.begin() )
                        {
                            sBuffer.append( (sal_Int32)pItem1->aStatus.dwAvailPhys      );
                            sBuffer.append( ", "                                        );
                            sBuffer.append( (sal_Int32)pItem1->aStatus.dwAvailPageFile  );
                            sBuffer.append( ", "                                        );
                            sBuffer.append( (sal_Int32)pItem1->aStatus.dwAvailVirtual   );
                            sBuffer.append( " ]\n\n"                                    );
                        }
                        else if( pItem2 != m_lSnapshots.end() )
                        {
                            sBuffer.append( (sal_Int32)(pItem2->aStatus.dwAvailPhys     - pItem1->aStatus.dwAvailPhys       ) );
                            sBuffer.append( ", "                                                                              );
                            sBuffer.append( (sal_Int32)(pItem2->aStatus.dwAvailPageFile - pItem1->aStatus.dwAvailPageFile   ) );
                            sBuffer.append( ", "                                                                              );
                            sBuffer.append( (sal_Int32)(pItem2->aStatus.dwAvailVirtual  - pItem1->aStatus.dwAvailVirtual    ) );
                            sBuffer.append( " ]\n\n"                                                                          );
                        }
                        else
                        {
                            sBuffer.append( "0, 0, 0 ]\n\n" );
                        }
                        if( pItem1!=m_lSnapshots.end() ) ++pItem1;
                        if( pItem2!=m_lSnapshots.end() ) ++pItem2;
                    }
                    // clear current list ... make it empty for further snapshots!
                    ::std::vector< _MemoryInfo >().swap( m_lSnapshots );
                }

                return sBuffer.makeStringAndClear();
            }

        //---------------------------------------------------------------------------------------------------------
        private:
            ::std::vector< _MemoryInfo > m_lSnapshots;
    };

    /*_____________________________________________________________________________________________________________
        START_MEMORY_MEASURE

        Create new object to measure memory access.
    _____________________________________________________________________________________________________________*/

    #define START_MEMORYMEASURE( AOBJECT )                                                                      \
                _DBGMemoryMeasure AOBJECT;

    /*_____________________________________________________________________________________________________________
        MAKE_MEMORY_SNAPSHOT

        Make snapshot of currently set memory informations of OS.
        see _DBGMemoryMeasure for further informations
    _____________________________________________________________________________________________________________*/

    #define MAKE_MEMORY_SNAPSHOT( AOBJECT, SCOMMENT )                                                           \
                AOBJECT.makeSnapshot( SCOMMENT );

    /*_____________________________________________________________________________________________________________
        LOG_MEMORYMEASURE( SOPERATION, SCOMMENT, AOBJECT )

        Write measured values to logfile.
    _____________________________________________________________________________________________________________*/

    #define LOG_MEMORYMEASURE( SOPERATION, SCOMMENT, AOBJECT )                                                  \
                {                                                                                               \
                    ::rtl::OStringBuffer _sBuffer( 256 );                                                       \
                    _sBuffer.append( SOPERATION         );                                                      \
                    _sBuffer.append( "\n"               );                                                      \
                    _sBuffer.append( SCOMMENT           );                                                      \
                    _sBuffer.append( "\n\n"             );                                                      \
                    _sBuffer.append( AOBJECT.getLog()   );                                                      \
                    WRITE_LOGFILE( LOGFILE_MEMORYMEASURE, _sBuffer.makeStringAndClear() )                       \
                }

#else   // #ifdef ENABLE_MEMORYMEASURE

    /*_____________________________________________________________________________________________________________
        If right testmode is'nt set - implements these macros empty!
    _____________________________________________________________________________________________________________*/

    #undef  LOGFILE_MEMORYMEASURE
    #define START_MEMORYMEASURE( AOBJECT )
    #define MAKE_MEMORY_SNAPSHOT( AOBJECT, SCOMMENT )
    #define LOG_MEMORYMEASURE( SOPERATION, SCOMMENT, AOBJECT )

#endif  // #ifdef ENABLE_MEMORYMEASURE

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_MEMORYMEASURE_HXX_
