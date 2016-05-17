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
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.*;

import com.sun.star.frame.XComponentLoader;

// property access
import com.sun.star.beans.*;

// application specific classes
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.frame.XModel;

// __________ Implementation __________

// Helper for creating a calc document adding cell values and charts

public class Helper
{
    public Helper( String[] args )
    {
        // connect to a running office and get the ServiceManager
        try {
            // get the remote office component context
            maContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");

            // get the remote office service manager
            maMCFactory = maContext.getServiceManager();
        }
        catch( Exception e) {
            System.out.println( "Couldn't get ServiceManager: " + e );
            e.printStackTrace();
            System.exit(1);
        }
    }



    public XSpreadsheetDocument createSpreadsheetDocument()
    {
        return UnoRuntime.queryInterface(
            XSpreadsheetDocument.class, createDocument( "scalc" ));
    }







    public XModel createDrawingDocument()
    {
        return createDocument( "sdraw" );
    }



    public XModel createTextDocument()
    {
        return createDocument( "swriter" );
    }



    private XModel createDocument( String sDocType )
    {
        XModel aResult = null;
        try
        {
            XComponentLoader aLoader = UnoRuntime.queryInterface(XComponentLoader.class,
            maMCFactory.createInstanceWithContext("com.sun.star.frame.Desktop",
                                                  maContext) );

            aResult = UnoRuntime.queryInterface(
                XModel.class,
                aLoader.loadComponentFromURL( "private:factory/" + sDocType,
                                              "_blank",
                                              0,
                                              new PropertyValue[ 0 ] ) );
        }
        catch( Exception e )
        {
            System.err.println("Couldn't create Document of type "+ sDocType +": "+e);
            e.printStackTrace();
            System.exit( 0 );
        }

        return aResult;
    }

    public XComponentContext getComponentContext(){
        return maContext;

    }

    // __________ private members and methods __________


    private XComponentContext      maContext;
    private XMultiComponentFactory maMCFactory;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
