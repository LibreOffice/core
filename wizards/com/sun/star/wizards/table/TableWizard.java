/*************************************************************************
*
*  $RCSfile: TableWizard.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: pjunck $ $Date: 2004-10-27 13:39:01 $
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

package com.sun.star.wizards.table;

import java.util.Hashtable;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.SQLException;
import com.sun.star.task.XJobExecutor;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.TableDescriptor;
import com.sun.star.wizards.ui.*;


public class TableWizard extends WizardDialog{

    static String slblFields;
    static String slblSelFields;
    Finalizer curFinalizer;
    ScenarioSelector curScenarioSelector;
    FieldFormatter curFieldFormatter;
    PrimaryKeyHandler curPrimaryKeyHandler;
    String sMsgWizardName = "";
    public Hashtable fielditems;
    int wizardmode;
    String tablename;
    TableDescriptor curTableDescriptor;
    public static final int SONULLPAGE = 0;
    public static final int SOMAINPAGE = 1;
    public static final int SOFIELDSFORMATPAGE = 2;
    public static final int SOPRIMARYKEYPAGE = 3;
    public static final int SOFINALPAGE = 4;


    String WizardHeaderText[] = new String[8];

    public TableWizard(XMultiServiceFactory xMSF) {
        super(xMSF, 41000);
        super.addResourceHandler("TableWizard", "dbw");
        String sTitle = oResource.getResText(UIConsts.RID_TABLE + 1);
        Helper.setUnoPropertyValues(xDialogModel,
            new String[] { "Height","Moveable","Name","PositionX","PositionY","Step","TabIndex","Title","Width"},
            new Object[] { new Integer(210),Boolean.TRUE, "DialogTable", new Integer(102),new Integer(41),new Integer(1), new Short((short)0), sTitle, new Integer(310)}  );
        drawNaviBar();
        fielditems = new Hashtable();
        //TODO if reportResouces cannot be gotten dispose officedocument
        if (getTableResources() == true)
            setRightPaneHeaders(oResource, UIConsts.RID_TABLE + 8, 4);
    }


    protected void enterStep(int nOldStep, int nNewStep) {
        switch (nNewStep){
            case SOMAINPAGE:
                break;
            case SOFIELDSFORMATPAGE:
                curFieldFormatter.initialize(curTableDescriptor, this.curScenarioSelector.getSelectedFieldNames());
                break;
            case SOPRIMARYKEYPAGE:
                curPrimaryKeyHandler.initialize(curTableDescriptor);
                break;
            case SOFINALPAGE:
                curFinalizer.initialize(curScenarioSelector.getFirstTableName());
                break;
             default:
                break;
        }
    }


    protected void leaveStep(int nOldStep, int nNewStep){
         switch (nOldStep){
            case SOMAINPAGE:
                curScenarioSelector.addColumnsToDescriptor();
                break;
            case SOFIELDSFORMATPAGE:
                curFieldFormatter.updateColumnofColumnDescriptor();
                curScenarioSelector.setSelectedFieldNames(curFieldFormatter.getFieldNames());
                break;
            case SOPRIMARYKEYPAGE:
                break;
            case SOFINALPAGE:
                break;
            default:
                 break;
          }
    }


    public static void main(String args[]) {
    String ConnectStr = "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService";      //localhost  ;Lo-1.Germany.sun.com; 10.16.65.155
    PropertyValue[] curproperties = null;
    try {
        XMultiServiceFactory xLocMSF = com.sun.star.wizards.common.Desktop.connect(ConnectStr);
        TableWizard CurTableWizard = new TableWizard(xLocMSF);
        if(xLocMSF != null){
            System.out.println("Connected to "+ ConnectStr);
            curproperties = new PropertyValue[1];
            curproperties[0] = Properties.createProperty("DatabaseLocation", "file:///C:/Documents and Settings/bc93774.EHAM02-DEV/My Documents/Myverynewdbase.odb"); // NewAccessDatabase, MyDocAssign baseLocation ); "DataSourceName", "db1");
//          curproperties[0] = Properties.createProperty("DataSourceName", "Bibliography"); //Bibliography
            CurTableWizard.startTableWizard(xLocMSF, curproperties);
        }
    }
    catch(Exception exception){
        exception.printStackTrace(System.out);
    }}


    public void buildSteps(){
        curScenarioSelector = new ScenarioSelector(this, this.curTableDescriptor, slblFields, slblSelFields);
        curFieldFormatter = new FieldFormatter(this, curTableDescriptor );
        if (this.curTableDescriptor.supportsCoreSQLGrammar())
            curPrimaryKeyHandler = new PrimaryKeyHandler(this);
        curFinalizer = new Finalizer(this, curTableDescriptor);
        enableNavigationButtons(false, false, false);
    }


    public boolean createTable(){
    tablename = curFinalizer.getTableName(curScenarioSelector.getFirstTableName());
    if (curTableDescriptor.supportsCoreSQLGrammar()){
        String[] keyfieldnames = curPrimaryKeyHandler.getPrimaryKeyFields(curTableDescriptor);
        if (keyfieldnames != null){
            if (keyfieldnames.length > 0){
                boolean bIsAutoIncrement = curPrimaryKeyHandler.IsAutoIncrement();
                return curTableDescriptor.createTable(tablename, keyfieldnames, bIsAutoIncrement, curScenarioSelector.getSelectedFieldNames());
            }
        }
    }
        return curTableDescriptor.createTable(tablename, curScenarioSelector.getSelectedFieldNames());
    }


    public void finishWizard(){
        super.switchToStep(super.getCurrentStep(), SOFINALPAGE);
        wizardmode = curFinalizer.finish();
        if (createTable()){
            if (wizardmode == Finalizer.MODIFYTABLEMODE)
                curTableDescriptor.switchtoDesignmode(tablename, com.sun.star.sdb.CommandType.TABLE);
            else if (wizardmode == Finalizer.WORKWITHTABLEMODE)
                curTableDescriptor.switchtoDataViewmode(tablename, com.sun.star.sdb.CommandType.TABLE);
            super.xDialog.endExecute();
        }
    }

    private void callFormWizard(){
    try {
        Object oFormWizard = this.xMSF.createInstance("com.sun.star.wizards.form.CallFormWizard");
        PropertyValue[] aProperties  = new PropertyValue[2];
        aProperties[0] = Properties.createProperty("ActiveConnection", curTableDescriptor.DBConnection);
        aProperties[1] = Properties.createProperty("DataSource", curTableDescriptor.xDataSource);
        XInitialization xInitialization = (XInitialization) UnoRuntime.queryInterface(XInitialization.class, oFormWizard);
        xInitialization.initialize(aProperties);
        XJobExecutor xJobExecutor = (XJobExecutor) UnoRuntime.queryInterface(XJobExecutor.class, oFormWizard);
        xJobExecutor.trigger("start");
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    public void cancelWizard() {
        xDialog.endExecute();
    }


    public void insertFormRelatedSteps(){
        addRoadmap();
        int i = 0;
        i = insertRoadmapItem(0, true, oResource.getResText(UIConsts.RID_TABLE + 2), SOMAINPAGE);
        i = insertRoadmapItem(i, false, oResource.getResText(UIConsts.RID_TABLE + 3), SOFIELDSFORMATPAGE);
        if (this.curTableDescriptor.supportsCoreSQLGrammar())
            i = insertRoadmapItem(i, false, oResource.getResText(UIConsts.RID_TABLE + 4), SOPRIMARYKEYPAGE);
        i = insertRoadmapItem(i, false, oResource.getResText(UIConsts.RID_TABLE + 5),  SOFINALPAGE);        // Orderby is always supported
        setRoadmapInteractive(true);
        setRoadmapComplete(true);
        setCurrentRoadmapItemID((short) 1);
    }


    public void startTableWizard(XMultiServiceFactory _xMSF, PropertyValue[] CurPropertyValue){
    try{
        curTableDescriptor = new TableDescriptor(xMSF);
        if (curTableDescriptor.getConnection(CurPropertyValue)){
            int[] nDataTypes  = curTableDescriptor.getsupportedDataTypes();
            buildSteps();
            createWindowPeer();
            curTableDescriptor.setWindowPeer(this.xControl.getPeer());
    //      setAutoMnemonic("lblDialogHeader", false);
            insertFormRelatedSteps();
            short RetValue = executeDialog();
            boolean bdisposeDialog = true;
            xComponent.dispose();
            switch (RetValue){
                case 0:                         // via Cancelbutton or via sourceCode with "endExecute"
                    if (wizardmode == Finalizer.STARTFORMWIZARDMODE)
                        callFormWizard();
                    break;
                case 1:

                    break;
            }
        }
    }
    catch(java.lang.Exception jexception ){
        jexception.printStackTrace(System.out);
    }}


    public boolean getTableResources(){
        sMsgWizardName = super.oResource.getResText(UIConsts.RID_TABLE+1);
        slblFields = oResource.getResText(UIConsts.RID_TABLE + 19);
        slblSelFields =  oResource.getResText(UIConsts.RID_TABLE + 25);
//      sMsgSavingImpossible = oResource.getResText(UIConsts.RID_DB_COMMON + 30);
        // TODO the following message also has to show up when saving failed: sLinkCreationImpossible
//          sMsgLinkCreationImpossible = oResource.getResText(UIConsts.RID_DB_COMMON + 31);
//      sMsgFilePathInvalid = oResource.getResText(UIConsts.RID_DB_COMMON + 36);
//      slblColumnTitles = oResource.getResText(UIConsts.RID_REPORT + 70);
//      slblColumnNames = oResource.getResText(UIConsts.RID_REPORT + 71);
        return true;
    }

    private void toggleWizardSteps(int _startStep, boolean _benable){
        super.setStepEnabled(SOFIELDSFORMATPAGE, _benable);
        super.setStepEnabled(SOPRIMARYKEYPAGE, _benable);
        super.setStepEnabled(SOFINALPAGE, _benable);
        setControlProperty("btnWizardNext", "Enabled", new Boolean(_benable));
        setControlProperty("btnWizardFinish", "Enabled", new Boolean(_benable));
    }
}