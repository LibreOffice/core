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
package com.sun.star.wizards.web;

import com.sun.star.awt.FontDescriptor;
import com.sun.star.awt.Size;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XFixedText;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.HelpIds;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.IRenderer;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.ImageList;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog2;
import com.sun.star.wizards.ui.ImageList.Counter;

/**
 * This class is a general implementation for a
 * dialog which displays a choice of images.
 * The model and the renderer are
 * still abstract in this class.
 * To use the class one should extend it,
 * in the constructor then set the imageList
 * properties (member name il) like image size, grid size,
 * model renderer aso, and then call "build".
 * This class uses a counter renderer which
 * gets an expression which contains %START %END %TOTAL and replaces
 * them with the actual numbers.
 * <BR/>
 * two buttons: "other" and "none" which are there
 * to enable choosing images that are not in the list and
 * "no image" respectivley, are optional, with default to true,
 * so dialogs which do not need those, should set the corresponding
 * members showDeselectButton and/or showOtherButton to false.
 * <br/>
 * the consturctor should recieve, among others, an Array of String resources - see
 * constructor documentation for details.
 *
 * @author rpiterman
 */
public abstract class ImageListDialog extends UnoDialog2 implements UIConsts
{

    private String[] resources;
    private final static int RES_TITLE = 0;
    private final static int RES_LABEL = 1;
    private final static int RES_OK = 2;
    private final static int RES_CANCEL = 3;
    private final static int RES_HELP = 4;
    private final static int RES_DESELECT = 5;
    private final static int RES_OTHER = 6;
    private final static int RES_COUNTER = 7;    //GUI Components as Class members.
    private XButton btnOK;
    private XButton btnCancel;
    private XButton btnHelp;
    private XButton btnOther;
    private XButton btnDeselect;
    private static final String START = "%START";
    private static final String END = "%END";
    private static final String TOTAL = "%TOTAL";
    protected int cutFilename = 0;
    protected boolean showDeselectButton = true;
    protected boolean showOtherButton = true;
    private XFixedText lblTitle;
    //Font Descriptors as Class members.
    private FontDescriptor fontDescriptor1 = new FontDescriptor();
    protected ImageList il;    //private FileAccess fileAccess;
    private Object result;
    private int hid;

    /**
     *
     * @param xmsf
     * @param resources_ a string array with the following strings :
     * dialog title, label text, ok, cancel, help, deselect, other.
     * <br/> if "deselect" and "other" are not displayed,
     * the array can also be shorter. but if "other" is displayed
     * and "deselect" not, both must be there :-(
     */
    public ImageListDialog(
            XMultiServiceFactory xmsf, int hid_, String[] resources_)
    {
        super(xmsf);
        hid = hid_;
        resources = resources_;
        il = new ImageList();
        il.counterRenderer = new ARenderer(resources[RES_COUNTER]);
    }

