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
#ifndef INCLUDED_SVX_SIDEBAR_VALUESETWITHTEXTCONTROL_HXX
#define INCLUDED_SVX_SIDEBAR_VALUESETWITHTEXTCONTROL_HXX

#include <svx/svxdllapi.h>

#include <svtools/valueset.hxx>

#include <vector>

namespace svx::sidebar
{
/** Specialization of class <ValueSet>.
    This specialization allows is a one-columned ValueSet which allow
    items containing an image and a text or a text and a second text.

    Especially, used for sidebar related controls.
*/
class SVX_DLLPUBLIC ValueSetWithTextControl final : public ValueSet
{
public:
    ValueSetWithTextControl();

    SVX_DLLPRIVATE virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    void AddItem(const OUString& rItemText, const OUString& rItemText2);

    SVX_DLLPRIVATE virtual void UserDraw(const UserDrawEvent& rUDEvt) override;

private:
    struct ValueSetWithTextItem
    {
        OUString maItemText;
        OUString maItemText2;
    };

    ::std::vector<ValueSetWithTextItem> maItems;
};

} // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
