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

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.beans.*;
import com.sun.star.container.*;
import com.sun.star.awt.*;
import com.sun.star.form.*;


/** provides global helpers
*/
public class FLTools
{




    /* ------------------------------------------------------------------ */
    /** returns the name of the given form component
    */
    private static String getName( Object aFormComponent )
    {
        XNamed xNamed = UnoRuntime.queryInterface( XNamed.class,
            aFormComponent );
        String sName = "";
        if ( null != xNamed )
            sName = xNamed.getName();
        return sName;
    }

    /* ------------------------------------------------------------------ */
    /** returns the label of the given form component
    */
    public static String getLabel( Object aFormComponent ) throws com.sun.star.uno.Exception
    {
        String sLabel = "";

        XPropertySet xProps = UNO.queryPropertySet( aFormComponent );
        XPropertySetInfo xPSI = ( null != xProps ) ? xProps.getPropertySetInfo() : null;
        if ( null == xPSI )
        {   // no property set or no property set info
            // can't do anything except falling back to the name
            return getName( aFormComponent );
        }

        // first check if the component has a LabelControl
        if ( xPSI.hasPropertyByName( "LabelControl" ) )
            sLabel = getLabel( xProps.getPropertyValue( "LabelControl" ) );

        // no LabelControl or no label at the LabelControl
        if ( 0 == sLabel.length() )
        {
            // a "Label" property?
            if ( xPSI.hasPropertyByName( "Label" ) )
                sLabel = (String)xProps.getPropertyValue( "Label" );

            if ( 0 == sLabel.length() )
            {   // no Label property or no label set
                // -> fallback to the component name
                sLabel = getName( aFormComponent );
            }
        }

        return sLabel;
    }



    /* ------------------------------------------------------------------ */
    /** retrieves the parent of the given object
    */
    private static <T> T getParent( Object aComponent, Class<T> aInterfaceClass )
    {
        XChild xAsChild = UnoRuntime.queryInterface( XChild.class, aComponent );

        return UnoRuntime.queryInterface( aInterfaceClass, xAsChild.getParent() );
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the parent of the given object
    */
    static XPropertySet getParent( Object aComponent )
    {
        return getParent( aComponent, XPropertySet.class );
    }

    /* ------------------------------------------------------------------ */
    /** disposes the component given
    */
    public static void disposeComponent( Object xComp ) throws java.lang.RuntimeException
    {
        XComponent xComponent = UnoRuntime.queryInterface( XComponent.class,
            xComp );
        if ( null != xComponent )
            xComponent.dispose();
    }

    /* ------------------------------------------------------------------ */
    /** gets the XControlModel for a control
    */
    public static <T> T getModel( Object aControl, Class<T> aInterfaceClass )
    {
        XControl xControl = UnoRuntime.queryInterface(
            XControl.class, aControl );
        XControlModel xModel = null;
        if ( null != xControl )
            xModel = xControl.getModel();

        return UnoRuntime.queryInterface( aInterfaceClass, xModel );
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the type of a form component.
        <p>Speaking strictly, the function recognizes more than form components. Especially,
        it survives a null argument. which means it can be safely applied to the a top-level
        forms container; and it is able to classify grid columns (which are no form components)
        as well.</p>
    */
    public static String classifyFormComponentType( XPropertySet xComponent ) throws com.sun.star.uno.Exception
    {
        String sType = "<unknown component>";

        XServiceInfo xSI = UNO.queryServiceInfo( xComponent );

        XPropertySetInfo xPSI = null;
        if ( null != xComponent )
            xPSI = xComponent.getPropertySetInfo();

        if ( ( null != xPSI ) && xPSI.hasPropertyByName( "ClassId" ) )
        {
            // get the ClassId property
            XPropertySet xCompProps = UNO.queryPropertySet( xComponent );

            Short nClassId = (Short)xCompProps.getPropertyValue( "ClassId" );
            switch ( nClassId.intValue() )
            {
                case FormComponentType.COMMANDBUTTON: sType = "Command button"; break;
                case FormComponentType.RADIOBUTTON  : sType = "Radio button"; break;
                case FormComponentType.IMAGEBUTTON  : sType = "Image button"; break;
                case FormComponentType.CHECKBOX     : sType = "Check Box"; break;
                case FormComponentType.LISTBOX      : sType = "List Box"; break;
                case FormComponentType.COMBOBOX     : sType = "Combo Box"; break;
                case FormComponentType.GROUPBOX     : sType = "Group Box"; break;
                case FormComponentType.FIXEDTEXT    : sType = "Fixed Text"; break;
                case FormComponentType.GRIDCONTROL  : sType = "Grid Control"; break;
                case FormComponentType.FILECONTROL  : sType = "File Control"; break;
                case FormComponentType.HIDDENCONTROL: sType = "Hidden Control"; break;
                case FormComponentType.IMAGECONTROL : sType = "Image Control"; break;
                case FormComponentType.DATEFIELD    : sType = "Date Field"; break;
                case FormComponentType.TIMEFIELD    : sType = "Time Field"; break;
                case FormComponentType.NUMERICFIELD : sType = "Numeric Field"; break;
                case FormComponentType.CURRENCYFIELD: sType = "Currency Field"; break;
                case FormComponentType.PATTERNFIELD : sType = "Pattern Field"; break;

                case FormComponentType.TEXTFIELD    :
                    // there are two known services with this class id: the usual text field,
                    // and the formatted field
                    sType = "Text Field";
                    if ( ( null != xSI ) && xSI.supportsService( "com.sun.star.form.component.FormattedField" ) )
                    {
                        sType = "Formatted Field";
                    }
                    break;

                default:
                    break;
            }
        }
        else
        {
            if ( ( null != xSI ) && xSI.supportsService( "com.sun.star.form.component.DataForm" ) )
            {
                sType = "Form";
            }
        }

        return sType;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
