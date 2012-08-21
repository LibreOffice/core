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

import com.sun.star.beans.Property;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;

import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XServiceName;
import com.sun.star.lang.XTypeProvider;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.Type;

import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapeDescriptor;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleRelationSet;
import com.sun.star.accessibility.XAccessibleStateSet;

public class InformationWriter
{
    public InformationWriter ()
    {
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
            MessageArea.println (prefix +
                xPropertySet.getPropertyValue (name));
        }
        catch (Exception e)
        {
            MessageArea.println ("caught exception while getting property "
                + name + " : " + e);
        }
    }



    public void showShapes (XDrawPage xPage)
    {
        try
        {
            XIndexAccess xShapeList = (XIndexAccess) UnoRuntime.queryInterface(
                XIndexAccess.class, xPage);

            MessageArea.println ("There are " + xShapeList.getCount()
                + " shapes");
            for (int i=0; i<xShapeList.getCount(); i++)
            {
                XShape xShape = (XShape) UnoRuntime.queryInterface(
                    XShape.class, xShapeList.getByIndex (i));

                XShapeDescriptor xShapeDescriptor =
                    (XShapeDescriptor) UnoRuntime.queryInterface(
                        XShapeDescriptor.class, xShape);
                String sName = xShapeDescriptor.getShapeType ();
                MessageArea.println ("   shape " + i + " : " + sName);

                XPropertySet xPropertySet =
                    (XPropertySet) UnoRuntime.queryInterface(
                        XPropertySet.class, xShape);
                Integer nZOrder =
                    (Integer) xPropertySet.getPropertyValue ("ZOrder");
                MessageArea.println ("   zorder = " + nZOrder);
            }
        }
        catch (Exception e)
        {
            MessageArea.println ("caught exception in showShapes: " + e);
        }
    }




    /** @descr Print all available services of the given object to the
                standard output.
    */
    public void showServices (XInterface xObject)
    {
        try
        {
            MessageArea.println ("Services:");
            XMultiServiceFactory xMSF = (XMultiServiceFactory) UnoRuntime.queryInterface (
                XMultiServiceFactory.class,
                xObject
                );
            if (xMSF == null)
                MessageArea.println ("    object does not support interface XMultiServiceFactory");
            else
            {
                String[] sServiceNames = xMSF.getAvailableServiceNames ();
                MessageArea.println ("    object can create "
                    + sServiceNames.length + " services");
                for (int i=0; i<sServiceNames.length; i++)
                    MessageArea.println ("        service " + i + " : " + sServiceNames[i]);
            }
        }
        catch (Exception e)
        {
            MessageArea.println ("caught exception in showServices : " + e);
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
                MessageArea.println ("    interface XServiceName not supported");
            else
            {
                MessageArea.println ("    Service name        : " + xSN.getServiceName ());
            }

            // Use interface XServiceInfo to retrieve information about
            // supported services.
            XServiceInfo xSI = (XServiceInfo) UnoRuntime.queryInterface (
                XServiceInfo.class, xObject);
            if (xSI == null)
                MessageArea.println ("    interface XServiceInfo not supported");
            else
            {
                MessageArea.println ("    Implementation name : "
                    + xSI.getImplementationName ());
            }
        }
        catch (Exception e)
        {
            MessageArea.println ("caught exception in showInfo : " + e);
        }
    }




    /** @descr Print information about supported interfaces.
    */
    public void showInterfaces (XInterface xObject)
    {
        try
        {
            MessageArea.println ("Interfaces:");
            // Use interface XTypeProvider to retrieve a list of supported
            // interfaces.
            XTypeProvider xTP = (XTypeProvider) UnoRuntime.queryInterface (
                XTypeProvider.class, xObject);
            if (xTP == null)
                MessageArea.println ("    interface XTypeProvider not supported");
            else
            {
                Type[] aTypeList = xTP.getTypes ();
                MessageArea.println ("    object supports " + aTypeList.length
                    + " interfaces");
                for (int i=0; i<aTypeList.length; i++)
                    MessageArea.println ("        " + i + " : "
                        + aTypeList[i].getTypeName());
            }
        }
        catch (Exception e)
        {
            MessageArea.println ("caught exception in showInterfaces : " + e);
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
                    MessageArea.println (sIndent + "given object " + xObject
                        + " is not accessible");
                    return false;
                }
                else
                    xContext = xAccessible.getAccessibleContext();
            }

            //  Print information about the accessible context.
            if (xContext != null)
            {
                MessageArea.println (sIndent + "Name         : "
                    + xContext.getAccessibleName());
                MessageArea.println (sIndent + "Description  : "
                    + xContext.getAccessibleDescription());
                MessageArea.println (sIndent + "Role         : "
                    + xContext.getAccessibleRole());
                String sHasParent;
                if (xContext.getAccessibleParent() != null)
                {
                    MessageArea.println (sIndent + "Has parent   : yes");
                    MessageArea.println (sIndent + "Parent index : "
                        + xContext.getAccessibleIndexInParent());
                }
                else
                    MessageArea.println (sIndent + "Has parent   : no");
                MessageArea.println (sIndent + "Child count  : "
                    + xContext.getAccessibleChildCount());
                MessageArea.print (sIndent + "Relation set : ");
                XAccessibleRelationSet xRelationSet
                    = xContext.getAccessibleRelationSet();
                if (xRelationSet != null)
                {
                    MessageArea.print (xRelationSet.getRelationCount() + " (");
                    for (int i=0; i<xRelationSet.getRelationCount(); i++)
                    {
                        if (i > 0)
                            MessageArea.print (", ");
                        MessageArea.print (xRelationSet.getRelation(i).toString());
                    }
                    MessageArea.println (")");
                }
                else
                    MessageArea.println ("no relation set");

                MessageArea.print (sIndent + "State set    : ");
                XAccessibleStateSet xStateSet =
                    xContext.getAccessibleStateSet();
                if (xStateSet != null)
                {
                    XIndexAccess xStates =
                        (XIndexAccess) UnoRuntime.queryInterface (
                            XIndexAccess.class, xStateSet);
                    MessageArea.print (xStates.getCount() + " (");
                    for (int i=0; i<xStates.getCount(); i++)
                    {
                        if (i > 0)
                            MessageArea.print (", ");
                        MessageArea.print (xStates.getByIndex(i).toString());
                    }
                    MessageArea.println (")");
                }
                else
                    MessageArea.println ("no state set");

                showAccessibleComponent (xContext, sIndent);
            }
            else
                MessageArea.println ("object has no accessible context.");

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
                MessageArea.println (sIndent + "Position        : "
                    + xComponent.getLocation().X+", "
                    + xComponent.getLocation().Y);
                MessageArea.println (sIndent + "Screen position : "
                    + xComponent.getLocationOnScreen().X+", "
                    + xComponent.getLocationOnScreen().Y);
                MessageArea.println (sIndent + "Size            : "
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
                    MessageArea.println (sIndent + "child " + i + " :");
                    showAccessibilityTree (xContext.getAccessibleChild(i),depth+1);
                }
            }
            else
                MessageArea.println ("Accessible object has no context");
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
            MessageArea.println ("object does not support XPropertySet");
        else
        {
            XPropertySetInfo xInfo = xSet.getPropertySetInfo ();
            Property[] aProperties = xInfo.getProperties ();
            int n = aProperties.length;
            for (int i=0; i<n; i++)
                MessageArea.println (i + " : " + aProperties[i].Name +", " + aProperties[i].Type);
        }
    }
}
