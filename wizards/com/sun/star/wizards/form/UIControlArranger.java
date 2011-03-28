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
package com.sun.star.wizards.form;

import com.sun.star.awt.ItemEvent;

import com.sun.star.awt.Size;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XRadioButton;
import com.sun.star.lang.EventObject;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.NoValidPathException;
import com.sun.star.wizards.document.Control;
// import com.sun.star.wizards.ui.ImageList;
import com.sun.star.wizards.ui.ButtonList;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;

import javax.swing.DefaultListModel;
import javax.swing.ListModel;
import javax.swing.event.ListDataEvent;
import javax.swing.event.ListDataListener;

public class UIControlArranger
{

    private FormWizard CurUnoDialog;
    private FormDocument curFormDocument;
    private short curtabindex;
    private XRadioButton optAlignLeft;
    private XRadioButton optAlignRight;
    private XControl flnLabelPlacement;
    private String[] HelpTexts = new String[4];
    // private String[] sArrangementHeader = new String[2];
    // private ArrangeImageList[] m_aArrangeList = new ArrangeImageList[2];
    private ArrangeButtonList[] m_aArrangeList = new ArrangeButtonList[2];
    private Integer IControlStep;
    private final int SOBASEIMAGEYPOSITION = 66;
    private final int SOIMAGELISTHEIGHT = 60;
    private final String SOALIGNMETHOD = "alignLabelControls";

    public UIControlArranger(FormWizard _CurUnoDialog, FormDocument _curFormDocument) throws NoValidPathException
    {
        this.CurUnoDialog = _CurUnoDialog;
        this.curFormDocument = _curFormDocument;
        curtabindex = (short) (FormWizard.SOCONTROL_PAGE * 100);
        IControlStep = new Integer(FormWizard.SOCONTROL_PAGE);
        String sLabelPlacment = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 32);
        String sAlignLeft = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 33);
        String sAlignRight = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 34);

        // Label "Label Placement" -----------------
        flnLabelPlacement = CurUnoDialog.insertFixedLine("lnLabelPlacement",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], sLabelPlacment, 97, 25, IControlStep, new Short(curtabindex++), 207
                });
        // Radio Button "Align Left"
        optAlignLeft = CurUnoDialog.insertRadioButton("optAlignLeft", SOALIGNMETHOD, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[10], "HID:WIZARDS_HID_DLGFORM_CMDALIGNLEFT", sAlignLeft, 107, 38, new Short((short) 1), IControlStep, new Short(curtabindex++), 171
                });
        // Radio Button "Align Right"
        optAlignRight = CurUnoDialog.insertRadioButton("optAlignRight", SOALIGNMETHOD, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[10], "HID:WIZARDS_HID_DLGFORM_CMDALIGNRIGHT", sAlignRight, Boolean.TRUE, 107, 50, IControlStep, new Short(curtabindex++), 171
                });


        HelpTexts[0] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 36); // "Columnar - Labels Left"
        HelpTexts[1] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 37); // "Columnar - Labels of Top"
        HelpTexts[2] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 40); // "As Data Sheet"
        HelpTexts[3] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 39); // In Blocks - Labels Above"

