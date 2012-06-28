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
import org.openoffice.test.vcl.client.SmartId;

/**
 * Proxy used to access all VCL field controls
 *
 */
public class VclField extends VclEditBox{

    public VclField(SmartId smartId) {
        super(smartId);
    }

    /**
     * Construct the field control with its string ID
     * @param uid
     */
    public VclField(String uid) {
        super(uid);
    }

    /**
     * Move one entry higher of Field
     *
     */
    public void more() {
        invoke(Constant.M_More);
    }

    /**
     * Move one entry lower of Field
     *
     */
    public void less() {
        invoke(Constant.M_Less);
    }

    /**
     * Goes to the maximum value of Field
     *
     */
    public void toMax() {
        invoke(Constant.M_ToMax);
    }


    /**
     * Goes to the minimum value of Field
     */
    public void toMin() {
        invoke(Constant.M_ToMin);
    }
}
