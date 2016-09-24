/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
import com.sun.star.uno.UnoRuntime;

import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.container.XIndexContainer;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.awt.Size;
import com.sun.star.awt.Point;
import com.sun.star.awt.XControlModel;
import com.sun.star.text.TextContentAnchorType;
import com.sun.star.drawing.XDrawPage;

public class FormLayer
{
    private DocumentHelper  m_document;
    private int             m_insertPage;

    /* ------------------------------------------------------------------ */
    /** Creates a new instance of FormLayer */
    public FormLayer( DocumentHelper _document )
    {
        m_document = _document;
        m_insertPage = -1;
    }





    /* ------------------------------------------------------------------ */
    /** creates a control in the document

        <p>Note that <em>control<em> here is an incorrect terminology. What the method really does is
        it creates a control shape, together with a control model, and inserts them into the document model.
        This will result in every view to this document creating a control described by the model-shape pair.
        </p>

        @param sFormComponentService
            the service name of the form component to create, e.g. "TextField"
        @param nXPos
            the abscissa of the position of the newly inserted shape
        @param nXPos
            the ordinate of the position of the newly inserted shape
        @param nWidth
            the width of the newly inserted shape
        @param nHeight
            the height of the newly inserted shape
        @param xParentForm
            the form to use as parent for the newly create form component. May be null, in this case
            a default parent is chosen by the implementation
        @return
            the property access to the control's model
    */
    protected XPropertySet createControlAndShape( String sFormComponentService, int nXPos,
        int nYPos, int nWidth, int nHeight, XIndexContainer xParentForm ) throws java.lang.Exception
    {
        // let the document create a shape
        XMultiServiceFactory xDocAsFactory = UnoRuntime.queryInterface(
            XMultiServiceFactory.class, m_document.getDocument() );
        XControlShape xShape = UnoRuntime.queryInterface( XControlShape.class,
            xDocAsFactory.createInstance( "com.sun.star.drawing.ControlShape" ) );

        // position and size of the shape
        xShape.setSize( new Size( nWidth * 100, nHeight * 100 ) );
        xShape.setPosition( new Point( nXPos * 100, nYPos * 100 ) );

        // adjust the anchor so that the control is tied to the page
        XPropertySet xShapeProps = UNO.queryPropertySet( xShape );
        TextContentAnchorType eAnchorType = TextContentAnchorType.AT_PARAGRAPH;
        xShapeProps.setPropertyValue( "AnchorType", eAnchorType );

        // create the form component (the model of a form control)
        String sQualifiedComponentName = "com.sun.star.form.component." + sFormComponentService;
        XControlModel xModel = UnoRuntime.queryInterface( XControlModel.class,
            m_document.getOrb().createInstance( sQualifiedComponentName ) );

        // insert the model into the form component hierarchy, if the caller gave us a location
        if ( null != xParentForm )
        {
            xParentForm.insertByIndex( xParentForm.getCount(), xModel );
        }

        // knitt them
        xShape.setControl( xModel );

        // add the shape to the shapes collection of the document
        XDrawPage pageWhereToInsert = ( m_insertPage != -1 ) ? m_document.getDrawPage( m_insertPage ) : m_document.getMainDrawPage();

        XShapes xDocShapes = UnoRuntime.queryInterface( XShapes.class, pageWhereToInsert );
        xDocShapes.add( xShape );

        // some initializations which are the same for all controls
        XPropertySet xModelProps = UNO.queryPropertySet( xModel );
        try
        {
            XPropertySetInfo xPSI = xModelProps.getPropertySetInfo();
            if ( xPSI.hasPropertyByName( "Border" ) )
            {
                if ( ((Short)xModelProps.getPropertyValue( "Border" )).shortValue() == com.sun.star.awt.VisualEffect.LOOK3D )
                    xModelProps.setPropertyValue( "Border", Short.valueOf( com.sun.star.awt.VisualEffect.FLAT ) );
            }
            if ( xPSI.hasPropertyByName( "VisualEffect" ) )
                xModelProps.setPropertyValue( "VisualEffect", Short.valueOf( com.sun.star.awt.VisualEffect.FLAT ) );
            if ( m_document.classify() != DocumentType.CALC )
                if ( xPSI.hasPropertyByName( "BorderColor" ) )
                    xModelProps.setPropertyValue( "BorderColor", Integer.valueOf( 0x00C0C0C0 ) );
        }
        catch( com.sun.star.uno.Exception e )
        {
            System.err.println(e);
            e.printStackTrace( System.err );
        }
        return xModelProps;
    }

