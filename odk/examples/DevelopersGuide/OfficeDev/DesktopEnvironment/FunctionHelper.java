/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
import com.sun.star.uno.AnyConverter;

import java.awt.*;
import javax.swing.*;
import java.io.*;
import java.net.*;

// __________ Implementation __________

/**
 * Is a collection of basic features.
 * This helper shows different functionality of framework api
 * in an example manner. You can use the follow ones:
 *      (1) parse URL's
 *      (2) create frames (inside/outside a java application)
 *      (3) dispatches (with[out] notifications)
 *      (4) loading/saving documents
 *      (5) convert documents to HTML (if possible)
 *      (6) close documents (and her frames) correctly
 *
 * There exist some other helper functionality too, which
 * doesn't use or demonstrate the office api:
 *      (a) getting file names by using a file chosser
 */
public class FunctionHelper
{


    /**
     * This convert a URL (formatted as a string) to a struct com.sun.star.util.URL.
     * It use a special service to do that: the URLTransformer.
     * Because some API calls need it and it's not allowed to set "Complete"
     * part of the util struct only. The URL must be parsed.
     *
     * @param sURL
     *          URL for parsing in string notation
     *
     * @return [com.sun.star.util.URL]
     *              URL in UNO struct notation
     */
    public static com.sun.star.util.URL parseURL(String sURL)
    {
        com.sun.star.util.URL aURL = null;

        if (sURL==null || sURL.equals(""))
        {
            System.out.println("wrong using of URL parser");
            return null;
        }

        try
        {
            com.sun.star.uno.XComponentContext xOfficeCtx =
                OfficeConnect.getOfficeContext();

            // Create special service for parsing of given URL.
            com.sun.star.util.XURLTransformer xParser =
                UnoRuntime.queryInterface(
                com.sun.star.util.XURLTransformer.class,
                xOfficeCtx.getServiceManager().createInstanceWithContext(
                    "com.sun.star.util.URLTransformer", xOfficeCtx));

            // Because it's an in/out parameter we must use an array of URL objects.
            com.sun.star.util.URL[] aParseURL = new com.sun.star.util.URL[1];
            aParseURL[0] = new com.sun.star.util.URL();
            aParseURL[0].Complete = sURL;

            // Parse the URL
            xParser.parseStrict(aParseURL);

            aURL = aParseURL[0];
        }
        catch(com.sun.star.uno.RuntimeException exRuntime)
        {
            // Any UNO method of this scope can throw this exception.
            // Reset the return value only.
        }
        catch(com.sun.star.uno.Exception exUno)
        {
            // "createInstance()" method of used service manager can throw it.
            // Then it wasn't possible to get the URL transformer.
            // Return default instead of really parsed URL.
        }

        return aURL;
    }



    /**
     * create a new empty target frame
     * Attention: Currently we must use special service com.sun.star.frame.Task instead of Frame.
     * Because desktop environment accept this special frame type only as direct children.
     * Note - This service will be deprecated and must be replaces by com.sun.star.frame.Frame in
     * further versions. To feature prove we use both service names. If for new versions
     * the deprecated one not exist we get an empty frame, we can try to use the new service.
     *
     * @param xSMGR
     *          we nee the remote service manager to create this task/frame service
     *
     * @return [com.sun.star.frame.XFrame]
     *          the new created frame reference in case of success or null otherwise
     */
    private static com.sun.star.frame.XFrame impl_createEmptyFrame(
        com.sun.star.uno.XComponentContext xCtx )
    {
        com.sun.star.frame.XFrame xFrame = null;

        try{
            xFrame = UnoRuntime.queryInterface(
                com.sun.star.frame.XFrame.class,
                xCtx.getServiceManager().createInstanceWithContext(
                    "com.sun.star.frame.Task", xCtx));
        } catch(com.sun.star.uno.Exception ex1) {}

        if (xFrame==null)
        {
            try{
                xFrame = UnoRuntime.queryInterface(
                    com.sun.star.frame.XFrame.class,
                    xCtx.getServiceManager().createInstanceWithContext(
                        "com.sun.star.frame.Frame", xCtx));
            } catch(com.sun.star.uno.Exception ex2) {}
        }

        return xFrame;
    }



