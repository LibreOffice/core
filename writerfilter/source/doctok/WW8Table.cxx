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

#include <doctok/resources.hxx>

namespace writerfilter {
namespace doctok {

/* WW8sprmTDefTable */

sal_uInt32 WW8sprmTDefTable::get_cellx_count()
{
    return getU8(0x4) + 1;
}

sal_uInt16 WW8sprmTDefTable::get_cellx(sal_uInt32 nIndex)
{
    return getU16(0x5 + nIndex * 2);
}

sal_uInt32 WW8sprmTDefTable::get_tc_count()
{
    return get_cellx_count() - 1;
}

writerfilter::Reference<Properties>::Pointer_t
WW8sprmTDefTable::get_tc(sal_uInt32 pos)
{
    return writerfilter::Reference<Properties>::Pointer_t
        (new WW8TC(this,
                   0x5 + get_cellx_count() * 2 + pos * WW8TC::getSize()));
}

/* WW8sprmTTableBorders */

sal_uInt32 WW8sprmTTableBorders::get_rgbbrc_count()
{
    return 6;
}

writerfilter::Reference<Properties>::Pointer_t
WW8sprmTTableBorders::get_rgbbrc(sal_uInt32 pos)
{
    return writerfilter::Reference<Properties>::Pointer_t
        (new WW8BRC(*this, 0x3 + pos * WW8BRC::getSize()));
}

/* WW8sprmTCellBottomColor */

sal_uInt32 WW8sprmTCellBottomColor::get_cellBottomColor_count()
{
    return getU8(0x2) / 4;
}

sal_uInt32 WW8sprmTCellBottomColor::get_cellBottomColor(sal_uInt32 pos)
{
    return getU32(0x3 + 4 * pos);
}

/* WW8sprmTCellLeftColor */

sal_uInt32 WW8sprmTCellLeftColor::get_cellLeftColor_count()
{
    return getU8(0x2) / 4;
}

sal_uInt32 WW8sprmTCellLeftColor::get_cellLeftColor(sal_uInt32 pos)
{
    return getU32(0x3 + 4 * pos);
}

/* WW8sprmTCellTopColor */

sal_uInt32 WW8sprmTCellTopColor::get_cellTopColor_count()
{
    return getU8(0x2) / 4;
}

sal_uInt32 WW8sprmTCellTopColor::get_cellTopColor(sal_uInt32 pos)
{
    return getU32(0x3 + 4 * pos);
}

/* WW8sprmTCellRightColor */

sal_uInt32 WW8sprmTCellRightColor::get_cellRightColor_count()
{
    return getU8(0x2) / 4;
}

sal_uInt32 WW8sprmTCellRightColor::get_cellRightColor(sal_uInt32 pos)
{
    return getU32(0x3 + 4 * pos);
}

/* WW8sprmTDefTableShd */

sal_uInt32 WW8sprmTDefTableShd::get_shd_count()
{
    return getU8(0x2) / WW8SHD::getSize();
}

writerfilter::Reference<Properties>::Pointer_t
WW8sprmTDefTableShd::get_shd(sal_uInt32 pos)
{
    return writerfilter::Reference<Properties>::Pointer_t
        (new WW8SHD(*this, 0x3 + pos * WW8SHD::getSize()));
}

/* WW8sprmTCellShd */

sal_uInt32 WW8sprmTCellShd::get_shd_count()
{
    return getU8(0x2) / WW8CellShd::getSize();
}

writerfilter::Reference<Properties>::Pointer_t
WW8sprmTCellShd::get_shd(sal_uInt32 pos)
{
    return writerfilter::Reference<Properties>::Pointer_t
        (new WW8CellShd(*this, 0x3 + pos * WW8CellShd::getSize()));
}

/* WW8sprmTCellShadow */

sal_uInt32 WW8sprmTCellShadow::get_cellShadow_count()
{
    return getU8(0x2) / WW8CellShd::getSize();
}

writerfilter::Reference<Properties>::Pointer_t
WW8sprmTCellShadow::get_cellShadow(sal_uInt32 pos)
{
    return writerfilter::Reference<Properties>::Pointer_t
        (new WW8CellShd(*this, 0x3 + pos * WW8CellShd::getSize()));
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
