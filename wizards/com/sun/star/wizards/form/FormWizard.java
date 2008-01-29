/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FormWizard.java,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-29 08:41:53 $
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

package com.sun.star.wizards.form;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.RelationController;
import com.sun.star.wizards.document.OfficeDocument;
import com.sun.star.wizards.ui.*;


public class FormWizard extends WizardDialog{

    CommandFieldSelection curDBCommandFieldSelection;
    FormConfiguration curFormConfiguration;
    CommandFieldSelection curSubFormFieldSelection;
    FormDocument curFormDocument;
    FieldLinker curFieldLinker;
    UIControlArranger curControlArranger;
    DataEntrySetter CurDataEntrySetter;
    StyleApplier curStyleApplier;
    Finalizer curFinalizer;
    static String slblFields;
    static String slblSelFields;

    String sShowBinaryFields = "";
    String sMsgWizardName = "";
    String serrFormNameexists = "";

    public static final int SONULLPAGE = 0;
    public static final int SOMAINPAGE = 1;
    public static final int SOSUBFORMPAGE = 2;
    public static final int SOSUBFORMFIELDSPAGE = 3;
    public static final int SOFIELDLINKERPAGE = 4;
    public static final int SOCONTROLPAGE = 5;
    public static final int SODATAPAGE = 6;
    public static final int SOSTYLEPAGE = 7;
    public static final int SOSTOREPAGE = 8;

    public static final int SOCOLUMNARLEFT = 1;
    public static final int SOCOLUMNARTOP = 2;
    public static final int SOGRID = 3;
    public static final int SOTOPJUSTIFIED = 4;

    String WizardHeaderText[] = new String[8];
    String slblTables;
    boolean bFormOpenMode;
    boolean bcreateForm = false;
    String  FormName;
    Short NBorderValue = null;

    public FormWizard(XMultiServiceFactory xMSF) {
        super(xMSF, 34400);
        super.addResourceHandler("FormWizard", "dbw");
        Helper.setUnoPropertyValues(xDialogModel,
            new String[] { "Height","Moveable","Name","PositionX","PositionY","Step","TabIndex","Title","Width"},
            new Object[] { new Integer(210),Boolean.TRUE, "DialogForm", new Integer(102),new Integer(41),new Integer(1), new Short((short)0), oResource.getResText(UIConsts.RID_FORM), new Integer(310)}  );
        drawNaviBar();
        if (getFormResources() == true)
            setRightPaneHeaders(oResource, UIConsts.RID_FORM + 90, 8);
    }