    /**
     * create a new window which can be used as container window of an office frame
     * We know two modes for creation:
     *   - the office window will be a child of one of our java windows
     *   - the office will be a normal system window outside this java application
     * This behaviour will be regulated by the second parameter of this operation.
     * If a parentview is given the first mode will be activated - otherwise
     * the second one.
     *
     * Note: First mode (creation of a child window) can be reached by two different
     *       ways.
     *   - pack the required window handle of our java window inside a UNO object
     *     to transport it to the remote office toolkit and get a child office
     *     window.
     *     This is the old way. It's better to use the second one - but to be
     *     future prove this old one should be tried too.
     *   - it's possible to pass the native window handle directly to the toolkit.
     *     A special interface method was enabled to accept that.
     *
     *   The right way to create an office window should be then:
     *   - try to use second creation mode (directly using of the window handle)
     *   - if it failed ... use the old way by packing the handle inside an object
     *
     * @param xSMGR
     *          we need a service manager to be able to create remote office
     *          services
     *
     * @param aParentView
     *          the java window as parent for the office window if an inplace office
     *          is required. If it is set to null the created office window will be
     *          a normal system window outside of our java application.
     *
     * @return [com.sun.star.awt.XWindow]
     *          The new created office window which can be used to set it as
     *          a ContainerWindow on an empty office frame.
     */
    private static com.sun.star.awt.XWindow impl_createWindow(
        com.sun.star.uno.XComponentContext xCtx, NativeView aParentView )
    {
        com.sun.star.awt.XWindow     xWindow  = null;
        com.sun.star.awt.XWindowPeer xPeer    = null;
        com.sun.star.awt.XToolkit    xToolkit = null;

        // get access to toolkit of remote office to create the container window of
        // new target frame
        try{
            xToolkit = UnoRuntime.queryInterface(
                com.sun.star.awt.XToolkit.class,
                xCtx.getServiceManager().createInstanceWithContext(
                    "com.sun.star.awt.Toolkit", xCtx));
        }
        catch(com.sun.star.uno.Exception ex)
        {
            return null;
        }

        // mode 1) create an external system window
        if (aParentView==null)
        {
            // Describe the properties of the container window.
            com.sun.star.awt.WindowDescriptor aDescriptor =
                new com.sun.star.awt.WindowDescriptor();
            aDescriptor.Type =   com.sun.star.awt.WindowClass.TOP;
            aDescriptor.WindowServiceName = "window";
            aDescriptor.ParentIndex = -1;
            aDescriptor.Parent = null;
            aDescriptor.Bounds = new com.sun.star.awt.Rectangle(0,0,0,0);
            aDescriptor.WindowAttributes = com.sun.star.awt.WindowAttribute.BORDER |
                com.sun.star.awt.WindowAttribute.MOVEABLE |
                com.sun.star.awt.WindowAttribute.SIZEABLE |
                com.sun.star.awt.WindowAttribute.CLOSEABLE;

            try{
                xPeer = xToolkit.createWindow( aDescriptor );
            } catch(com.sun.star.lang.IllegalArgumentException exIllegal) {}
        }
        // mode 2) create an internal office window as child of our given java
        // parent window
        else
        {
            // try new version of creation first: directly using of the window
            // handle. The old implementation of the corresponding toolkit method
            // requires a process ID. If this id isn't the right one a null object
            // is returned. But normally nobody outside the office knows this id.
            // New version of this method ignore the id parameter and creation will
            // work.
            // Note: You must be sure if your window handle can be really used by
            // the remote office. Means if this java client and the remote office
            // use the same display!
            com.sun.star.awt.XSystemChildFactory xChildFactory =
                UnoRuntime.queryInterface(
                com.sun.star.awt.XSystemChildFactory.class, xToolkit);

            try
            {
                Integer nHandle = aParentView.getHWND();
                short   nSystem = (short)aParentView.getNativeWindowSystemType();
                byte[]  lProcID = new byte[0];

                xPeer = xChildFactory.createSystemChild(nHandle,
                                                        lProcID, nSystem);

                if (xPeer==null)
                {
                    // mode 3) OK - new version doesn't work. It requires the
                    // process id which we don't have.
                    // So we must use the old way to get the right window peer.
                    // Pack the handle inside a wrapper object.
                    JavaWindowPeerFake aWrapper = new
                        JavaWindowPeerFake(aParentView);

                    com.sun.star.awt.XWindowPeer xParentPeer =
                        UnoRuntime.queryInterface(
                        com.sun.star.awt.XWindowPeer.class, aWrapper);

                    com.sun.star.awt.WindowDescriptor aDescriptor =
                        new com.sun.star.awt.WindowDescriptor();
                    aDescriptor.Type = com.sun.star.awt.WindowClass.TOP;
                    aDescriptor.WindowServiceName = "workwindow";
                    aDescriptor.ParentIndex = 1;
                    aDescriptor.Parent = xParentPeer;
                    aDescriptor.Bounds = new com.sun.star.awt.Rectangle(0,0,0,0);
                    if (nSystem == com.sun.star.lang.SystemDependent.SYSTEM_WIN32)
                        aDescriptor.WindowAttributes =
                            com.sun.star.awt.WindowAttribute.SHOW;
                    else
                        aDescriptor.WindowAttributes =
                            com.sun.star.awt.WindowAttribute.SYSTEMDEPENDENT;

                    try{
                        xPeer = xToolkit.createWindow( aDescriptor );
                    } catch(com.sun.star.lang.IllegalArgumentException exIllegal) {}
                }
            }
            catch(java.lang.RuntimeException exJRun)
            {
                // This exception is thrown by the native JNI code if it try to get
                // the systemw window handle. A possible reason can be an invisible
                // java window. In this case it should be enough to set return
                // values to null. All other resources (which was created before)
                // will be freed automatically if scope will be leaved.
                System.out.println("May be the NativeView object wasn't really visible at calling time of getNativeWindow()?");
                xPeer   = null;
                xWindow = null;
            }
        }

        // It doesn't matter which way was used to get the window peer.
        // Cast it to the right return interface and return it.
        xWindow = UnoRuntime.queryInterface(
            com.sun.star.awt.XWindow.class,
            xPeer);

        return xWindow;
    }



