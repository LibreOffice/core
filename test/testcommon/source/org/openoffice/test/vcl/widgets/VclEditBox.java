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



package org.openoffice.test.vcl.widgets;

import org.openoffice.test.vcl.client.Constant;

/**
 * Proxy used to access VCL EditField/MultiLineEditField
 *
 */
public class VclEditBox extends VclControl {


    public VclEditBox(String id) {
        super(id);
    }


    public VclEditBox(VclApp app, String id) {
        super(app, id);
    }


    /**
     * Set the text of edit box
     * @param str
     */
    public void setText(String str) {
        invoke(Constant.M_SetText, new Object[]{str});
    }


    /**
     * Is the edit box writable?
     * @return true if it is writable, false otherwise
     */
    public boolean isWritable() {
        return (Boolean)invoke(Constant.M_IsWritable);
    }


    /**
     * Get the text of edit box
     * @return the text of edit box
     */
    public String getText(){
        return (String) invoke(Constant.M_GetText);
    }
}
