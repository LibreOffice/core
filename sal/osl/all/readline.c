/*************************************************************************
 *
 *  $RCSfile: readline.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tra $ $Date: 2001-10-30 08:54:31 $
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

#include <osl/diagnose.h>
#include <osl/file.h>
#include <rtl/byteseq.h>
#include <rtl/alloc.h>

/* defines */

#define CR 0x0D
#define LF 0x0A

#define INITIAL_BUFF_SIZE  128
#define ENLARGEMENT_FACTOR 2

/***************************************************************************

    osl_readLine (platform independent)
    Reads a line from given file. The new line delimiter(s) are NOT returned!

    @param  Handle [in] Handle to an open file.
    @param  ppSequence [in/out] a pointer to a valid sequence.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>

    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOMEM        the necessary memory could not be allocated

    These errorcodes can (eventually) be returned:<p>
    osl_File_E_INTR         function call was interrupted<br>
    osl_File_E_IO           I/O error<br>
    osl_File_E_ISDIR        Is a directory<br>
    osl_File_E_BADF         Bad file<br>
    osl_File_E_FAULT        Bad address<br>
    osl_File_E_AGAIN        Operation would block<br>
    osl_File_E_NOLINK       Link has been severed<p>

    @see    osl_openFile
    @see    osl_readFile
    @see    osl_writeFile
    @see    osl_setFilePos

****************************************************************************/

oslFileError SAL_CALL osl_readLine( oslFileHandle Handle, sal_Sequence** ppSeq )
{
    oslFileError ferr;
    sal_uInt64   nReadTotal    = 0;
    sal_uInt64   nRead         = 0;
    sal_uInt32   sizeBuff      = INITIAL_BUFF_SIZE;
    sal_Char    *pchBuff       = 0;

    OSL_PRECOND( ppSeq, "invalid parameter detected" );

    /* initial allocate a buffer */

    pchBuff = (sal_Char*)rtl_allocateZeroMemory( sizeBuff );
    if ( 0 == pchBuff )
        return osl_File_E_NOMEM;

    /* read character by character */

    for( ;; )
    {
        /* read the next character fro file into buffer */

        ferr = osl_readFile( Handle, pchBuff + nReadTotal, 1, &nRead );

        if ( ferr != osl_File_E_None )
        {
            rtl_freeMemory( pchBuff );
            return ferr;
        }

        if ( 0 == nRead )
        {
            if ( nReadTotal > 0 )
                rtl_byte_sequence_constructFromArray( ppSeq, (sal_Int8*)pchBuff, (sal_Int32)nReadTotal );
            else
                rtl_byte_sequence_construct( ppSeq, 0 );

            rtl_freeMemory( pchBuff );
            return osl_File_E_None;
        }

        OSL_ASSERT( 1 == nRead );

        nReadTotal++;

        if ( LF == *(pchBuff + nReadTotal - 1) )
        {
            rtl_byte_sequence_constructFromArray( ppSeq, (sal_Int8*)pchBuff, (sal_Int32)(nReadTotal - 1) );
            rtl_freeMemory( pchBuff );
            return osl_File_E_None;
        }
        else if ( CR == *(pchBuff + nReadTotal - 1) )
        {
            /* read one more character to detect possible '\n' */

            ferr = osl_readFile( Handle, pchBuff + nReadTotal, 1, &nRead );

            if ( ferr != osl_File_E_None )
            {
                rtl_freeMemory( pchBuff );
                return ferr;
            }

            if ( nRead > 0 )
            {
                OSL_ASSERT( 1 == nRead );

                /* we don't increment nReadTotal, so now last in buff is pchBuff + nReadTotal !!! */

                if ( LF != *(pchBuff + nReadTotal) )
                {
                    /* correct the file pointer */

                    ferr = osl_setFilePos( Handle, osl_Pos_Current, -1 );

                    if ( ferr != osl_File_E_None )
                    {
                        rtl_freeMemory( pchBuff );
                        return ferr;
                    }
                }
            }

            rtl_byte_sequence_constructFromArray( ppSeq, (sal_Int8*)pchBuff, (sal_Int32)(nReadTotal - 1));
            rtl_freeMemory( pchBuff );
            return osl_File_E_None;
        }

        /* buffer handling */

        if ( nReadTotal == sizeBuff )
        {
            sal_Char *pchTmp = (sal_Char*)rtl_reallocateMemory(
                pchBuff, sizeBuff * ENLARGEMENT_FACTOR );

            if ( 0 == pchTmp )
            {
                rtl_freeMemory( pchBuff );
                return osl_File_E_NOMEM;
            }

            /* exchange pointer and update size info */

            pchBuff   = pchTmp;
            sizeBuff *= ENLARGEMENT_FACTOR;
        }

    } /* end for */

    OSL_POSTCOND( sal_False, "Should not arrive here" );

    return osl_File_E_None;
}
