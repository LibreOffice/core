/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

//import com.sun.star.lang.XMultiServiceFactory;
//import cpo.uno.Exception;
//import org.openoffice.test.tools.DocumentType;
//import org.openoffice.test.tools.OfficeDocument;

enum class DocumentType
{
    WRITER,
    CALC,
    DRAWING,
    XMLFORM,
    PRESENTATION,
    FORMULA,
    UNKNOWN
};

class DocumentTestBase : public DocumentTest
{
public:
    DocumentTestBase(const Reference<XComponent>& rxDoc)
        : m_document(rxDoc)
    {
    }

    Reference<XComponent> getDocument() { return m_document; }

    virtual void closeDocument() override
    {
        try
        {
            Reference<XCloseable> closeDoc(m_document, UNO_QUERY);
            closeDoc->close(true);
        }
        catch (CloseVetoException)
        {
        }
    }

    Reference<XComponent> m_document;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
