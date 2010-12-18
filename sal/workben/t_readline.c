/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * t_readline.c
 */

#include "osl/file.h"

#include "osl/diagnose.h"
#include "rtl/ustring.h"
#include "rtl/byteseq.h"

#include <stdio.h>

/* main */
int main (int argc, char ** argv)
{
  if (argc > 1)
  {
    oslFileError  result;
    oslFileHandle hFile = 0;

    rtl_uString * pSystemPath = 0;
    rtl_uString * pFileUrl = 0;

    rtl_uString_newFromAscii (&pSystemPath, argv[1]);

    result = osl_getFileURLFromSystemPath (pSystemPath, &pFileUrl);
    rtl_uString_release (pSystemPath), pSystemPath = 0;
    if (result != osl_File_E_None)
      return (result);

    result = osl_openFile (pFileUrl, &hFile, osl_File_OpenFlag_Read);
    rtl_uString_release (pFileUrl), pFileUrl = 0;
    if (result == osl_File_E_None)
    {
      sal_Sequence * pBuffer = 0;
      for ( ;; )
      {
        sal_Int32 i, n;

        result = osl_readLine (hFile, &pBuffer);
        if (result != osl_File_E_None)
          break;
        for (i = 0, n = pBuffer->nElements; i < n; i++)
          printf ("%c", (char)(pBuffer->elements[i]));
        printf("\n");
      }

      rtl_byte_sequence_release (pBuffer), pBuffer = 0;
      (void) osl_closeFile (hFile);
    }
  }
  return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
