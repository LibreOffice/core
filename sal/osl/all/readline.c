/*************************************************************************
 *
 *  $RCSfile: readline.c,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 17:12:52 $
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


/* Test cases:

    1.  A file without line ends
    2.  A file with lines longer than the initial buffer size
    3.  An empty file
    4.
*/

/** Some defines
*/
#define CR 0x0D
#define LF 0x0A

#define INITIAL_BUFF_SIZE  80
#define BUFFER_GROW_FACTOR 2
#define READ_BLOCK_SIZE (INITIAL_BUFF_SIZE - 1)

/** Helper data and function
*/

struct _Buffer
{
    sal_Char*   m_pMem;
    sal_uInt64  m_Capacity;           /* elements possible in buffer */
    sal_uInt64  m_Size;               /* elements actually in buffer */
    sal_uInt64  m_ActiveSectionStart; /* buffer was lastly filled from here to
                                         (m_Size - 1) */
};

typedef struct _Buffer Buffer;


/** Allocate the memory of the buffer
    @Returns    sal_True on succes
*/
static sal_Bool AllocateBuffer(Buffer* pBuffer, sal_uInt64 Capacity)
{
    sal_Bool rc = sal_False;

    OSL_ASSERT(pBuffer);

    if ((pBuffer->m_pMem = (sal_Char*)rtl_allocateZeroMemory((sal_uInt32)Capacity)))
    {
        pBuffer->m_Capacity = Capacity;
        pBuffer->m_Size = 0;
        pBuffer->m_ActiveSectionStart = 0;
        rc = sal_True;
    }

    return rc;
}

/** Release the memory occupied by the buffer
*/
static void FreeBuffer(Buffer* pBuffer)
{
    OSL_ASSERT(pBuffer);

    rtl_freeMemory(pBuffer->m_pMem);
    pBuffer->m_pMem  = 0;
    pBuffer->m_Capacity = 0;
    pBuffer->m_Size   = 0;
    pBuffer->m_ActiveSectionStart = 0;
}

/** Grow the buffer by the specified factor (usually doubling
    the buffer size)
    In case of failure, growing the buffer, the original buffer
    stays untouched

    @Returns    sal_True on success
*/
static sal_Bool GrowBuffer(Buffer* pBuffer, size_t factor)
{
    sal_Bool rc = sal_False;
    void*    p  = 0;

    OSL_ASSERT(pBuffer);

    if ((p = rtl_reallocateMemory(
        pBuffer->m_pMem, (sal_uInt32)(pBuffer->m_Capacity * factor))))
    {
        pBuffer->m_pMem      = (sal_Char*)p;
        pBuffer->m_Capacity *= factor;
        rc                   = sal_True;
    }

    return rc;
}

/** Read n bytes from file into buffer,
    grow the buffer if necessary

    @Returns osl_File_E_None on success else
    an error code
*/
static oslFileError ReadFromFile(oslFileHandle hFile, Buffer* pBuffer, sal_uInt64 Requested, sal_uInt64* pRead)
{
    oslFileError rc;

    OSL_ASSERT(pBuffer);
    OSL_ASSERT(hFile);
    OSL_ASSERT(pRead);

    if (((pBuffer->m_Size + Requested) > pBuffer->m_Capacity) &&
        !GrowBuffer(pBuffer, BUFFER_GROW_FACTOR))
        return osl_File_E_NOMEM;

    pBuffer->m_ActiveSectionStart = pBuffer->m_Size;

    rc = osl_readFile(
        hFile,
        pBuffer->m_pMem + pBuffer->m_ActiveSectionStart,
        Requested,
        pRead);

    if (osl_File_E_None == rc)
        pBuffer->m_Size += *pRead;

    return rc;
}

/** Makes a sequence from the given buffer and release the memory
    occupied by the buffer
*/
static void MakeSequenceFreeBuffer(sal_Sequence** ppSequence, Buffer* pBuffer, sal_uInt64 Length)
{
    OSL_ASSERT(ppSequence);
    OSL_ASSERT(pBuffer);
    OSL_ASSERT(Length <= pBuffer->m_Capacity);

    rtl_byte_sequence_constructFromArray(ppSequence, (sal_Int8*)pBuffer->m_pMem, (sal_Int32)Length);
    FreeBuffer(pBuffer);
}

