/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

/*
 * HelpTextProvider.java
 *
 * Created on 16. November 2006, 09:44
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
