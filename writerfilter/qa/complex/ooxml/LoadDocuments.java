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


package complex.ooxml;
import com.sun.star.lang.XMultiServiceFactory;
import complexlib.ComplexTestCase;
import java.io.File;
import com.sun.star.text.XTextDocument;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author hb137859
 */
public class LoadDocuments extends ComplexTestCase {
    private XMultiServiceFactory m_xMSF;

    @Override
    public String[] getTestMethodNames() {
        return new String [] {
            "test1"
        };
    }

    public void before() throws Exception {
        m_xMSF = (XMultiServiceFactory) param.getMSF();
    }

    public void test1() {
        String testDocumentsPath = util.utils.getFullTestDocName("");
        log.println("Test documents in:" + testDocumentsPath);

        File dir = new File(testDocumentsPath);
        String [] files = dir.list();

        if (files != null) {
            for (int i = 0; i < files.length; ++i) {
                log.println(files[i]);
                String url = util.utils.getFullTestURL(files[i]);
                log.println(url);

                XTextDocument xDoc = util.WriterTools.loadTextDoc(m_xMSF, url);
                util.DesktopTools.closeDoc(xDoc);
            }
        } else {
            failed();
        }
    }
}
