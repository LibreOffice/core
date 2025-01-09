/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svl/eitem.hxx>
#include <svx/svxdllapi.h>

/** Item to enable slide background for filled objects */

class SVXCORE_DLLPUBLIC XFillUseSlideBackgroundItem final : public SfxBoolItem
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(XFillUseSlideBackgroundItem)
    XFillUseSlideBackgroundItem(bool bFill = false);

    virtual XFillUseSlideBackgroundItem* Clone(SfxItemPool* pPool = nullptr) const override;

    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric,
                                 MapUnit ePresMetric, OUString& rText,
                                 const IntlWrapper&) const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
