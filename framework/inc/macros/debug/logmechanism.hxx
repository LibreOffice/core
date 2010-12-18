/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
