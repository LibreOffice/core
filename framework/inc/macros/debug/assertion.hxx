/*************************************************************************
 *
 *  $RCSfile: assertion.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-21 14:30:52 $
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

#ifndef __FRAMEWORK_MACROS_DEBUG_ASSERTION_HXX_
#define __FRAMEWORK_MACROS_DEBUG_ASSERTION_HXX_

//*****************************************************************************************************************
//  special macros for assertion handling
//      2)  LOGTYPE                                                         use it to define the output of all assertions, errors, exception infos
//      1)  LOGFILE_ASSERTIONS                                              use it to define the file name of log file if LOGTYPE=LOGTYPE_FILE...
//      3)  LOG_ASSERT( BCONDITION, STEXT )                                 show/log an assertion if BCONDITION == false (depends from LOGTYPE)
//      4)  LOG_ERROR( STEXT )                                              show/log an error (depends from LOGTYPE)
//      5)  LOG_EXCEPTION( SMETHOD, SOWNMESSAGE, SEXCEPTIONMESSAGE )        show/log an error (depends from LOGTYPE)
//*****************************************************************************************************************

#ifdef  ENABLE_ASSERTIONS

    //_____________________________________________________________________________________________________________
    //  includes
    //_____________________________________________________________________________________________________________

    #ifndef _OSL_DIAGNOSE_H_
    #include <osl/diagnose.h>
    #endif

    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif

    /*_____________________________________________________________________________________________________________
        LOGFILE_ASSERTIONS

        For follow macros we need a special log file. If user forget to specify anyone, we must do it for him!
    _____________________________________________________________________________________________________________*/

    #ifndef LOGFILE_ASSERTIONS
        #define LOGFILE_ASSERTIONS  "assertions.log"
    #endif

    /*_____________________________________________________________________________________________________________
        LOG_ASSERT( BCONDITION, STEXT )

        Forward assertion to logfile (if condition is FALSE - like a DBG_ASSERT!) and continue with program.
        Set LOGTYPE to LOGTYPE_FILECONTINUE to do this.
        BCONDITION is inserted in "(...)" because user can call this macro with an complex expression!
    _____________________________________________________________________________________________________________*/
    #if LOGTYPE==LOGTYPE_FILECONTINUE

        #define LOG_ASSERT( BCONDITION, STEXT )                                                                 \
                    if ( ( BCONDITION ) == sal_False )                                                          \
                    {                                                                                           \
                        WRITE_LOGFILE( LOGFILE_ASSERTIONS, STEXT )                                              \
                    }

        #define LOG_ASSERT2( BCONDITION, SMETHOD, STEXT )                                                       \
                    if ( ( BCONDITION ) == sal_True )                                                           \
                    {                                                                                           \
                        ::rtl::OStringBuffer sBuffer( 256 );                                                    \
                        sBuffer.append( "ASSERT:\n\t"   );                                                      \
                        sBuffer.append( SMETHOD         );                                                      \
                        sBuffer.append( "\n\t\""        );                                                      \
                        sBuffer.append( STEXT           );                                                      \
                        sBuffer.append( "\"\n"          );                                                      \
                        WRITE_LOGFILE( LOGFILE_ASSERTIONS, sBuffer.makeStringAndClear().getStr() )              \
                    }

    #endif

    /*_____________________________________________________________________________________________________________
        LOG_ASSERT( BCONDITION, STEXT )

        Forward assertion to file and exit the program.
        Set LOGTYPE to LOGTYPE_FILEEXIT to do this.
        BCONDITION is inserted in "(...)" because user can call this macro with an complex expression!
    _____________________________________________________________________________________________________________*/
    #if LOGTYPE==LOGTYPE_FILEXIT

        #define LOG_ASSERT( BCONDITION, STEXT )                                                                 \
                    if ( ( BCONDITION ) == sal_False )                                                          \
                    {                                                                                           \
                        WRITE_LOGFILE( LOGFILE_ASSERTIONS, STEXT )                                              \
                        exit(-1);                                                                               \
                    }

        #define LOG_ASSERT2( BCONDITION, SMETHODE, STEXT )                                                      \
                    if ( ( BCONDITION ) == sal_True )                                                           \
                    {                                                                                           \
                        ::rtl::OStringBuffer sBuffer( 256 );                                                    \
                        sBuffer.append( "ASSERT:\n\t"   );                                                      \
                        sBuffer.append( SMETHOD         );                                                      \
                        sBuffer.append( "\n\t\""        );                                                      \
                        sBuffer.append( STEXT           );                                                      \
                        sBuffer.append( "\"\n"          );                                                      \
                        WRITE_LOGFILE( LOGFILE_ASSERTIONS, sBuffer.makeStringAndClear().getStr() )              \
                        exit(-1);                                                                               \
                    }

    #endif

    /*_____________________________________________________________________________________________________________
        LOG_ASSERT( BCONDITION, STEXT )

        Forward assertions to messagebox. (We use OSL_ENSURE to do this.)
        Set LOGTYPE to LOGTYPE_MESSAGEBOX to do this.
        BCONDITION is inserted in "(...)" because user can call this macro with an complex expression!
    _____________________________________________________________________________________________________________*/
    #if LOGTYPE==LOGTYPE_MESSAGEBOX

        #define LOG_ASSERT( BCONDITION, STEXT )                                                                 \
                    OSL_ENSURE( ( BCONDITION ), STEXT );

        #define LOG_ASSERT2( BCONDITION, SMETHOD, STEXT )                                                       \
                    {                                                                                           \
                        ::rtl::OStringBuffer sBuffer( 256 );                                                    \
                        sBuffer.append( "ASSERT:\n\t"   );                                                      \
                        sBuffer.append( SMETHOD         );                                                      \
                        sBuffer.append( "\n\t\""        );                                                      \
                        sBuffer.append( STEXT           );                                                      \
                        sBuffer.append( "\"\n"          );                                                      \
                        OSL_ENSURE( !( BCONDITION ), sBuffer.makeStringAndClear().getStr() );                   \
                    }

    #endif

    /*_____________________________________________________________________________________________________________
        LOG_ERROR( STEXT )

        Show an error by using current set output mode by define LOGTYPE!
    _____________________________________________________________________________________________________________*/

    #define LOG_ERROR( SMETHOD, STEXT )                                                                         \
                LOG_ASSERT2( sal_True, SMETHOD, STEXT )

    /*_____________________________________________________________________________________________________________
        LOG_EXCEPTION( SMETHOD, SOWNMESSAGE, SEXCEPTIONMESSAGE )

        Show some exception info by using current set output mode by define LOGTYPE!
        We use a seperated scope {} do protect us against multiple variable definitions.
    _____________________________________________________________________________________________________________*/

    #define LOG_EXCEPTION( SMETHOD, SOWNMESSAGE, SEXCEPTIONMESSAGE )                                            \
                {                                                                                               \
                    ::rtl::OStringBuffer sBuffer( 256 );                                                        \
                    sBuffer.append( SOWNMESSAGE             );                                                  \
                    sBuffer.append( "\n"                    );                                                  \
                    sBuffer.append( U2B(SEXCEPTIONMESSAGE)  );                                                  \
                    LOG_ERROR( SMETHOD, sBuffer.makeStringAndClear().getStr() )                                 \
                }

#else   // #ifdef ENABLE_ASSERTIONS

    /*_____________________________________________________________________________________________________________
        If right testmode is'nt set - implements these macro empty!
    _____________________________________________________________________________________________________________*/

    #undef  LOGFILE_ASSERTIONS
    #define LOG_ASSERT( BCONDITION, STEXT )
    #define LOG_ASSERT2( BCONDITION, SMETHOD, STEXT )
    #define LOG_ERROR( SMETHOD, STEXT )
    #define LOG_EXCEPTION( SMETHOD, SOWNMESSAGE, SEXCEPTIONMESSAGE )

#endif  // #ifdef ENABLE_ASSERTIONS

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_ASSERTION_HXX_
