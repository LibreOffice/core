/*************************************************************************
 *
 *  $RCSfile: logmechanism.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:23 $
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

#ifndef __FRAMEWORK_MACROS_DEBUG_LOGMECHANISM_HXX_
#define __FRAMEWORK_MACROS_DEBUG_LOGMECHANISM_HXX_

//*****************************************************************************************************************
//  generic macros for logging
//*****************************************************************************************************************

#ifdef  ENABLE_LOGMECHANISM

    //_____________________________________________________________________________________________________________
    //  includes
    //_____________________________________________________________________________________________________________

    #include <stdio.h>

    /*_____________________________________________________________________________________________________________
        WRITE_LOGFILE( SFILENAME, STEXT )

        Log any information in file. We append any information at file and don't clear it anymore.
        ( Use new scope in macro to declare pFile more then on time in same "parentscope"!
            Don't control pFile before access! What will you doing if its not valid? Log an error ...
              An error and an error is an error ... )
    _____________________________________________________________________________________________________________*/

    #define WRITE_LOGFILE( SFILENAME, STEXT )                                                                   \
                {                                                                                               \
                    FILE* pFile = fopen( SFILENAME, "a" );                                                      \
                    fprintf( pFile, STEXT );                                                                    \
                    fclose ( pFile        );                                                                    \
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
