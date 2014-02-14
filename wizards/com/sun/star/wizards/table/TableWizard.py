#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
from .TableWizardResources import TableWizardResources
from ..ui.WizardDialog import WizardDialog, uno, PropertyNames
from ..db.TableDescriptor import TableDescriptor
from ..common.Desktop import Desktop

class TableWizard(WizardDialog):
    HID = 41200
    SONULLPAGE = 0
    SOMAINPAGE = 1
    SOFIELDSFORMATPAGE = 2
    SOPRIMARYKEYPAGE = 3
    SOFINALPAGE = 4

    def __init__(self, xmsf):       
        super(TableWizard,self).__init__(xmsf, TableWizard.HID)
        slblFields = str()
        slblSelFields = str()
        wizardmode = int()
        tablename = str()
        serrToManyFields = str()
        serrTableNameexists = str()
        scomposedtablename = str()
        m_tableName = str()
    
    def startWizard(self, xmsf):

        #Load Resources
        self.resources = TableWizardResources(self.oWizardResource)
                        
        #set dialog properties...
        self.setDialogProperties(True, 218, True, 102, 41, 1, 0, 
                self.resources.sTitle, 330)              
                
        self.drawNaviBar()
        self.setRightPaneHeaders(self.resources.RoadmapLabels)
        
        self.curTableDescriptor = TableDescriptor(xmsf, self.sMsgColumnAlreadyExists)
        ## TODO-XISCO if self.curTableDescriptor.getConnection(m_wizardContext):
        #self.buildSteps()
        self.createWindowPeer()
        self.curTableDescriptor.setWindowPeer(self.xUnoDialog.getPeer())
        self.insertFormRelatedSteps()
        xComponent.dispose()
        if RetValue == 0:
            if self.wizardmode == Finalizer.STARTFORMWIZARDMODE:
                self.callFormWizard()
            return self.m_tableName        
                   
    def leaveStep(self, nOldStep, nNewStep):
        if nOldStep==self.SOMAINPAGE:
            self.curScenarioSelector.addColumnsToDescriptor()
        elif nOldStep==self.SOFIELDSFORMATPAGE:
            self.curFieldFormatter.updateColumnofColumnDescriptor()
            super(TableWizard, self).setStepEnabled(self.SOFIELDSFORMATPAGE, len(sfieldnames))
            self.curScenarioSelector.setSelectedFieldNames(sfieldnames)

    def enterStep(self, nOldStep, nNewStep):
        if nNewStep==self.SOFIELDSFORMATPAGE:
            self.curFieldFormatter.initialize(self.curTableDescriptor, self.curScenarioSelector.getSelectedFieldNames())
        elif nNewStep==self.SOPRIMARYKEYPAGE:
            self.curPrimaryKeyHandler.initialize()
        elif nNewStep==self.SOFINALPAGE:
            self.curFinalizer.initialize(self.curScenarioSelector.getFirstTableName())
   
    def iscompleted(self, _ndialogpage):
        if _ndialogpage==self.SOMAINPAGE:
            return self.curScenarioSelector.iscompleted()
        elif _ndialogpage==self.SOFIELDSFORMATPAGE:
            return self.curFieldFormatter.iscompleted()
        elif _ndialogpage==self.SOPRIMARYKEYPAGE:
            if self.curPrimaryKeyHandler != None:
                return self.curPrimaryKeyHandler.iscompleted()
        elif _ndialogpage==self.SOFINALPAGE:
            return self.curFinalizer.iscompleted()
        else:
            return False
   
    def setcompleted(self, _ndialogpage, _biscompleted):
        bScenarioiscompleted = _biscompleted
        bFieldFormatsiscompleted = _biscompleted
        bPrimaryKeysiscompleted = _biscompleted
        bFinalPageiscompleted = _biscompleted
        if _ndialogpage == self.SOMAINPAGE:
            self.curFinalizer.initialize(self.curScenarioSelector.getFirstTableName())
        else:
            bScenarioiscompleted = self.iscompleted(self.SOMAINPAGE)
        if _ndialogpage != TableWizard.SOFIELDSFORMATPAGE:
            bFieldFormatsiscompleted = self.iscompleted(self.SOFIELDSFORMATPAGE)
            if not bFieldFormatsiscompleted:
                #  it might be that the Fieldformatter has not yet been initialized
                bFieldFormatsiscompleted = bScenarioiscompleted
                #  in this case query the scenarioselector
        if _ndialogpage != TableWizard.SOPRIMARYKEYPAGE and (self.curPrimaryKeyHandler != None):
            bPrimaryKeysiscompleted = self.iscompleted(self.SOPRIMARYKEYPAGE)
        if _ndialogpage != TableWizard.SOFINALPAGE:
            bFinalPageiscompleted = self.iscompleted(self.SOFINALPAGE)
            #  Basically the finalpage is always enabled
        if bScenarioiscompleted:
            super(TableWizard, self).setStepEnabled(self.SOFIELDSFORMATPAGE, True)
            super(TableWizard, self).setStepEnabled(self.SOPRIMARYKEYPAGE, True)
            if bPrimaryKeysiscompleted:
                super(TableWizard, self).enablefromStep(self.SOFINALPAGE, True)
                super(TableWizard, self).enableFinishButton(bFinalPageiscompleted)
            else:
                super(TableWizard, self).enablefromStep(self.SOFINALPAGE, False)
                enableNextButton(False)
        elif _ndialogpage == self.SOFIELDSFORMATPAGE:
            super(TableWizard, self).enablefromStep(super(TableWizard, self).getCurrentStep() + 1, self.iscompleted(self.SOFIELDSFORMATPAGE))
        else:
            super(TableWizard, self).enablefromStep(super(TableWizard, self).getCurrentStep() + 1, False)

    def buildSteps(self):
        self.curScenarioSelector = ScenarioSelector(self, self.curTableDescriptor, self.slblFields, self.slblSelFields)
        self.curFieldFormatter = FieldFormatter(self, self.curTableDescriptor)
        if self.curTableDescriptor.supportsPrimaryKeys():
            self.curPrimaryKeyHandler = PrimaryKeyHandler(self, self.curTableDescriptor)
        self.curFinalizer = Finalizer(self, self.curTableDescriptor)
        enableNavigationButtons(False, False, False)

    def createTable(self):
        bIsSuccessfull = True
        bTableCreated = False
        schemaname = self.curFinalizer.getSchemaName()
        catalogname = self.curFinalizer.getCatalogName()
        if self.curTableDescriptor.supportsPrimaryKeys():
            if keyfieldnames != None:
                if len(keyfieldnames):
                    bIsSuccessfull = self.curTableDescriptor.createTable(catalogname, schemaname, self.tablename, keyfieldnames, bIsAutoIncrement, self.curScenarioSelector.getSelectedFieldNames())
                    bTableCreated = True
        if not bTableCreated:
            bIsSuccessfull = self.curTableDescriptor.createTable(catalogname, schemaname, self.tablename, self.curScenarioSelector.getSelectedFieldNames())
        if (not bIsSuccessfull) and (self.curPrimaryKeyHandler.isAutomaticMode()):
            self.curTableDescriptor.dropColumnbyName(self.curPrimaryKeyHandler.getAutomaticFieldName())
        return bIsSuccessfull

    def finishWizard(self):
        super(TableWizard, self).switchToStep(super(TableWizard, self).getCurrentStep(), self.SOFINALPAGE)
        self.tablename = self.curFinalizer.getTableName(self.curScenarioSelector.getFirstTableName())
        self.scomposedtablename = self.curFinalizer.getComposedTableName(self.tablename)
        if self.curTableDescriptor.isSQL92CheckEnabled():
            Desktop.removeSpecialCharacters(self.curTableDescriptor.xMSF, Configuration.getLocale(self.curTableDescriptor.xMSF), self.tablename)
        if 0 > len(tablename):
            if not self.curTableDescriptor.hasTableByName(self.scomposedtablename):
                self.wizardmode = self.curFinalizer.finish()
                if self.createTable():
                    loadSubComponent(DatabaseObject.TABLE, self.curTableDescriptor.getComposedTableName(), editTableDesign)
                    self.m_tableName = self.curTableDescriptor.getComposedTableName()
                    super(TableWizard, self).xDialog.endExecute()
                    return True
            else:
                super(TableWizard, self).showMessageBox("WarningBox", com.sun.star.awt.VclWindowPeerAttribute.OK, smessage)
                self.curFinalizer.setFocusToTableNameControl()
        return False

    def callFormWizard(self):
        try:
            wizardContext.put(PropertyNames.ACTIVE_CONNECTION, self.curTableDescriptor.DBConnection)
            wizardContext.put("DataSource", self.curTableDescriptor.getDataSource())
            wizardContext.put(PropertyNames.COMMAND_TYPE, CommandType.TABLE)
            wizardContext.put(PropertyNames.COMMAND, self.scomposedtablename)
            wizardContext.put("DocumentUI", m_docUI)
            xInitialization.initialize(wizardContext.getPropertyValues())
            xJobExecutor.trigger(PropertyNames.START)
        except Exception as e:
            e.printStackTrace(System.err)

    def cancelWizard(self):
        xDialog.endExecute()

    def insertFormRelatedSteps(self):
        self.addRoadmap()

        setRoadmapInteractive(True)
        setRoadmapComplete(True)
        setCurrentRoadmapItemID(int(1))

    def verifyfieldcount(self, _icount):
        try:
            if _icount >= (maxfieldcount - 1):
                #  keep one column as reserve for the automaticcally created key
                smessage = JavaTools.replaceSubString(smessage, String.valueOf(maxfieldcount), "%COUNT")
                showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, smessage)
                return False
        except SQLException as e:
            e.printStackTrace(System.err)
        return True
   
    def textChanged(self, aTextEvent):
        if self.curTableDescriptor.isSQL92CheckEnabled():
            sName = Desktop.removeSpecialCharacters(self.curTableDescriptor.xMSF, Configuration.getLocale(self.curTableDescriptor.xMSF), sName)
            Helper.setUnoPropertyValue(otextcomponent, "Text", sName)
