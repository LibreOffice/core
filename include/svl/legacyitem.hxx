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
#ifndef INCLUDED_SVL_LEGACYITEM_HXX
#define INCLUDED_SVL_LEGACYITEM_HXX

#include <svl/svldllapi.h>

//////////////////////////////////////////////////////////////////////////////
// // svl
//     SfxBoolItem                 aLinebreak;
//     SfxInt32Item                aRotateAngle; -> CntInt32Item
//////////////////////////////////////////////////////////////////////////////

class SvStream;
class SfxBoolItem;
class CntInt32Item;

namespace legacy
{
    namespace SfxBool
    {
        sal_uInt16 SVL_DLLPUBLIC GetVersion(sal_uInt16 nFileFormatVersion);
        void SVL_DLLPUBLIC Create(SfxBoolItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        SVL_DLLPUBLIC SvStream& Store(const SfxBoolItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }
    namespace CntInt32
    {
        sal_uInt16 SVL_DLLPUBLIC GetVersion(sal_uInt16 nFileFormatVersion);
        void SVL_DLLPUBLIC Create(CntInt32Item& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        SVL_DLLPUBLIC SvStream& Store(const CntInt32Item& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }
}

#endif // INCLUDED_SVL_LEGACYITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
