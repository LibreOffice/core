/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

//import org.openoffice.test.tools.OfficeDocument;

/**
 * wrapper around an OfficeDocument, for running a standardized test procedure (related do Undo functionality)
 * on the document.
 *
 */
class DocumentTest
{
public:
    virtual ~DocumentTest();

    /**
     * returns a human-readable description for the document/type which the tests operates on
     */
    virtual OUString getDocumentDescription() = 0;

    /**
     * initializes the document to a state where the subsequent tests can be ran
     */
    virtual void initializeDocument() = 0;

    /**
     * closes the document which the test is based on
     */
    virtual void closeDocument() = 0;

    /**
     * does a simple modification to the document, which results in one Undo action being auto-generated
     * by the OOo implementation
     */
    virtual void doSingleModification() = 0;

    /**
     * verifies the document is in the same state as after {@link #initializeDocument}
     */
    virtual void verifyInitialDocumentState() = 0;

    /**
     * verifies the document is in the state as expected after {@link #doSingleModification}
     */
    virtual void verifySingleModificationDocumentState() = 0;

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
    virtual int doMultipleModifications() = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
