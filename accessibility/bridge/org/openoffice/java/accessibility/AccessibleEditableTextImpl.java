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

package org.openoffice.java.accessibility;

import com.sun.star.awt.*;
import com.sun.star.style.*;
import com.sun.star.accessibility.XAccessibleEditableText;
import com.sun.star.beans.PropertyValue;

import javax.swing.text.StyleConstants;

/** The GenericAccessibleEditableText mapps the calls to the java AccessibleEditableText
 *  interface to the corresponding methods of the UNO XAccessibleEditableText interface.
 */
public class AccessibleEditableTextImpl extends AccessibleTextImpl implements javax.accessibility.AccessibleEditableText {
    final static double toPointFactor = 1 / (7/10 + 34.5);

    /** Creates new GenericAccessibleEditableText object */
    public AccessibleEditableTextImpl(XAccessibleEditableText xAccessibleEditableText) {
        super(xAccessibleEditableText);
    }

    /** Cuts the text between two indices into the system clipboard */
    public void cut(int startIndex, int endIndex) {
        try {
            ((XAccessibleEditableText) unoObject).cutText(startIndex, endIndex);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        } catch (com.sun.star.uno.RuntimeException e) {
        }
    }

    /** Deletes the text between two indices */
    public void delete(int startIndex, int endIndex) {
        try {
            ((XAccessibleEditableText) unoObject).deleteText(startIndex, endIndex);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        } catch (com.sun.star.uno.RuntimeException e) {
        }
    }

    /** Returns the text range between two indices */
    public String getTextRange(int startIndex, int endIndex) {
        try {
            return unoObject.getTextRange(startIndex, endIndex);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        } catch (com.sun.star.uno.RuntimeException e) {
        }
        return null;
    }

    /** Inserts the specified string at the given index */
    public void insertTextAtIndex(int index, String s){
        try {
            ((XAccessibleEditableText) unoObject).insertText(s, index);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        } catch (com.sun.star.uno.RuntimeException e) {
        }
    }

    /** Pastes the text form the system clipboard into the text starting at the specified index */
    public void paste(int startIndex) {
        try {
            ((XAccessibleEditableText) unoObject).pasteText(startIndex);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        } catch (com.sun.star.uno.RuntimeException e) {
        }
    }

    /** Replaces the text between two indices with the specified string */
    public void replaceText(int startIndex, int endIndex, String s) {
        try {
            ((XAccessibleEditableText) unoObject).replaceText(startIndex, endIndex, s);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        } catch (com.sun.star.uno.RuntimeException e) {
        }
    }

    /** Selects the text between two indices */
    public void selectText(int startIndex, int endIndex) {
        try {
            unoObject.setSelection(startIndex, endIndex);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        } catch (com.sun.star.uno.RuntimeException e) {
        }
    }

    /** Sets the attributes for the text between two indices */
    public void setAttributes(int startIndex, int endIndex, javax.swing.text.AttributeSet as) {
        java.util.ArrayList<PropertyValue> propertyValues = new java.util.ArrayList<PropertyValue>();

        // Convert Alignment attribute
        Object attribute = as.getAttribute(StyleConstants.Alignment);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "ParaAdjust";

            switch (StyleConstants.getAlignment(as)) {
                case StyleConstants.ALIGN_RIGHT:
                    propertyValue.Value = ParagraphAdjust.RIGHT;
                    break;
                case StyleConstants.ALIGN_CENTER:
                    propertyValue.Value = ParagraphAdjust.CENTER;
                    break;
                case StyleConstants.ALIGN_JUSTIFIED:
                    propertyValue.Value = ParagraphAdjust.BLOCK;
                    break;
                default:
                    propertyValue.Value = ParagraphAdjust.LEFT;
                    break;
            }
            propertyValues.add(propertyValue);
        }

