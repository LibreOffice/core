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

#pragma once

#include <sal/types.h>

class SwTextFormatColl;
class SwCharFormat;
class SwFormat;
class SwFrameFormat;
class SwNumRule;
class SwPageDesc;
enum class SwPoolFormatId : sal_uInt16;

/** Access to the style pool
 */
class IDocumentStylePoolAccess
{
public:
    /** Return "Auto-Collection with ID.
        Create, if it does not yet exist.
        If string pointer is defined request only description
        of attributes, do not create style sheet!
    */
    virtual SwTextFormatColl* GetTextCollFromPool(SwPoolFormatId nId, bool bRegardLanguage = true)
        = 0;

    /** Return required automatic format base class.
    */
    virtual SwFormat* GetFormatFromPool(SwPoolFormatId nId) = 0;

    /** Return required automatic format.
     */
    virtual SwFrameFormat* GetFrameFormatFromPool(SwPoolFormatId nId) = 0;

    virtual SwCharFormat* GetCharFormatFromPool(SwPoolFormatId nId) = 0;

    /** Return required automatic page style.
     */
    virtual SwPageDesc* GetPageDescFromPool(SwPoolFormatId nId, bool bRegardLanguage = true) = 0;

    virtual SwNumRule* GetNumRuleFromPool(SwPoolFormatId nId) = 0;

    /** Check whether this "auto-collection" is used in document.
     */
    virtual bool IsPoolTextCollUsed(SwPoolFormatId nId) const = 0;
    virtual bool IsPoolFormatUsed(SwPoolFormatId nId) const = 0;
    virtual bool IsPoolPageDescUsed(SwPoolFormatId nId) const = 0;

protected:
    virtual ~IDocumentStylePoolAccess(){};
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
