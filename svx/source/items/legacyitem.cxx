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

#include <svx/legacyitem.hxx>
#include <tools/stream.hxx>
#include <svx/algitem.hxx>
#include <svx/rotmodit.hxx>

namespace legacy
{
    namespace SvxOrientation
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SvxOrientationItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            sal_uInt16 nVal(0);
            rStrm.ReadUInt16( nVal );
            rItem.SetValue(static_cast<::SvxCellOrientation>(nVal));
        }

        SvStream& Store(const SvxOrientationItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteUInt16( static_cast<sal_uInt16>(rItem.GetValue()) );
            return rStrm;
        }
    }

    namespace SvxMargin
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SvxMarginItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            sal_Int16 nLeft(0);
            sal_Int16 nTop(0);
            sal_Int16 nRight(0);
            sal_Int16 nBottom(0);

            rStrm.ReadInt16( nLeft );
            rStrm.ReadInt16( nTop );
            rStrm.ReadInt16( nRight );
            rStrm.ReadInt16( nBottom );

            rItem.SetLeftMargin(nLeft);
            rItem.SetTopMargin(nTop);
            rItem.SetRightMargin(nRight);
            rItem.SetBottomMargin(nBottom);
        }

        SvStream& Store(const SvxMarginItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteInt16( rItem.GetLeftMargin() );
            rStrm.WriteInt16( rItem.GetTopMargin() );
            rStrm.WriteInt16( rItem.GetRightMargin() );
            rStrm.WriteInt16( rItem.GetBottomMargin() );
            return rStrm;
        }
    }

    namespace SvxRotateMode
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SvxRotateModeItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            sal_uInt16 nVal(0);
            rStrm.ReadUInt16( nVal );
            rItem.SetValue(static_cast<::SvxRotateMode>(nVal));
        }

        SvStream& Store(const SvxRotateModeItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteUInt16( static_cast<sal_uInt16>(rItem.GetValue()) );
            return rStrm;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
