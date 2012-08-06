/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVX_UNOEDHLP_HXX
#define _SVX_UNOEDHLP_HXX

#include <memory>
#include <tools/solar.h>
#include <vcl/textdata.hxx>
#include <svl/hint.hxx>
#include <tools/gen.hxx>
#include "editeng/editengdllapi.h"

struct EENotify;
class EditEngine;

#define EDITSOURCE_HINT_PARASMOVED          20
#define EDITSOURCE_HINT_SELECTIONCHANGED    21

/** Extends TextHint by two additional parameters which are necessary
    for the EDITSOURCE_HINT_PARASMOVED hint. TextHint's value in this
    case denotes the destination position, the two parameters the
    start and the end of the moved paragraph range.
 */
class EDITENG_DLLPUBLIC SvxEditSourceHint : public TextHint
{
private:
    sal_uLong   mnStart;
    sal_uLong   mnEnd;

public:
            TYPEINFO();
            SvxEditSourceHint( sal_uLong nId );
            SvxEditSourceHint( sal_uLong nId, sal_uLong nValue, sal_uLong nStart=0, sal_uLong nEnd=0 );

    sal_uLong   GetValue() const;
    sal_uLong   GetStartValue() const;
    sal_uLong   GetEndValue() const;
};

/** Helper class for common functionality in edit sources
 */
class EDITENG_DLLPUBLIC SvxEditSourceHelper
{
public:

    /** Translates EditEngine notifications into broadcastable hints

        @param aNotify
        Notification object send by the EditEngine.

        @return the translated hint
     */
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    static ::std::auto_ptr<SfxHint> EENotification2Hint( EENotify* aNotify );
    SAL_WNODEPRECATED_DECLARATIONS_POP

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

        @return sal_True, if the range has been successfully determined
     */
    static sal_Bool GetAttributeRun( sal_uInt16& nStartIndex, sal_uInt16& nEndIndex, const EditEngine& rEE, sal_uInt16 nPara, sal_uInt16 nIndex );

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
    static Rectangle EEToUserSpace( const Rectangle& rRect, const Size& rEESize, bool bIsVertical );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
