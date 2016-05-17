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
import com.sun.star.lang.XComponent;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;

import com.sun.star.container.XIndexAccess;

import com.sun.star.document.XViewDataSupplier;

import com.sun.star.frame.XModel;
import com.sun.star.frame.XController;



// __________ Implementation __________

// text demo

public class DrawViewDemo
{
    public static void main( String args[] )
    {
        if ( args.length < 1 )
        {
            System.out.println( "usage: DrawViewDemo SourceURL" );
            System.exit(1);
        }

        XComponent xComponent = null;
        try
        {
            // get the remote office context of a running office (a new office
            // instance is started if necessary)
            com.sun.star.uno.XComponentContext xOfficeContext = Helper.connect();

            // suppress Presentation Autopilot when opening the document
            // properties are the same as described for
            // com.sun.star.document.MediaDescriptor
            PropertyValue[] pPropValues = new PropertyValue[ 1 ];
            pPropValues[ 0 ] = new PropertyValue();
            pPropValues[ 0 ].Name = "Silent";
            pPropValues[ 0 ].Value = Boolean.TRUE;

            java.io.File sourceFile = new java.io.File(args[0]);
            StringBuffer sUrl = new StringBuffer("file:///");
            sUrl.append(sourceFile.getCanonicalPath().replace('\\', '/'));

            xComponent = Helper.createDocument( xOfficeContext,
                                                sUrl.toString(), "_blank", 0,
                                                pPropValues );
            XModel xModel =
                UnoRuntime.queryInterface(
                        XModel.class, xComponent );


            // print all available properties of first view
            System.out.println("*** print all available properties of first view");
            XViewDataSupplier xViewDataSupplier =
                UnoRuntime.queryInterface(
                        XViewDataSupplier.class, xModel );
            XIndexAccess xIndexAccess = xViewDataSupplier.getViewData();
            if ( xIndexAccess.getCount() != 0 )
            {
                PropertyValue[] aPropSeq = (PropertyValue[])
                    xIndexAccess.getByIndex( 0 );

                for( int i = 0; i < aPropSeq.length; i++ )
                {
                    System.out.println( aPropSeq[ i ].Name + " = " +
                                        aPropSeq[ i ].Value );
                }
            }


            // print all properties that are supported by the controller
            // and change into masterpage mode
            System.out.println("*** print all properties that are supported by the controller");
            XController xController = xModel.getCurrentController();
            XPropertySet xPropSet =
                UnoRuntime.queryInterface(
                        XPropertySet.class, xController );
            XPropertySetInfo xPropSetInfo = xPropSet.getPropertySetInfo();
            Property[] aPropSeq = xPropSetInfo.getProperties();
            for( int i = 0; i < aPropSeq.length; i++ )
            {
                System.out.println( aPropSeq[ i ].Name );
            }
            System.out.println("*** change into masterpage mode");
            xPropSet.setPropertyValue( "IsMasterPageMode", Boolean.TRUE );

        }
        catch( Exception ex )
        {
            System.out.println( ex.getMessage() );
            ex.printStackTrace(System.err);
        }

        System.exit( 0 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
