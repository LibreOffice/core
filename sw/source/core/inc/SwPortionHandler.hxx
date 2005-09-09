/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwPortionHandler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:40:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SW_PORTIONHANDLER_HXX
#define _SW_PORTIONHANDLER_HXX

#include <tools/solar.h>
class String;

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
        USHORT nLength,      /// length of this portion in the model string
        USHORT nType         /// type of this portion
        ) = 0;

    /** special portion. This method is called for every non-text
     * portion. The parameters describe the length of the
     * corresponding characters in the model string (often 0 or 1),
     * the text which is displayed, and the type of the portion.
     */
    virtual void Special(
        USHORT nLength,      /// length of this portion in the model string
        const String& rText, /// text which is painted on-screen
        USHORT nType         /// type of this portion
        ) = 0;

    /** line break. This method is called whenever a line break in the
     * layout occurs.
     */
    virtual void LineBreak() = 0;

    /** skip characters. The SwTxtFrame may only display partially
     * display a certain paragraph (e.g. when the paragaph is split
     * across multiple pages). In this case, the Skip() method must be
     * called to inform the portion handler to ignore a certain run of
     * characters in the 'model string'. Skip(), if used at all, must
     * be called before any of the other methods is called. Calling
     * Skip() between portions is not allowed.
     */
    virtual void Skip(
        USHORT nLength   /// number of 'model string' characters to be skipped
        ) = 0;

    /** end of paragraph. This method is to be called when all the
     * paragraph's portions have been processed.
     */
    virtual void Finish() = 0;
};

#endif
