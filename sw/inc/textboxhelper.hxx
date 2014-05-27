/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_INC_TEXTBOXHELPER_HXX
#define INCLUDED_SW_INC_TEXTBOXHELPER_HXX

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Type.h>

class SwFrmFmt;

/**
 * A TextBox is a TextFrame, that is tied to a drawinglayer shape.
 *
 * This class provides helper methods to create, query and maintain such
 * TextBoxes.
 */
class SwTextBoxHelper
{
public:
    /// Create a TextBox for a shape.
    static void create(SwFrmFmt* pShape);
    /// Get XTextAppend of a shape's TextBox, if there is any.
    static css::uno::Any getXTextAppend(SwFrmFmt* pShape, const css::uno::Type& rType);
    /// Sync property of TextBox with the one of the shape.
    static void syncProperty(SwFrmFmt* pShape, sal_uInt16 nWID, sal_uInt8 nMemberID, const OUString& rPropertyName, const css::uno::Any& rValue);
};

#endif // INCLUDED_SW_INC_TEXTBOXHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
