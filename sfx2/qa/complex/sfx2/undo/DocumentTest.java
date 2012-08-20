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

import org.openoffice.test.tools.OfficeDocument;

/**
 * wrapper around an OfficeDocument, for running a standardized test procedure (related do Undo functionality)
 * on the document.
 *
 * @author frank.schoenheit@oracle.com
 */
public interface DocumentTest
{
    /**
     * returns a human-readable description for the document/type which the tests operates on
     */
    public String getDocumentDescription();

    /**
     * initializes the document to a state where the subsequent tests can be ran
     */
    public void initializeDocument() throws com.sun.star.uno.Exception;

    /**
     * closes the document which the test is based on
     */
    public void closeDocument();

    /**
     * does a simple modification to the document, which results in one Undo action being auto-generated
     * by the OOo implementation
     */
    public void doSingleModification() throws com.sun.star.uno.Exception;

    /**
     * verifies the document is in the same state as after {@link #initializeDocument}
     */
    public void verifyInitialDocumentState() throws com.sun.star.uno.Exception;

    /**
     * verifies the document is in the state as expected after {@link #doSingleModification}
     * @throws com.sun.star.uno.Exception
     */
    public void verifySingleModificationDocumentState() throws com.sun.star.uno.Exception;

    /**
     * does multiple modifications do the document, which would normally result in multiple Undo actions.
     *
     * The test framework will encapsulate the call into an {@link com.sun.star.document.XUndoManager#enterUndoContext} and
     * {@link com.sun.star.document.XUndoManager#leaveUndoContext} call.
     *
     * @return
     *  the number of modifications done to the document. The caller assumes (and asserts) that the number
     *  of actions on the Undo stack equals this number.
     */
    public int doMultipleModifications() throws com.sun.star.uno.Exception;

    /**
     * returns the document which the test operates on
     */
    public OfficeDocument   getDocument();
}
