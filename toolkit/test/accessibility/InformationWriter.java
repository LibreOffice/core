import java.lang.Thread;

import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XWindow;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XChild;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XEnumeration;

import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
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

import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleComponent;
import drafts.com.sun.star.accessibility.XAccessibleRelationSet;
import drafts.com.sun.star.accessibility.XAccessibleStateSet;

public class InformationWriter
    implements Print
{
    Print maPrinter;

    public InformationWriter ()
    {
        this(null);
    }

    public InformationWriter (Print aPrinter)
    {
        maPrinter = aPrinter;
    }


    public void drawPageTest (XInterface xPage)
    {
        try
        {
            printProperty (xPage, "BorderBottom  ", "BorderBottom");
            printProperty (xPage, "BorderLeft    ", "BorderLeft");
            printProperty (xPage, "BorderRight   ", "BorderRight");
            printProperty (xPage, "BorderTop     ", "BorderTop");
            printProperty (xPage, "Height        ", "Height");
            printProperty (xPage, "Width         ", "Width");
            printProperty (xPage, "Number        ", "Number");
        }
        catch  (Exception e)
        {
            System.out.println ("caught exception while testing draw page:" + e);
        }
    }

    public void printProperty (XInterface xObject, String prefix, String name)
    {
        try
        {
            XPropertySet xPropertySet =  (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xObject);
            println (prefix +
                xPropertySet.getPropertyValue (name));
        }
        catch (Exception e)
        {
            println ("caught exception while getting property "
                + name + " : " + e);
        }
    }



    public void showShapes (XDrawPage xPage)
    {
        try
        {
            XIndexAccess xShapeList = (XIndexAccess) UnoRuntime.queryInterface(
                XIndexAccess.class, xPage);

            println ("There are " + xShapeList.getCount()
                + " shapes");
            for (int i=0; i<xShapeList.getCount(); i++)
            {
                XShape xShape = (XShape) UnoRuntime.queryInterface(
                    XShape.class, xShapeList.getByIndex (i));

                XShapeDescriptor xShapeDescriptor =
                    (XShapeDescriptor) UnoRuntime.queryInterface(
                        XShapeDescriptor.class, xShape);
                String sName = xShapeDescriptor.getShapeType ();
                println ("   shape " + i + " : " + sName);

                XPropertySet xPropertySet =
                    (XPropertySet) UnoRuntime.queryInterface(
                        XPropertySet.class, xShape);
                Integer nZOrder =
                    (Integer) xPropertySet.getPropertyValue ("ZOrder");
                println ("   zorder = " + nZOrder);
            }
        }
        catch (Exception e)
        {
            println ("caught exception in showShapes: " + e);
        }
    }




    /** @descr Print all available services of the given object to the
                standard output.
    */
    public void showServices (XInterface xObject)
    {
        try
        {
            println ("Services:");
            XMultiServiceFactory xMSF = (XMultiServiceFactory) UnoRuntime.queryInterface (
                XMultiServiceFactory.class,
                xObject
                );
            if (xMSF == null)
                println ("    object does not support interface XMultiServiceFactory");
            else
            {
                String[] sServiceNames = xMSF.getAvailableServiceNames ();
                println ("    object can create "
                    + sServiceNames.length + " services");
                for (int i=0; i<sServiceNames.length; i++)
                    println ("        service " + i + " : " + sServiceNames[i]);
            }
        }
        catch (Exception e)
        {
            println ("caught exception in showServices : " + e);
        }
    }

    /** @descr Print the service and implementation name of the given
                object.
    */
    public void showInfo (XInterface xObject)
    {
        try
        {
            System.out.println ("Info:");
            // Use interface XServiceName to retrieve name of (main) service.
            XServiceName xSN = (XServiceName) UnoRuntime.queryInterface (
                XServiceName.class, xObject);
            if (xSN == null)
                println ("    interface XServiceName not supported");
            else
            {
                println ("    Service name        : " + xSN.getServiceName ());
            }

            // Use interface XServiceInfo to retrieve information about
            // supported services.
            XServiceInfo xSI = (XServiceInfo) UnoRuntime.queryInterface (
                XServiceInfo.class, xObject);
            if (xSI == null)
                println ("    interface XServiceInfo not supported");
            else
            {
                println ("    Implementation name : "
                    + xSI.getImplementationName ());
            }
        }
        catch (Exception e)
        {
            println ("caught exception in showInfo : " + e);
        }
    }




    /** @descr Print information about supported interfaces.
    */
    public void showInterfaces (XInterface xObject)
    {
        try
        {
            println ("Interfaces:");
            // Use interface XTypeProvider to retrieve a list of supported
            // interfaces.
            XTypeProvider xTP = (XTypeProvider) UnoRuntime.queryInterface (
                XTypeProvider.class, xObject);
            if (xTP == null)
                println ("    interface XTypeProvider not supported");
            else
            {
                Type[] aTypeList = xTP.getTypes ();
                println ("    object supports " + aTypeList.length
                    + " interfaces");
                for (int i=0; i<aTypeList.length; i++)
                    println ("        " + i + " : "
                        + aTypeList[i].getTypeName());
            }
        }
        catch (Exception e)
        {
            println ("caught exception in showInterfaces : " + e);
        }
    }


    /** @descr Print information concerning the accessibility of the given
        object.
    */
    public boolean showAccessibility (XInterface xObject, int depth)
    {
        try
        {
            // Create indentation string.
            String sIndent = "";
            while (depth-- > 0)
                sIndent += "    ";

            //  Get XAccessibleContext object if given object does not
            //  already support this interface.
            XAccessibleContext xContext
                = (XAccessibleContext) UnoRuntime.queryInterface (
                    XAccessibleContext.class, xObject);
            if (xContext == null)
            {
                XAccessible xAccessible
                    = (XAccessible) UnoRuntime.queryInterface (
                        XAccessible.class, xObject);
                if (xAccessible == null)
                {
                    println (sIndent + "given object " + xObject
                        + " is not accessible");
                    return false;
                }
                else
                    xContext = xAccessible.getAccessibleContext();
            }

            //  Print information about the accessible context.
            if (xContext != null)
            {
                println (sIndent + "Name         : "
                    + xContext.getAccessibleName());
                println (sIndent + "Description  : "
                    + xContext.getAccessibleDescription());
                println (sIndent + "Role         : "
                    + xContext.getAccessibleRole());
                String sHasParent;
                if (xContext.getAccessibleParent() != null)
                {
                    println (sIndent + "Has parent   : yes");
                    println (sIndent + "Parent index : "
                        + xContext.getAccessibleIndexInParent());
                }
                else
                    println (sIndent + "Has parent   : no");
                println (sIndent + "Child count  : "
                    + xContext.getAccessibleChildCount());
                print (sIndent + "Relation set : ");
                XAccessibleRelationSet xRelationSet
                    = xContext.getAccessibleRelationSet();
                if (xRelationSet != null)
                {
                    print (xRelationSet.getRelationCount() + " (");
                    for (int i=0; i<xRelationSet.getRelationCount(); i++)
                    {
                        if (i > 0)
                            print (", ");
                        print (xRelationSet.getRelation(i).toString());
                    }
                    println (")");
                }
                else
                    println ("no relation set");

                print (sIndent + "State set    : ");
                XAccessibleStateSet xStateSet =
                    xContext.getAccessibleStateSet();
                if (xStateSet != null)
                {
                    XIndexAccess xStates =
                        (XIndexAccess) UnoRuntime.queryInterface (
                            XIndexAccess.class, xStateSet);
                    print (xStates.getCount() + " (");
                    for (int i=0; i<xStates.getCount(); i++)
                    {
                        if (i > 0)
                            print (", ");
                        print (xStates.getByIndex(i).toString());
                    }
                    println (")");
                }
                else
                    println ("no state set");

                showAccessibleComponent (xContext, sIndent);
            }
            else
                println ("object has no accessible context.");

            //            showInfo (xContext);
            //            showServices (xContext);
            //            showInterfaces (xContext);
        }
        catch (Exception e)
        {
            System.out.println ("caught exception in showAccessibility :" + e);
        }
        return true;
    }




    /** @descr Print information about the given accessible component.
    */
    public void showAccessibleComponent (XInterface xObject, String sIndent)
    {
        try
        {
            XAccessibleComponent xComponent =
                (XAccessibleComponent) UnoRuntime.queryInterface (
                    XAccessibleComponent.class, xObject);

            //  Print information about the accessible context.
            if (xComponent != null)
            {
                println (sIndent + "Position        : "
                    + xComponent.getLocation().X+", "
                    + xComponent.getLocation().Y);
                println (sIndent + "Screen position : "
                    + xComponent.getLocationOnScreen().X+", "
                    + xComponent.getLocationOnScreen().Y);
                println (sIndent + "Size            : "
                    + xComponent.getSize().Width+", "
                    + xComponent.getSize().Height);
            }
        }
        catch (Exception e)
        {
            System.out.println (
                "caught exception in showAccessibleComponent : " + e);
        }
    }


    /** Show a textual representation of the accessibility subtree rooted in
        xRoot.
    */
    public boolean showAccessibilityTree (XAccessible xRoot, int depth)
    {
        try
        {
            if ( ! showAccessibility (xRoot, depth))
                return false;

            String sIndent = "";
            for (int i=0; i<depth; i++)
                sIndent += "    ";

            //  Iterate over children and show them.
            XAccessibleContext xContext = xRoot.getAccessibleContext();
            if (xContext != null)
            {
                int n = xContext.getAccessibleChildCount();
                for (int i=0; i<n; i++)
                {
                    println (sIndent + "child " + i + " :");
                    showAccessibilityTree (xContext.getAccessibleChild(i),depth+1);
                }
            }
            else
                println ("Accessible object has no context");
        }
        catch (Exception e)
        {
            System.out.println (
                "caught exception in showAccessibleTree : " + e);
            return false;
        }

        return true;
    }

    public void showProperties (XInterface xObject)
    {
        XPropertySet xSet = (XPropertySet) UnoRuntime.queryInterface (
            XPropertySet.class, xObject);
        if (xSet == null)
            println ("object does not support XPropertySet");
        else
        {
            XPropertySetInfo xInfo = xSet.getPropertySetInfo ();
            Property[] aProperties = xInfo.getProperties ();
            int n = aProperties.length;
            for (int i=0; i<n; i++)
                println (i + " : " + aProperties[i].Name +", " + aProperties[i].Type);
        }
    }

    public  void    print (String text)
    {
        if (maPrinter == null)
            System.out.print (text);
        else
            maPrinter.print (text);
    }

    public  void    println (String text)
    {
        if (maPrinter == null)
            System.out.println (text);
        else
            maPrinter.println (text);
    }
}
