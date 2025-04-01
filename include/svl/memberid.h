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

#pragma once


       1
       2
    3
     4
   5
  6
   7

// SvxSizeItem
       0
      1
     2

// SvxSearchItem
// XXX When changing the MID count here, also increment the corresponding
// SvxSearchItem SFX_DECL_TYPE(n) value in include/sfx2/msg.hxx to match, and
// add a member to struct SvxSearch in sfx2/sdi/sfxitems.sdi so that dependent
// slot items get generated.
          1
             2
         3
            4
       5
             6
              7
              8
         9
        10
                11
         12
        13
               14
         15
         16
        17
   18
              19
          20
          21
      22
       23

// SfxDocumentInfoItem
              0x13
                 0x17
                  0x1b
                    0x1d
          0x2d
              0x2e
             0x2f
            0x30
              0x31
           0x32
         0x33
              0x34
                 0x35
               0x38
                0x3a
                 0x3b
                   0x3c
                   0x3d
                     0x3e

// only for FastPropertySet
                             0x3f
                            0x40
                         0x41

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