    protected void enterStep(int nOldStep, int nNewStep) {
    try {
        if ((nOldStep < SOCONTROLPAGE) && (nNewStep >= SOCONTROLPAGE)){
            curFormDocument.initialize(curDBCommandFieldSelection.isModified(), curFormConfiguration.hasSubForm(), curSubFormFieldSelection.isModified(), getBorderType());
            curDBCommandFieldSelection.setModified(false);
            curSubFormFieldSelection.setModified(false);
        }
        switch (nNewStep){
            case SOMAINPAGE:
                curDBCommandFieldSelection.setModified(false);
                break;
            case SOSUBFORMPAGE:
                RelationController oRelationController = new RelationController(curFormDocument.oMainFormDBMetaData,curDBCommandFieldSelection.getSelectedCommandName());
                curFormConfiguration.initialize(curSubFormFieldSelection, oRelationController);
                break;
            case SOSUBFORMFIELDSPAGE:
                String spreselectedTableName = curFormConfiguration.getreferencedTableName();
                curSubFormFieldSelection.preselectCommand(spreselectedTableName, (spreselectedTableName.length() > 0));
                break;
            case SOFIELDLINKERPAGE:
                curFieldLinker.initialize(curFormDocument.oMainFormDBMetaData.getFieldNames(), curFormDocument.oSubFormDBMetaData.getFieldNames(), curFormDocument.LinkFieldNames);
                break;
            case SOCONTROLPAGE:
                curControlArranger.enableSubFormImageList(curFormConfiguration.hasSubForm());
                break;
            case SODATAPAGE:
                break;
            case SOSTYLEPAGE:
                break;
            case SOSTOREPAGE:
                this.curFinalizer.initialize(this.curDBCommandFieldSelection.getSelectedCommandName(), curFormDocument);
                break;
             default:
             break;
        }
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}

    protected Short getBorderType(){
        return curStyleApplier.getBorderType();
    }

    protected void leaveStep(int nOldStep, int nNewStep){
         switch (nOldStep){
            case SOMAINPAGE:
//              curFormDocument.oMainFormDBMetaData.setFieldNames(curDBCommandFieldSelection.getSelectedFieldNames());
                curFormDocument.oMainFormDBMetaData.initializeFieldColumns(true, curDBCommandFieldSelection.getSelectedCommandName(), curDBCommandFieldSelection.getSelectedFieldNames());
                curFormDocument.LinkFieldNames = JavaTools.removeOutdatedFields(curFormDocument.LinkFieldNames, curFormDocument.oMainFormDBMetaData.getFieldNames(), 1);
                break;
            case SOSUBFORMPAGE:
                break;
            case SOSUBFORMFIELDSPAGE:
                curFormDocument.oSubFormDBMetaData.initializeFieldColumns(true, curSubFormFieldSelection.getSelectedCommandName(), curSubFormFieldSelection.getSelectedFieldNames());
                curFormDocument.LinkFieldNames = JavaTools.removeOutdatedFields(curFormDocument.LinkFieldNames, curFormDocument.oSubFormDBMetaData.getFieldNames(), 0);
                break;
            case SOFIELDLINKERPAGE:
                break;
            case SOCONTROLPAGE:
                break;
            case SODATAPAGE:
                break;
            case SOSTYLEPAGE:
                break;
            case SOSTOREPAGE:
                break;
             default:
             break;
          }
    }


    public static void main(String args[]) {

    String ConnectStr = "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";      //localhost  ;Lo-1.Germany.sun.com; 10.16.65.155
    PropertyValue[] curproperties = null;
    try {
        XMultiServiceFactory xLocMSF = com.sun.star.wizards.common.Desktop.connect(ConnectStr);
        FormWizard CurFormWizard = new FormWizard(xLocMSF);
        if(xLocMSF != null){
            System.out.println("Connected to "+ ConnectStr);
            curproperties = new PropertyValue[1];
//            curproperties[0] = Properties.createProperty("DatabaseLocation", "file:///C:/Documents and Settings/bc93774.EHAM02-DEV/My Documents/MyHSQL.odb");
            curproperties[0] = Properties.createProperty("DatabaseLocation", "file:///C:/Documents and Settings/bc93774.EHAM02-DEV/My Documents/MyHSQL.odb");
          curproperties[0] = Properties.createProperty("DataSourceName", "MyHSQLDatabase");  //file:///C:/Documents and Settings/bc93774.EHAM02-DEV/My Documents/myjapanesehsqldatasourceMyDocAssign.odb"); //MyDBase; Mydbwizard2DocAssign.odb; MyDBase.odb, Mydbwizard2DocAssign.odb ; Mydbwizard2DocAssign.odb; NewAccessDatabase, MyDocAssign baseLocation ); "DataSourceName", "db1");*//          /--/curproperties[0] = Properties.createProperty("DatabaseLocation", "file:///x:/bc/MyHSQL Database.odb"); //MyDBase; Mydbwizard2DocAssign.odb; MyDBase.odb, Mydbwizard2DocAssign.odb ; Mydbwizard2DocAssign.odb; NewAccessDatabase,  baseLocation ); "DataSourceName", "db1");*//          curproperties[0] = Properties.createProperty("DataSourceName", "Bibliography");*                        //          curproperties[0] = Properties.createProperty("DataSourceName", "Bibliography"); //Bibliography*         CurTableWizard.startTableWizard(xLocMSF, curproperties);
            CurFormWizard.startFormWizard(xLocMSF, curproperties);
        }
    }
    catch(Exception exception){
        exception.printStackTrace(System.out);
    }}


    public void buildSteps() throws NoValidPathException{
        curDBCommandFieldSelection = new CommandFieldSelection(this, curFormDocument.oMainFormDBMetaData, 92, slblFields, slblSelFields,  slblTables, true, 34411);
        curDBCommandFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
        curFormDocument.xProgressBar.setValue(20);

        insertLabel("lblBinaryHelpText",
                        new String[] {"Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "Width"},
                        new Object[] { new Integer(28), sShowBinaryFields, Boolean.TRUE, new Integer(95), new Integer(154), new Integer(SOMAINPAGE),new Integer(210)});

        curFormConfiguration = new FormConfiguration(this);
        curFormDocument.xProgressBar.setValue(30);

        curSubFormFieldSelection = new CommandFieldSelection(this, curFormDocument.oSubFormDBMetaData, SOSUBFORMFIELDSPAGE, 92, slblFields, slblSelFields,  slblTables, true, 34431);
        curSubFormFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
        insertLabel("lblSubFormBinaryHelpText",
            new String[] {"Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "Width"},
            new Object[] { new Integer(28), sShowBinaryFields, Boolean.TRUE, new Integer(95), new Integer(154), new Integer(SOSUBFORMFIELDSPAGE), new Integer(210)});

        curFormDocument.xProgressBar.setValue(40);

        curFieldLinker = new FieldLinker(this, SOFIELDLINKERPAGE,  95, 30, 210, 34441);
        curFormDocument.xProgressBar.setValue(50);

        curControlArranger = new UIControlArranger(this, curFormDocument);
        curFormDocument.addUIFormController(curControlArranger);
        curFormDocument.xProgressBar.setValue(60);

        CurDataEntrySetter = new DataEntrySetter(this);
        curFormDocument.xProgressBar.setValue(70);

        curStyleApplier = new StyleApplier(this, curFormDocument);
        curFormDocument.addStyleApplier(curStyleApplier);
        curFormDocument.xProgressBar.setValue(80);

        curFinalizer = new Finalizer(this);
        curFormDocument.xProgressBar.setValue(100);

        enableNavigationButtons(false, false, false);
        curFormDocument.xProgressBar.end();
    }



    public void finishWizard(){
        int ncurStep = getCurrentStep();
        if ((switchToStep(ncurStep, SOSTOREPAGE)) || (ncurStep == SOSTOREPAGE)){
            this.curFinalizer.initialize(curDBCommandFieldSelection.getSelectedCommandName(), this.curFormDocument);
            String sNewFormName = curFinalizer.getName();
            if (!curFormDocument.oMainFormDBMetaData.hasFormDocumentByName(sNewFormName)){
                bFormOpenMode = curFinalizer.getOpenMode();
                FormName = curFinalizer.getName();
                if (curFormDocument.finalizeForms(CurDataEntrySetter, curFieldLinker, curFormConfiguration)){;
                    if (curFinalizer.finish()){
                        bcreateForm = true;
                        xDialog.endExecute();
                    }
                }
            }
            else{
                String smessage = JavaTools.replaceSubString(serrFormNameexists, sNewFormName, "%FORMNAME");
                showMessageBox("WarningBox", com.sun.star.awt.VclWindowPeerAttribute.OK, smessage );
            }
        }
    }


    public void cancelWizard() {
        bcreateForm = false;
        xDialog.endExecute();
    }


    public void insertFormRelatedSteps(){
        addRoadmap();
        int i = 0;
        i = insertRoadmapItem(0, true, oResource.getResText(UIConsts.RID_FORM + 80), SOMAINPAGE);
        i = insertRoadmapItem(i, false, oResource.getResText(UIConsts.RID_FORM + 81), SOSUBFORMPAGE);
        i = insertRoadmapItem(i, false, oResource.getResText(UIConsts.RID_FORM + 82), SOSUBFORMFIELDSPAGE);
        i = insertRoadmapItem(i, false, oResource.getResText(UIConsts.RID_FORM + 83),  SOFIELDLINKERPAGE);
        i = insertRoadmapItem(i, false, oResource.getResText(UIConsts.RID_FORM + 84), SOCONTROLPAGE);
        i = insertRoadmapItem(i, false, oResource.getResText(UIConsts.RID_FORM + 85), SODATAPAGE);
        i = insertRoadmapItem(i, false, oResource.getResText(UIConsts.RID_FORM + 86), SOSTYLEPAGE);
        i = insertRoadmapItem(i, false, oResource.getResText(UIConsts.RID_FORM + 87), SOSTOREPAGE);
        setRoadmapInteractive(true);
        setRoadmapComplete(true);
        setCurrentRoadmapItemID((short) 1);
    }


    public XComponent[] startFormWizard(XMultiServiceFactory _xMSF, PropertyValue[] CurPropertyValue){
        XComponent[] ret = null;
        try{
            curFormDocument =  new FormDocument(xMSF, oResource);
            if (curFormDocument.oMainFormDBMetaData.getConnection(CurPropertyValue) ){
                curFormDocument.oSubFormDBMetaData.getConnection(new PropertyValue[]{Properties.createProperty("ActiveConnection", curFormDocument.oMainFormDBMetaData.DBConnection)});
                curFormDocument.xProgressBar.setValue(20);
                buildSteps();
                this.curDBCommandFieldSelection.preselectCommand(CurPropertyValue, false);
                createWindowPeer(curFormDocument.xWindowPeer);
                curFormDocument.oMainFormDBMetaData.setWindowPeer(xControl.getPeer());
        //      setAutoMnemonic("lblDialogHeader", false);
                insertFormRelatedSteps();
                short RetValue = executeDialog(curFormDocument.xFrame);
                xComponent.dispose();
                if (bcreateForm){
                    curFormDocument.oMainFormDBMetaData.addFormDocument(curFormDocument.xComponent);
                    ret = curFormDocument.oMainFormDBMetaData.openFormDocument( FormName, bFormOpenMode);
                }
            }
        }
        catch(java.lang.Exception jexception ){
            jexception.printStackTrace(System.out);
        }
        if ((!bcreateForm) && (curFormDocument != null))
            OfficeDocument.close(curFormDocument.xComponent);
        return ret;
    }



    public boolean getFormResources(){
        sMsgWizardName = super.oResource.getResText(UIConsts.RID_FORM);
        sShowBinaryFields = oResource.getResText(UIConsts.RID_FORM + 2);
        slblTables = oResource.getResText(UIConsts.RID_FORM + 6);
        slblFields = oResource.getResText(UIConsts.RID_FORM + 12);
        slblSelFields = oResource.getResText(UIConsts.RID_FORM + 1);
        serrFormNameexists = oResource.getResText(UIConsts.RID_FORM + 98);

        return true;
    }


    public class FieldSelectionListener implements com.sun.star.wizards.ui.XFieldSelectionListener{
         protected int ID;

         public int getID(){
             return ID;
         }

         public void setID(String sIncSuffix){
             ID = 1;
             if (sIncSuffix != null){
                if ((!sIncSuffix.equals("")) && (!sIncSuffix.equals("_"))) {
                     String sID = JavaTools.ArrayoutofString(sIncSuffix, "_")[1];
                     ID = Integer.parseInt(sID);
                     int a = 0;
                 }
             }
         }

         public void shiftFromLeftToRight(String[] SelItems, String[] NewItems) {
             if (ID == 1){
                toggleMainFormSteps();
             }
             else{
                toggleSubFormSteps();
             }
         }


         public void shiftFromRightToLeft(String[] SelItems, String[] NewItems ) {
             // TODO When the ListFieldbox is refilled only fields of the current Command may be merged into the Listbox
             if (ID == 1){
                toggleMainFormSteps();
             }
             else{
                toggleSubFormSteps();
             }
         }

         public void moveItemDown(String item){
         }

         public void moveItemUp(String item){
         }

         private boolean toggleSubFormSteps(){
            curSubFormFieldSelection.setModified(true);
            boolean benable = curSubFormFieldSelection.getSelectedFieldNames().length > 0;
            enablefromStep(SOFIELDLINKERPAGE, benable);
            if (benable)
                curFieldLinker.enable(!curFormConfiguration.areexistingRelationsdefined());
            return benable;
         }

        private void toggleMainFormSteps(){
            curDBCommandFieldSelection.setModified(true);
            boolean benable = curDBCommandFieldSelection.getSelectedFieldNames().length > 0;
            enablefromStep(SOSUBFORMPAGE, benable);
            setControlProperty("btnWizardNext", "Enabled", new Boolean(benable));
            if (benable){
                if (curFormConfiguration.hasSubForm())
                    benable = toggleSubFormSteps();
                else{
                    setStepEnabled(SOSUBFORMFIELDSPAGE, false);
                    setStepEnabled(SOFIELDLINKERPAGE, false);
                }
            }
            setControlProperty("btnWizardFinish", "Enabled", new Boolean(benable));
        }
     }
}





