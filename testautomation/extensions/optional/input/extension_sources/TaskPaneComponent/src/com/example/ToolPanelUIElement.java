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



package com.example;

import com.sun.star.awt.XContainerWindowProvider;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.deployment.PackageInformationProvider;
import com.sun.star.deployment.XPackageInformationProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.DisposedException;
import com.sun.star.lang.XComponent;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.ui.UIElementType;
import com.sun.star.ui.XUIElement;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author frank.schoenheit@oracle.com
 */
public class ToolPanelUIElement extends ComponentBase implements XUIElement
{

    protected final XFrame          m_docFrame;
    protected final XWindow         m_parentWindow;
    private XWindow                 m_panelRootWindow = null;
    private Object                  m_toolPanel = null;
    XComponentContext context = null;


    protected ToolPanelUIElement( XComponentContext _context, final XFrame i_docFrame, final XWindow i_parentWindow )
    {
        context = _context;
        m_docFrame = i_docFrame;
        m_parentWindow = i_parentWindow;
    }

    public static String getResourceURLBase()
    {
        return "private:resource/toolpanel/com.example.TaskPaneComponent/";
    }

    // XUIElement overridables
    public XFrame getFrame()            { return m_docFrame; }
    public short getType()              { return UIElementType.TOOLPANEL; }
    public String getResourceURL()      { return getResourceURLBase() + getPanelResourceURLSuffix(); }

    public Object getRealInterface()
    {
        synchronized( this )
        {
            if ( this.bDisposed || this.bInDispose )
                throw new DisposedException( new String(), this );

            if ( m_toolPanel == null )
                m_toolPanel = new ToolPanel( getOrCreatePanelRootWindow() );
        }
        return m_toolPanel;
    }

    // own overridables
    /** returns the name of the dialog description file of the tool panel, without .xdl suffix
     */
    protected String getPanelResourceLocation()
    {
        return "TaskPaneComponentUI";

    }

    /** returns the resource URL suffix to use for the tool panel
     * @return
     */
    protected String getPanelResourceURLSuffix()
    {
        return "TaskPaneComponentUI";
    }


    // ComponentBase overridables
    protected void postDisposing()
    {
        super.postDisposing();
        synchronized ( this )
        {
            if ( m_panelRootWindow != null )
            {
                XComponent panelWindowComp = UnoRuntime.queryInterface( XComponent.class, m_panelRootWindow );
                panelWindowComp.dispose();
                m_panelRootWindow = null;

            }
            m_toolPanel = null;
        }
    }

    // helpers
    synchronized private XWindow getOrCreatePanelRootWindow()
    {
        try
        {
            XPackageInformationProvider pip = PackageInformationProvider.get(context);
            String s = pip.getPackageLocation("com.example.TaskPaneComponent");

            String dialogUrl = s + "/toolpanels/" + getPanelResourceLocation() + ".xdl";

            XContainerWindowProvider provider = UnoRuntime.queryInterface(
                XContainerWindowProvider.class,
                context.getServiceManager().createInstanceWithContext("com.sun.star.awt.ContainerWindowProvider",context)
            );

            m_panelRootWindow = provider.createContainerWindow(
                dialogUrl,
                "",
                UnoRuntime.queryInterface( XWindowPeer.class, m_parentWindow ),
                null
            );

        }
        catch ( final Exception e )
        {
            Logger.getLogger( ToolPanelUIElement.class.getName() ).log( Level.SEVERE, null, e );
        }
        return m_panelRootWindow;
    }
}