//        flnLabelPlacement = CurUnoDialog.insertFixedLine("lnArrangementHeader1",
//                new String[]
//                {
//                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
//                },
//                new Object[]
//                {
//                    UIConsts.INTEGERS[8], sArrangementHeader[0], 97, 60, IControlStep, new Short(curtabindex++), 207
//                });
//
//        boolean bEnabled = true;
//        int nBtnWidth = 16;
//        int nXPos = 97;
//        int nYPos = 70;
//        CurUnoDialog.insertButton("btnLayout1", "btnLayout1",
//                new String[]
//                {
//                    PropertyNames.PROPERTY_ENABLED,
//                    PropertyNames.PROPERTY_HEIGHT,
//                    PropertyNames.PROPERTY_HELPURL,
//                    PropertyNames.PROPERTY_LABEL,
//                    PropertyNames.PROPERTY_POSITION_X,
//                    PropertyNames.PROPERTY_POSITION_Y,
//                    PropertyNames.PROPERTY_STEP,
//                    PropertyNames.PROPERTY_TABINDEX,
//                    PropertyNames.PROPERTY_WIDTH
//                },
//                new Object[]
//                {
//                    Boolean.valueOf(bEnabled),
//                    14,
///* TODO: WRONG!*/   "HID:WIZARDS_HID_DLGFORM_CMDALIGNRIGHT",
//                    "1",
//                    new Integer(nXPos + nBtnWidth),
//                    new Integer(nYPos),
//                    IControlStep,
//                    new Short(curtabindex++),
//                    16
//                });

        DefaultListModel imageModel = new DefaultListModel();
        for (int i = 0; i < HelpTexts.length; i++)
        {
            imageModel.addElement(new Integer(i));
        }
         String sMainArrangementHeader = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 41); // "Arrangement of the main form"
         m_aArrangeList[0] = new ArrangeButtonList(0, imageModel, sMainArrangementHeader);

         String sSubArrangementHeader = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 42); // "Arrangement of the sub form"
         m_aArrangeList[1] = new ArrangeButtonList(1, imageModel, sSubArrangementHeader);
        enableAlignControlGroup(false);
    }

    public int getSelectedArrangement(int _formindex)
    {
        return m_aArrangeList[_formindex].m_aButtonList.getSelected() + 1;
    }

    private class LayoutRenderer implements ButtonList.IImageRenderer
    {
        int Index = 1;

        public Object[] getImageUrls(Object listitem)
        {

            int ResId = UIConsts.RID_IMG_FORM + (2 * ((Integer) listitem).intValue());
            Index++;
            return new Integer[]
                    {
                        new Integer(ResId), new Integer(ResId + 1)
                    };
//          String s = CurUnoDialog.getWizardImageUrl(ResId, ResId + 1); //((Integer)listitem).intValue(), )sBitmapPath + "/Arrange_" + (((Integer)listitem).intValue() + 1) + ".gif";
//          return s;
        }

        public String render(Object listItem)
        {
            if (listItem == null)
            {
                return "";
            }
            return HelpTexts[((Integer) listItem).intValue()];

        }
    }

    public void enableSubFormImageList(boolean _bdoEnable)
    {
        m_aArrangeList[1].m_aButtonList.setenabled(_bdoEnable);
        CurUnoDialog.setControlProperty("lnLabelPlacment_2", PropertyNames.PROPERTY_ENABLED, new Boolean(_bdoEnable));
    }

    public short getAlignValue()
    {
        return optAlignLeft.getState() ? (short)0 : (short)2;
    }

    public void alignLabelControls()
    {
        try
        {
            short iAlignValue = getAlignValue();
            for (int m = 0; m < curFormDocument.oControlForms.size(); m++)
            {
                FormDocument.ControlForm curControlForm = (FormDocument.ControlForm) curFormDocument.oControlForms.get(m);
                if (curControlForm.getArrangemode() == FormWizard.COLUMNAR_LEFT)
                {
                    Control[] LabelControls = curControlForm.getLabelControls();
                    for (int n = 0; n < LabelControls.length; n++)
                    {
                        LabelControls[n].xPropertySet.setPropertyValue(PropertyNames.PROPERTY_ALIGN, new Short(iAlignValue));
                    }
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    private void enableAlignControlGroup(boolean _bEnableAlignControlGroup)
    {
        Helper.setUnoPropertyValue(UnoDialog.getModel(flnLabelPlacement), PropertyNames.PROPERTY_ENABLED, new Boolean(_bEnableAlignControlGroup));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optAlignLeft), PropertyNames.PROPERTY_ENABLED, new Boolean(_bEnableAlignControlGroup));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optAlignRight), PropertyNames.PROPERTY_ENABLED, new Boolean(_bEnableAlignControlGroup));
    }

