/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: logmechanism.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:51:13 $
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

#ifndef __FRAMEWORK_MACROS_DEBUG_LOGMECHANISM_HXX_
#define __FRAMEWORK_MACROS_DEBUG_LOGMECHANISM_HXX_

//*****************************************************************************************************************
//  generic macros for logging
//*****************************************************************************************************************

#ifdef  ENABLE_LOGMECHANISM

    //_____________________________________________________________________________________________________________
    //  includes
    //_____________________________________________________________________________________________________________

    #ifndef _RTL_STRING_HXX_
    #include <rtl/string.hxx>
    #endif

    #include <stdio.h>

    /*_____________________________________________________________________________________________________________
        WRITE_LOGFILE( SFILENAME, STEXT )

        Log any information in file. We append any information at file and don't clear it anymore.
        ( Use new scope in macro to declare pFile more then on time in same "parentscope"!
            Don't control pFile before access! What will you doing if its not valid? Log an error ...
              An error and an error is an error ... )

        Attention:  You must use "%s" and STEXT as parameter ... because otherwise encoded strings (they include e.g. %...)
                    are handled wrong.
    _____________________________________________________________________________________________________________*/

    #define WRITE_LOGFILE( SFILENAME, STEXT )                                                                   \
                {                                                                                               \
                    ::rtl::OString  _swriteLogfileFileName  ( SFILENAME );                                      \
                    ::rtl::OString  _swriteLogfileText      ( STEXT     );                                      \
                    FILE* pFile = fopen( _swriteLogfileFileName.getStr(), "a" );                                \
                    fprintf( pFile, "%s", _swriteLogfileText.getStr() );                                        \
                    fclose ( pFile                                    );                                        \
                }

    /*_____________________________________________________________________________________________________________
        LOGTYPE

        For other debug macros we need information about the output mode. If user forget to set this information we
        do it for him. Valid values are:    LOGTYPE_FILECONTINUE
                                            LOGTYPE_FILEEXIT
                                            LOGTYPE_MESSAGEBOX
        The normal case is LOGTYPE_MESSAGEBOX to show assertions in normal manner!
    _____________________________________________________________________________________________________________*/

    #define LOGTYPE_MESSAGEBOX      1
    #define LOGTYPE_FILECONTINUE    2
    #define LOGTYPE_FILEEXIT        3

    #ifndef LOGTYPE
        #define LOGTYPE                                                                                         \
                    LOGTYPE_MESSAGEBOX
    #endif

#else   // #ifdef ENABLE_LOGMECHANISM

    /*_____________________________________________________________________________________________________________
        If right testmode is'nt set - implements these macro empty!
    _____________________________________________________________________________________________________________*/

    #define WRITE_LOGFILE( SFILENAME, STEXT )
    #undef  LOGTYPE

#endif  // #ifdef ENABLE_LOGMECHANISM

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  // #ifndef __FRAMEWORK_MACROS_DEBUG_LOGMECHANISM_HXX_
