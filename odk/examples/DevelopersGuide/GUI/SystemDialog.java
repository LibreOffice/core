/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.ui.dialogs.XExecutableDialog;
import com.sun.star.ui.dialogs.XFilePicker;
import com.sun.star.ui.dialogs.XFilePickerControlAccess;
import com.sun.star.ui.dialogs.XFilterManager;
import com.sun.star.ui.dialogs.XFolderPicker;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;



public class SystemDialog  {

    protected XComponentContext m_xContext = null;
    protected com.sun.star.lang.XMultiComponentFactory m_xMCF;

    /** Creates a new instance of MessageBox */
    public SystemDialog(XComponentContext _xContext, XMultiComponentFactory _xMCF){
        m_xContext = _xContext;
        m_xMCF = _xMCF;
    }

    public static void main(String args[]){
        try {
            XComponentContext xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            if(xContext != null )
                System.out.println("Connected to a running office ...");
            XMultiComponentFactory xMCF = xContext.getServiceManager();
            SystemDialog oSystemDialog = new SystemDialog(xContext, xMCF);
            oSystemDialog.raiseSaveAsDialog();
            oSystemDialog.raiseFolderPicker(oSystemDialog.getWorkPath(), "My Title");
        }catch( Exception e ) {
            System.err.println( e + e.getMessage());
            e.printStackTrace();
        }

        System.exit( 0 );
    }


    public String raiseSaveAsDialog() {
        String sStorePath = "";
        XComponent xComponent = null;
        try {
            // the filepicker is instantiated with the global Multicomponentfactory...
            Object oFilePicker = m_xMCF.createInstanceWithContext("com.sun.star.ui.dialogs.FilePicker", m_xContext);
            XFilePicker xFilePicker = (XFilePicker) UnoRuntime.queryInterface(XFilePicker.class, oFilePicker);

            // the defaultname is the initially proposed filename..
            xFilePicker.setDefaultName("MyExampleDocument");

            // set the initial displaydirectory. In this example the user template directory is used
            Object oPathSettings = m_xMCF.createInstanceWithContext("com.sun.star.util.PathSettings",m_xContext);
            XPropertySet xPropertySet = (XPropertySet) com.sun.star.uno.UnoRuntime.queryInterface(XPropertySet.class, oPathSettings);
            String sTemplateUrl = (String) xPropertySet.getPropertyValue("Template_writable");
            xFilePicker.setDisplayDirectory(sTemplateUrl);

            // set the filters of the dialog. The filternames may be retrieved from
            // http://wiki.services.openoffice.org/wiki/Framework/Article/Filter
            XFilterManager xFilterManager = (XFilterManager) UnoRuntime.queryInterface(XFilterManager.class, xFilePicker);
            xFilterManager.appendFilter("OpenDocument Text Template", "writer8_template");
            xFilterManager.appendFilter("OpenDocument Text", "writer8");

            // choose the template that defines the capabilities of the filepicker dialog
            XInitialization xInitialize = (XInitialization) UnoRuntime.queryInterface(XInitialization.class, xFilePicker);
            Short[] listAny = new Short[] { new Short(com.sun.star.ui.dialogs.TemplateDescription.FILESAVE_AUTOEXTENSION)};
            xInitialize.initialize(listAny);

            // add a control to the dialog to add the extension automatically to the filename...
            XFilePickerControlAccess xFilePickerControlAccess = (XFilePickerControlAccess) UnoRuntime.queryInterface(XFilePickerControlAccess.class, xFilePicker);
            xFilePickerControlAccess.setValue(com.sun.star.ui.dialogs.ExtendedFilePickerElementIds.CHECKBOX_AUTOEXTENSION, (short) 0, new Boolean(true));

            xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, xFilePicker);

            // execute the dialog...
            XExecutableDialog xExecutable = (XExecutableDialog) UnoRuntime.queryInterface(XExecutableDialog.class, xFilePicker);
            short nResult = xExecutable.execute();

            // query the resulting path of the dialog...
            if (nResult == com.sun.star.ui.dialogs.ExecutableDialogResults.OK){
                String[] sPathList = xFilePicker.getFiles();
                if (sPathList.length > 0){
                    sStorePath = sPathList[0];
                }
            }
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace();
        } finally{
            //make sure always to dispose the component and free the memory!
            if (xComponent != null){
                xComponent.dispose();
            }
        }
        return sStorePath;
    }

    public String getWorkPath(){
        String sWorkUrl = "";
        try{
            // retrieve the configured Work path...
            Object oPathSettings = m_xMCF.createInstanceWithContext("com.sun.star.util.PathSettings",m_xContext);
            XPropertySet xPropertySet = (XPropertySet) com.sun.star.uno.UnoRuntime.queryInterface(XPropertySet.class, oPathSettings);
            sWorkUrl = (String) xPropertySet.getPropertyValue("Work");
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace();
        }
        return sWorkUrl;
    }

    /** raises a folderpicker in which the user can browse and select a path
     *  @param _sDisplayDirectory the path to the directory that is initially displayed
     *  @param _sTitle the title of the folderpicker
     *  @return the path to the folder that the user has selected. if the user has closed
     *  the folderpicker by clicking the "Cancel" button
     *  an empty string is returned
     *  @see com.sun.star.ui.dialogs.FolderPicker
     */
    public String raiseFolderPicker(String _sDisplayDirectory, String _sTitle) {
        String sReturnFolder = "";
        XComponent xComponent = null;
        try {
            // instantiate the folder picker and retrieve the necessary interfaces...
            Object oFolderPicker = m_xMCF.createInstanceWithContext("com.sun.star.ui.dialogs.FolderPicker", m_xContext);
            XFolderPicker xFolderPicker = (XFolderPicker) UnoRuntime.queryInterface(XFolderPicker.class, oFolderPicker);
            XExecutableDialog xExecutable = (XExecutableDialog) UnoRuntime.queryInterface(XExecutableDialog.class, oFolderPicker);
            xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, oFolderPicker);
            xFolderPicker.setDisplayDirectory(_sDisplayDirectory);
            // set the dialog title...
            xFolderPicker.setTitle(_sTitle);
            // show the dialog...
            short nResult = xExecutable.execute();

            // User has clicked "Select" button...
            if (nResult == com.sun.star.ui.dialogs.ExecutableDialogResults.OK){
                sReturnFolder = xFolderPicker.getDirectory();
            }

        }catch( Exception exception ) {
            exception.printStackTrace(System.out);
        } finally{
            //make sure always to dispose the component and free the memory!
            if (xComponent != null){
                xComponent.dispose();
            }
        }
        // return the selected path. If the user has clicked cancel an empty string is
        return sReturnFolder;
    }
}

