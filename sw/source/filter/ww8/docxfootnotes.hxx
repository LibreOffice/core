/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef DOCXFOOTNOTES_HXX
#define DOCXFOOTNOTES_HXX

#include <vector>

class SwFmtFtn;

namespace docx {

typedef ::std::vector< const SwFmtFtn* > FootnotesVector;

/** Remember footnotes/endnotes so that we can dump them in one go.

    Also rememeber the last added footnote Id to be able to write it in the
    DocxAttributeOutput::EndRunProperties() method.
*/
class FootnotesList {
    /// The current footnote, that was not written yet.
    sal_Int32 m_nCurrent;

    /// List of the footnotes.
    FootnotesVector m_aFootnotes;

public:
    FootnotesList() : m_nCurrent( -1 ) {}

    void add( const SwFmtFtn& rFootnote )
    {
        m_aFootnotes.push_back( &rFootnote );
        m_nCurrent = m_aFootnotes.size() - 1;
    }

    /// Return the current footnote/endnote and clear the 'current' state.
    const SwFmtFtn* getCurrent( sal_Int32& rId )
    {
        // anything to write at all?
        if ( m_nCurrent < 0 )
        {
            rId = -1;
            return NULL;
        }

        // skip ids 0 and 1 - they are reserved for separator and
        // continuationSeparator
        rId = m_nCurrent + 2;

        const SwFmtFtn *pFootnote = m_aFootnotes[m_nCurrent];
        m_nCurrent = -1;

        return pFootnote;
    }

    /// Return all the footnotes/endnotes.
    const FootnotesVector& getVector() const
    {
        return m_aFootnotes;
    }

    /// Do we have any footnotes/endnotes at all?
    bool isEmpty() const
    {
        return m_aFootnotes.empty();
    }
};

} // namespace docx

#endif // DOCXFOOTNOTES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
