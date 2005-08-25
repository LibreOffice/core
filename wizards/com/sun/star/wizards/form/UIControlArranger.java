/*************************************************************************
 *
 *  $RCSfile: UIControlArranger.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $  $Date: 2005-08-25 13:13:15 $
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
 *  Contributor(s): Berend Cornelius
 *
 */
package com.sun.star.wizards.form;
import com.sun.star.awt.ItemEvent;

import com.sun.star.awt.Size;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XItemEventBroadcaster;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XRadioButton;
import com.sun.star.lang.EventObject;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.NoValidPathException;
import com.sun.star.wizards.document.Control;
import com.sun.star.wizards.ui.ImageList;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.WizardDialog;

import javax.swing.DefaultListModel;
import javax.swing.ListModel;
import javax.swing.event.ListDataEvent;
import javax.swing.event.ListDataListener;



public class UIControlArranger {
    WizardDialog CurUnoDialog;
    FormDocument curFormDocument;
    short curtabindex;
    XRadioButton optAlignLeft;
    XRadioButton optAlignRight;
    XControl flnLabelPlacement;
    int Index = 1;
    String[] HelpTexts = new String[4];
    String[] sArrangementHeader = new String[2];
    ArrangeImageList[] oImageList = new ArrangeImageList[2];
    Integer IControlStep;
    final int SOBASEIMAGEYPOSITION = 66;
    final int SOIMAGELISTHEIGHT = 60;
    final String SOALIGNMETHOD = "alignLabelControls";