    /**
     * adds the controls to the dialog, depending on
     * the size of the image list.
     * This method should be called by subclasses after setting
     * the il ImageList member properties
     */
    protected void build()
    {
        //set dialog properties...

        int ilWidth = (il.getImageSize().Width + il.getGap().Width) * il.getCols() + il.getGap().Width;
        int ilHeight = (il.getImageSize().Height + il.getGap().Height) * il.getRows() + il.getGap().Height;

        int dialogWidth = 6 + ilWidth + 6 + 50 + 6;
        int dialogHeight = 3 + 16 + 3 + (ilHeight + 8 + 14) + 6;

        Helper.setUnoPropertyValues(xDialogModel,
                new String[]
                {
                    "Closeable", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "Moveable", PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, "Title", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, new Integer(dialogHeight), HelpIds.getHelpIdString(hid), Boolean.TRUE, "imgDialog", new Integer(59), new Integer(24), INTEGERS[1], resources[RES_TITLE], new Integer(dialogWidth)
                });
        //Set member- FontDescriptors...
        fontDescriptor1.Weight = 150;

        final String[] PROPNAMES = new String[]
        {
            "DefaultButton", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "PushButtonType", PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
        };

        Integer iButtonsX = new Integer(6 + ilWidth + 6);

        btnOK = insertButton("btnOK", null,
                PROPNAMES,
                new Object[]
                {
                    Boolean.TRUE, INTEGER_14, HelpIds.getHelpIdString(hid + 3), resources[RES_OK], "btnOK", iButtonsX, new Integer(22), new Short((short) com.sun.star.awt.PushButtonType.OK_value), new Short((short) 7), INTEGER_50
                });
        btnCancel = insertButton("btnCancel", null,
                PROPNAMES,
                new Object[]
                {
                    Boolean.FALSE, INTEGER_14, HelpIds.getHelpIdString(hid + 4), resources[RES_CANCEL], "btnCancel", iButtonsX, new Integer(41), new Short((short) com.sun.star.awt.PushButtonType.CANCEL_value), new Short((short) 8), INTEGER_50
                });
        btnHelp = insertButton("btnHelp", null,
                PROPNAMES,
                new Object[]
                {
                    Boolean.FALSE, INTEGER_14, "", resources[RES_HELP], "CommandButton3", iButtonsX, new Integer(71), new Short((short) com.sun.star.awt.PushButtonType.HELP_value), new Short((short) 9), INTEGER_50
                });

        if (showOtherButton)
        {

            int otherY = 22 + ilHeight - 14 - (showDeselectButton ? 19 : 0);

            btnOther = insertButton("btnOther", "other",
                    PROPNAMES,
                    new Object[]
                    {
                        Boolean.FALSE, INTEGER_14, HelpIds.getHelpIdString(hid + 1), resources[RES_OTHER], "btnOther", iButtonsX, new Integer(otherY), new Short((short) com.sun.star.awt.PushButtonType.STANDARD_value), new Short((short) 5), INTEGER_50
                    });
        }

        if (showDeselectButton)
        {

            int deselectY = 22 + ilHeight - 14;

            btnDeselect = insertButton("btnNoImage", "deselect",
                    PROPNAMES,
                    new Object[]
                    {
                        Boolean.FALSE, INTEGER_14, HelpIds.getHelpIdString(hid + 2), resources[RES_DESELECT], "btnNoImage", iButtonsX, new Integer(deselectY), new Short((short) com.sun.star.awt.PushButtonType.STANDARD_value), new Short((short) 4), INTEGER_50
                    });
        }

        il.setStep(new Short((short) 1));
        il.setPos(new Size(6, 22));
        il.helpURL = hid + 5;
        il.tabIndex = 1;
        il.create(this);

        /*lblContainer = insertLabel("lblContainer",
        new String[] {PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH},
        new Object[] { new Integer(176),"lblContainer",new Integer(6),new Integer(17),new Short((short)5),new Integer(214)}
        );*/

        lblTitle = insertLabel("lblTitle",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor1, INTEGERS[8], resources[RES_LABEL], "lblTitle", INTEGERS[6], INTEGERS[6], INTEGERS[1], new Short((short) 4), new Integer(216)
                });

    }

    /**
     * is called when the user clicks "none"
     */
    public void deselect()
    {
        il.setSelected(-1);
    }

    /**
     * is called when the user clicks "other"
     *
     */
    public void other()
    {
    }

    /**
     * @return the currently elected object.
     */
    public Object getSelected()
    {
        return il.getSelectedObject();
    }

    /**
     * sets the currently selected object.
     * @param obj the object (out of the model) to be selected.
     */
    public void setSelected(Object obj)
    {
        il.setSelected(obj);
        il.showSelected();
    }

    /**
     * The counter renderer, which uses a template.
     * The template replaces the Strings "%START", "%END" and
     * "%TOTAL" with the respective values.
     * @author rpiterman
     *
     */
    public static class ARenderer implements IRenderer
    {

        String template;

        /**
         * @param aTempalte a template for this renderer.
         * The strings %START, %END ,%TOTAL will be replaced
         * with the actual values.
         */
        public ARenderer(String aTemplate)
        {
            template = aTemplate;
        }

        public String render(Object counter)
        {
            String s = JavaTools.replaceSubString(template, "" + ((Counter) counter).start, START);
            s = JavaTools.replaceSubString(s, "" + ((Counter) counter).end, END);
            s = JavaTools.replaceSubString(s, "" + ((Counter) counter).max, TOTAL);
            return s;
        }
    }
}
