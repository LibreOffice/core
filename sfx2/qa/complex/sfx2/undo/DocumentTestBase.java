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

package complex.sfx2.undo;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import org.openoffice.test.tools.DocumentType;
import org.openoffice.test.tools.OfficeDocument;

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
