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

#ifndef INCLUDED_EDITENG_UNOEDHLP_HXX
#define INCLUDED_EDITENG_UNOEDHLP_HXX

#include <memory>
#include <vcl/textdata.hxx>
#include <svl/hint.hxx>
#include <tools/gen.hxx>
#include <editeng/editengdllapi.h>

struct EENotify;
class EditEngine;

/** Extends TextHint by two additional parameters which are necessary
    for the SfxHintId::EditSourceParasMoved hint. TextHint's value in this
    case denotes the destination position, the two parameters the
    start and the end of the moved paragraph range.
 */
class SAL_DLLPUBLIC_RTTI SvxEditSourceHint : public TextHint
{
private:
    sal_Int32   mnStart;
    sal_Int32   mnEnd;

public:
            SvxEditSourceHint( SfxHintId nId );
            SvxEditSourceHint( SfxHintId nId, sal_Int32 nValue, sal_Int32 nStart, sal_Int32 nEnd );

    using TextHint::GetValue;
    sal_Int32   GetStartValue() const { return mnStart;}
    sal_Int32   GetEndValue() const { return mnEnd;}
};
class SvxEditSourceHintEndPara final : public SvxEditSourceHint
{
public:
    SvxEditSourceHintEndPara() : SvxEditSourceHint(SfxHintId::EditSourceSelectionChanged) {}
};
/** Helper class for common functionality in edit sources
 */
class EDITENG_DLLPUBLIC SvxEditSourceHelper
{
public:

    /** Translates EditEngine notifications into broadcastable hints

        @param aNotify
        Notification object sent by the EditEngine.

        @return the translated hint
     */
    static ::std::unique_ptr<SfxHint> EENotification2Hint( EENotify const * aNotify );

    /** Calculate attribute run for EditEngines

        Please note that the range returned is half-open: [nStartIndex,nEndIndex)

        @param nStartIndex
        Herein, the start index of the range of similar attributes is returned

        @param nEndIndex
        Herein, the end index (exclusive) of the range of similar attributes is returned

        @param rEE
        The EditEngine to query for attributes

        @param nPara
        The paragraph the following index value is to be interpreted in

        @param nIndex
        The character index from which the range of similar attributed characters is requested
     */
     static void GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, const EditEngine& rEE, sal_Int32 nPara, sal_Int32 nIndex, bool bInCell = false);

    /** Convert point from edit engine to user coordinate space

        As the edit engine internally keeps vertical text unrotated,
        all internal edit engine methods return their stuff unrotated,
        too. This method rotates and shifts given point appropriately,
        if vertical writing is on.

        @param rPoint
        Point to transform

        @param rEESize
        Paper size of the edit engine

        @param  bIsVertical
        Whether output text is vertical or not

        @return the possibly transformed point
     */
    static Point EEToUserSpace( const Point& rPoint, const Size& rEESize, bool bIsVertical );

    /** Convert point from user to edit engine coordinate space

        As the edit engine internally keeps vertical text unrotated,
        all internal edit engine methods return their stuff unrotated,
        too. This method rotates and shifts given point appropriately,
        if vertical writing is on.

        @param rPoint
        Point to transform

        @param rEESize
        Paper size of the edit engine

        @param  bIsVertical
        Whether output text is vertical or not

        @return the possibly transformed point
     */
    static Point UserSpaceToEE( const Point& rPoint, const Size& rEESize, bool bIsVertical );

    /** Convert rect from edit engine to user coordinate space

        As the edit engine internally keeps vertical text unrotated,
        all internal edit engine methods return their stuff unrotated,
        too. This method rotates and shifts given rect appropriately,
        if vertical writing is on.

        @param rRect
        Rectangle to transform

        @param rEESize
        Paper size of the edit engine

        @param  bIsVertical
        Whether output text is vertical or not

        @return the possibly transformed rect
     */
    static tools::Rectangle EEToUserSpace( const tools::Rectangle& rRect, const Size& rEESize, bool bIsVertical );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
