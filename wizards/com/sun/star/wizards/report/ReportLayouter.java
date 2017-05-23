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
package com.sun.star.wizards.report;

import com.sun.star.awt.XListBox;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.ui.*;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XSearchable;
import com.sun.star.util.XSearchDescriptor;
import com.sun.star.container.XIndexAccess;
import com.sun.star.text.*;
import com.sun.star.wizards.common.TextElement;
import com.sun.star.wizards.common.PlaceholderTextElement;

import java.util.List;
import java.util.ArrayList;

public class ReportLayouter
{

    private UnoDialog CurUnoDialog;
    private static final int SOCONTENTLST = 29;
    final public static int SOOPTLANDSCAPE = 30;
    final public static int SOOPTPORTRAIT = 31;
    private static final int SOLAYOUTLST = 32;
    private XListBox xContentListBox;
    private XListBox xLayoutListBox;
    private int iOldContentPos;
    private int iOldLayoutPos;
    private IReportDocument CurReportDocument;
    private String[][] LayoutFiles;
    private String[][] ContentFiles;
    private Object aOrientationImage;
    private final XMultiServiceFactory m_xMSF;
    private XTextRange          trTitleconst, trAuthorconst, trDateconst, trPageconst;
    private TextElement         teTitleconst, teAuthorconst, teDateconst, tePageconst;
    private List<XTextRange>    constRangeList = new ArrayList<XTextRange>();
    private final boolean isBuilderInstalled;

    public ReportLayouter(XMultiServiceFactory _xMSF, IReportDocument _CurReportDocument, UnoDialog _CurUnoDialog, boolean _isBuilderInstalled)
    {
        m_xMSF = _xMSF;
        isBuilderInstalled = _isBuilderInstalled;
        try
        {
            short curtabindex = (short) (100 * ReportWizard.SOTEMPLATEPAGE);
            this.CurUnoDialog = _CurUnoDialog;
            this.CurReportDocument = _CurReportDocument;
            //TODO the constructor for the OfficePathRetriever is redundant and should be instantiated elsewhere
            String slblDataStructure = CurUnoDialog.m_oResource.getResText("RID_REPORT_15");
            String slblPageLayout = CurUnoDialog.m_oResource.getResText("RID_REPORT_16");

            String sOrientationHeader = CurUnoDialog.m_oResource.getResText("RID_REPORT_22");
            String sOrientVertical = CurUnoDialog.m_oResource.getResText("RID_REPORT_23");
            String sOrientHorizontal = CurUnoDialog.m_oResource.getResText("RID_REPORT_24");

            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblContent",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        8, slblDataStructure, 95, 27, Integer.valueOf(ReportWizard.SOTEMPLATEPAGE), Short.valueOf(curtabindex++), 99
                    });

