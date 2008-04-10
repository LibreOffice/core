/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FormTools.java,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package util;

// access the implementations via names
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XComponent;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.awt.Size;
import com.sun.star.awt.Point;
import com.sun.star.awt.XControlModel;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XIndexContainer;
import com.sun.star.form.XFormsSupplier;
import com.sun.star.form.XForm;
import com.sun.star.form.XLoadable;
import com.sun.star.text.XTextDocument;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;

/**
 * contains helper methods forms
 */

public class FormTools {


    /**
     * creates a XControlShape
     *
     * @param oDoc the document
     * @param height the height of the shape
     * @param width the width of the shape
     * @param x the x-position of the shape
     * @param y the y-position of the shape
     * @param kind the kind of the shape
     * @return the created XControlShape
    */
    public static XControlShape createControlShape( XComponent oDoc, int height,
                                        int width, int x, int y, String kind ) {

         Size size = new Size();
        Point position = new Point();
        XControlShape oCShape = null;
        XControlModel aControl = null;

        //get MSF
        XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface( XMultiServiceFactory.class, oDoc );

        try{
            Object oInt = oDocMSF.createInstance("com.sun.star.drawing.ControlShape");
            Object aCon = oDocMSF.createInstance("com.sun.star.form.component."+kind);
            XPropertySet model_props = (XPropertySet)
                    UnoRuntime.queryInterface(XPropertySet.class,aCon);
            model_props.setPropertyValue("DefaultControl","com.sun.star.form.control."+kind);
            aControl = (XControlModel) UnoRuntime.queryInterface( XControlModel.class, aCon );
            oCShape = (XControlShape) UnoRuntime.queryInterface( XControlShape.class, oInt );
            size.Height = height;
            size.Width = width;
            position.X = x;
            position.Y = y;
            oCShape.setSize(size);
            oCShape.setPosition(position);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            System.out.println( "Couldn't create instance "+ e );
        }

        oCShape.setControl(aControl);

        return oCShape;
    } // finish createControlShape

    public static XControlShape createUnoControlShape( XComponent oDoc, int height,
                                        int width, int x, int y, String kind, String defControl ) {

         Size size = new Size();
        Point position = new Point();
        XControlShape oCShape = null;
        XControlModel aControl = null;

        //get MSF
           XMultiServiceFactory oDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface( XMultiServiceFactory.class, oDoc );

           try{
         Object oInt = oDocMSF.createInstance("com.sun.star.drawing.ControlShape");
         Object aCon = oDocMSF.createInstance("com.sun.star.form.component."+kind);
         XPropertySet model_props = (XPropertySet)
                        UnoRuntime.queryInterface(XPropertySet.class,aCon);
         model_props.setPropertyValue("DefaultControl","com.sun.star.awt."+defControl);
         aControl = (XControlModel) UnoRuntime.queryInterface( XControlModel.class, aCon );
         oCShape = (XControlShape) UnoRuntime.queryInterface( XControlShape.class, oInt );
         size.Height = height;
         size.Width = width;
         position.X = x;
         position.Y = y;
         oCShape.setSize(size);
         oCShape.setPosition(position);


           } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            System.out.println( "Couldn't create instance "+ e );
        }

        oCShape.setControl(aControl);

