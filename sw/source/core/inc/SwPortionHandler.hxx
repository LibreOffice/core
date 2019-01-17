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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_SWPORTIONHANDLER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SWPORTIONHANDLER_HXX

#include <swtypes.hxx>
#include "TextFrameIndex.hxx"

class SwFont;
enum class PortionType;

/** The SwPortionHandler interface implements a visitor for the layout
 * engine's text portions. This can be used to gather information of
 * the on-screen representation of a single paragraph.
 *
 * For each text portion, one of the methods text(...) or special(...)
 * is called, depending on whether it is a 'normal' run of text, or
 * any other portion. Additionally, the linebreak() method is called
 * once at the end of every on-screen line.
 *
 * All parameters relate to the 'view string', which is the text string
 * held by the sequence of all corresponding SwTextFrames.
 *
 * The SwPortionHandler can be used with the
 * SwTextFrame::VisitPortions(...) method.
 */
class SW_DLLPUBLIC SwPortionHandler
{
public:

    SwPortionHandler() {}           /// (empty) constructor

    virtual ~SwPortionHandler() {}  /// (empty) destructor

    /** text portion. A run of nLength characters from the view
     * string, that contains no special characters like embedded
     * fields, etc. Thus, the on-screen text of this portion
     * corresponds exactly to the corresponding characters in the
     * view string.
     */
    virtual void Text(
        TextFrameIndex nLength, ///< length of this portion in the view string
        PortionType nType,       /// type of this portion
        sal_Int32 nHeight = 0,   /// height of this portion
        sal_Int32 nWidth = 0     /// width of this portion
        ) = 0;

    /** special portion. This method is called for every non-text
     * portion. The parameters describe the length of the
     * corresponding characters in the view string (often 0 or 1),
     * the text which is displayed, and the type of the portion.
     */
    virtual void Special(
        TextFrameIndex nLength, ///< length of this portion in the view string
        const OUString& rText, /// text which is painted on-screen
        PortionType nType,         /// type of this portion
        sal_Int32 nHeight = 0,     /// font height of the painted text
        sal_Int32 nWidth = 0,     /// width of this portion
        const SwFont* pFont = nullptr /// font of this portion
        ) = 0;

    /** line break. This method is called whenever a line break in the
     * layout occurs.
     */
    virtual void LineBreak(sal_Int32 nWidth) = 0;

    /** skip characters. The SwTextFrame may only display partially
     * display a certain paragraph (e.g. when the paragraph is split
     * across multiple pages). In this case, the Skip() method must be
     * called to inform the portion handler to ignore a certain run of
     * characters in the 'view string'. Skip(), if used at all, must
     * be called before any of the other methods is called. Calling
     * Skip() between portions is not allowed.
     */
    virtual void Skip(
        TextFrameIndex nLength   /// number of 'view string' characters to be skipped
        ) = 0;

    /** end of paragraph. This method is to be called when all the
     * paragraph's portions have been processed.
     */
    virtual void Finish() = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
