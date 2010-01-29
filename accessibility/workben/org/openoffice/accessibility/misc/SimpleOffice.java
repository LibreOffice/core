package org.openoffice.accessibility.misc;

import java.lang.Thread;

import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XChild;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XEnumeration;

import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XTasksSupplier;
import com.sun.star.frame.XTask;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XServiceName;
import com.sun.star.lang.XTypeProvider;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.Type;

import com.sun.star.drawing.XDrawView;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XShapes;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapeDescriptor;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleRelationSet;
import com.sun.star.accessibility.XAccessibleStateSet;


/** This singleton class tries to simplify some tasks like loading a document
    or getting various objects.
*/
public class SimpleOffice
{
    synchronized static public SimpleOffice Instance ()
    {
        if (saInstance == null)
            saInstance = new SimpleOffice ();

        return saInstance;
    }

    synchronized static public void Clear ()
    {
        saInstance = null;
    }


    public XModel LoadDocument (String URL)
    {
        XModel xModel = null;
        try
        {
            //  Load the document from the specified URL.
            XComponentLoader xLoader =
                (XComponentLoader)UnoRuntime.queryInterface(
                    XComponentLoader.class, mxDesktop);

            XComponent xComponent = xLoader.loadComponentFromURL (
                URL,
                "_blank",
                0,
                new PropertyValue[0]
                );

            xModel = (XModel) UnoRuntime.queryInterface(
                XModel.class, xComponent);
        }
        catch (java.lang.NullPointerException e)
        {
            MessageArea.println ("caught exception while loading "
                + URL + " : " + e);
        }
        catch (Exception e)
        {
            MessageArea.println ("caught exception while loading "
                + URL + " : " + e);
        }
        return xModel;
    }




    public XModel GetModel (String name)
    {
        XModel xModel = null;
        try
        {
            XTasksSupplier xTasksSupplier =
                (XTasksSupplier) UnoRuntime.queryInterface(
                    XTasksSupplier.class, mxDesktop);
            XEnumerationAccess xEA = xTasksSupplier.getTasks();
            XEnumeration xE = xEA.createEnumeration();
            while (xE.hasMoreElements())
            {
                XTask xTask = (XTask) UnoRuntime.queryInterface(
                    XTask.class, xE.nextElement());
                MessageArea.print (xTask.getName());
            }
        }
        catch (Exception e)
        {
            MessageArea.println ("caught exception while getting Model " + name
                + ": " + e);
        }
        return xModel;
    }


    public XModel GetModel (XDrawView xView)
    {
        XController xController = (XController) UnoRuntime.queryInterface(
            XController.class, xView);
        if (xController != null)
            return xController.getModel();
        else
        {
            MessageArea.println ("can't cast view to controller");
            return null;
        }
    }




    public  XDesktop GetDesktop ()
    {
        if (mxDesktop != null)
            return mxDesktop;
        try
        {
            //  Get the factory of the connected office.
            XMultiServiceFactory xMSF =
                    OfficeConnection.Instance().GetServiceManager ();
            if (xMSF == null)
            {
                MessageArea.println ("can't connect to office");
                return null;
            }
            else
                MessageArea.println ("Connected successfully.");

            //  Create a new desktop.
            mxDesktop = (XDesktop) UnoRuntime.queryInterface(
                XDesktop.class,
                xMSF.createInstance ("com.sun.star.frame.Desktop")
                );
        }
        catch (Exception e)
        {
            MessageArea.println ("caught exception while creating desktop: "
                + e);
        }

        return mxDesktop;
    }


    /** Return a reference to the extended toolkit which is a broadcaster of
        top window, key, and focus events.
    */
    public XExtendedToolkit GetExtendedToolkit ()
    {
        XExtendedToolkit xToolkit = null;
        if (this != null)
            try
            {
                //  Get the factory of the connected office.
                XMultiServiceFactory xMSF =
                        OfficeConnection.Instance().GetServiceManager ();
                if (xMSF != null)
                {
                    xToolkit = (XExtendedToolkit) UnoRuntime.queryInterface(
                        XExtendedToolkit.class,
                        xMSF.createInstance ("stardiv.Toolkit.VCLXToolkit")
                        );
                }
            }
            catch (Exception e)
            {
                MessageArea.println (
                    "caught exception while creating extended toolkit: " + e);
            }

        return xToolkit;
    }



    static public XAccessible GetAccessibleObject (XInterface xObject)
    {
        XAccessible xAccessible = null;
        try
        {
            xAccessible = (XAccessible) UnoRuntime.queryInterface(
                XAccessible.class, xObject);
        }
        catch (Exception e)
        {
            System.err.println (
                "caught exception while getting accessible object" + e);
            e.printStackTrace (System.err);
        }
        return xAccessible;
    }