    /**
     * This method create a new empty child frame on desktop instance of remote office.
     * It use a special JNI functionality to pass the office XWindow over a java window.
     * This java window can be inserted into another java window container for complex layouting.
     * If this parent java window isn't used, a top level system window will be created.
     * The resulting office frame isn't plugged into this java application.
     *
     * @param sName
     *          name to set it on the new created frame
     *
     * @param aParentView
     *          java window which should be used as parent window of new created office frame window
     *          May be set to null.
     *
     * @return [com.sun.star.frame.XFrame]
     *          reference to the new created frame for success or null if it failed
     */
    public static com.sun.star.frame.XFrame createViewFrame(String sName, NativeView aParentView)
    {
        com.sun.star.frame.XFrame xFrame = null;

        try
        {
            com.sun.star.uno.XComponentContext xCtx =
                OfficeConnect.getOfficeContext();

            // create an empty office frame first
            xFrame = impl_createEmptyFrame(xCtx);

            // create an office window then
            // Depending from the given parameter aParentView it will be a child or a top level
            // system window. (see impl method for further information)
            // But before we call this helper - prepare the possible parent window: show it.
            // JNI calls to get system window handle of java window can't work without that!
            if (aParentView!=null)
                aParentView.setVisible(true);
            com.sun.star.awt.XWindow xWindow = impl_createWindow(xCtx, aParentView);

            // pass the window the frame as his new container window.
            // It's necessary to do it first  - before you call anything else there.
            // Otherwise the frame throws some exceptions for "uninitialized state".
            xFrame.initialize( xWindow );

            // Insert the new frame in desktop hierarchy.
            // Use XFrames interface to do so. It provides access to the child frame container of that instance.
            com.sun.star.frame.XFramesSupplier xTreeRoot = UnoRuntime.queryInterface(
                com.sun.star.frame.XFramesSupplier.class,
                xCtx.getServiceManager().createInstanceWithContext(
                    "com.sun.star.frame.Desktop", xCtx));
            com.sun.star.frame.XFrames xChildContainer = xTreeRoot.getFrames();
            xChildContainer.append(xFrame);

            // Make some further initializations on frame and window.
            xWindow.setVisible(true);
            xFrame.setName(sName);
        }
        catch(com.sun.star.uno.RuntimeException exRuntime)
        {
            // Any UNO method of this scope can throw this exception.
            // So the frame can be already created and he must be freed
            // correctly. May be he was inserted into the desktop tree too ...
            if(xFrame!=null)
            {
                // Try to dispose the frame. He should deregister himself at the desktop object
                // and free all internal used resources (e.g. the container window) automatically.
                // It's possible to do that here - because frame has no component inside yet.
                // So nobody can disagree with that.
                // After the dispose() call forget all references to this frame and let him die.
                // If a new exception will occur ... no generell solution exist then.
                // Nobody can guarantee if next call will work or not.
                com.sun.star.lang.XComponent xComponent = UnoRuntime.queryInterface(
                    com.sun.star.lang.XComponent.class,
                    xFrame);
                xComponent.dispose();
                xComponent = null;
                xFrame     = null;
            }
        }
        catch(com.sun.star.uno.Exception exUno)
        {
            // "createInstance()" method of used service manager can throw it.
            // If it occurred during creation of desktop service the frame already was created.
            // Free it by decreasing his refcount. Changes on the desktop tree couldn't exist.
            // Without the desktop service that wasn't possible. So no further rollbacks must follow.
            if(xFrame!=null)
            {
                com.sun.star.lang.XComponent xComponent = UnoRuntime.queryInterface(
                    com.sun.star.lang.XComponent.class,
                    xFrame);
                xComponent.dispose();
                xComponent = null;
                xFrame     = null;
            }
        }

        return xFrame;
    }



