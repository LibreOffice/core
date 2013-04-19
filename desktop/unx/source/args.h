/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <sal/types.h>
#include <rtl/ustring.h>

typedef struct {
  rtl_uString *pAppPath;
  const char  *pPageinType;     // @pagein-writer for - writer etc. else NULL
  sal_Bool     bInhibitSplash;  // should we show a splash screen
  sal_Bool     bInhibitPagein;  // should we run pagein ?
  sal_Bool     bInhibitJavaLdx; // should we run javaldx ?
  sal_Bool     bInhibitPipe;    // for --help and --version

  sal_uInt32   nArgsEnv;        // number of -env: style args
  sal_uInt32   nArgsTotal;      // number of -env: as well as -writer style args
  rtl_uString *ppArgs[1];       // sorted argument array
} Args;

Args *args_parse (void);
void  args_free  (Args *args);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