    static public XAccessibleContext GetAccessibleContext (XInterface xObject)
    {
        XAccessible xAccessible = GetAccessibleObject (xObject);
        if (xAccessible != null)
            return xAccessible.getAccessibleContext();
        else
            return null;
    }

    /** Return the root object of the accessibility hierarchy.
    */
    public XAccessible GetAccessibleRoot (XAccessible xAccessible)
    {
        try
        {
            XAccessible xParent = null;
            do
            {
                XAccessibleContext xContext = xAccessible.getAccessibleContext();
                if (xContext != null)
                    xParent = xContext.getAccessibleParent();
                if (xParent != null)
                    xAccessible = xParent;
            }
            while (xParent != null);
        }
        catch (Exception e)
        {
            MessageArea.println (
                "caught exception while getting accessible root" + e);
            e.printStackTrace();
        }
        return xAccessible;
    }




    /** @descr Return the current window associated with the given
                model.
    */
    public XWindow GetCurrentWindow ()
    {
        return GetCurrentWindow ((XModel) UnoRuntime.queryInterface(
                XModel.class, GetDesktop()));
    }





    public XWindow GetCurrentWindow (XModel xModel)
    {
        XWindow xWindow = null;
        try
        {
            if (xModel == null)
                MessageArea.println ("invalid model (==null)");
            XController xController = xModel.getCurrentController();
            if (xController == null)
                MessageArea.println ("can't get controller from model");
            XFrame xFrame = xController.getFrame();
            if (xFrame == null)
                MessageArea.println ("can't get frame from controller");
            xWindow = xFrame.getComponentWindow ();
            if (xWindow == null)
                MessageArea.println ("can't get window from frame");
        }
        catch (Exception e)
        {
            MessageArea.println ("caught exception while getting current window" + e);
        }

        return xWindow;
    }


    /** @descr Return the current draw page of the given desktop.
    */
    public XDrawPage GetCurrentDrawPage ()
    {
        return GetCurrentDrawPage (
            (XDrawView) UnoRuntime.queryInterface(
                XDrawView.class,
                GetCurrentView()));
    }




    public XDrawPage GetCurrentDrawPage (XDrawView xView)
    {
        XDrawPage xPage = null;
        try
        {
            if (xView == null)
                MessageArea.println ("can't get current draw page from null view");
            else
                xPage = xView.getCurrentPage();
        }
        catch (Exception e)
        {
            MessageArea.println ("caught exception while getting current draw page : " + e);
        }

        return xPage;
    }




    /** @descr Return the current view of the given desktop.
    */
    public XDrawView GetCurrentView ()
    {
        return GetCurrentView (GetDesktop());
    }

    public XDrawView GetCurrentView (XDesktop xDesktop)
    {
        if (xDesktop == null)
            MessageArea.println ("can't get desktop to retrieve current view");

        XDrawView xView = null;
        try
        {
            XComponent xComponent = xDesktop.getCurrentComponent();
            if (xComponent == null)
                MessageArea.println ("can't get component to retrieve current view");

            XFrame xFrame = xDesktop.getCurrentFrame();
            if (xFrame == null)
                MessageArea.println ("can't get frame to retrieve current view");

            XController xController = xFrame.getController();
            if (xController == null)
                MessageArea.println ("can't get controller to retrieve current view");

            xView = (XDrawView) UnoRuntime.queryInterface(
                XDrawView.class, xController);
            if (xView == null)
                MessageArea.println ("could not cast controller into view");
        }
        catch (Exception e)
        {
            MessageArea.println ("caught exception while getting current view : " + e);
        }

        return xView;
    }




    //  Return the accessible object of the document window.
    public static XAccessible GetAccessibleDocumentWindow (XDrawPage xPage)
    {
        XIndexAccess xShapeList = (XIndexAccess) UnoRuntime.queryInterface(
            XIndexAccess.class, xPage);
        if (xShapeList.getCount() > 0)
        {
            // All shapes return as accessible object the document window's
            // accessible object.  This is, of course, a hack and will be
            // removed as soon as the missing infrastructure for obtaining
            // the object directly is implemented.
            XShape xShape = null;
            try{
                xShape = (XShape) UnoRuntime.queryInterface(
                    XShape.class, xShapeList.getByIndex (0));
            } catch (Exception e)
            {}
            XAccessible xAccessible = (XAccessible) UnoRuntime.queryInterface (
                XAccessible.class, xShape);
            return xAccessible;
        }
        else
            return null;
    }

    private SimpleOffice ()
    {
    }



    private XDesktop mxDesktop;
    private static SimpleOffice saInstance = null;
}
