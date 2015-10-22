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
package com.sun.star.wizards.form;

import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.Size;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XRadioButton;
import com.sun.star.lang.EventObject;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.document.Control;
import com.sun.star.wizards.ui.ButtonList;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.event.XItemListenerAdapter;

import javax.swing.DefaultListModel;
import javax.swing.ListModel;

public class UIControlArranger
{

    private final FormWizard CurUnoDialog;
    private final FormDocument curFormDocument;
    private short curtabindex;
    private final XRadioButton optAlignLeft;
    private final XRadioButton optAlignRight;
    private final XControl flnLabelPlacement;
    private final String[] HelpTexts = new String[4];
    private final ArrangeButtonList[] m_aArrangeList = new ArrangeButtonList[2];
    private final Integer IControlStep;
    private static final int SOBASEIMAGEYPOSITION = 66;
    private static final int SOIMAGELISTHEIGHT = 60;

    public UIControlArranger(FormWizard _CurUnoDialog, FormDocument _curFormDocument)
    {
        this.CurUnoDialog = _CurUnoDialog;
        this.curFormDocument = _curFormDocument;
        curtabindex = (short) (FormWizard.SOCONTROL_PAGE * 100);
        IControlStep = Integer.valueOf(FormWizard.SOCONTROL_PAGE);
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
                    UIConsts.INTEGERS[8], sLabelPlacment, 97, 25, IControlStep, Short.valueOf(curtabindex++), 207
                });
        // Radio Button "Align Left"
        optAlignLeft = CurUnoDialog.insertRadioButton("optAlignLeft", new XItemListenerAdapter() {
                    @Override
                    public void itemStateChanged(ItemEvent event) {
                        alignLabelControls();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[10], "HID:WIZARDS_HID_DLGFORM_CMDALIGNLEFT", sAlignLeft, 107, 38, Short.valueOf((short) 1), IControlStep, Short.valueOf(curtabindex++), 171
                });
        // Radio Button "Align Right"
        optAlignRight = CurUnoDialog.insertRadioButton("optAlignRight", new XItemListenerAdapter() {
                    @Override
                    public void itemStateChanged(ItemEvent event) {
                        alignLabelControls();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[10], "HID:WIZARDS_HID_DLGFORM_CMDALIGNRIGHT", sAlignRight, Boolean.TRUE, 107, 50, IControlStep, Short.valueOf(curtabindex++), 171
                });


        HelpTexts[0] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 36); // "Columnar - Labels Left"
        HelpTexts[1] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 37); // "Columnar - Labels of Top"
        HelpTexts[2] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 40); // "As Data Sheet"
        HelpTexts[3] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 39); // In Blocks - Labels Above"

        DefaultListModel imageModel = new DefaultListModel();
        for (int i = 0; i < HelpTexts.length; i++)
        {
            imageModel.addElement(Integer.valueOf(i));
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
        public Object[] getImageUrls(Object listitem)
        {

            int ResId = UIConsts.RID_IMG_FORM + (2 * ((Integer) listitem).intValue());
            return new Integer[]
                    {
                    Integer.valueOf(ResId), Integer.valueOf(ResId + 1)
                    };
        }

        public String render(Object listItem)
        {
            if (listItem == null)
            {
                return PropertyNames.EMPTY_STRING;
            }
            return HelpTexts[((Integer) listItem).intValue()];

        }
    }

    public void enableSubFormImageList(boolean _bdoEnable)
    {
        m_aArrangeList[1].m_aButtonList.setenabled(_bdoEnable);
        CurUnoDialog.setControlProperty("lnLabelPlacment_2", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(_bdoEnable));
    }

    public short getAlignValue()
    {
        return optAlignLeft.getState() ? (short)0 : (short)2;
    }

    private void alignLabelControls()
    {
        try
        {
            short iAlignValue = getAlignValue();
            for (int m = 0; m < curFormDocument.oControlForms.size(); m++)
            {
                FormDocument.ControlForm curControlForm = curFormDocument.oControlForms.get(m);
                if (curControlForm.getArrangemode() == FormWizard.COLUMNAR_LEFT)
                {
                    Control[] LabelControls = curControlForm.getLabelControls();
                    if (LabelControls == null) {
                        continue;
                    }
                    for (int n = 0; n < LabelControls.length; n++)
                    {
                        LabelControls[n].xPropertySet.setPropertyValue(PropertyNames.PROPERTY_ALIGN, Short.valueOf(iAlignValue));
                    }
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    private void enableAlignControlGroup(boolean _bEnableAlignControlGroup)
    {
        Helper.setUnoPropertyValue(UnoDialog.getModel(flnLabelPlacement), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(_bEnableAlignControlGroup));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optAlignLeft), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(_bEnableAlignControlGroup));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optAlignRight), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(_bEnableAlignControlGroup));
    }

    private class ArrangeButtonList implements XItemListener
    {

        private final int formindex;
        private final ButtonList m_aButtonList;

        public ArrangeButtonList(int _formindex, ListModel model, String _sArrangementHeader)
        {
            formindex = _formindex;
            Integer YPos = Integer.valueOf(SOBASEIMAGEYPOSITION + _formindex * SOIMAGELISTHEIGHT);
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
                        Short.valueOf(curtabindex++),
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
            m_aButtonList.setGap(new Size(3, 3));
//            m_aButtonList.scaleImages = Boolean.FALSE;
            m_aButtonList.tabIndex = curtabindex++;
            m_aButtonList.helpURL = 34453 + (formindex * 4);

            m_aButtonList.setListModel(model);
            m_aButtonList.create(CurUnoDialog);
            m_aButtonList.setSelected(FormWizard.AS_GRID - 1);
            m_aButtonList.addItemListener(this);
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
                    curFormDocument.oControlForms.get(formindex).initialize(nSelected, nBorderType);
                }
            }
            catch (RuntimeException e)
            {
                e.printStackTrace();
            }
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