        return oCShape;
    } // finish createControlShape

    public static XControlShape createControlShapeWithDefaultControl( XComponent oDoc, int height,
                                        int width, int x, int y, String kind ) {

         Size size = new Size();
        Point position = new Point();
        XControlShape oCShape = null;
        XControlModel aControl = null;

        //get MSF
           XMultiServiceFactory oDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface( XMultiServiceFactory.class, oDoc );

           try{
         Object oInt = oDocMSF.createInstance("com.sun.star.drawing.ControlShape");
         Object aCon = oDocMSF.createInstance("com.sun.star.form.component."+kind);

         aControl = (XControlModel) UnoRuntime.queryInterface( XControlModel.class, aCon );
         oCShape = (XControlShape) UnoRuntime.queryInterface( XControlShape.class, oInt );
         size.Height = height;
         size.Width = width;
         position.X = x;
         position.Y = y;
         oCShape.setSize(size);
         oCShape.setPosition(position);


           } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            System.out.println( "Couldn't create instance "+ e );
        }

        oCShape.setControl(aControl);

        return oCShape;
    } // finish createControlShape

    public static XInterface createControl( XComponent oDoc, String kind ) {

        XInterface oControl = null;

           XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface( XMultiServiceFactory.class, oDoc );

           try{
            oControl = (XInterface) oDocMSF.createInstance(
                                        "com.sun.star.form.component."+kind);
           } catch ( Exception e ) {
            // Some exception occures.FAILED
            System.out.println( "Couldn't create instance "+ kind + ": "+ e );
        }
        return oControl;
    } // finish createControl

    public static XNameContainer getForms ( XDrawPage oDP ) {
        XFormsSupplier oFS = (XFormsSupplier) UnoRuntime.queryInterface(
                                                    XFormsSupplier.class,oDP);
        return oFS.getForms();
    } //finish getForms

    public static void insertForm ( XComponent aDoc, XNameContainer Forms,
                                                                String aName ) {
        try {
            XInterface oControl = createControl(aDoc, "Form");
            XForm oForm = (XForm) UnoRuntime.queryInterface(XForm.class, oControl);
            Forms.insertByName(aName,oForm);
        } catch ( Exception e ) {
            throw new IllegalArgumentException( "Couldn't insert Form" );
        }
    }

    public static XControlShape insertControlShape( XComponent oDoc, int height,
                                        int width, int x, int y, String kind ) {

        XControlShape aShape = createControlShape(oDoc,height,width,x,y,kind);
        XDrawPage oDP = DrawTools.getDrawPage(oDoc,0);
        DrawTools.getShapes(oDP).add(aShape);
        return aShape;
    }

    public static XLoadable bindForm( XTextDocument aDoc ) {
        XLoadable formLoader = null;

        try {
            Object aForm = FormTools.getForms(WriterTools.getDrawPage(aDoc)).getByName("Standard");
            XForm the_form = null;
            try {
                the_form = (XForm) AnyConverter.toObject(new Type(XForm.class), aForm);
            } catch (com.sun.star.lang.IllegalArgumentException iae) {
                System.out.println("### Couldn't convert Any");
            }
            XPropertySet formProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, the_form);
            formProps.setPropertyValue("DataSourceName","Bibliography");
            formProps.setPropertyValue("Command","biblio");
            formProps.setPropertyValue("CommandType",new Integer(com.sun.star.sdb.CommandType.TABLE));
            formLoader = (XLoadable) UnoRuntime.queryInterface(XLoadable.class, the_form);
        }
        catch (Exception ex) {
            System.out.println("Exception: "+ex);
            ex.printStackTrace(System.out);
        }

        return formLoader;
    }

    /**
    * Binds <code>'Standard'</code> form of <code>aDoc</code> Writer document
    * to the <code>tableName</code> table of <code>sourceName</code>
    * Data Source.
    * @param aDoc Writer document where DB controls are added.
    * @param sourceName The name of DataSource in the <code>DatabaseContext</code>.
    * @param tableName The name of the table to which controls are bound.
    * @return <code>com.sun.star.form.component.DatabaseForm</code> service
    * implementation which is the bound form inside the document.
    */
    public static XLoadable bindForm( XTextDocument aDoc, String sourceName, String tableName )
        throws com.sun.star.uno.Exception {

        XForm the_form = (XForm) AnyConverter.toObject(new Type(XForm.class),
            FormTools.getForms(WriterTools.getDrawPage(aDoc)).getByName("Standard"));
        XPropertySet formProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, the_form);
        formProps.setPropertyValue("DataSourceName",sourceName);
        formProps.setPropertyValue("Command",tableName);
        formProps.setPropertyValue("CommandType",new Integer(com.sun.star.sdb.CommandType.TABLE));

        return (XLoadable) UnoRuntime.queryInterface(XLoadable.class, the_form);
    }

    public static XLoadable bindForm( XTextDocument aDoc, String formName ) {
        XLoadable formLoader = null;

        try {
            XForm the_form = (XForm) FormTools.getForms(WriterTools.getDrawPage(aDoc)).getByName(formName);
            XPropertySet formProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, the_form);
            formProps.setPropertyValue("DataSourceName","Bibliography");
            formProps.setPropertyValue("Command","biblio");
            formProps.setPropertyValue("CommandType",new Integer(com.sun.star.sdb.CommandType.TABLE));
            formLoader = (XLoadable) UnoRuntime.queryInterface(XLoadable.class, the_form);
        }
        catch (Exception ex) {
            System.out.println("Exception: "+ex);
            ex.printStackTrace(System.out);
        }

        return formLoader;
    }

    /**
    * Binds the form with the name specified of <code>aDoc</code> Writer document
    * to the <code>tableName</code> table of <code>sourceName</code>
    * Data Source.
    * @param aDoc Writer document where DB controls are added.
    * @param formName The name of the form to be bound.
    * @param sourceName The name of DataSource in the <code>DatabaseContext</code>.
    * @param tableName The name of the table to which controls are bound.
    * @return <code>com.sun.star.form.component.DatabaseForm</code> service
    * implementation which is the bound form inside the document.
    */
    public static XLoadable bindForm( XTextDocument aDoc, String formName, String sourceName,
        String tableName) throws com.sun.star.uno.Exception {

        XForm the_form = (XForm) AnyConverter.toObject(new Type(XForm.class),
            FormTools.getForms(WriterTools.getDrawPage(aDoc)).getByName(formName));
        XPropertySet formProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, the_form);
        formProps.setPropertyValue("DataSourceName",sourceName);
        formProps.setPropertyValue("Command",tableName);
        formProps.setPropertyValue("CommandType",new Integer(com.sun.star.sdb.CommandType.TABLE));

        return (XLoadable) UnoRuntime.queryInterface(XLoadable.class, the_form);
    }

    public static void switchDesignOf(XMultiServiceFactory xMSF, XTextDocument aDoc) {
    try {
        com.sun.star.frame.XController aController = aDoc.getCurrentController();
        com.sun.star.frame.XFrame aFrame = aController.getFrame();
        com.sun.star.frame.XDispatchProvider aDispProv = (com.sun.star.frame.XDispatchProvider)
                UnoRuntime.queryInterface(com.sun.star.frame.XDispatchProvider.class,aFrame);
        com.sun.star.util.URL aURL = new com.sun.star.util.URL();
        aURL.Complete = ".uno:SwitchControlDesignMode";

        Object instance = xMSF.createInstance("com.sun.star.util.URLTransformer");
        com.sun.star.util.XURLTransformer atrans =
                (com.sun.star.util.XURLTransformer)UnoRuntime.queryInterface(
                                    com.sun.star.util.XURLTransformer.class,instance);
        com.sun.star.util.URL[] aURLA = new com.sun.star.util.URL[1];
        aURLA[0] = aURL;
        atrans.parseStrict(aURLA);
        aURL = aURLA[0];

        com.sun.star.frame.XDispatch aDisp = (com.sun.star.frame.XDispatch)aDispProv.queryDispatch(aURL, "",
                                com.sun.star.frame.FrameSearchFlag.SELF |
                                    com.sun.star.frame.FrameSearchFlag.CHILDREN);

        com.sun.star.beans.PropertyValue[] noArgs = new com.sun.star.beans.PropertyValue[0];
        aDisp.dispatch(aURL, noArgs);
        } catch (Exception e) {
            System.out.println("******* Mist");
            e.printStackTrace();
            }
    }

}