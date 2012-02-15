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

package complex.sfx2.undo;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import org.openoffice.test.tools.DocumentType;
import org.openoffice.test.tools.OfficeDocument;

/**
 * @author frank.schoenheit@oracle.com
 */
abstract class DocumentTestBase implements DocumentTest
{
    DocumentTestBase( final XMultiServiceFactory i_orb, final DocumentType i_docType ) throws Exception
    {
        m_document = OfficeDocument.blankDocument( i_orb, i_docType );
    }

    public OfficeDocument getDocument()
    {
        return m_document;
    }

    public void closeDocument()
    {
        m_document.close();
    }

    protected final OfficeDocument  m_document;
}
