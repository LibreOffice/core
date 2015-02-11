/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
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
      return result;

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
