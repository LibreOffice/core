/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Novell, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Major Contributor(s):
 *  Michael Meeks <michael.meeks@novell.com>
 * Portions created by the Ted are Copyright (C) 2010 Ted. All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
