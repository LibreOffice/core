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

#include <svx/AccessibleShapeInfo.hxx>
#include <utility>


namespace accessibility {

AccessibleShapeInfo::AccessibleShapeInfo (
        css::uno::Reference<css::drawing::XShape> xShape,
        css::uno::Reference<css::accessibility::XAccessible> xParent,
        IAccessibleParent* pChildrenManager)
    : mxShape (std::move(xShape)),
      mxParent (std::move(xParent)),
      mpChildrenManager (pChildrenManager)
{
    // empty.
}


AccessibleShapeInfo::AccessibleShapeInfo (
        css::uno::Reference<css::drawing::XShape> xShape,
        css::uno::Reference<css::accessibility::XAccessible> xParent)
    : mxShape (std::move(xShape)),
      mxParent (std::move(xParent)),
      mpChildrenManager (nullptr)
{
    // empty.
}

AccessibleShapeInfo::AccessibleShapeInfo (const AccessibleShapeInfo &rOther)
    : mxShape (rOther.mxShape),
      mxParent (rOther.mxParent),
      mpChildrenManager (rOther.mpChildrenManager)
{
    // empty.
}


AccessibleShapeInfo::~AccessibleShapeInfo()
{
    // empty.
}

} // end of namespace accessibility.

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