        // Convert Background attribute
        attribute = as.getAttribute(StyleConstants.Background);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "CharBackColor";
            propertyValue.Value = new Integer(StyleConstants.getBackground(as).getRGB());
            propertyValues.add(propertyValue);
        }

        // FIXME: BidiLevel

        // Set Bold attribute
        attribute = as.getAttribute(StyleConstants.Bold);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "CharWeight";
            if (StyleConstants.isBold(as)) {
                propertyValue.Value =  new Float(150);
            } else {
                propertyValue.Value =  new Float(100);
            }
            propertyValues.add(propertyValue);
        }

        // FIXME: Java 1.4 ComponentAttribute, ComponentElementName, ComposedTextAttribute

        // Set FirstLineIndent attribute
        attribute = as.getAttribute(StyleConstants.FirstLineIndent);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "ParaFirstLineIndent";
            propertyValue.Value = new Double(StyleConstants.getFirstLineIndent(as) / toPointFactor);
            propertyValues.add(propertyValue);
        }

        // Set font family attribute
        attribute = as.getAttribute(StyleConstants.FontFamily);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "CharFontPitch";

            if (StyleConstants.getFontFamily(as).equals( "Proportional" )) {
                propertyValue.Value = new Short("2");
            } else {
                propertyValue.Value = new Short("1");
            }
            propertyValues.add(propertyValue);
        }

        // Set font size attribute
        attribute = as.getAttribute(StyleConstants.FontSize);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "CharHeight";
            propertyValue.Value = new Integer(StyleConstants.getFontSize(as));
            propertyValues.add(propertyValue);
        }

        // Map foreground color
        attribute = as.getAttribute(StyleConstants.Foreground);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "CharColor";
            propertyValue.Value = new Integer (StyleConstants.getForeground(as).getRGB());
            propertyValues.add(propertyValue);
        }

        // FIXME: IconAttribute, IconElementName

        // Set italic attribute
        attribute = as.getAttribute(StyleConstants.Italic);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "CharPosture";

            if (StyleConstants.isItalic(as)) {
                propertyValue.Value = FontSlant.ITALIC;
            } else {
                propertyValue.Value = FontSlant.DONTKNOW;
            }
            propertyValues.add(propertyValue);
        }

        // Set left indent attribute
        attribute = as.getAttribute(StyleConstants.LeftIndent);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "ParaFirstLeftMargin";
            propertyValue.Value = new Integer(new Double(StyleConstants.getLeftIndent(as) / toPointFactor).intValue());
            propertyValues.add(propertyValue);
        }

        // Set right indent attribute
        attribute = as.getAttribute(StyleConstants.RightIndent);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "ParaFirstRightMargin";
            propertyValue.Value = new Integer(new Double(StyleConstants.getRightIndent(as) / toPointFactor).intValue());
            propertyValues.add(propertyValue);
        }

        // Set line spacing attribute
        attribute = as.getAttribute(StyleConstants.LineSpacing);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "ParaLineSpacing";
            propertyValue.Value = new Integer(new Double(StyleConstants.getLineSpacing(as) / toPointFactor).intValue());
            propertyValues.add(propertyValue);
        }

        // FIXME: Java 1.4 NameAttribute, Orientation, ResolveAttribute

        // Set space above attribute
        attribute = as.getAttribute(StyleConstants.SpaceAbove);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "ParaTopMargin";
            propertyValue.Value = new Integer(new Double( StyleConstants.getSpaceAbove(as) / toPointFactor).intValue());
            propertyValues.add(propertyValue);
        }

        // Set space below attribute
        attribute = as.getAttribute(StyleConstants.SpaceBelow);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
                        propertyValue.Name = "ParaBottomMargin";
            propertyValue.Value = new Integer(new Double(StyleConstants.getSpaceBelow(as) / toPointFactor).intValue());
            propertyValues.add(propertyValue);
        }

        // Set strike through attribute
        attribute = as.getAttribute(StyleConstants.StrikeThrough);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
                        propertyValue.Name = "CharPosture";
            if (StyleConstants.isStrikeThrough(as)) {
                propertyValue.Value = new Short(FontStrikeout.SINGLE);
            } else {
                propertyValue.Value = new Short(FontStrikeout.NONE);
            }
            propertyValues.add(propertyValue);
        }

        // Set sub-/superscript attribute
        attribute = as.getAttribute(StyleConstants.Superscript);
        if (null == attribute) {
            attribute = as.getAttribute(StyleConstants.Subscript);
        }
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "CharEscapement";

            if (StyleConstants.isSuperscript(as)) {
                propertyValue.Value = new Short( "1" );
            } else if (StyleConstants.isSubscript(as)) {
                propertyValue.Value = new Short( "-1" );
            } else {
                propertyValue.Value = new Short( "0" );
            }
            propertyValues.add(propertyValue);
        }

        // Set tabset attribute
        attribute = as.getAttribute(StyleConstants.TabSet);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "ParaTabStops";

            javax.swing.text.TabSet tabSet = StyleConstants.getTabSet(as);
            java.util.ArrayList<TabStop> tabStops = new java.util.ArrayList<TabStop>(tabSet.getTabCount());

            for (int i = 0, max = tabSet.getTabCount(); i < max; i++) {
                javax.swing.text.TabStop tab = tabSet.getTab(i);
                com.sun.star.style.TabStop unoTab = new com.sun.star.style.TabStop();

                unoTab.Position = new Double(tab.getPosition() / toPointFactor).intValue();

                switch (tab.getAlignment()) {
                    case javax.swing.text.TabStop.ALIGN_CENTER:
                        unoTab.Alignment = TabAlign.CENTER;
                        break;
                    case javax.swing.text.TabStop.ALIGN_RIGHT:
                        unoTab.Alignment = TabAlign.RIGHT;
                        break;
                    case javax.swing.text.TabStop.ALIGN_DECIMAL:
                        unoTab.Alignment = TabAlign.DECIMAL;
                        break;
                    default:
                        unoTab.Alignment = TabAlign.LEFT;
                        break;
                }

                tabStops.add(unoTab);
            }
            propertyValue.Value = tabStops.toArray();
            propertyValues.add(propertyValue);
        }

        // Set underline attribute
        attribute = as.getAttribute(StyleConstants.Underline);
        if (null != attribute) {
            com.sun.star.beans.PropertyValue propertyValue = new com.sun.star.beans.PropertyValue();
            propertyValue.Name = "CharUnderline";

            if (StyleConstants.isUnderline(as)) {
                propertyValue.Value = new Short(FontUnderline.SINGLE);
            } else {
                propertyValue.Value = new Short(FontUnderline.NONE);
            }
            propertyValues.add(propertyValue);
        }

        try {
            ((XAccessibleEditableText) unoObject).setAttributes(startIndex, endIndex, (com.sun.star.beans.PropertyValue[]) propertyValues.toArray());
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        } catch (com.sun.star.uno.RuntimeException e) {
        }
    }

    /** Sets the text contents to the specified string */
    public void setTextContents(String s) {
        try {
            ((XAccessibleEditableText) unoObject).setText(s);
        } catch (com.sun.star.uno.RuntimeException e) {
        }
    }
}
