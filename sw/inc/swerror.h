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


// Import & Export
       ErrCode(ErrCodeArea::Sw, ErrCodeClass::Read, 1 )
              ErrCode(ErrCodeArea::Sw, ErrCodeClass::Read, 2 )
         ErrCode(ErrCodeArea::Sw, ErrCodeClass::Read, 8 )
               ErrCode(ErrCodeArea::Sw, ErrCodeClass::Read, 12)
         ErrCode(ErrCodeArea::Sw, ErrCodeClass::Read, 14)
          ErrCode(ErrCodeArea::Sw, ErrCodeClass::Read, 15)
             ErrCode(ErrCodeArea::Sw, ErrCodeClass::Write, 30 )
            ErrCode(ErrCodeArea::Sw, ErrCodeClass::Write, 35 )
          ErrCode(ErrCodeArea::Sw, 50 )
      ErrCode(ErrCodeArea::Sw, ErrCodeClass::Locking, 55 )
              ErrCode(ErrCodeArea::Sw, ErrCodeClass::Path, 55 )
              ErrCode(ErrCodeArea::Sw, ErrCodeClass::NONE, 56 )
             ErrCode(ErrCodeArea::Sw, ErrCodeClass::NONE, 57 )
             ErrCode(ErrCodeArea::Sw, ErrCodeClass::NONE, 58 )
          ErrCode(WarningFlag::Yes, ErrCodeArea::Sw, ErrCodeClass::NONE, 72 )
              ErrCode(WarningFlag::Yes, ErrCodeArea::Sw, ErrCodeClass::Write, 74 )
         ErrCode(WarningFlag::Yes, ErrCodeArea::Sw, ErrCodeClass::Write, 75)
           ErrCode(WarningFlag::Yes, ErrCodeArea::Sw, ErrCodeClass::Write, 76)
         ErrCode(WarningFlag::Yes, ErrCodeArea::Sw, ErrCodeClass::Read, 77)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