    /* ------------------------------------------------------------------ */
    /** creates a control in the document

        <p>Note that <em>control<em> here is an incorrect terminology. What the method really does is
        it creates a control shape, together with a control model, and inserts them into the document model.
        This will result in every view to this document creating a control described by the model-shape pair.
        </p>

        @param sFormComponentService
            the service name of the form component to create, e.g. "TextField"
        @param nXPos
            the abscissa of the position of the newly inserted shape
        @param nXPos
            the ordinate of the position of the newly inserted shape
        @param nWidth
            the width of the newly inserted shape
        @param nHeight
            the height of the newly inserted shape
        @return
            the property access to the control's model
    */
    protected XPropertySet createControlAndShape( String sFormComponentService, int nXPos,
        int nYPos, int nWidth, int nHeight ) throws java.lang.Exception
    {
        return createControlAndShape( sFormComponentService, nXPos, nYPos, nWidth, nHeight, null );
    }

    /* ------------------------------------------------------------------ */
    /** creates a line of controls, consisting of a label and a field for data input.

        <p>In opposite to the second form of this method, here the height of the field,
        as well as the abscissa of the label, are under the control of the caller.</p>

        @param sControlType
            specifies the type of the data input control
        @param sFieldName
            specifies the field name the text field should be bound to
        @param sControlNamePostfix
            specifies a postfix to append to the logical control names
        @param nYPos
            specifies the Y position of the line to start at
        @param nHeight
            the height of the field
        @return
            the control model of the created data input field
    */
    protected XPropertySet insertControlLine( String sControlType, String sFieldName, String sControlNamePostfix, int nXPos, int nYPos, int nHeight )
        throws java.lang.Exception
    {
        // insert the label control
        XPropertySet xLabelModel = createControlAndShape( "FixedText", nXPos, nYPos, 25, 6 );
        xLabelModel.setPropertyValue( "Label", sFieldName );

        // insert the text field control
        XPropertySet xFieldModel = createControlAndShape( sControlType, nXPos + 26, nYPos, 40, nHeight );
        xFieldModel.setPropertyValue( "DataField", sFieldName );
        // knit it to its label component
        xFieldModel.setPropertyValue( "LabelControl", xLabelModel );

        // some names, so later on we can find them
        xLabelModel.setPropertyValue( "Name", sFieldName + sControlNamePostfix + "_Label" );
        xFieldModel.setPropertyValue( "Name", sFieldName + sControlNamePostfix );

        return xFieldModel;
    }

    /* ------------------------------------------------------------------ */
    /** creates a line of controls, consisting of a label and a field for data input.

        @param sControlType
            specifies the type of the data input control
        @param sFieldName
            specifies the field name the text field should be bound to
        @param nYPos
            specifies the Y position of the line to start at
        @return
            the control model of the created data input field
    */
    protected XPropertySet insertControlLine( String sControlType, String sFieldName, String sControlNamePostfix, int nYPos )
        throws java.lang.Exception
    {
        return insertControlLine( sControlType, sFieldName, sControlNamePostfix, 2, nYPos, 6 );
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the radio button model with the given name and the given ref value
     *  @param form
     *      the parent form of the radio button model to find
     *  @param name
     *      the name of the radio button
     *  @param refValue
     *      the reference value of the radio button
    */
    public XPropertySet getRadioModelByRefValue( XPropertySet form, String name, String refValue ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XIndexAccess indexAccess = UnoRuntime.queryInterface( XIndexAccess.class,
            form );

        for ( int i=0; i<indexAccess.getCount(); ++i )
        {
            XPropertySet control = UNO.queryPropertySet( indexAccess.getByIndex( i ) );

            if ( ((String)control.getPropertyValue( "Name" )).equals( name ) )
                if ( ((String)control.getPropertyValue( "RefValue" )).equals( refValue ) )
                    return control;
        }
        return null;
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the radio button model with the given name and the given tag
     *  @param form
     *      the parent form of the radio button model to find
     *  @param name
     *      the name of the radio button
     *  @param tag
     *      the tag of the radio button
    */
    public XPropertySet getRadioModelByTag( XPropertySet form, String name, String tag ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XIndexAccess indexAccess = UnoRuntime.queryInterface( XIndexAccess.class,
            form );

        for ( int i=0; i<indexAccess.getCount(); ++i )
        {
            XPropertySet control = UNO.queryPropertySet( indexAccess.getByIndex( i ) );

            if ( ((String)control.getPropertyValue( "Name" )).equals( name ) )
                if ( ((String)control.getPropertyValue( "Tag" )).equals( tag ) )
                    return control;
        }
        return null;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
