/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package integration.forms;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleEditableText;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexContainer;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.awt.Size;
import com.sun.star.awt.Point;
import com.sun.star.awt.VisualEffect;
import com.sun.star.awt.XControlModel;
import com.sun.star.container.XNameAccess;
import com.sun.star.text.TextContentAnchorType;
import com.sun.star.drawing.XDrawPage;

public class FormLayer
{
    private DocumentHelper  m_document;
    private XDrawPage       m_page;

    /* ------------------------------------------------------------------ */
    /** Creates a new instance of FormLayer */
    public FormLayer( DocumentHelper _document )
    {
        m_document = _document;
    }

    /* ------------------------------------------------------------------ */
    /** sets the page which is to be used for subsequent insertions of controls/shapes
     */
    void setInsertPage( int page ) throws com.sun.star.lang.IndexOutOfBoundsException, com.sun.star.lang.WrappedTargetException
    {
        m_page = m_document.getDrawPage( page );
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
    public XPropertySet createControlAndShape( String sFormComponentService, int nXPos,
        int nYPos, int nWidth, int nHeight, Object _parentForm ) throws java.lang.Exception
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
        XPropertySet xShapeProps = dbfTools.queryPropertySet( xShape );
        TextContentAnchorType eAnchorType = TextContentAnchorType.AT_PARAGRAPH;
        xShapeProps.setPropertyValue( "AnchorType", eAnchorType );

        // create the form component (the model of a form control)
        String sQualifiedComponentName = "com.sun.star.form.component." + sFormComponentService;
        XControlModel xModel = UnoRuntime.queryInterface( XControlModel.class,
            m_document.getOrb().createInstance( sQualifiedComponentName ) );

        // insert the model into the form component hierarchy, if the caller gave us a location
        if ( null != _parentForm )
        {
            XIndexContainer parentForm = null;
            if ( _parentForm instanceof XIndexContainer )
                parentForm = (XIndexContainer)_parentForm;
            else
                parentForm = UnoRuntime.queryInterface( XIndexContainer.class, _parentForm );
            parentForm.insertByIndex( parentForm.getCount(), xModel );
        }

        // knitt them
        xShape.setControl( xModel );

        // add the shape to the shapes collection of the document
        XDrawPage pageWhereToInsert = ( m_page != null ) ? m_page : m_document.getMainDrawPage();

        XShapes xDocShapes = UnoRuntime.queryInterface( XShapes.class, pageWhereToInsert );
        xDocShapes.add( xShape );

        // and outta here with the XPropertySet interface of the model
        XPropertySet xModelProps = dbfTools.queryPropertySet( xModel );
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
    public XPropertySet createControlAndShape( String sFormComponentService, int nXPos,
        int nYPos, int nWidth, int nHeight ) throws java.lang.Exception
    {
        return createControlAndShape( sFormComponentService, nXPos, nYPos, nWidth, nHeight, null );
    }

    /** creates a pair of controls, namely a label control, and another control labeled by it
     *
     * @param _formComponentServiceName
     *      the service name for the control which is not the label control
     * @param _label
     *      the label to be shown in the label control
     * @param _xPos
     *      the horizontal position of the control pair
     * @param _yPos
     *      the vertical position of the control pair
     * @param _height
     *      the height of the control which is not the label control
     * @return
     *      the model of the control which is not the label control
     * @throws java.lang.Exception
     */
    public XPropertySet createLabeledControl( String _formComponentServiceName, String _label, int _xPos,
            int _yPos, int _height )
        throws java.lang.Exception
    {
        // insert the label control
        XPropertySet label = createControlAndShape( "FixedText", _xPos, _yPos, 25, 6 );
        label.setPropertyValue( "Label", _label );

        // insert the text field control
        XPropertySet field = createControlAndShape( _formComponentServiceName,
            _xPos + 25, _yPos, 40, _height );
        // knit it to it's label component
        field.setPropertyValue( "LabelControl", label );

        // names
        label.setPropertyValue( "Name", _label + "_Label" );
        field.setPropertyValue( "Name", _label );

        return field;
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
    public XPropertySet insertControlLine( String sControlType, String sFieldName, String _controlNamePostfix,
            int nXPos, int nYPos, int nHeight )
        throws java.lang.Exception
    {
        // insert the label control
        XPropertySet xLabelModel = createControlAndShape( "FixedText", nXPos, nYPos, 25, 6 );
        xLabelModel.setPropertyValue( "Label", sFieldName );

        // insert the text field control
        XPropertySet xFieldModel = createControlAndShape( sControlType, nXPos + 26, nYPos, 40, nHeight );
        xFieldModel.setPropertyValue( "DataField", sFieldName );
        if ( xFieldModel.getPropertySetInfo().hasPropertyByName( "Border" ) )
        {
            xFieldModel.setPropertyValue( "Border", new Short( VisualEffect.FLAT ) );
            if ( xFieldModel.getPropertySetInfo().hasPropertyByName( "BorderColor" ) )
                xFieldModel.setPropertyValue( "BorderColor", new Integer( 0x00C0C0C0 ) );
        }
        // knit it to it's label component
        xFieldModel.setPropertyValue( "LabelControl", xLabelModel );

        // some names, so later on we can find them
        if ( _controlNamePostfix == null )
            _controlNamePostfix = "";
        xLabelModel.setPropertyValue( "Name", sFieldName + _controlNamePostfix + "_Label" );
        xFieldModel.setPropertyValue( "Name", sFieldName + _controlNamePostfix );

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
    public XPropertySet insertControlLine( String sControlType, String sFieldName, String sControlNamePostfix, int nYPos )
        throws java.lang.Exception
    {
        return insertControlLine( sControlType, sFieldName, sControlNamePostfix, 10, nYPos, 6 );
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
        XIndexAccess indexAccess = UnoRuntime.queryInterface( XIndexAccess.class, form );

        for ( int i=0; i<indexAccess.getCount(); ++i )
        {
            XPropertySet control = dbfTools.queryPropertySet( indexAccess.getByIndex( i ) );

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
     *  @param refValue
     *      the tag of the radio button
    */
    public XPropertySet getRadioModelByTag( XPropertySet form, String name, String tag ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XIndexAccess indexAccess = UnoRuntime.queryInterface( XIndexAccess.class, form );

        for ( int i=0; i<indexAccess.getCount(); ++i )
        {
            XPropertySet control = dbfTools.queryPropertySet( indexAccess.getByIndex( i ) );

            if ( ((String)control.getPropertyValue( "Name" )).equals( name ) )
                if ( ((String)control.getPropertyValue( "Tag" )).equals( tag ) )
                    return control;
        }
        return null;
    }

    /* ------------------------------------------------------------------ */
    /** retrieves a control model with a given (integer) access path
     */
    public XPropertySet getControlModel( int[] _accessPath ) throws com.sun.star.uno.Exception
    {
        XIndexAccess indexAcc = (XIndexAccess)UnoRuntime.queryInterface( XIndexAccess.class,
            m_document.getFormComponentTreeRoot() );
        XPropertySet controlModel = null;
        int i=0;
        while ( ( indexAcc != null ) && ( i < _accessPath.length ) )
        {
            controlModel = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class,
                indexAcc.getByIndex( _accessPath[i] ) );
            indexAcc = (XIndexAccess)UnoRuntime.queryInterface( XIndexAccess.class,
                controlModel );
            ++i;
        }
        return controlModel;
    }

    /* ------------------------------------------------------------------ */
    /** retrieves a control model with a given (string) access path
     */
    public XPropertySet getControlModel( String[] _accessPath ) throws com.sun.star.uno.Exception
    {
        XNameAccess nameAcc = m_document.getFormComponentTreeRoot();
        XPropertySet controlModel = null;
        int i=0;
        while ( ( nameAcc != null ) && ( i < _accessPath.length ) )
        {
            controlModel = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class,
                nameAcc.getByName( _accessPath[i] ) );
            nameAcc = (XNameAccess)UnoRuntime.queryInterface( XNameAccess.class,
                controlModel );
            ++i;
        }
        return controlModel;
    }

    /* ------------------------------------------------------------------ */
    /** simulates a user's text input into a control given by control model
     */
    public void userTextInput( XPropertySet controlModel, String text ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // we will *not* simply set the value property at the model. This is not the same as
        // doing a user input, as the latter will trigger a lot of notifications, which the forms runtime environment
        // (namely the FormController) relies on to notice that the control changed.
        // Instead, we use the Accessibility interfaces of the control to simulate text input
        XAccessible formattedAccessible = UnoRuntime.queryInterface( XAccessible.class,
            m_document.getCurrentView().getControl( controlModel )
        );
        XAccessibleEditableText textAccess = UnoRuntime.queryInterface( XAccessibleEditableText.class,
            formattedAccessible.getAccessibleContext() );
        textAccess.setText( text );
    }
}
