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

/*
 * HelpTextProvider.java
 *
 * Created on 16. November 2006, 09:44
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package integration.extensions;

import com.sun.star.inspection.XObjectInspectorUI;
import com.sun.star.inspection.XPropertyControl;
import com.sun.star.inspection.XPropertyControlObserver;
import com.sun.star.lang.NoSupportException;

/** displays help text for the currently selected method
 */
public class HelpTextProvider implements XPropertyControlObserver
{
    private XObjectInspectorUI  m_inspectorUI;

    /**
     * Creates a new instance of HelpTextProvider
     */
    public HelpTextProvider( XObjectInspectorUI _inspectorUI )
    {
        m_inspectorUI = _inspectorUI;
        m_inspectorUI.registerControlObserver( this );
    }

    public void focusGained( XPropertyControl _propertyControl )
    {
        try
        {
            String helpText = "here could be the help for:\n";
            helpText += _propertyControl.getValue().toString();
            m_inspectorUI.setHelpSectionText( helpText );
        }
        catch (NoSupportException ex)
        {
            ex.printStackTrace();
        }
    }

    public void valueChanged( XPropertyControl _propertyControl )
    {
        // not interested in
    }
}