//    private class ArrangeImageList implements XItemListener
//    {
//
//        private int formindex;
//        private ImageList m_aButtonList = null; // new ImageList();
//
//        public ArrangeImageList(int _formindex, ListModel model, String _sArrangementHeader)
//        {
//            formindex = _formindex;
//            Integer YPos = new Integer(SOBASEIMAGEYPOSITION + _formindex * SOIMAGELISTHEIGHT);
//            // Label ArrangementHeader ----------------------
//            CurUnoDialog.insertFixedLine("lnLabelPlacment_" + (_formindex + 1),
//                    new String[]
//                    {
//                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
//                    },
//                    new Object[]
//                    {
//                        UIConsts.INTEGERS[8], _sArrangementHeader, 97, YPos, IControlStep, new Short(curtabindex++), 207
//                    });
//
//            int nypos = SOBASEIMAGEYPOSITION + 12 + _formindex * SOIMAGELISTHEIGHT;
//            m_aButtonList = new ImageList();
//            m_aButtonList.setPos(new Size(107, nypos));
//            m_aButtonList.setImageSize(new Size(26, 26));
//            m_aButtonList.setCols(4);
//            m_aButtonList.setRows(1);
//            m_aButtonList.m_aControlName = "ImageList_" + formindex;
//            m_aButtonList.setStep(new Short((short) FormWizard.SOCONTROL_PAGE));
//            m_aButtonList.setShowButtons(false);
//            m_aButtonList.setRenderer(new LayoutRenderer());
//            m_aButtonList.setSelectionGap(new Size(2, 2));
//            m_aButtonList.setGap(new Size(3, 3));
//            m_aButtonList.scaleImages = Boolean.FALSE;
//            m_aButtonList.tabIndex = (int) curtabindex++;
//            m_aButtonList.helpURL = 34453 + (formindex * 4);
//
//            m_aButtonList.setListModel(model);
//            m_aButtonList.create(CurUnoDialog);
//            m_aButtonList.setSelected(FormWizard.SOGRID - 1);
//            m_aButtonList.addItemListener(this);
//        }
//
//        public void setToLeftAlign()
//        {
//            Helper.setUnoPropertyValue(UnoDialog.getModel(optAlignLeft), PropertyNames.PROPERTY_STATE, new Short((short) 1));
//        }
//
//        /* (non-Javadoc)
//         * @see javax.swing.ListModel#addListDataListener(javax.swing.event.ListDataListener)
//         */
//        public void addListDataListener(ListDataListener arg0)
//        {
//            // TODO Auto-generated method stub
//        }
//
//        public void itemStateChanged(ItemEvent arg0)
//        {
//            try
//            {
//                if (m_aArrangeList[formindex].m_aButtonList.isenabled())
//                {
//                    boolean bEnableAlignControlGroup;
//                    if (curFormDocument.oControlForms.size() == 2)
//                    {
//                        final int nSelected0 = (m_aArrangeList[0].m_aButtonList.getSelected() + 1);
//                        final int nSelected1 = (m_aArrangeList[1].m_aButtonList.getSelected() + 1);
//
//                        bEnableAlignControlGroup = ((nSelected0 == FormWizard.SOCOLUMNARLEFT) ||
//                                                    (nSelected1 == FormWizard.SOCOLUMNARLEFT));
//                    }
//                    else
//                    {
//                        final int nSelected0 = (m_aArrangeList[0].m_aButtonList.getSelected() + 1);
//                        bEnableAlignControlGroup = (nSelected0 == FormWizard.SOCOLUMNARLEFT);
//                    }
//                    enableAlignControlGroup(bEnableAlignControlGroup);
//                    final Short nBorderType = CurUnoDialog.getBorderType();
//                    final int nSelected = m_aButtonList.getSelected() + 1;
//                    ((FormDocument.ControlForm) curFormDocument.oControlForms.get(formindex)).initialize(nSelected, nBorderType);
//                }
//            }
//            catch (RuntimeException e)
//            {
//                e.printStackTrace();
//            }
//        }
//
//        /* (non-Javadoc)
//         * @see javax.swing.event.ListDataListener#contentsChanged(javax.swing.event.ListDataEvent)
//         */
//        public void contentsChanged(ListDataEvent arg0)
//        {
//            // TODO Auto-generated method stub
//        }
//
//        /* (non-Javadoc)
//         * @see javax.swing.event.ListDataListener#intervalAdded(javax.swing.event.ListDataEvent)
//         */
//        public void intervalAdded(ListDataEvent arg0)
//        {
//            // TODO Auto-generated method stub
//        }
//
//        /* (non-Javadoc)
//         * @see javax.swing.event.ListDataListener#intervalRemoved(javax.swing.event.ListDataEvent)
//         */
//        public void intervalRemoved(ListDataEvent arg0)
//        {
//            // TODO Auto-generated method stub
//        }
//
//        /* (non-Javadoc)
//         * @see com.sun.star.lang.XEventListener#disposing(com.sun.star.lang.EventObject)
//         */
//        public void disposing(EventObject arg0)
//        {
//            // TODO Auto-generated method stub
//        }
//    }

    private class ArrangeButtonList implements XItemListener
    {

        private int formindex;
        private ButtonList m_aButtonList = null; // new ImageList();

        public ArrangeButtonList(int _formindex, ListModel model, String _sArrangementHeader)
        {
            formindex = _formindex;
            Integer YPos = new Integer(SOBASEIMAGEYPOSITION + _formindex * SOIMAGELISTHEIGHT);
            // Label ArrangementHeader ----------------------
            CurUnoDialog.insertFixedLine("lnLabelPlacment_" + (_formindex + 1),
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT,
                        PropertyNames.PROPERTY_LABEL,
                        PropertyNames.PROPERTY_POSITION_X,
                        PropertyNames.PROPERTY_POSITION_Y,
                        PropertyNames.PROPERTY_STEP,
                        PropertyNames.PROPERTY_TABINDEX,
                        PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[8],
                        _sArrangementHeader,
                        97,
                        YPos,
                        IControlStep,
                        new Short(curtabindex++),
                        207
                    });

            int nypos = SOBASEIMAGEYPOSITION + 12 - 5 + _formindex * SOIMAGELISTHEIGHT;
            m_aButtonList = new ButtonList();
            m_aButtonList.setPos(new Size(107, nypos));
            m_aButtonList.setButtonSize(new Size(26 + 6, 26 + 5));
            m_aButtonList.setCols(4);
            m_aButtonList.setRows(1);
            m_aButtonList.setName( "ButtonList_" + formindex );
            m_aButtonList.setStep(Short.valueOf((short) FormWizard.SOCONTROL_PAGE));
            m_aButtonList.setShowButtons(false);      // shows a button line at ''wrong'' position like |<| 1..4/4 |>|
            m_aButtonList.setRenderer(new LayoutRenderer());
            m_aButtonList.setSelectionGap(new Size(2, 2));
            m_aButtonList.setGap(new Size(3, 3));
//            m_aButtonList.scaleImages = Boolean.FALSE;
            m_aButtonList.tabIndex = (int) curtabindex++;
            m_aButtonList.helpURL = 34453 + (formindex * 4);

            m_aButtonList.setListModel(model);
            m_aButtonList.create(CurUnoDialog);
            m_aButtonList.setSelected(FormWizard.AS_GRID - 1);
            m_aButtonList.addItemListener(this);
        }

        public void setToLeftAlign()
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(optAlignLeft), PropertyNames.PROPERTY_STATE, new Short((short) 1));
        }

        /* (non-Javadoc)
         * @see javax.swing.ListModel#addListDataListener(javax.swing.event.ListDataListener)
         */
        public void addListDataListener(ListDataListener arg0)
        {
            // TODO Auto-generated method stub
        }

        public void itemStateChanged(ItemEvent arg0)
        {
            try
            {
                if (m_aArrangeList[formindex].m_aButtonList.isenabled())
                {
                    boolean bEnableAlignControlGroup;
                    if (curFormDocument.oControlForms.size() == 2)
                    {
                        final int nSelected0 = (m_aArrangeList[0].m_aButtonList.getSelected() + 1);
                        final int nSelected1 = (m_aArrangeList[1].m_aButtonList.getSelected() + 1);

                        bEnableAlignControlGroup = ((nSelected0 == FormWizard.COLUMNAR_LEFT) ||
                                                    (nSelected1 == FormWizard.COLUMNAR_LEFT));
                    }
                    else
                    {
                        final int nSelected0 = (m_aArrangeList[0].m_aButtonList.getSelected() + 1);
                        bEnableAlignControlGroup = (nSelected0 == FormWizard.COLUMNAR_LEFT);
                    }
                    enableAlignControlGroup(bEnableAlignControlGroup);
                    final Short nBorderType = CurUnoDialog.getBorderType();
                    final int nSelected = m_aButtonList.getSelected() + 1;
                    ((FormDocument.ControlForm) curFormDocument.oControlForms.get(formindex)).initialize(nSelected, nBorderType);
                }
            }
            catch (RuntimeException e)
            {
                e.printStackTrace();
            }
        }

        /* (non-Javadoc)
         * @see javax.swing.event.ListDataListener#contentsChanged(javax.swing.event.ListDataEvent)
         */
        public void contentsChanged(ListDataEvent arg0)
        {
            // TODO Auto-generated method stub
        }

        /* (non-Javadoc)
         * @see javax.swing.event.ListDataListener#intervalAdded(javax.swing.event.ListDataEvent)
         */
        public void intervalAdded(ListDataEvent arg0)
        {
            // TODO Auto-generated method stub
        }

        /* (non-Javadoc)
         * @see javax.swing.event.ListDataListener#intervalRemoved(javax.swing.event.ListDataEvent)
         */
        public void intervalRemoved(ListDataEvent arg0)
        {
            // TODO Auto-generated method stub
        }

        /* (non-Javadoc)
         * @see com.sun.star.lang.XEventListener#disposing(com.sun.star.lang.EventObject)
         */
        public void disposing(EventObject arg0)
        {
            // TODO Auto-generated method stub
        }
    }
}
