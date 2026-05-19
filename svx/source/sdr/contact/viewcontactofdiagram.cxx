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

#include <sdr/contact/viewcontactofdiagram.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xdef.hxx>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

namespace sdr::contact
{
ViewContactOfDiagram::ViewContactOfDiagram(SdrObjGroup& rGroup)
    : ViewContactOfGroup(rGroup)
    , maBGClone()
    , maFGClone()
{
}

ViewContactOfDiagram::~ViewContactOfDiagram() {}

bool ViewContactOfDiagram::isLineStyleActiveInBGObject() const
{
    if (0 == ViewContactOfGroup::GetObjectCount())
        return false;

    SdrObject* pObj(GetSdrObjGroup().GetSubList()->GetObj(0));
    if (nullptr == pObj)
        return false;

    auto& rLineStyleItem(pObj->GetMergedItem(XATTR_LINESTYLE));
    return (css::drawing::LineStyle_NONE != rLineStyleItem.GetValue());
}

sal_uInt32 ViewContactOfDiagram::GetObjectCount() const
{
    if (isLineStyleActiveInBGObject())
        return ViewContactOfGroup::GetObjectCount() + 1;
    return ViewContactOfGroup::GetObjectCount();
}

ViewContact& ViewContactOfDiagram::GetViewContact(sal_uInt32 nIndex) const
{
    if (0 == nIndex && isLineStyleActiveInBGObject())
    {
        // need to suppress LineStyle in ViewContact[0] the BGObject
        if (maBGClone)
            return maBGClone->GetViewContact();

        if (0 != ViewContactOfGroup::GetObjectCount())
        {
            SdrObject* pObj(GetSdrObjGroup().GetSubList()->GetObj(0));
            if (nullptr != pObj)
            {
                maBGClone = pObj->CloneSdrObject(pObj->getSdrModelFromSdrObject());
                maBGClone->SetMergedItem(XLineStyleItem(css::drawing::LineStyle_NONE));
                return maBGClone->GetViewContact();
            }
        }

        return ViewContactOfGroup::GetViewContact(0);
    }
    else if (nIndex >= ViewContactOfGroup::GetObjectCount())
    {
        // need to provide an extra-ViewContact with the
        // LineStyle
        if (maFGClone)
            return maFGClone->GetViewContact();

        if (0 != ViewContactOfGroup::GetObjectCount())
        {
            SdrObject* pObj(GetSdrObjGroup().GetSubList()->GetObj(0));
            if (nullptr != pObj)
            {
                maFGClone = pObj->CloneSdrObject(pObj->getSdrModelFromSdrObject());
                maFGClone->SetMergedItem(XFillStyleItem(css::drawing::FillStyle_NONE));
                return maFGClone->GetViewContact();
            }
        }

        return ViewContactOfGroup::GetViewContact(0);
    }

    return ViewContactOfGroup::GetViewContact(nIndex);
}

void ViewContactOfDiagram::ActionChanged()
{
    ViewContactOfGroup::ActionChanged();

    maBGClone.clear();
    maFGClone.clear();
}
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
