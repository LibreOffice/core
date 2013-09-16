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

#ifndef _SW_PORTIONHANDLER_HXX
#define _SW_PORTIONHANDLER_HXX

#include <tools/solar.h>
#include <txttypes.hxx>

/** The SwPortionHandler interface implements a visitor for the layout
 * engine's text portions. This can be used to gather information of
 * the on-screen representation of a single paragraph.
 *
 * For each text portion, one of the methods text(...) or special(...)
 * is called, depending on whether it is a 'normal' run of text, or
 * any other portion. Additionally, the linebreak() method is called
 * once at the end of every on-screen line.
 *
 * All parameters relate to the 'model string', which is the text string
 * held by the corresponding SwTxtNode.
 *
 * The SwPortionHandler can be used with the
 * SwTextFrame::VisitPortions(...) method.
 */
class SwPortionHandler
{
public:

    SwPortionHandler() {}           /// (emtpy) constructor

    virtual ~SwPortionHandler() {}  /// (empty) destructor

    /** text portion. A run of nLength characters from the model
     * string, that contains no special characters like embedded
     * fields, etc. Thus, the on-screen text of this portion
     * corresponds exactly to the corresponding characters in the
     * model string.
     */
    virtual void Text(
        sal_uInt16 nLength,      /// length of this portion in the model string
        sal_uInt16 nType,         /// type of this portion
        sal_Int32 nHeight = 0,   /// height of this portion
        sal_Int32 nWidth = 0     /// width of this portion
        ) = 0;

    /** special portion. This method is called for every non-text
     * portion. The parameters describe the length of the
     * corresponding characters in the model string (often 0 or 1),
     * the text which is displayed, and the type of the portion.
     */
    virtual void Special(
        sal_uInt16 nLength,      /// length of this portion in the model string
        const OUString& rText, /// text which is painted on-screen
        sal_uInt16 nType,         /// type of this portion
        sal_Int32 nHeight = 0,     /// font height of the painted text
        sal_Int32 nWidth = 0     /// width of this portion
        ) = 0;

    /** line break. This method is called whenever a line break in the
     * layout occurs.
     */
    virtual void LineBreak(KSHORT nWidth) = 0;

    /** skip characters. The SwTxtFrame may only display partially
     * display a certain paragraph (e.g. when the paragaph is split
     * across multiple pages). In this case, the Skip() method must be
     * called to inform the portion handler to ignore a certain run of
     * characters in the 'model string'. Skip(), if used at all, must
     * be called before any of the other methods is called. Calling
     * Skip() between portions is not allowed.
     */
    virtual void Skip(
        sal_uInt16 nLength   /// number of 'model string' characters to be skipped
        ) = 0;

    /** end of paragraph. This method is to be called when all the
     * paragraph's portions have been processed.
     */
    virtual void Finish() = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
