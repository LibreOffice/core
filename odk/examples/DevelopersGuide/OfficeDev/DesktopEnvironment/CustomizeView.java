/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.uno.UnoRuntime;

import java.awt.*;
import javax.swing.*;
import java.lang.String;
import java.awt.event.*;
import java.awt.*;

/**
 * Makes it possible to change some states of currently loaded
 * document (e.g. enable/disable menubar, toolbar, objectbar)
 *
 * @author     Andreas Schl&uuml;ns
 */
public class CustomizeView extends    JPanel
                           implements IShutdownListener
{
    /**
     * These const URL's describe feature for toggling some properties of loaded document.
     * Dispatch it with the corresponding parameter to the frame.
     */
    private static final String FEATUREURL_MENUBAR      = "slot:6661"         ;
    private static final String FEATUREURL_TOOLBAR      = "slot:5909"         ;
    private static final String FEATUREURL_OBJECTBAR    = "slot:5905"         ;

    private static final String FEATUREPROP_MENUBAR     = "MenuBarVisible"    ;
    private static final String FEATUREPROP_TOOLBAR     = "ToolBarVisible"    ;
    private static final String FEATUREPROP_OBJECTBAR   = "ObjectBarVisible"  ;

    private static final String ACTION_MENUBAR          = "toogle_menu"       ;
    private static final String ACTION_TOOLBAR          = "toogle_toolbar"    ;
    private static final String ACTION_OBJECTBAR        = "toogle_objectbar"  ;

    private static final String MENUBAR_ON              = "menubar on"        ;
    private static final String TOOLBAR_ON              = "toolbar on"        ;
    private static final String OBJECTBAR_ON            = "objectbar on"      ;

    private static final String MENUBAR_OFF             = "menubar off"       ;
    private static final String TOOLBAR_OFF             = "toolbar off"       ;
    private static final String OBJECTBAR_OFF           = "objectbar off"     ;

    // ____________________
    // member

    /**
     * @member  m_cbMenuBar             reference to checkbox for toggling menubar
     * @member  m_cbToolBar             reference to checkbox for toggling toolbar
     * @member  m_cbObjectBar           reference to checkbox for toggling objectbar
     *
     * @member  m_aMenuBarListener      listener for status events of the menu bar
     * @member  m_aToolBarListener      listener for status events of the tool bar
     * @member  m_aObjectBarListener    listener for status events of the object bar
     */
    private JCheckBox           m_cbMenuBar         ;
    private JCheckBox           m_cbToolBar         ;
    private JCheckBox           m_cbObjectBar       ;

    private StatusListener      m_aMenuBarListener  ;
    private StatusListener      m_aToolBarListener  ;
    private StatusListener      m_aObjectBarListener;

    // ____________________

    /**
     * ctor
     * Create view controls on startup and initialize it.
     * We don't start listening here. see setFrame()!
     */
    CustomizeView()
    {
        this.setLayout(new GridLayout(3,0));

        m_cbMenuBar   = new JCheckBox(MENUBAR_OFF  , false);
        m_cbToolBar   = new JCheckBox(TOOLBAR_OFF  , false);
        m_cbObjectBar = new JCheckBox(OBJECTBAR_OFF, false);

        m_cbMenuBar.setEnabled  (false);
        m_cbToolBar.setEnabled  (false);
        m_cbObjectBar.setEnabled(false);

        m_cbMenuBar.setActionCommand  (ACTION_MENUBAR  );
        m_cbToolBar.setActionCommand  (ACTION_TOOLBAR  );
        m_cbObjectBar.setActionCommand(ACTION_OBJECTBAR);

        this.add(m_cbMenuBar  );
        this.add(m_cbToolBar  );
        this.add(m_cbObjectBar);
    }

    // ____________________

    /**
     * set new frame for this view
     * We start listening for frame action/status and click events instandly.
     * If an event occure we use it to synchronize our controls
     * with states of a (my be) new document view of this frame.
     *
     * @param xFrame
     *          the reference to the frame, which provides the
     *          possibility to get the required status informations
     *
     *          Attention: We don't accept new frames here.
     *          We get one after startup and work with him.
     *          That's it!
     */
    public void setFrame(com.sun.star.frame.XFrame xFrame)
    {
        if (xFrame==null)
            return;

        // be listener for click events
        // They will toogle the UI controls.
        ClickListener aMenuBarHandler   = new ClickListener(FEATUREURL_MENUBAR  ,FEATUREPROP_MENUBAR  ,xFrame);
        ClickListener aToolBarHandler   = new ClickListener(FEATUREURL_TOOLBAR  ,FEATUREPROP_TOOLBAR  ,xFrame);
        ClickListener aObjectBarHandler = new ClickListener(FEATUREURL_OBJECTBAR,FEATUREPROP_OBJECTBAR,xFrame);

        m_cbMenuBar.addActionListener  (aMenuBarHandler  );
        m_cbToolBar.addActionListener  (aToolBarHandler  );
        m_cbObjectBar.addActionListener(aObjectBarHandler);

        // be frame action listener
        // The callback will update listener connections
        // for status updates automaticly!
        m_aMenuBarListener   = new StatusListener(m_cbMenuBar  ,MENUBAR_ON  ,MENUBAR_OFF  ,xFrame, FEATUREURL_MENUBAR  );
        m_aToolBarListener   = new StatusListener(m_cbToolBar  ,TOOLBAR_ON  ,TOOLBAR_OFF  ,xFrame, FEATUREURL_TOOLBAR  );
        m_aObjectBarListener = new StatusListener(m_cbObjectBar,OBJECTBAR_ON,OBJECTBAR_OFF,xFrame, FEATUREURL_OBJECTBAR);

        m_aMenuBarListener.startListening();
        m_aToolBarListener.startListening();
        m_aObjectBarListener.startListening();
    }

    // ____________________

    /**
     * react for click events of the used check boxes
     * We use our internal set dispatch objects to
     * call it. This calls toogle the menu/object- or toolbar.
     * Note: Because we are listener status events too - hopefully
     * we get a notification, if toogling was successfully or not.
     * We use this information to update our check boxes again.
     * But such update doesn't force (hopefully) an action event. Otherwhise
     * we can produce a never ending recursion!
     *
     * @param aEvent
     *          describes the used check box and his current state
     *          we can use to dispatch the right URL to the office
     */
    class ClickListener implements ActionListener,
                                   com.sun.star.lang.XEventListener
    {
        /// URL, to toogle the requested UI item
        String m_sURL;
        /// name of the property which must be used in combination with the URL
        String m_sProp;
        /// we must use this frame to dispatch a request
        com.sun.star.frame.XFrame m_xFrame;

        //_____________________

        /**
         * ctor
         * It initialize an instance of this clas only.
         */
        ClickListener( String                    sURL   ,
                       String                    sProp  ,
                       com.sun.star.frame.XFrame xFrame )
        {
            m_sURL   = sURL  ;
            m_sProp  = sProp ;
            m_xFrame = xFrame;
        }

        //_____________________

        /**
         * callback for action events
         * Such events occure if somehwere click the
         * JCheckBox control on which we are registered.
         * Such events doesn't occure if we set it programmaticly
         * (e.g. if we get status events to -> see class StatusListener too)
         *
         * @param aEvent
         *          describes the check box and his state
         *          we can use to toogle the requested office
         *          resource.
         */
        public void actionPerformed(ActionEvent aEvent)
        {
            synchronized(this)
            {
                if (m_xFrame==null)
                    return;
            }

            // define parameters for following dispatch
            boolean bState = ((JCheckBox)aEvent.getSource()).isSelected();

            // prepare the dispatch
            com.sun.star.util.URL aURL = FunctionHelper.parseURL(m_sURL);
            if (aURL==null)
                return;

            com.sun.star.beans.PropertyValue[] lProperties = new com.sun.star.beans.PropertyValue[1];
            lProperties[0]       = new com.sun.star.beans.PropertyValue();
            lProperties[0].Name  = m_sProp;
            lProperties[0].Value = new Boolean(bState);

            // execute (dispatch) it into the frame
            if (m_xFrame==null)
                return;
            FunctionHelper.execute(m_xFrame,aURL,lProperties,null);
        }

        // ____________________

        /**
         * callback for disposing events
         * Internaly we save a reference to an office frame.
         * Of course he can die and inform us then. We should react
         * and forget his reference.
         *
         * @param aEvent
         *          describes the source which fire this event
         *          Must be our internal saved frame. Otherwhise
         *          somewhere know us without a registration ...
         */
        public void disposing(com.sun.star.lang.EventObject aEvent)
        {
            synchronized(this)
            {
                m_xFrame = null;
            }
        }
    }

    // ____________________

    /**
     * If this java application shutdown - we must cancel all current existing
     * listener connections. Otherwhise the office will run into some
     * DisposedExceptions if it tries to use these forgotten listener references.
     * And of course it can die doing that.
     * We are registered at a central object to be informed if the VM will exit.
     * So we can react.
     */
    public void shutdown()
    {
        m_aMenuBarListener.shutdown();
        m_aToolBarListener.shutdown();
        m_aObjectBarListener.shutdown();

        m_aMenuBarListener   = null;
        m_aToolBarListener   = null;
        m_aObjectBarListener = null;
    }
}
