/*************************************************************************
 *
 *  $RCSfile: timemeasure.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2001-06-05 10:18:30 $
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

#ifndef __FRAMEWORK_MACROS_DEBUG_TIMEMEASURE_HXX_
#define __FRAMEWORK_MACROS_DEBUG_TIMEMEASURE_HXX_

//*************************************************************************************************************
//  special macros for time measures
//  1) LOGFILE_TIMEMEASURE                  used it to define log file for this operations (default will be set automaticly)
//  2) START_TIMEMEASURE                    start new measure by using given variable names
//  3) START_TIMEMEASURE                    stop current measure by using given variable names and return time
//  4) LOG_TIMEMEASURE                      write measured time to logfile
//*************************************************************************************************************

#ifdef  ENABLE_TIMEMEASURE

    //_________________________________________________________________________________________________________________
    //  includes
    //_________________________________________________________________________________________________________________

    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif

    #ifndef _OSL_TIME_H_
    #include <osl/time.h>
    #endif

    /*_____________________________________________________________________________________________________________
        LOGFILE_TIMEMEASURE

        For follow macros we need a special log file. If user forget to specify anyone, we must do it for him!
    _____________________________________________________________________________________________________________*/

    #ifndef LOGFILE_TIMEMEASURE
        #define LOGFILE_TIMEMEASURE "timemeasure.log"
    #endif

    /*_____________________________________________________________________________________________________________
        class TimeMeasure

        We need this inline class as global timer to make it possible measure times over different objects!
        zB. Use it as baseclass to start timer at ctor (must be first called baseclass!!!) and stop it by calling stop method.
    _____________________________________________________________________________________________________________*/

    class DBGTimeMeasureBase
    {
        public:
            inline DBGTimeMeasureBase()
            {
                m_nEnd   = 0                   ;
                m_nStart = osl_getGlobalTimer();
            }

            inline sal_Int32 stopAndGet()
            {
                m_nEnd = osl_getGlobalTimer();
                return( m_nEnd-m_nStart );
            }

        private:
            sal_Int32 m_nStart ;
            sal_Int32 m_nEnd   ;
    };

    /*_____________________________________________________________________________________________________________
        START_TIMEMEASURE( NSTART, NEND )
        STOP_TIMEMEASURE( NSTART, NEND, NTIME )

        If you doesn't need a time measure above different classes ... you can try this macros!
        They initialize your given members with start end end time ... You can calculate differenz by himself.
    _____________________________________________________________________________________________________________*/

    #define START_TIMEMEASURE( NSTART, NEND )                                                                   \
                sal_Int32   NSTART  = 0;                                                                        \
                sal_Int32   NEND    = 0;                                                                        \
                NSTART = osl_getGlobalTimer();

    #define STOP_TIMEMEASURE( NSTART, NEND, NTIME )                                                             \
                          NEND  = osl_getGlobalTimer();                                                         \
                sal_Int32 NTIME = NEND-NSTART;

    /*_____________________________________________________________________________________________________________
        LOG_TIMEMEASURE( SOPERATION, NSTART )

        Write measured time to logfile.
    _____________________________________________________________________________________________________________*/

    #define LOG_TIMEMEASURE( SOPERATION, NTIME )                                                                \
                {                                                                                               \
                    ::rtl::OStringBuffer _sBuffer( 256 );                                                       \
                    _sBuffer.append( SOPERATION         );                                                      \
                    _sBuffer.append( "\t=\t"            );                                                      \
                    _sBuffer.append( (sal_Int32)(NTIME) );                                                      \
                    _sBuffer.append( " ms\n"            );                                                      \
                    WRITE_LOGFILE( LOGFILE_TIMEMEASURE, _sBuffer.makeStringAndClear().getStr() )                \
                }

#else   // #ifdef ENABLE_TIMEMEASURE

    /*_____________________________________________________________________________________________________________
        If right testmode is'nt set - implements these macros empty!
    _____________________________________________________________________________________________________________*/

    #undef  LOGFILE_TIMEMEASURE
    #define START_TIMEMEASURE( NSTART, NEND )
    #define STOP_TIMEMEASURE( NSTART, NEND, NTIME )
    #define LOG_TIMEMEASURE( SOPERATION, NTIME )

#endif  // #ifdef ENABLE_TIMEMEASURE

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_TIMEMEASURE_HXX_