    /**
     * Dispatch a URL to given frame.
     * Caller can register himself for following status events for dispatched
     * URL too. But nobody guarantee that such notifications will occur.
     * (see dispatchWithNotification() if you interest on that)
     * The returned dispatch object should be hold alive by caller
     * till he doesn't need it any longer. Otherwise the dispatcher can(!)
     * die by decreasing his refcount.
     *
     * @param   xFrame      frame which should be the target of this dispatch
     * @param   aURL        full parsed and converted office URL for dispatch
     * @param   lProperties optional arguments for dispatch
     * @param   xListener   optional listener which is registered automatically for status events
     *                      (Note: Deregistration is part of this listener himself!)
     *
     * @return  [XDispatch] It's the used dispatch object and can be used for deregistration of an optional listener.
     *                      Otherwise caller can ignore it.
     */
    public static com.sun.star.frame.XDispatch execute(com.sun.star.frame.XFrame          xFrame     ,
                                                       com.sun.star.util.URL              aURL       ,
                                                       com.sun.star.beans.PropertyValue[] lProperties,
                                                       com.sun.star.frame.XStatusListener xListener  )
    {
        com.sun.star.frame.XDispatch xDispatcher = null;

        try
        {
            // Query the frame for right interface which provides access to all available dispatch objects.
            com.sun.star.frame.XDispatchProvider xProvider = UnoRuntime.queryInterface(
                com.sun.star.frame.XDispatchProvider.class,
                xFrame);

            // Ask him for right dispatch object for given URL.
            // Force given frame as target for following dispatch by using "".
            // It means the same like "_self".
            xDispatcher = xProvider.queryDispatch(aURL,"",0);

            // Dispatch the URL into the frame.
            if(xDispatcher!=null)
            {
                if(xListener!=null)
                    xDispatcher.addStatusListener(xListener,aURL);

                xDispatcher.dispatch(aURL,lProperties);
            }
        }
        catch(com.sun.star.uno.RuntimeException exUno)
        {
            // Any UNO method of this scope can throw this exception.
            // But there will be nothing to do then - because
            // we haven't changed anything inside the remote objects
            // except method "addStatusListener().
            // But in this case the source of this exception has to
            // rollback all his operations. There is no chance to
            // make anything right then.
            // Reset the return value to a default - that's it.
            exUno.printStackTrace();
            xDispatcher = null;
        }

        return xDispatcher;
    }







