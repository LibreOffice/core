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



package org.openoffice.test.assistant.views;

import org.eclipse.jface.viewers.ITableLabelProvider;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.swt.graphics.Image;

public class VclExplorerLabelProvider extends LabelProvider implements ITableLabelProvider {
    public VclExplorerLabelProvider() {

    }

    public String getColumnText(Object obj, int index) {
        ControlInfo info = (ControlInfo) obj;
        switch (index) {
        case 0:
            return info.id.toString();
        case 1:
            return info.name;
        case 2:
            return Long.toString(info.type) + " " + info.getVclType();
        case 3:
            return info.tip;
        }
        return "";
    }

    public Image getColumnImage(Object obj, int index) {
        return null;
    }
}