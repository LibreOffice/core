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



import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.XItemListener;
import com.sun.star.lang.EventObject;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.UnoRuntime;


public class RoadmapItemStateChangeListener implements XItemListener {
    protected com.sun.star.lang.XMultiServiceFactory m_xMSFDialogModel;

    public RoadmapItemStateChangeListener(com.sun.star.lang.XMultiServiceFactory xMSFDialogModel) {
        m_xMSFDialogModel = xMSFDialogModel;
    }

    public void itemStateChanged(com.sun.star.awt.ItemEvent itemEvent) {
        try {
            // get the new ID of the roadmap that is supposed to refer to the new step of the dialogmodel
            int nNewID = itemEvent.ItemId;
            XPropertySet xDialogModelPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, m_xMSFDialogModel);
            int nOldStep = ((Integer) xDialogModelPropertySet.getPropertyValue("Step")).intValue();
            // in the following line "ID" and "Step" are mixed together.
            // In fact in this case they denot the same
            if (nNewID != nOldStep){
                xDialogModelPropertySet.setPropertyValue("Step", new Integer(nNewID));
            }
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void disposing(EventObject eventObject) {
    }
}