    /**
     * Load document specified by a URL into given frame synchronously.
     * The result of this operation will be the loaded document for success
     * or null if loading failed.
     *
     * @param   xFrame          frame which should be the target of this load call
     * @param   sURL            unparsed URL for loading
     * @param   lProperties     optional arguments
     *
     * @return  [XComponent]    the loaded document for success or null if it's failed
     */
    public static com.sun.star.lang.XComponent loadDocument(
        com.sun.star.frame.XFrame xFrame, String sURL,
        com.sun.star.beans.PropertyValue[] lProperties)
    {
        com.sun.star.lang.XComponent xDocument = null;
        String                       sOldName  = null;

        try
        {
            com.sun.star.uno.XComponentContext xCtx =
                OfficeConnect.getOfficeContext();

            // First prepare frame for loading
            // We must address it inside the frame tree without any complications.
            // So we set an unambigous (we hope it) name and use it later.
            // Don't forget to reset original name after that.
                   sOldName = xFrame.getName();
            String sTarget  = "odk_officedev_desk";
            xFrame.setName(sTarget);

            // Get access to the global component loader of the office
            // for synchronous loading the document.
            com.sun.star.frame.XComponentLoader xLoader =
                UnoRuntime.queryInterface(
                com.sun.star.frame.XComponentLoader.class,
                xCtx.getServiceManager().createInstanceWithContext(
                    "com.sun.star.frame.Desktop", xCtx));

            // Load the document into the target frame by using his name and
            // special search flags.
            xDocument = xLoader.loadComponentFromURL(
                sURL,
                sTarget,
                com.sun.star.frame.FrameSearchFlag.CHILDREN,
                lProperties);

            // don't forget to restore old frame name...
            xFrame.setName(sOldName);
        }
        catch(com.sun.star.io.IOException exIO)
        {
            // Can be thrown by "loadComponentFromURL()" call.
            // The only thing we should do then is to reset changed frame name!
            exIO.printStackTrace();
            xDocument = null;
            if(sOldName!=null)
                xFrame.setName(sOldName);
        }
        catch(com.sun.star.lang.IllegalArgumentException exIllegal)
        {
            // Can be thrown by "loadComponentFromURL()" call.
            // The only thing we should do then is to reset changed frame name!
            exIllegal.printStackTrace();
            xDocument = null;
            if(sOldName!=null)
                xFrame.setName(sOldName);
        }
        catch(com.sun.star.uno.RuntimeException exRuntime)
        {
            // Any UNO method of this scope can throw this exception.
            // The only thing we can try(!) is to reset changed frame name.
            exRuntime.printStackTrace();
            xDocument = null;
            if(sOldName!=null)
                xFrame.setName(sOldName);
        }
        catch(com.sun.star.uno.Exception exUno)
        {
            // "createInstance()" method of used service manager can throw it.
            // The only thing we should do then is to reset changed frame name!
            exUno.printStackTrace();
            xDocument = null;
            if(sOldName!=null)
                xFrame.setName(sOldName);
        }

        return xDocument;
    }



    /**
     * Save currently loaded document of given frame.
     *
     * @param   xDocument   document for saving changes
     */
    public static void saveDocument(com.sun.star.lang.XComponent xDocument)
    {
        try
        {
            // Check for supported model functionality.
            // Normally the application documents (text, spreadsheet ...) do so
            // but some other ones (e.g. db components) doesn't do that.
            // They can't be save then.
            com.sun.star.frame.XModel xModel = UnoRuntime.queryInterface(
                com.sun.star.frame.XModel.class,
                xDocument);
            if(xModel!=null)
            {
                // Check for modifications => break save process if there is nothing to do.
                com.sun.star.util.XModifiable xModified = UnoRuntime.queryInterface(
                    com.sun.star.util.XModifiable.class,
                    xModel);
                if(xModified.isModified())
                {
                    com.sun.star.frame.XStorable xStore = UnoRuntime.queryInterface(
                        com.sun.star.frame.XStorable.class,
                        xModel);

                    xStore.store();
                }
            }
        }
        catch(com.sun.star.io.IOException exIO)
        {
            // Can be thrown by "store()" call.
            // But there is nothing we can do then.
            exIO.printStackTrace();
        }
        catch(com.sun.star.uno.RuntimeException exUno)
        {
            // Any UNO method of this scope can throw this exception.
            // But there is nothing we can do then.
            exUno.printStackTrace();
        }
    }



