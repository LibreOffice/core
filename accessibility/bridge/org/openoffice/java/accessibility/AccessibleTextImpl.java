/*************************************************************************
 *
 *  $RCSfile: AccessibleTextImpl.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obr $ $Date: 2002-10-02 07:05:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.java.accessibility;

import java.text.BreakIterator;
import java.util.Locale;
import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleComponent;
import javax.accessibility.AccessibleText;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;

import org.openoffice.accessibility.internal.*;

import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.uno.UnoRuntime;

import drafts.com.sun.star.accessibility.XAccessibleText;

/** This class mapps the methods of a AccessibleText the corresponding UNO
    XAccessibleText methods with two exceptions: because the gnome java
    accessbridge relies on the behavior of the java word iterator that
    handles punctuation characters as word (see java docu for details),
    for objects not implementing AccessibleEditableText, the iteration
    is done locally. Also the character attributes are calculated through
    the AccessibleContext interface ..
*/
public class AccessibleTextImpl implements javax.accessibility.AccessibleText {
    XAccessibleText unoAccessibleText;

    AccessibleContext context;
    String text;

    public AccessibleTextImpl(XAccessibleText xText, AccessibleContext ac) {
        unoAccessibleText = xText;
        context = ac;

        if( unoAccessibleText != null ) {
            text = unoAccessibleText.getText();
        } else {
            text = "";
        }
    }

    public void update() {
        if( unoAccessibleText != null ) {
            text = unoAccessibleText.getText();
        }
    }

    /*
    * AccessibleText
    */

    /** Returns the string after a given index */
    public java.lang.String getAfterIndex(int part, int index) {
        BreakIterator bi;
        int start;
        int end;

        switch (part) {
            case AccessibleText.CHARACTER:
                start = index + 1;
                end = index + 2;
                break;
            case AccessibleText.WORD:
                bi = BreakIterator.getWordInstance(context.getLocale());
                bi.setText(text);
                start = bi.following(index);
                end = bi.next();
                break;
            case AccessibleText.SENTENCE:
                bi = BreakIterator.getSentenceInstance(context.getLocale());
                bi.setText(text);
                start = bi.following(index);
                end = bi.next();
                break;
            default:
                throw new IllegalArgumentException();
        }

        // Ensure start is in valid range
        if( start < 0 ) {
            start = 0;
        }

        // Ensure end is in valid range
        if( end > text.length() ) {
            end = text.length();
        }

        return text.substring(start, end);
    }

    /** Returns the zero-based offset of the caret */
    public int getCaretPosition() {
        // A fixed text does not really have a caret
        return -1;
    }

    /** Returns the start offset within the selected text */
    public int getSelectionStart() {
        // In fixed text nothing can be selected, so return the caret position as specified
        // in AccessibleText docu.
        return getCaretPosition();
    }

    /** Returns the AttributSet for a given character at a given index */
    public javax.swing.text.AttributeSet getCharacterAttribute(int index) {
        if( index > 0 && index < getCharCount() ) {
            SimpleAttributeSet as = new SimpleAttributeSet();
            AccessibleComponent ac = context.getAccessibleComponent();

            // Fixed text has always the default colors
            StyleConstants.setBackground(as, ac.getBackground());
            StyleConstants.setForeground(as, ac.getForeground());

            // Retrieve the information by querying the UI font
            java.awt.Font f = ac.getFont();
            if(f != null) {
                StyleConstants.setFontFamily(as, f.getFamily());
                StyleConstants.setFontSize(as, f.getSize());
                StyleConstants.setBold(as, f.isBold());
                StyleConstants.setItalic(as, f.isItalic());
            }

            // Retrieve the real character attributes from the live object
    //      StyleConstants.setUnderline(as, 0 != (attributes & CharacterAttributes.UNDERLINE));

            return as;
        }

        return null;
    }