/** Handle occurence of LF character:
    construct a sequence from buffer
    correct file pointer (maybe we have read more than necessary)

    @Returns osl_File_E_None on success else
    an error code
*/
static oslFileError HandleLFFreeBuffer(oslFileHandle hFile, sal_Sequence** ppSequence, Buffer* pBuffer, sal_uInt64 Pos)
{
    sal_Int64    offset = 0;
    oslFileError rc     = osl_File_E_None;

    OSL_ASSERT(hFile);
    OSL_ASSERT(pBuffer);
    OSL_ASSERT(LF == pBuffer->m_pMem[Pos]);

    /* correct file pointer pos in case we have read to far */
    offset = pBuffer->m_Size - (Pos + 1);
    rc = osl_setFilePos(hFile, osl_Pos_Current, -offset);

    if (osl_File_E_None == rc)
        MakeSequenceFreeBuffer(ppSequence, pBuffer, Pos);
    else
        FreeBuffer(pBuffer);

    return rc;
}

/** Handle occurence of CR character
    construct a sequence from buffer
    correct file pointer (maybe we have read more than necessary)

    @Returns osl_File_E_None on success else
    an error code
*/
static oslFileError HandleCRFreeBuffer(oslFileHandle hFile, sal_Sequence** ppSequence, Buffer* pBuffer, sal_uInt64 Pos)
{
    sal_Int64    offset = 0;
    sal_uInt64   nread  = 0;
    oslFileError rc     = osl_File_E_None;

    OSL_ASSERT(hFile);
    OSL_ASSERT(pBuffer);
    OSL_ASSERT(CR == pBuffer->m_pMem[Pos]);

    if (Pos == (pBuffer->m_Size - 1))
    {
        /*  only need to check if the next byte is a LF
            that's why reading only one byte from file */
        rc = ReadFromFile(hFile, pBuffer, 1, &nread);

        if (osl_File_E_None != rc)
        {
            FreeBuffer(pBuffer);
            return rc;
        }
        else if (0 == nread)
        {
            MakeSequenceFreeBuffer(ppSequence, pBuffer, Pos);
            return osl_File_E_None;
        }
    }

    if (LF == pBuffer->m_pMem[Pos + 1])
        Pos++;

    /* correct the file pointer */
    offset = pBuffer->m_Size - (Pos + 1);
    rc = osl_setFilePos(hFile, osl_Pos_Current, -offset);

    if (osl_File_E_None == rc)
        MakeSequenceFreeBuffer(ppSequence, pBuffer, Pos - 1);
    else
        FreeBuffer(pBuffer);

    return rc;
}

/***************************************************************************
    osl_readLine (platform independent)
    Reads a line from given file. The new line delimiter(s) are NOT returned!
    Valid line ends: \n, \r\n or \r

    @param  Handle [in] Handle to an open file.
    @param  ppSequence [in/out] a pointer to a valid sequence.

    @return osl_File_E_None on success otherwise one of the following errorcodes:<p>

    osl_File_E_INVAL        the format of the parameters was not valid<br>
    osl_File_E_NOMEM        the necessary memory could not be allocated
****************************************************************************/

oslFileError SAL_CALL osl_readLine(oslFileHandle Handle, sal_Sequence** ppSeq)
{
    oslFileError rc;
    sal_uInt64   nread = 0;
    Buffer       line_buffer;
    sal_uInt64       pos;

    OSL_PRECOND(Handle, "invalid handle");
    OSL_PRECOND(ppSeq,  "invalid parameter detected");

    if (!AllocateBuffer(&line_buffer, INITIAL_BUFF_SIZE))
        return osl_File_E_NOMEM;

    for(;;)
    {
        rc = ReadFromFile(Handle, &line_buffer, READ_BLOCK_SIZE, &nread);

        if (osl_File_E_None != rc)
        {
            FreeBuffer(&line_buffer);
            return rc;
        }
        else if (0 == nread)
        {
            MakeSequenceFreeBuffer(ppSeq, &line_buffer, line_buffer.m_Size);
            return osl_File_E_None;
        }

        /* scan buffer for line end */
        for (pos = line_buffer.m_ActiveSectionStart; pos < line_buffer.m_Size; pos++)
        {
            switch(line_buffer.m_pMem[pos])
            {
            case LF:
                return HandleLFFreeBuffer(Handle, ppSeq, &line_buffer, pos);
            case CR:
                return HandleCRFreeBuffer(Handle, ppSeq, &line_buffer, pos);
            }
        }
    } /* end for */

    OSL_POSTCOND(sal_False, "Should not be here");

    return osl_File_E_None;
}
