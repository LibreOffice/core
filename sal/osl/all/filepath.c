/*************************************************************************
 *
 *  $RCSfile: filepath.c,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 16:46:00 $
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

#include <osl/file.h>
#include <rtl/ustring.h>

static sal_uInt32 SAL_CALL osl_defCalcTextWidth( rtl_uString *ustrText )
{
    return ustrText ? ustrText->length : 0;
}


oslFileError SAL_CALL osl_abbreviateSystemPath( rtl_uString *ustrSystemPath, rtl_uString **pustrCompacted, sal_uInt32 uMaxWidth, oslCalcTextWidthFunc pfnCalcWidth )
{
    oslFileError    error = osl_File_E_None;
    rtl_uString     *ustrPath = NULL;
    rtl_uString     *ustrFile = NULL;
    sal_uInt32      uPathWidth, uFileWidth;

    if ( !pfnCalcWidth )
        pfnCalcWidth = osl_defCalcTextWidth;

    {
        sal_Int32   iLastSlash = rtl_ustr_lastIndexOfChar_WithLength( ustrSystemPath->buffer, ustrSystemPath->length, SAL_PATHDELIMITER );

        if ( iLastSlash >= 0 )
        {
            rtl_uString_newFromStr_WithLength( &ustrPath, ustrSystemPath->buffer, iLastSlash );
            rtl_uString_newFromStr_WithLength( &ustrFile, &ustrSystemPath->buffer[iLastSlash], ustrSystemPath->length - iLastSlash );
        }
        else
        {
            rtl_uString_new( &ustrPath );
            rtl_uString_newFromString( &ustrFile, ustrSystemPath );
        }
    }

    uPathWidth = pfnCalcWidth( ustrPath );
    uFileWidth = pfnCalcWidth( ustrFile );

    /* First abbreviate the directory component of the path */

    while ( uPathWidth + uFileWidth > uMaxWidth )
    {
        if ( ustrPath->length > 3 )
        {
            ustrPath->length--;
            ustrPath->buffer[ustrPath->length-3] = '.';
            ustrPath->buffer[ustrPath->length-2] = '.';
            ustrPath->buffer[ustrPath->length-1] = '.';
            ustrPath->buffer[ustrPath->length] = 0;

            uPathWidth = pfnCalcWidth( ustrPath );
        }
        else
            break;
    }

    /* Now abbreviate file component */

    while ( uPathWidth + uFileWidth > uMaxWidth )
    {
        if ( ustrFile->length > 4 )
        {
            ustrFile->length--;
            ustrFile->buffer[ustrFile->length-3] = '.';
            ustrFile->buffer[ustrFile->length-2] = '.';
            ustrFile->buffer[ustrFile->length-1] = '.';
            ustrFile->buffer[ustrFile->length] = 0;

            uFileWidth = pfnCalcWidth( ustrFile );
        }
        else
            break;
    }

    rtl_uString_newConcat( pustrCompacted, ustrPath, ustrFile );

    /* Event now if path was compacted to ".../..." it can be to large */

    uPathWidth += uFileWidth;

    while ( uPathWidth > uMaxWidth )
    {
        (*pustrCompacted)->length--;
        (*pustrCompacted)->buffer[(*pustrCompacted)->length] = 0;
        uPathWidth = pfnCalcWidth( *pustrCompacted );
    }

    if ( ustrPath )
        rtl_uString_release( ustrPath );

    if ( ustrFile )
        rtl_uString_release( ustrFile );

    return error;
}