            short iSelPos = 0;
            ContentFiles = CurReportDocument.getDataLayout();
            iSelPos = (short) JavaTools.FieldInList(ContentFiles[1], CurReportDocument.getContentPath());
            if (iSelPos < 0)
            {
                iSelPos = 0;
            }
            iOldContentPos = iSelPos;
            xContentListBox = CurUnoDialog.insertListBox("lstContent", SOCONTENTLST, new ActionListenerImpl(), new ItemListenerImpl(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.SELECTED_ITEMS, PropertyNames.PROPERTY_STEP, PropertyNames.STRING_ITEM_LIST, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        108, "HID:WIZARDS_HID_DLGREPORT_4_DATALAYOUT", 95, 37, new short[]
                        {
                            iSelPos
                        }, Integer.valueOf(ReportWizard.SOTEMPLATEPAGE), ContentFiles[0], Short.valueOf(curtabindex++), 99
                    });

            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblLayout",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        8, slblPageLayout, 205, 27, Integer.valueOf(ReportWizard.SOTEMPLATEPAGE), Short.valueOf(curtabindex++), 99
                    });

            short iSelLayoutPos = 0;
            LayoutFiles = CurReportDocument.getHeaderLayout();
            iSelLayoutPos = (short) JavaTools.FieldInList(LayoutFiles[1], CurReportDocument.getLayoutPath());
            if (iSelLayoutPos < 0)
            {
                iSelLayoutPos = (short) JavaTools.FieldInList(LayoutFiles[0], CurReportDocument.getLayoutPath());
                if (iSelLayoutPos < 0)
                {
                    iSelLayoutPos = 0;
                }
            }
            xLayoutListBox = CurUnoDialog.insertListBox("lstLayout", SOLAYOUTLST, new ActionListenerImpl(), new ItemListenerImpl(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.SELECTED_ITEMS, PropertyNames.PROPERTY_STEP, PropertyNames.STRING_ITEM_LIST, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        108, "HID:WIZARDS_HID_DLGREPORT_4_PAGELAYOUT", 205, 37, new short[]
                        {
                            iSelLayoutPos
                        }, Integer.valueOf(ReportWizard.SOTEMPLATEPAGE), LayoutFiles[0], Short.valueOf(curtabindex++), 99
                    });
            iOldLayoutPos = iSelPos;
            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblOrientation",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        8, sOrientationHeader, 95, 148, Integer.valueOf(ReportWizard.SOTEMPLATEPAGE), Short.valueOf(curtabindex++), 74
                    });

            short m_nLandscapeState = CurReportDocument.getDefaultPageOrientation() == SOOPTLANDSCAPE ? (short) 1 : 0;
            CurUnoDialog.insertRadioButton("optLandscape", SOOPTLANDSCAPE, new ItemListenerImpl(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        10, "HID:WIZARDS_HID_DLGREPORT_4_LANDSCAPE", sOrientHorizontal, 101, 158, Short.valueOf(m_nLandscapeState), Integer.valueOf(ReportWizard.SOTEMPLATEPAGE), Short.valueOf(curtabindex++), 60
                    });

            short m_nPortraitState = CurReportDocument.getDefaultPageOrientation() == SOOPTPORTRAIT ? (short) 1 : (short) 0;
            CurUnoDialog.insertRadioButton("optPortrait", SOOPTPORTRAIT, new ItemListenerImpl(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        10, "HID:WIZARDS_HID_DLGREPORT_4_PORTRAIT", sOrientVertical, 101, 171, Short.valueOf(m_nPortraitState), Integer.valueOf(ReportWizard.SOTEMPLATEPAGE), Short.valueOf(curtabindex++), 60
                    });

            aOrientationImage = CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlImageControlModel", "imgOrientation",
                    new String[]
                    {
                        PropertyNames.PROPERTY_BORDER, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        Short.valueOf("0"), 23, 164, 158, Boolean.FALSE, Integer.valueOf(ReportWizard.SOTEMPLATEPAGE), 30
                    });

            String sNote = ReportWizard.getBlindTextNote(CurReportDocument, CurUnoDialog.m_oResource);
            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblBlindTextNote_2",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        34, sNote, Boolean.TRUE, 205, 148, Integer.valueOf(ReportWizard.SOTEMPLATEPAGE), 99
                    });
            if (m_nLandscapeState == 1)
            {
                CurUnoDialog.getPeerConfiguration().setImageUrl(aOrientationImage, "private:graphicrepository/wizards/res/landscape_32.png");
            }
            else
            {
                CurUnoDialog.getPeerConfiguration().setImageUrl(aOrientationImage, "private:graphicrepository/wizards/res/portrait_32.png");
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void initialize(String _defaultTemplatePath)
    {
        CurReportDocument.layout_setupRecordSection(_defaultTemplatePath);
        if (CurUnoDialog.getControlProperty("txtTitle", "Text").equals(PropertyNames.EMPTY_STRING))
        {
            String[] sCommandNames = CurReportDocument.getRecordParser().getIncludedCommandNames();
            CurUnoDialog.setControlProperty("txtTitle", "Text", sCommandNames[0]);
        }
        CurReportDocument.layout_selectFirstPage();
        CurUnoDialog.setFocus("lblContent");
    }

    class ItemListenerImpl implements com.sun.star.awt.XItemListener
    {

        public void itemStateChanged(com.sun.star.awt.ItemEvent EventObject)
        {
            try
            {
                int iPos;
                Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
                // LLA: should we lock controllers here?
                //                    CurReportDocument.getDoc().xTextDocument.lockControllers();
                int iKey = CurUnoDialog.getControlKey(EventObject.Source, CurUnoDialog.ControlList);
                switch (iKey)
                {
                    case SOCONTENTLST:
                        iPos = xContentListBox.getSelectedItemPos();
                        if (iPos != iOldContentPos)
                        {
                            iOldContentPos = iPos;
                            CurReportDocument.liveupdate_changeContentTemplate(ContentFiles[1][iPos]);
                            if(!isBuilderInstalled)
                            {
                              clearConstants();
                              drawConstants();
                            }
                        }
                        break;

                    case SOLAYOUTLST:
                        iPos = xLayoutListBox.getSelectedItemPos();
                        if (iPos != iOldLayoutPos)
                        {
                            iOldLayoutPos = iPos;
                            CurReportDocument.liveupdate_changeLayoutTemplate(LayoutFiles[1][iPos]/*, Desktop.getBitmapPath(m_xMSF)*/);
                            if(!isBuilderInstalled)
                            {
                              clearConstants();
                              drawConstants();
                            }
                        }
                        break;

                    case SOOPTLANDSCAPE:
                        // CurReportDocument.getDoc().changePageOrientation(true);
                        try
                        {
                            CurReportDocument.setPageOrientation(SOOPTLANDSCAPE);
                        }
                        catch (com.sun.star.lang.IllegalArgumentException e)
                        {
                        }
                        Helper.setUnoPropertyValue(aOrientationImage, PropertyNames.PROPERTY_IMAGEURL, "private:graphicrepository/wizards/res/landscape_32.png");

                        break;

                    case SOOPTPORTRAIT:
                        // CurReportDocument.getDoc().changePageOrientation( false);
                        try
                        {
                            CurReportDocument.setPageOrientation(SOOPTPORTRAIT);
                        }
                        catch (com.sun.star.lang.IllegalArgumentException e)
                        {
                        }

                        CurUnoDialog.setControlProperty("imgOrientation", PropertyNames.PROPERTY_IMAGEURL, Desktop.getBitmapPath(m_xMSF) + "/portrait.gif");
                        Helper.setUnoPropertyValue(aOrientationImage, PropertyNames.PROPERTY_IMAGEURL, "private:graphicrepository/wizards/res/portrait_32.png");
                        break;

                    default:
                        break;
                }
            }
            catch (RuntimeException e)
            {
                e.printStackTrace();
            }
            Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        }

        public void disposing(EventObject eventObject)
        {
        }
    }

    class ActionListenerImpl implements com.sun.star.awt.XActionListener
    {

        public void disposing(EventObject eventObject)
        {
        }

        public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent)
        {
            try
            {
                Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
                boolean blandscape = (((Short) CurUnoDialog.getControlProperty("optLandscape", PropertyNames.PROPERTY_STATE)).shortValue() == 1);
                CurReportDocument.setPageOrientation((blandscape) ? SOOPTLANDSCAPE : SOOPTPORTRAIT);
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.err);
            }
            Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        }
    }

    public void drawConstants()
    {
            constRangeList = searchFillInItems(1);

            XTextRange item = null;

            for (int i = 0; i < constRangeList.size(); i++)
            {
                item = constRangeList.get(i);
                String text = item.getString().trim().toLowerCase();
                if (text.equals("#titleconst#"))
                {
                    teTitleconst = new PlaceholderTextElement(item, CurUnoDialog.m_oResource.getResText("RID_REPORT_86"), PropertyNames.EMPTY_STRING, m_xMSF);
                    trTitleconst = item;
                    constRangeList.remove(i--);
                    writeTitle(teTitleconst, trTitleconst, CurUnoDialog.m_oResource.getResText("RID_REPORT_86"));
                }
                else if (text.equals("#authorconst#"))
                {
                    teAuthorconst = new PlaceholderTextElement(item, CurUnoDialog.m_oResource.getResText("RID_REPORT_87"), PropertyNames.EMPTY_STRING, m_xMSF);
                    trAuthorconst = item;
                    constRangeList.remove(i--);
                    writeTitle(teAuthorconst, trAuthorconst, CurUnoDialog.m_oResource.getResText("RID_REPORT_87"));
                }
                else if (text.equals("#dateconst#"))
                {
                    teDateconst = new PlaceholderTextElement(item, CurUnoDialog.m_oResource.getResText("RID_REPORT_88"), PropertyNames.EMPTY_STRING, m_xMSF);
                    trDateconst = item;
                    constRangeList.remove(i--);
                    writeTitle(teDateconst, trDateconst, CurUnoDialog.m_oResource.getResText("RID_REPORT_88"));
                }
                else if (text.equals("#pageconst#"))
                {
                    tePageconst = new PlaceholderTextElement(item, CurUnoDialog.m_oResource.getResText("RID_REPORT_93"), PropertyNames.EMPTY_STRING, m_xMSF);
                    trPageconst = item;
                    constRangeList.remove(i--);
                    writeTitle(tePageconst, trPageconst, CurUnoDialog.m_oResource.getResText("RID_REPORT_93"));
                }
            }
    }

    private void clearConstants()
    {
        constRangeList.clear();
        trTitleconst = null;
        teTitleconst = null;
        trAuthorconst = null;
        teAuthorconst = null;
        trDateconst = null;
        teDateconst = null;
        trPageconst = null;
        tePageconst = null;
    }

    private void writeTitle(TextElement te, XTextRange tr, String text)
    {
        te.setText(text == null ? PropertyNames.EMPTY_STRING : text);
        te.write(tr);
    }

    private List<XTextRange> searchFillInItems(int type)
    {
      try
      {
            XSearchable xSearchable = UnoRuntime.queryInterface(XSearchable.class, CurReportDocument.getComponent());
            XSearchDescriptor sd = xSearchable.createSearchDescriptor();

            if(type == 0)
            {
              sd.setSearchString("<[^>]+>");
            }
            else if(type == 1)
            {
              sd.setSearchString("#[^#]+#");
            }
            sd.setPropertyValue("SearchRegularExpression", Boolean.TRUE);
            sd.setPropertyValue("SearchWords", Boolean.TRUE);

            XIndexAccess ia = xSearchable.findAll(sd);
            List<XTextRange> l = new ArrayList<XTextRange>(ia.getCount());
            for (int i = 0; i < ia.getCount(); i++)
            {
                try
                {
                    l.add(UnoRuntime.queryInterface(XTextRange.class, ia.getByIndex(i)));
                }
                catch (Exception ex)
                {
                    System.err.println("Nonfatal Error in finding fillins.");
                }
            }
            return l;
        }
        catch (Exception ex)
        {
            throw new IllegalArgumentException("Fatal Error: Loading template failed: searching fillins failed", ex);
        }
    }
}