    /**
     * It try to export given document in HTML format.
     * Current document will be converted to HTML and moved to new place on disk.
     * A "new" file will be created by given URL (may be overwritten
     * if it already exist). Right filter will be used automatically if factory of
     * this document support it. If no valid filter can be found for export,
     * nothing will be done here.
     *
     * @param   xDocument   document which should be exported
     * @param   sURL        target URL for converted document
     */
    public static void saveAsHTML(com.sun.star.lang.XComponent xDocument,
                                  String                       sURL     )
    {
        try
        {
            // First detect factory of this document.
            // Ask for the supported service name of this document.
            // If information is available it can be used to find out which
            // filter exist for HTML export. Normally this filter should be searched
            // inside the filter configuration but this little demo doesn't do so.
            // (see service com.sun.star.document.FilterFactory for further
            // information too)
            // Well known filter names are used directly. They must exist in current
            // office installation. Otherwise this code will fail. But to prevent
            // this code against missing filters it check for existing state of it.
            com.sun.star.lang.XServiceInfo xInfo = UnoRuntime.queryInterface(com.sun.star.lang.XServiceInfo.class,
                                      xDocument);

            if(xInfo!=null)
            {
                // Find out possible filter name.
                String sFilter = null;
                if(xInfo.supportsService("com.sun.star.text.TextDocument"))
                    sFilter = "HTML (StarWriter)";
                else
                if(xInfo.supportsService("com.sun.star.text.WebDocument"))
                    sFilter = "HTML";
                else
                if(xInfo.supportsService("com.sun.star.sheet.SpreadsheetDocument"))
                    sFilter = "HTML (StarCalc)";

                // Check for existing state of this filter.
                if(sFilter!=null)
                {
                    com.sun.star.uno.XComponentContext xCtx =
                        OfficeConnect.getOfficeContext();

                    com.sun.star.container.XNameAccess xFilterContainer =
                        UnoRuntime.queryInterface(
                        com.sun.star.container.XNameAccess.class,
                        xCtx.getServiceManager().createInstanceWithContext(
                            "com.sun.star.document.FilterFactory", xCtx));

                    if(!xFilterContainer.hasByName(sFilter))
                        sFilter=null;
                }

                // Use this filter for export.
                if(sFilter!=null)
                {
                    // Export can be forced by saving the document and using a
                    // special filter name which can write needed format. Build
                    // necessary argument list now.
                    // Use special flag "Overwrite" too, to prevent operation
                    // against possible exceptions, if file already exist.
                    com.sun.star.beans.PropertyValue[] lProperties =
                        new com.sun.star.beans.PropertyValue[2];
                    lProperties[0] = new com.sun.star.beans.PropertyValue();
                    lProperties[0].Name = "FilterName";
                    lProperties[0].Value = sFilter;
                    lProperties[1] = new com.sun.star.beans.PropertyValue();
                    lProperties[1].Name = "Overwrite";
                    lProperties[1].Value = Boolean.TRUE;

                    com.sun.star.frame.XStorable xStore =
                        UnoRuntime.queryInterface(
                        com.sun.star.frame.XStorable.class, xDocument);

                    xStore.storeAsURL(sURL,lProperties);
                }
            }
        }
        catch(com.sun.star.io.IOException exIO)
        {
            // Can be thrown by "store()" call.
            // Do nothing then. Saving failed - that's it.
            exIO.printStackTrace();
        }
        catch(com.sun.star.uno.RuntimeException exRuntime)
        {
            // Can be thrown by any uno call.
            // Do nothing here. Saving failed - that's it.
            exRuntime.printStackTrace();
        }
        catch(com.sun.star.uno.Exception exUno)
        {
            // Can be thrown by "createInstance()" call of service manager.
            // Do nothing here. Saving failed - that's it.
            exUno.printStackTrace();
        }
    }







    /*
     * Try to close the frame instead of the document.
     * It shows the possible interface to do so.
     *
     * @param xFrame
     *          frame which should be closed
     *
     * @return  <TRUE/> in case frame could be closed
     *          <FALSE/> otherwise
     */
    public static boolean closeFrame(com.sun.star.frame.XFrame xFrame)
    {
        boolean bClosed = false;

        try
        {
            // first try the new way: use new interface XCloseable
            // It replace the deprecated XTask::close() and should be preferred ...
            // if it can be queried.
            com.sun.star.util.XCloseable xCloseable =
                UnoRuntime.queryInterface(
                com.sun.star.util.XCloseable.class, xFrame);
            if (xCloseable!=null)
            {
                // We deliver the ownership of this frame not to the (possible)
                // source which throw a CloseVetoException. We wish to have it
                // under our own control.
                try
                {
                    xCloseable.close(false);
                    bClosed = true;
                }
                catch( com.sun.star.util.CloseVetoException exVeto )
                {
                    bClosed = false;
                }
            }
            else
            {
                // OK: the new way isn't possible. Try the old one.
                com.sun.star.frame.XTask xTask = UnoRuntime.queryInterface(com.sun.star.frame.XTask.class,
                                          xFrame);
                if (xTask!=null)
                {
                    // return value doesn't interest here. Because
                    // we forget this task ...
                    bClosed = xTask.close();
                }
            }
        }
        catch (com.sun.star.lang.DisposedException exDisposed)
        {
            // Of course - this task can be already dead - means disposed.
            // But for us it's not important. Because we tried to close it too.
            // And "already disposed" or "closed" should be the same ...
            bClosed = true;
        }

        return bClosed;
    }



