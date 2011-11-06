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



package org.openoffice.setup;

import javax.swing.JPanel;

public abstract class PanelController {

    private SetupFrame frame;
    private JPanel panel;
    private String name;
    private String next;
    private String prev;

    private PanelController () {
    }

    public PanelController (String name, JPanel panel) {
        this.name  = name;
        this.panel = panel;
    }

    public final JPanel getPanel () {
        return this.panel;
    }

    public final void setPanel (JPanel panel) {
        this.panel = panel;
    }

    public final String getName () {
        return this.name;
    }

    public final void setName (String name) {
        this.name = name;
    }

    final void setSetupFrame (SetupFrame frame) {
        this.frame = frame;
    }

    public final SetupFrame getSetupFrame () {
        return this.frame;
    }

    public String getNext () {
        return null;
    }

    public String getDialogText () {
        return null;
    }

    public String getPrevious () {
        return null;
    }

    public void beforeShow () {
    }

    public void duringShow () {
    }

    public boolean afterShow (boolean nextButtonPressed) {
        boolean repeatDialog = false;
        return repeatDialog;
    }

    public abstract String getHelpFileName();

}
