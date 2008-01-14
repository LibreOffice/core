/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Test01.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 17:27:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package complex.standalonedocumentinfo;

import com.sun.star.beans.Property;
import com.sun.star.beans.XProperty;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.io.IOException;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import complexlib.ComplexTestCase;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.document.XStandaloneDocumentInfo;
import com.sun.star.io.XTempFile;
import com.sun.star.frame.XLoadable;
import com.sun.star.frame.XStorable;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.task.ErrorCodeIOException;
import java.util.Properties;

import java.util.Random;
import share.LogWriter;

public class Test01 implements StandaloneDocumentInfoTest {
    XMultiServiceFactory m_xMSF = null;
    TestHelper m_aTestHelper = null;

    public Test01 ( XMultiServiceFactory xMSF, LogWriter aLogWriter ) {
        m_xMSF = xMSF;
        m_aTestHelper = new TestHelper( aLogWriter, "Test01: " );
    }

    public boolean test() {
        try {
            final String sDocTitle [] = new String [] {
                "ODF_Doc", "OOo6_Doc", "old_binary_Doc" };
            final String sFilterName [] = new String [] {
                "writer8", "StarOffice XML (Writer)", "StarWriter 5.0" };

            for (int i = 0; i < 3; ++i ) {
                m_aTestHelper.Message ( "==============================" );
                m_aTestHelper.Message ( sFilterName[i] );
                m_aTestHelper.Message ( "==============================" );
                //create a new temporary file
                Object oTempFile = m_xMSF.createInstance ( "com.sun.star.io.TempFile" );
                XTempFile xTempFile = (XTempFile) UnoRuntime.queryInterface (
                    XTempFile.class, oTempFile );

                //create a text document and initiallize it
                Object oTextDocument = m_xMSF.createInstance ( "com.sun.star.text.TextDocument" );
                XLoadable xLoadable = (XLoadable) UnoRuntime.queryInterface (
                        XLoadable.class, oTextDocument );
                xLoadable.initNew();
                m_aTestHelper.Message ( "New document initialized." );

                //store the instance to the temporary file URL
                XStorable xStorable = (XStorable) UnoRuntime.queryInterface (
                        XStorable.class, oTextDocument );
                String sURL = AnyConverter.toString ( xTempFile.getUri () );
                PropertyValue aProps[] = new PropertyValue[2];
                aProps[0] = new PropertyValue();
                aProps[0].Name = "DocumentTitle";
                aProps[0].Value = sDocTitle[i];
                aProps[1] = new PropertyValue();
                aProps[1].Name = "FilterName";
                aProps[1].Value = sFilterName[i];
                m_aTestHelper.Message ( "Set title: " +
                        sDocTitle[i] );
                xStorable.storeToURL ( sURL, aProps );
                m_aTestHelper.Message ( "Document stored." );

                //create StandaloneDocumentInfo object and load it from the file
                Object oStandaloneDocInfo = m_xMSF.createInstance (
                        "com.sun.star.document.StandaloneDocumentInfo" );
                XStandaloneDocumentInfo xStandaloneDocInfo =
                        (XStandaloneDocumentInfo) UnoRuntime.queryInterface (
                        XStandaloneDocumentInfo.class, oStandaloneDocInfo );
                xStandaloneDocInfo.loadFromURL ( sURL );
                m_aTestHelper.Message ( "StandaloneDocumentInfo loaded." );

                //get the title from the object and check it
                XPropertySet xPropSet =
                        (XPropertySet)UnoRuntime.queryInterface (
                        XPropertySet.class, oStandaloneDocInfo );
                String sTitle = xPropSet.getPropertyValue ( "Title" ).toString ();
                m_aTestHelper.Message ( "Get title: " + sTitle );
                if ( sTitle.compareTo ( sDocTitle[i] ) != 0 ) {
                    m_aTestHelper.Error ( "Title not match. Expected \""
                            + sDocTitle[i] +
                            "\"" );
                    return false;
                } else {
                    m_aTestHelper.Message ( "Title matched." );
                }

                //set a new title to the object
                sTitle += "_new";
                xPropSet.setPropertyValue ( "Title", sTitle );
                m_aTestHelper.Message ( "Set new title: " + sTitle );

                //store the object to the same file
                xStandaloneDocInfo.storeIntoURL ( sURL );
                m_aTestHelper.Message ( "Document info stored." );

                //create a new StandaloneDocumentInfo object and load it from the file
                Object oStandaloneDocInfo_ = m_xMSF.createInstance (
                        "com.sun.star.document.StandaloneDocumentInfo" );
                XStandaloneDocumentInfo xStandaloneDocInfo_ =
                        (XStandaloneDocumentInfo)UnoRuntime.queryInterface (
                        XStandaloneDocumentInfo.class, oStandaloneDocInfo_ );
                xStandaloneDocInfo_.loadFromURL ( sURL );
                m_aTestHelper.Message ( "New StandaloneDocumentInfo loaded." );

                //get the title and check it
                XPropertySet xPropSet_ = (XPropertySet)UnoRuntime.queryInterface (
                        XPropertySet.class, oStandaloneDocInfo_ );
                String sTitle_ = xPropSet_.getPropertyValue ( "Title" ).toString ();
                m_aTestHelper.Message ( "Get new title: " + sTitle_ );
                if ( sTitle_.compareTo ( sTitle ) != 0 ) {
                    m_aTestHelper.Error ( "New title not matched. Expected: \"" + sTitle
                            + "\"." );
                    return false;
                } else {
                    m_aTestHelper.Message ( "New title matched." );
                }
            }
        } catch ( Exception e ) {
            m_aTestHelper.Error( "Exception: " + e );
            return false;
        }
        return true;
    }
}
