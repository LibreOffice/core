/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package complex.sfx2.standalonedocinfo;

import complex.sfx2.standalonedocinfo.TestHelper;
import complex.sfx2.standalonedocinfo.StandaloneDocumentInfoTest;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.document.XStandaloneDocumentInfo;
import com.sun.star.io.XTempFile;
import com.sun.star.frame.XLoadable;
import com.sun.star.frame.XStorable;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;


public class Test01 implements StandaloneDocumentInfoTest {
    XMultiServiceFactory m_xMSF = null;
    TestHelper m_aTestHelper = null;

    public Test01 ( XMultiServiceFactory xMSF ) {
        m_xMSF = xMSF;
        m_aTestHelper = new TestHelper( "Test01: " );
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
                XTempFile xTempFile = UnoRuntime.queryInterface(XTempFile.class, oTempFile);

                //create a text document and initiallize it
                Object oTextDocument = m_xMSF.createInstance ( "com.sun.star.text.TextDocument" );
                XLoadable xLoadable = UnoRuntime.queryInterface(XLoadable.class, oTextDocument);
                xLoadable.initNew();
                m_aTestHelper.Message ( "New document initialized." );

                //store the instance to the temporary file URL
                XStorable xStorable = UnoRuntime.queryInterface(XStorable.class, oTextDocument);
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
                        UnoRuntime.queryInterface(XStandaloneDocumentInfo.class, oStandaloneDocInfo);
                xStandaloneDocInfo.loadFromURL ( sURL );
                m_aTestHelper.Message ( "StandaloneDocumentInfo loaded." );

                //get the title from the object and check it
                XPropertySet xPropSet =
                        UnoRuntime.queryInterface(XPropertySet.class, oStandaloneDocInfo);
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
                        UnoRuntime.queryInterface(XStandaloneDocumentInfo.class, oStandaloneDocInfo_);
                xStandaloneDocInfo_.loadFromURL ( sURL );
                m_aTestHelper.Message ( "New StandaloneDocumentInfo loaded." );

                //get the title and check it
                XPropertySet xPropSet_ = UnoRuntime.queryInterface(XPropertySet.class, oStandaloneDocInfo_);
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
