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

// __________ Imports __________

import com.sun.star.uno.UnoRuntime;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import java.awt.AWTEvent;
import java.awt.event.WindowEvent;

// __________ Implementation __________

/**
 * This implement a java frame wich contains
 * an office document, shows some status informations
 * about that, provides simple functionality on it
 * (e.g. toggle menubar, save document) and
 * react for different situations independent
 * (e.g. closing the document from outside).
 * Every instance of this class will be a member
 * inside the global "ViewContainer" of this java
 * demo application which holds all opened views alive.
 *
 * @author     Andreas Schl&uuml;ns
 */
public class DocumentView extends    JFrame
                          implements com.sun.star.lang.XEventListener, // react for Frame::disposing()
                                     IShutdownListener                 // react for System.exit()
{
    // ____________________

    /**
     * const
     * These command strings are used to identify a received action
     * of buttons on which we listen for action events.
     */
    public static final String COMMAND_OPEN   = "open"   ;
    public static final String COMMAND_SAVE   = "save"   ;
    public static final String COMMAND_EXPORT = "export" ;
    public static final String COMMAND_EXIT   = "exit"   ;

    // ____________________

    /**
     * @member  mxFrame             office frame which contains the document of this view
     *
     * @member  maStatusView        special panel wich show available status informations of currently loaded document
     * @member  maDocumentView      use JNI mechanism to plug an office window into our own java UI container (used for inplace mode only!)
     * @member  maCustomizeView     special panel makes it possible to toggle menubar/toolbar or objectbar of loaded document
     * @member  maInterceptor       interceptor thread which intercept "new" menu of office frame to open new frames inside this java application
     *
     * @member  msName              unique name of this view (returned by the global ViewContainer during registration)
     *
     * @member  mbOpen              button to open documents
     * @member  mbSave              button to save currently loaded document
     * @member  mbExport            button to save currently loaded document in HTML format (if it is possible!)
     * @member  mbExit              button to exit this demo
     *
     * @member  maInterception      we try to intercept the file->new menu to open new document inside this java application
     */
    private com.sun.star.frame.XFrame       mxFrame             ;

    private StatusView                      maStatusView        ;
    private NativeView                      maDocumentView      ;
    private CustomizeView                   maCustomizeView     ;
    private Interceptor                     maInterceptor       ;

    private String                          msName              ;

    private JButton                         mbtOpen             ;
    private JButton                         mbtSave             ;
    private JButton                         mbtExport           ;
    private JButton                         mbtExit             ;

    private boolean                         mbDead              ;

    // ____________________

    /**
     * ctor
     * Create view controls on startup and initialize it with default values.
     */
    DocumentView()
    {
        this.setSize( new Dimension(800,600) );

        JPanel paMainPanel = (JPanel)this.getContentPane();

        // create and add command buttons to a panel
        // it will be a sub panel of later layouted UI
        mbtOpen   = new JButton("Open ..."        );
        mbtSave   = new JButton("Save"            );
        mbtExport = new JButton("Save as HTML ...");
        mbtExit   = new JButton("Exit"            );

        mbtOpen.setEnabled  (true );
        mbtSave.setEnabled  (false);
        mbtExport.setEnabled(false);
        mbtExit.setEnabled  (true );

        mbtOpen.setActionCommand  (COMMAND_OPEN  );
        mbtSave.setActionCommand  (COMMAND_SAVE  );
        mbtExport.setActionCommand(COMMAND_EXPORT);
        mbtExit.setActionCommand  (COMMAND_EXIT  );

        Reactor aListener = new Reactor();
        mbtOpen.addActionListener  (aListener);
        mbtSave.addActionListener  (aListener);
        mbtExport.addActionListener(aListener);
        mbtExit.addActionListener  (aListener);

        JPanel paCommands = new JPanel( new GridLayout(4,0) );
        paCommands.add(mbtOpen);
        paCommands.add(mbtSave);
        paCommands.add(mbtExport);
        paCommands.add(mbtExit);

        // create view to show status informations of opened file
        maStatusView = new StatusView();

        // create view for toggle different bar's of document
        maCustomizeView = new CustomizeView();

        paCommands.setBorder     ( new TitledBorder(BorderFactory.createEtchedBorder(),"Commands")                );
        maStatusView.setBorder   ( new TitledBorder(BorderFactory.createEtchedBorder(),"Status Informations")     );
        maCustomizeView.setBorder( new TitledBorder(BorderFactory.createEtchedBorder(),"Customize Document View") );

        // layout the whole UI
        JPanel paTest = new JPanel(new GridLayout(3,0));
        paTest.add(paCommands     );
        paTest.add(maStatusView   );
        paTest.add(maCustomizeView);
        JScrollPane paScroll = new JScrollPane();
        paScroll.getViewport().add(paTest,null);

        if(ViewContainer.mbInplace==true)
        {
            // create view to show opened documents
            // This special view is neccessary for inplace mode only!
            maDocumentView = new NativeView();

            JSplitPane paSplit = new JSplitPane();
            paSplit.setOneTouchExpandable( true );

            paSplit.setLeftComponent (maDocumentView);
            paSplit.setRightComponent(paScroll      );

            paMainPanel.add(paSplit);
        }
        else
        {
            paMainPanel.add(paScroll);
        }

        // Register this new view on our global view container.
        msName = FunctionHelper.getUniqueFrameName();
        this.setTitle(msName);
        ViewContainer.getGlobalContainer().addView(this);
        ViewContainer.getGlobalContainer().addListener(this);
        // be listener for closing the application
        this.enableEvents(AWTEvent.WINDOW_EVENT_MASK);
    }

    // ____________________

    /**
     * Create the view frame for showing the office documents on demand.
     * Dependend from given command line parameter we create
     * an office XFrame and initialize it with a window. This
     * window can be a pure toolkit window (means toolkit of office!)
     * or a plugged java canvas - office window combination.
     */
    public void createFrame()
    {
        // create view frame (as a XFrame!) here
        // Look for right view mode setted by user command line parameter.
        // First try to get a new unambigous frame name from our global ViewContainer.
        if(ViewContainer.mbInplace==true)
        {
            // inplace document view can't be initialized without a visible parent window hierarchy!
            // So make shure that we are visible in every case!
            this.setVisible(true);
            mxFrame = FunctionHelper.createViewFrame(msName,maDocumentView);
        }
        else
            mxFrame = FunctionHelper.createViewFrame(msName,null);

        if(mxFrame!=null)
        {
            // start interception
            maInterceptor = new Interceptor(mxFrame);
            maInterceptor.startListening();

            // start listening for status events and actualization
            // of our status view
            // (of course for our CustomizeView too)
            maStatusView.setFrame   (mxFrame);
            maCustomizeView.setFrame(mxFrame);

            // be listener for closing the remote target view frame
            com.sun.star.lang.XComponent xBroadcaster = (com.sun.star.lang.XComponent)UnoRuntime.queryInterface(
                com.sun.star.lang.XComponent.class,
                mxFrame);

            if(xBroadcaster!=null)
                xBroadcaster.addEventListener(this);
        }
    }

    // ____________________

    /**
     * Different ways to load any URL from outside (may be by the command line)
     * into this document view or to save it.
     */
    public void load(String sURL)
    {
        load(sURL,new com.sun.star.beans.PropertyValue[0]);
    }

    // ____________________

    public void load(String sURL, com.sun.star.beans.PropertyValue[] lArguments)
    {
        com.sun.star.lang.XComponent xDocument = FunctionHelper.loadDocument(mxFrame,sURL,lArguments);
        if(xDocument!=null)
        {
            mbtSave.setEnabled  (true);
            mbtExport.setEnabled(true);
        }
        else
        {
            mbtSave.setEnabled  (false);
            mbtExport.setEnabled(false);
        }
    }

    // ____________________

    public void save()
    {
        com.sun.star.frame.XController xController = mxFrame.getController();
        if (xController==null)
            return;
        com.sun.star.frame.XModel xDocument = xController.getModel();
        if (xDocument==null)
            return;
        FunctionHelper.saveDocument(xDocument);
    }

    // ____________________

    public void exportHTML(String sURL)
    {
        com.sun.star.frame.XController xController = mxFrame.getController();
        if (xController==null)
            return;
        com.sun.star.frame.XModel xDocument = xController.getModel();
        if (xDocument==null)
            return;
        FunctionHelper.saveAsHTML(xDocument,sURL);
    }

    // ____________________

    /**
     * Overridden so we can react for window closing of this view.
     */
    protected void processWindowEvent(WindowEvent aEvent)
    {
        if (aEvent.getID()!=WindowEvent.WINDOW_CLOSING)
        {
            super.processWindowEvent(aEvent);
        }
        else
        if (FunctionHelper.closeFrame(mxFrame))
        {
            mxFrame = null;
            shutdown();
            super.processWindowEvent(aEvent);
        }
    }

    // ____________________

    /**
     * Here we can react for System.exit() normaly.
     * But we use it for disposing() or windowClosing() too.
     */
    public void shutdown()
    {
        if (mbDead)
            return;
        mbDead=true;

        // force these sub view to release her remote
        // refrences too!
        maStatusView.shutdown();
        maCustomizeView.shutdown();

        maStatusView    = null;
        maCustomizeView = null;

        // disable all interceptions
        maInterceptor.shutdown();
        maInterceptor = null;

        // close the frame and his document
        // Relaesing of our listener connections for disposing()
        // will be forced automaticly then. Because the frame
        // will call us back ...
        if (mxFrame!=null)
            FunctionHelper.closeFrame(mxFrame);

        // deregister this view in the global container
        // Normaly we should die afterwards by garbage collection ...
        // In cease this was the last view - it force a system.exit().
        // But then we are no longer a member of the global container
        // of possible shutdown listener ... and this method should be
        // called again.
        ViewContainer.getGlobalContainer().removeView(this);
    }

    // ____________________

    /**
     * callback from our internal saved frame
     * which wish to die. Its not neccessary to remove listener connections
     * here. Because the broadcaster do it automaticly.
     * We have to release all references to him only.
     *
     * @param aSource
     *          describe the broadcaster of this event
     *          Must be our internal saved frame.
     */
    public void disposing(com.sun.star.lang.EventObject aSource)
    {
        mxFrame = null;
    }

    // ____________________

    /**
     * This inner class is used to react for events of our own UI controls.
     * So we can start different actions then.
     */
    private class Reactor implements ActionListener
    {
        // ____________________

        /**
        * This method react for pressed buttons or selected check boxes.
        */
        public void actionPerformed(ActionEvent aEvent)
        {
            String sCommand = aEvent.getActionCommand();
            //-----------------------------
            // open any file from disk
            if( sCommand.compareTo(COMMAND_OPEN) == 0 )
            {
                String sURL = FunctionHelper.askUserForFileURL(DocumentView.this,true);
                if(sURL!=null)
                    DocumentView.this.load(sURL);
            }
            else
            //-----------------------------
            // save current document
            if( sCommand.compareTo(COMMAND_SAVE) == 0 )
            {
                DocumentView.this.save();
            }
            else
            //-----------------------------
            // export current document to html
            if( sCommand.compareTo(COMMAND_EXPORT) == 0 )
            {
                String sURL = FunctionHelper.askUserForFileURL(DocumentView.this,false);
                if(sURL!=null)
                    DocumentView.this.exportHTML(sURL);
            }
            else
            //-----------------------------
            // exit application
            if( sCommand.compareTo(COMMAND_EXIT) == 0 )
            {
                // This will force deleting of this and
                // all other currently opened views automaticly!
                System.exit(0);
            }
        }
    }
}
