/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_options.h>
#include <svl/eitem.hxx>
#include <svx/svxdllapi.h>

namespace model
{
enum class RectangleAlignment;
}

/** Item that holds a rectangle alignment value.

    e.g. Top Left, Top, Top Right, Center.
    @see model::RectangleAlignment
 */
class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) SvxRectangleAlignmentItem final
    : public SfxEnumItem<model::RectangleAlignment>
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(SvxRectangleAlignmentItem)
    SvxRectangleAlignmentItem(sal_uInt16 nWhich, model::RectangleAlignment nValue);
    virtual ~SvxRectangleAlignmentItem() override;

    SvxRectangleAlignmentItem(SvxRectangleAlignmentItem const&) = default;
    SvxRectangleAlignmentItem(SvxRectangleAlignmentItem&&) = default;
    SvxRectangleAlignmentItem& operator=(SvxRectangleAlignmentItem const&) = delete;
    SvxRectangleAlignmentItem& operator=(SvxRectangleAlignmentItem&&) = delete;

    virtual SvxRectangleAlignmentItem* Clone(SfxItemPool* pPool = nullptr) const override;

    virtual sal_uInt16 GetValueCount() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
