/*************************************************************************
 *
 *  $RCSfile: ReportLayouter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:21:27 $
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
 */

package com.sun.star.wizards.report;

import com.sun.star.awt.XListBox;
import com.sun.star.awt.XTextComponent;
import com.sun.star.lang.EventObject;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.XInterface;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.ui.*;

public class ReportLayouter {
    UnoDialog CurUnoDialog;
    static String sOrientationHeader;
    static String sOrientVertical;
    static String sOrientHorizontal;
    final int SOTXTTITLE = 28;
    final int SOCONTENTLST = 29;

    final int SOOPTLANDSCAPE = 30;
    final int SOOPTPORTRAIT = 31;
    final int SOLAYOUTLST = 32;
    static String sReportTitle;
    static String slblDataStructure;
    static String slblPageLayout;
    static String sOrganizeFields;
    XListBox xContentListBox;
    XListBox xLayoutListBox;
    int iOldContentPos;
    int iOldLayoutPos;
    ReportDocument CurReportDocument;
    public String[][] LayoutFiles;
    public String[][] ContentFiles;
    private Desktop.OfficePathRetriever curofficepath;

    public ReportLayouter(ReportDocument _CurReportDocument, UnoDialog _CurUnoDialog) {
        try {
            short curtabindex = (short) (100 * ReportWizard.SOTEMPLATEPAGE);
            this.CurUnoDialog = _CurUnoDialog;
            this.CurReportDocument = _CurReportDocument;
            //TODO the constructor for the OfficePathRetriever is redundant and should be instantiated elsewhere
            Desktop odesktop = new Desktop();
            curofficepath = odesktop.new OfficePathRetriever(CurReportDocument.xMSF);
            slblDataStructure = CurUnoDialog.oResource.getResText(UIConsts.RID_REPORT + 15);
            slblPageLayout = CurUnoDialog.oResource.getResText(UIConsts.RID_REPORT + 16);
            sOrganizeFields = CurUnoDialog.oResource.getResText(UIConsts.RID_REPORT + 19);

            sOrientationHeader = CurUnoDialog.oResource.getResText(UIConsts.RID_REPORT + 22);
            sOrientVertical = CurUnoDialog.oResource.getResText(UIConsts.RID_REPORT + 23);
            sOrientHorizontal = CurUnoDialog.oResource.getResText(UIConsts.RID_REPORT + 24);

            ContentFiles = FileAccess.getFolderTitles(CurReportDocument.xMSF, "cnt", ReportWizard.ReportPath);
            LayoutFiles = FileAccess.getFolderTitles(CurReportDocument.xMSF, "stl", ReportWizard.ReportPath);
            XInterface xUcbInterface = (XInterface) CurReportDocument.xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            XSimpleFileAccess xSimpleFileAccess = (XSimpleFileAccess) com.sun.star.uno.UnoRuntime.queryInterface(XSimpleFileAccess.class, xUcbInterface);
            boolean bcntexists = xSimpleFileAccess.exists(ReportWizard.ReportPath + "/cnt-default.stw");
            boolean bstlexists = xSimpleFileAccess.exists(ReportWizard.ReportPath + "/stl-default.stw");
            //TODO Die exception wieder rein; ggfs mit rp absprechen
            //      if ((bcntexists == false) || (bstlexists == false))
            //          throw  new NoValidPathException(CurReportDocument.xMSF);


            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblContent",
                                            new String[] { "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                            new Object[] { new Integer(8), slblDataStructure, new Integer(95), new Integer(27), new Integer(ReportWizard.SOTEMPLATEPAGE), new Short(curtabindex++), new Integer(99)});

            short iSelPos = (short) JavaTools.FieldInList(ContentFiles[1], ReportWizard.ReportPath + "/cnt-default.stw");
            iOldContentPos = (int) iSelPos;
            xContentListBox = CurUnoDialog.insertListBox("lstContent", SOCONTENTLST, new ActionListenerImpl(), new ItemListenerImpl(),
                                            new String[] { "Height", "HelpURL", "PositionX", "PositionY", "SelectedItems", "Step", "StringItemList", "TabIndex", "Width" },
                                            new Object[] { new Integer(108), "HID:34363", new Integer(95), new Integer(37), new short[] { iSelPos }, new Integer(ReportWizard.SOTEMPLATEPAGE), ContentFiles[0], new Short(curtabindex++), new Integer(99)
            });

            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblLayout",
                                            new String[] { "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                            new Object[] { new Integer(8), slblPageLayout, new Integer(205), new Integer(27), new Integer(ReportWizard.SOTEMPLATEPAGE), new Short(curtabindex++), new Integer(99)});

            short iSelLayoutPos = (short) JavaTools.FieldInList(LayoutFiles[1], ReportWizard.ReportPath + "/stl-default.stw");
            xLayoutListBox = CurUnoDialog.insertListBox("lstLayout", SOLAYOUTLST, new ActionListenerImpl(), new ItemListenerImpl(),
                                                    new String[] { "Height", "HelpURL", "PositionX", "PositionY", "SelectedItems", "Step", "StringItemList", "TabIndex", "Width" },
                                                    new Object[] { new Integer(108), "HID:34364", new Integer(205), new Integer(37), new short[] { iSelLayoutPos }, new Integer(ReportWizard.SOTEMPLATEPAGE), LayoutFiles[0], new Short(curtabindex++), new Integer(99)
            });
            iOldLayoutPos = (int) iSelPos;
            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblOrientation",
                                        new String[] { "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                        new Object[] { new Integer(8), sOrientationHeader, new Integer(95), new Integer(148), new Integer(ReportWizard.SOTEMPLATEPAGE), new Short(curtabindex++), new Integer(74)});

            CurUnoDialog.insertRadioButton("optLandscape", SOOPTLANDSCAPE, new ItemListenerImpl(),
                                        new String[] { "Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width" },
                                        new Object[] { new Integer(10), "HID:34365", sOrientHorizontal, new Integer(101), new Integer(158), new Short((short) 1), new Integer(ReportWizard.SOTEMPLATEPAGE), new Short(curtabindex++), new Integer(60)});

            CurUnoDialog.insertRadioButton("optPortrait", SOOPTPORTRAIT, new ItemListenerImpl(),
                                        new String[] { "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                        new Object[] { new Integer(10), "HID:34366", sOrientVertical, new Integer(101), new Integer(171), new Integer(ReportWizard.SOTEMPLATEPAGE), new Short(curtabindex++), new Integer(60)});

            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlImageControlModel", "imgOrientation",
                                        new String[] { "Border", "Height", "ImageURL", "PositionX", "PositionY", "ScaleImage", "Step", "Width" },
                                        new Object[] { new Short("0"), new Integer(23), curofficepath.BitmapPath + "/landscape.gif", new Integer(164), new Integer(158), new Boolean(false), new Integer(ReportWizard.SOTEMPLATEPAGE), new Integer(30)});

            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblBlindTextNote_2",
                                        new String[] { "Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "Width" },
                                        new Object[] { new Integer(34), ReportWizard.sBlindTextNote, new Boolean(true), new Integer(205), new Integer(148), new Integer(ReportWizard.SOTEMPLATEPAGE), new Integer(99)});
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void initialize(String _defaultTemplatePath) {
//      CurReportDocument.xTextDocument.lockControllers();
        CurReportDocument.setupRecordSection(_defaultTemplatePath);

        if (CurUnoDialog.getControlProperty("txtTitle", "Text").equals("")) {
            String[] sCommandNames = CurReportDocument.CurDBMetaData.getIncludedCommandNames();
            CurUnoDialog.setControlProperty("txtTitle", "Text", sCommandNames[0]);
        }
        CurUnoDialog.setFocus("lblContent");
    }

    class ItemListenerImpl implements com.sun.star.awt.XItemListener {

        public void itemStateChanged(com.sun.star.awt.ItemEvent EventObject) {
            try {
                int iPos;
                Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, "Enabled", new Boolean(false));
                CurReportDocument.xTextDocument.lockControllers();
                int iKey = CurUnoDialog.getControlKey(EventObject.Source, CurUnoDialog.ControlList);
                switch (iKey) {
                    case SOCONTENTLST :
                        iPos = xContentListBox.getSelectedItemPos();
                        if (iPos != iOldContentPos) {
                            iOldContentPos = iPos;
                            CurReportDocument.swapContentTemplate(ContentFiles[1][iPos]);
                        }
                        break;

                    case SOLAYOUTLST :
                        iPos = xLayoutListBox.getSelectedItemPos();
                        if (iPos != iOldLayoutPos) {
                            iOldLayoutPos = iPos;
                            CurReportDocument.swapLayoutTemplate(LayoutFiles[1][iPos], curofficepath.BitmapPath);
                        }
                        break;

                    case SOOPTLANDSCAPE :
                        CurReportDocument.changePageOrientation(curofficepath.BitmapPath, true);
                        CurUnoDialog.setControlProperty("imgOrientation", "ImageURL", curofficepath.BitmapPath + "/landscape.gif");

                        break;

                    case SOOPTPORTRAIT :
                        CurReportDocument.changePageOrientation(curofficepath.BitmapPath, false);
                        CurUnoDialog.setControlProperty("imgOrientation", "ImageURL", curofficepath.BitmapPath + "/portrait.gif");
                        break;

                    default :
                        break;
                }
            } catch (RuntimeException e) {
                e.printStackTrace();
            }
            CurReportDocument.unlockallControllers();
            Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, "Enabled", new Boolean(true));
        }

        public void disposing(EventObject eventObject) {
        }
    }

    class ActionListenerImpl implements com.sun.star.awt.XActionListener {

        public void disposing(EventObject eventObject) {
        }

        public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent) {
            try {
                Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, "Enabled", new Boolean(false));
                CurReportDocument.xTextDocument.lockControllers();
                boolean blandscape = (((Short) CurUnoDialog.getControlProperty("optLandscape", "State")).shortValue() == 1);
                CurReportDocument.changePageOrientation(curofficepath.BitmapPath, blandscape);
            } catch (Exception exception) {
                exception.printStackTrace(System.out);
            }
            CurReportDocument.unlockallControllers();
            Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, "Enabled", new Boolean(true));
        }
    }

/*  class TextListenerImpl implements com.sun.star.awt.XTextListener {

        public void textChanged(com.sun.star.awt.TextEvent EventObject) {
            try {
                String TitleName = xTitleTextBox.getText();
                CurReportDocument.updateReportTitle(xTitleTextBox);
            } catch (Exception exception) {
                exception.printStackTrace(System.out);
            }
        }

        public void disposing(EventObject EventObject) {
        }
    } */

}