    /** Given a point in local ccordinates, return the zero-based index of the character under that point */
    public int getIndexAtPoint(java.awt.Point point) {
        int ret = -1;

        try {
            if( unoAccessibleText != null ) {
                ret = unoAccessibleText.getIndexAtPoint(new Point( point.x, point.y ));
            }
        }

        catch(com.sun.star.uno.RuntimeException e) {
            if( Build.DEBUG ) {
                AccessibleObject.printDebugString("Exception caught for getIndexAtPoint(" + point.x + ", " + point.y + ")");
                AccessibleObject.printDebugString(e.getMessage());
            }
        }

        catch(Exception e) {
            if( Build.DEBUG ) {
                AccessibleObject.printDebugString("Exception caught for getIndexAtPoint(" + point.x + ", " + point.y + ")");
                AccessibleObject.printDebugString(e.getMessage());
            }
        }

        return ret;
    }

    /** Returns the end offset within the selected text */
    public int getSelectionEnd() {
        // In fixed text nothing can be selected, so return the caret position as specified
        // in AccessibleText docu.
        return getCaretPosition();
    }

    /** Returns the string before a given index */
    public java.lang.String getBeforeIndex(int part, int index) {
        BreakIterator bi;
        int start;
        int end;

        switch (part) {
            case AccessibleText.CHARACTER:
                start = index - 1;
                end = index;
                break;
            case AccessibleText.WORD:
                bi = BreakIterator.getWordInstance(context.getLocale());
                bi.setText(text);
                end = bi.preceding(index);
                start = bi.previous();
                break;
            case AccessibleText.SENTENCE:
                bi = BreakIterator.getSentenceInstance(context.getLocale());
                bi.setText(text);
                end = bi.preceding(index);
                start = bi.previous();
                break;
            default:
                throw new IllegalArgumentException();
        }

        // Ensure start is in valid range
        if( start < 0 ) {
            start = 0;
        }

        // Ensure end is in valid range
        if( end > text.length() ) {
            end = text.length();
        }

        return text.substring(start, end);
    }

    /** Returns the string at a given index */
        public java.lang.String getAtIndex(int part, int index) {
        BreakIterator bi;
        int start;
        int end;

        switch (part) {
            case AccessibleText.CHARACTER:
                start = index;
                end = index + 1;
                break;
            case AccessibleText.WORD:
                bi = BreakIterator.getWordInstance(context.getLocale());
                bi.setText(text);
                end = bi.following(index);
                start = bi.previous();
                break;
            case AccessibleText.SENTENCE:
                bi = BreakIterator.getSentenceInstance(context.getLocale());
                bi.setText(text);
                end = bi.following(index);
                start = bi.previous();
                break;
            default:
                throw new IllegalArgumentException();
        }

        // Ensure start is in valid range
        if( start < 0 ) {
            start = 0;
        }

        // Ensure end is in valid range
        if( end > text.length() ) {
            end = text.length();
        }

        return text.substring(start, end);
    }

    /** Returns the number of characters (valid indicies) */
    public int getCharCount() {
        return text.length();
    }

    /** Returns the portion of the text that is selected */
    public java.lang.String getSelectedText() {
        // Nothing selected
        return null;
    }

    /** Determines the bounding box of the character at the given index into the string */
    public java.awt.Rectangle getCharacterBounds(int index) {
        if( index < text.length() ) {
            try {
                Rectangle unoRect = unoAccessibleText.getCharacterBounds(index);
                return new java.awt.Rectangle(unoRect.X, unoRect.Y, unoRect.Width, unoRect.Height);
            }

            catch ( com.sun.star.lang.IndexOutOfBoundsException exception ) {
                // FIXME: The java AccessBridge currently does not handle such exceptions gracefully
                //          throw new IndexOutOfBoundsException( exception.getMessage() );
            }

            catch ( NullPointerException e ) {
            }
        }

        return new java.awt.Rectangle();
    }
}
