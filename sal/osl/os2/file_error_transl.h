/*************************************************************************
 *
 *  $RCSfile: file_error_transl.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-20 15:10:11 $
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

#ifndef _FILE_ERROR_TRANSL_H_
#define _FILE_ERROR_TRANSL_H_

#ifndef _OSL_FILE_H_
#include <osl/file.h>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************
   oslTranslateFileError
   Translate errno's to osl file errors

   @param bIsError [in] specifies if nErrno
           should be interpreted as error,
          some libc functions signaling an error
           but errno is nevertheless 0 in this
           case the function should at least
           return osl_File_E_Unknown but in no
           case osl_File_E_None!

   @param nErrno [in] the errno if errno is 0
            and bIsError is true the function
           returns osl_File_E_Unknown

   @returns the osl error code appropriate to
              the errno

 *********************************************/

#define OSL_FET_SUCCESS sal_False
#define OSL_FET_ERROR   sal_True

oslFileError oslTranslateFileError(sal_Bool bIsError, int Errno);


#ifdef __cplusplus
}
#endif

#endif