    /**
     * Try to find a unique frame name, which isn't currently used inside
     * remote office instance. Because we create top level frames
     * only, it's enough to check the names of existing child frames on the
     * desktop only.
     *
     *          should represent a unique frame name, which currently isn't
     *          used inside the remote office frame tree
     *          (Couldn't guaranteed for a real multithreaded environment.
     *           But we try it ...)
     */
    private static final String BASEFRAMENAME = "Desk View ";

    public static String getUniqueFrameName()
    {
        String sName = null;

        com.sun.star.uno.XComponentContext xCtx = OfficeConnect.getOfficeContext();

        try
        {
            com.sun.star.frame.XFramesSupplier xSupplier =
                UnoRuntime.queryInterface(
                com.sun.star.frame.XFramesSupplier.class,
                xCtx.getServiceManager().createInstanceWithContext(
                    "com.sun.star.frame.Desktop", xCtx));

            com.sun.star.container.XIndexAccess xContainer =
                UnoRuntime.queryInterface(
                com.sun.star.container.XIndexAccess.class,
                xSupplier.getFrames());

            int nCount = xContainer.getCount();
            for (int i=0; i<nCount; ++i )
            {
                com.sun.star.frame.XFrame xFrame = (com.sun.star.frame.XFrame)AnyConverter.toObject(new com.sun.star.uno.Type(com.sun.star.frame.XFrame.class), xContainer.getByIndex(i));
                                        sName  = BASEFRAMENAME+mnViewCount;
                while(sName.equals(xFrame.getName()))
                {
                    ++mnViewCount;
                    sName = BASEFRAMENAME+mnViewCount;
                }
            }
        }
        catch(com.sun.star.uno.Exception exCreateFailed)
        {
            sName = BASEFRAMENAME;
        }

        if (sName==null)
        {
            System.out.println("invalid name!");
            sName = BASEFRAMENAME;
        }

        return sName;
    }



    /**
     * helper to get a file URL selected by user
     * This method doesn't show any API concepts...
     * but is necessary for this demo application.
     *
     * @param   aParent parent window of this dialog
     * @param   bOpen   If it is set to true =>
     *                  dialog is opened in "file open" mode -
     *                  otherwise in "file save" mode.
     */
    public static String askUserForFileURL(Component aParent,boolean bOpen)
    {
        String        sFileURL  = null;
        int           nDecision = JFileChooser.CANCEL_OPTION;
        JFileChooser  aChooser  = null;

        // set last visited directory on new file chosser
        // (if this information is available)
        if( maLastDir==null )
            aChooser = new JFileChooser();
        else
            aChooser = new JFileChooser(maLastDir);

        // decide between file open/save dialog
        if( bOpen )
            nDecision = aChooser.showOpenDialog(aParent);
        else
            nDecision = aChooser.showSaveDialog(aParent);

        // react for "OK" result only
        if(nDecision == JFileChooser.APPROVE_OPTION)
        {
            // save current directory as last visited one
            maLastDir = aChooser.getCurrentDirectory();
            // get file URL from the dialog
            try
            {
                sFileURL = aChooser.getSelectedFile().toURI().toURL().toExternalForm();
            }
            catch( MalformedURLException ex )
            {
                ex.printStackTrace();
                sFileURL = null;
            }
            // problem of java: file URL's are coded with 1 slash instead of 3 ones!
            // => correct this problem first, otherwise office can't use these URL's
            if(
                ( sFileURL                      !=null  ) &&
                ( sFileURL.startsWith("file:/")  ) &&
                ( !sFileURL.startsWith("file://") )
              )
            {
                StringBuffer sWorkBuffer = new StringBuffer(sFileURL);
                sWorkBuffer.insert(6,"//");
                sFileURL = sWorkBuffer.toString();
            }
        }

        return sFileURL;
    }



    /**
     * @member  maLastDir   save the last visited directory of used file open/save dialog
     * @member  mnViewCount we try to set unique names on every frame we create (that's why we must count it)
     */
    private static File maLastDir   = null;
    private static int  mnViewCount = 0   ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