    public UIControlArranger(WizardDialog _CurUnoDialog, FormDocument _curFormDocument) throws NoValidPathException {
        this.CurUnoDialog = _CurUnoDialog;
        this.curFormDocument = _curFormDocument;
        curtabindex = (short) (FormWizard.SOCONTROLPAGE * 100);
        IControlStep = new Integer(FormWizard.SOCONTROLPAGE);
        String sLabelPlacment = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 32);
        String sAlignLeft = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 33);
        String sAlignRight = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 34);
        sArrangementHeader[0] = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 41);
        sArrangementHeader[1] = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 42);
        HelpTexts[0] = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 36);
        HelpTexts[1] = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 37);
        HelpTexts[2] = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 40);
        HelpTexts[3] = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 39);

        flnLabelPlacement = CurUnoDialog.insertFixedLine("lnLabelPlacement",
          new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
          new Object[] {UIConsts.INTEGERS[8],sLabelPlacment, new Integer(97), new Integer(25), IControlStep, new Short(curtabindex++), new Integer(207)}
        );
        optAlignLeft = CurUnoDialog.insertRadioButton("optAlignLeft", SOALIGNMETHOD, this,
          new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
          new Object[] {UIConsts.INTEGERS[10],"HID:34451",sAlignLeft, new Integer(107), new Integer(38), new Short((short)1), IControlStep, new Short(curtabindex++), new Integer(171)}
        );
        optAlignRight = CurUnoDialog.insertRadioButton("optAlignRight", SOALIGNMETHOD, this,
          new String[] {"Height", "HelpURL", "Label", "MultiLine", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
          new Object[] {UIConsts.INTEGERS[10],"HID:34452",sAlignRight, Boolean.TRUE, new Integer(107), new Integer(50), IControlStep, new Short(curtabindex++), new Integer(171)}
        );

        DefaultListModel imageModel = new DefaultListModel();
        for (int i = 0; i< HelpTexts.length; i++)
            imageModel.addElement(new Integer(i));

        oImageList[0] = new ArrangeImageList(0,imageModel);
        oImageList[1] = new ArrangeImageList(1,imageModel);
        enableAlignControlGroup(false);
    }


    public int getSelectedArrangement(int _formindex){
        return oImageList[_formindex].ilLayouts.getSelected()+1;
    }


    private class LayoutRenderer implements ImageList.ImageRenderer {
        public Object[] getImageUrls(Object listitem) {

            int ResId = UIConsts.RID_IMG_FORM + (2 * ((Integer)listitem).intValue());
            Index++;
            return new Integer[]{new Integer(ResId), new Integer(ResId + 1)};
//          String s = CurUnoDialog.getWizardImageUrl(ResId, ResId + 1); //((Integer)listitem).intValue(), )sBitmapPath + "/Arrange_" + (((Integer)listitem).intValue() + 1) + ".gif";
//          return s;
        }

        public String render(Object listItem) {
            if (listItem == null)
                return "";
            return HelpTexts[((Integer)listItem).intValue()];

        }
    }


    public void enableSubFormImageList(boolean _bdoEnable){
        oImageList[1].ilLayouts.setenabled(_bdoEnable);
        CurUnoDialog.setControlProperty("lnLabelPlacment_2", "Enabled", new Boolean(_bdoEnable));
    }


    public short getAlignValue(){
        Short IAlignValue = null;
        if (optAlignLeft.getState())
            IAlignValue = new Short((short) 0);
        else
            IAlignValue =  new Short((short) 2);
        return IAlignValue.shortValue();
    }


    public void alignLabelControls(){
    try {
        short iAlignValue = getAlignValue();
        for (int m = 0; m < curFormDocument.oControlForms.size(); m++){
            FormDocument.ControlForm curControlForm = (FormDocument.ControlForm) curFormDocument.oControlForms.get(m);
            if (curControlForm.getArrangemode() == FormWizard.SOCOLUMNARLEFT){
                Control[] LabelControls = curControlForm.getLabelControls();
                for (int n = 0; n < LabelControls.length; n++){
                    LabelControls[n].xPropertySet.setPropertyValue("Align", new Short(iAlignValue));
                }
            }
        }
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    private void enableAlignControlGroup(boolean _bEnableAlignControlGroup){
        Helper.setUnoPropertyValue(UnoDialog.getModel(flnLabelPlacement), "Enabled", new Boolean(_bEnableAlignControlGroup));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optAlignLeft), "Enabled", new Boolean(_bEnableAlignControlGroup));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optAlignRight), "Enabled", new Boolean(_bEnableAlignControlGroup));
    }


    private class ArrangeImageList implements XItemListener{
        int formindex;
        ImageList ilLayouts = new ImageList();


        public ArrangeImageList(int _formindex, ListModel model){
            formindex = _formindex;
            Integer YPos = new Integer(SOBASEIMAGEYPOSITION + _formindex * SOIMAGELISTHEIGHT);
            CurUnoDialog.insertFixedLine("lnLabelPlacment_" + (_formindex +1),
              new String[] {"Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
              new Object[] {UIConsts.INTEGERS[8], sArrangementHeader[_formindex], new Integer(97), YPos, IControlStep, new Short(curtabindex++), new Integer(207)}
            );

            int nypos = SOBASEIMAGEYPOSITION + 12+ _formindex * SOIMAGELISTHEIGHT;
            ilLayouts = new ImageList();
            ilLayouts.setPos(new Size( 107,nypos));
            ilLayouts.setImageSize(new Size(26,26));
            ilLayouts.setCols(4);
            ilLayouts.setRows(1);
            ilLayouts.name = "ImageList_" + formindex;
            ilLayouts.setStep(new Short((short) FormWizard.SOCONTROLPAGE));
            ilLayouts.setShowButtons(false);
            ilLayouts.setRenderer(new LayoutRenderer());
            ilLayouts.setSelectionGap(new Size(2,2));
            ilLayouts.setGap(new Size(3,3));
            ilLayouts.scaleImages = Boolean.FALSE;
            ilLayouts.tabIndex = (int) curtabindex++;
            ilLayouts.helpURL = 34453 + (formindex * 4);

            ilLayouts.setListModel(model);
            ilLayouts.create(CurUnoDialog);
            ilLayouts.setSelected(FormWizard.SOGRID-1);
            ilLayouts.addItemListener(this);
        }


        public void setToLeftAlign(){
            Helper.setUnoPropertyValue(UnoDialog.getModel(optAlignLeft), "State", new Short((short) 1));
        }

        /* (non-Javadoc)
         * @see javax.swing.ListModel#addListDataListener(javax.swing.event.ListDataListener)
         */
        public void addListDataListener(ListDataListener arg0) {
            // TODO Auto-generated method stub

        }


        public void itemStateChanged(ItemEvent arg0) {
        try {
            if (oImageList[formindex].ilLayouts.isenabled()){
                boolean bEnableAlignControlGroup;
                if (curFormDocument.oControlForms.size() == 2)
                    bEnableAlignControlGroup = (((oImageList[0].ilLayouts.getSelected()+1) == FormWizard.SOCOLUMNARLEFT) || ((oImageList[1].ilLayouts.getSelected()+1) == FormWizard.SOCOLUMNARLEFT));
                else
                    bEnableAlignControlGroup = ((oImageList[0].ilLayouts.getSelected()+1) == FormWizard.SOCOLUMNARLEFT);
                enableAlignControlGroup(bEnableAlignControlGroup);
                ((FormDocument.ControlForm) curFormDocument.oControlForms.get(formindex)).initialize(ilLayouts.getSelected() + 1);
            }
        } catch (RuntimeException e) {
            e.printStackTrace();
        }}

        /* (non-Javadoc)
         * @see javax.swing.event.ListDataListener#contentsChanged(javax.swing.event.ListDataEvent)
         */
        public void contentsChanged(ListDataEvent arg0) {
            // TODO Auto-generated method stub

        }

        /* (non-Javadoc)
         * @see javax.swing.event.ListDataListener#intervalAdded(javax.swing.event.ListDataEvent)
         */
        public void intervalAdded(ListDataEvent arg0) {
            // TODO Auto-generated method stub

        }

        /* (non-Javadoc)
         * @see javax.swing.event.ListDataListener#intervalRemoved(javax.swing.event.ListDataEvent)
         */
        public void intervalRemoved(ListDataEvent arg0) {
            // TODO Auto-generated method stub

        }

        /* (non-Javadoc)
         * @see com.sun.star.lang.XEventListener#disposing(com.sun.star.lang.EventObject)
         */
        public void disposing(EventObject arg0) {
            // TODO Auto-generated method stub

        }
    }

}
