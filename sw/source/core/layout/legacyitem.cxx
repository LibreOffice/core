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

#include <legacyitem.hxx>
#include <tools/stream.hxx>
#include <sal/log.hxx>
#include <fmtornt.hxx>

namespace legacy::SwFormatVert
{
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SwFormatVertOrient& rItem, SvStream& rStrm, sal_uInt16 nVersionAbusedAsSize)
        {
            SwTwips yPos(0);
            sal_Int16 orient(0);
            sal_Int16 relation(0);

            switch (nVersionAbusedAsSize)
            {
                // compatibility hack for Table Auto Format: SwTwips is "long" :(
                // (this means that the file format is platform dependent)
                case 14:
                {
                    sal_Int64 n(0);
                    rStrm.ReadInt64(n);
                    yPos = n;
                    break;
                }
                case 10:
                {
                    sal_Int32 n(0);
                    rStrm.ReadInt32(n);
                    yPos = n;
                    break;
                }
                default:
                    SAL_WARN("sw.core", "SwFormatVertOrient::Create: unknown size");
            }

            rStrm.ReadInt16( orient ).ReadInt16( relation );

            rItem.SetPos(yPos);
            rItem.SetVertOrient(orient);
            rItem.SetRelationOrient(relation);
        }

        SvStream& Store(const SwFormatVertOrient& rItem, SvStream& rStrm, sal_uInt16)
        {
            if constexpr(sizeof(rItem.GetPos()) >= 8)
                rStrm.WriteInt64(rItem.GetPos());
            else
                rStrm.WriteInt32(rItem.GetPos());
            rStrm.WriteInt16(rItem.GetVertOrient()).WriteInt16(rItem.GetRelationOrient());
            return rStrm;
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
