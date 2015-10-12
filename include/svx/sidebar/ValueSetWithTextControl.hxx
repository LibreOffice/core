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
#include <limits.h>
#include <vcl/image.hxx>

#include <vector>

namespace svx { namespace sidebar {

/** Specialization of class <ValueSet>.
    This specialization allows is a one-columned ValueSet which allow
    items containing an image and a text or a text and a second text.

    Especially, used for sidebar related controls.
*/
class SVX_DLLPUBLIC ValueSetWithTextControl : public ValueSet
{
public:
    // control type of specialized <ValueSet>:
    // - image + text
    // - text + text
    enum tControlType
    {
        IMAGE_TEXT,
        TEXT_TEXT
    };

    ValueSetWithTextControl(
        const tControlType eControlType,
        vcl::Window* pParent,
        const ResId& rResId);

    // add item for control type IMAGE_TEXT
    // if control type does not match IMAGE_TEXT no item is added.
    // @param pSelectedItemImage
    // selection item image is optional. if not provided, it is the same as the image item
    // @param pItemHelpText
    // help text is optional. if not provided, it is the same as the item text
    void AddItem(
        const Image& rItemImage,
        const Image* pSelectedItemImage,
        const OUString& rItemText,
        const OUString* pItemHelpText );

    // replace item images for control type IMAGE_TEXT
    void ReplaceItemImages(
        const sal_uInt16 nItemId,
        const Image& rItemImage,
        const Image* pSelectedItemImage );

    // add item for control type TEXT_TEXT
    // if control type does not match TEXT_TEXT no item is added.
    // @param pItemHelpText
    // help text is optional. if not provided, it is the same as the item text
    void AddItem(
        const OUString& rItemText,
        const OUString& rItemText2,
        const OUString* pItemHelpText );

    virtual void UserDraw( const UserDrawEvent& rUDEvt ) override;

private:
    struct ValueSetWithTextItem
    {
        Image maItemImage;
        Image maSelectedItemImage;
        OUString maItemText;
        OUString maItemText2;
    };

    typedef ::std::vector< ValueSetWithTextItem > tItemList;

    const tControlType meControlType;
    tItemList maItems;
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
