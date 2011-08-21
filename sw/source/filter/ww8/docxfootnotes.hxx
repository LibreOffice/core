/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DOCXFOOTNOTES_HXX_
#define _DOCXFOOTNOTES_HXX_

#include <fmtftn.hxx>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sax/fshelper.hxx>

#include <vector>

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
        // skip ids 0 and 1 - they are reserved for separator and
        // continuationSeparator
        rId = m_nCurrent + 2;

        // anything to write at all?
        if ( m_nCurrent < 0 )
        {
            rId = -1;
            return NULL;
        }

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

#endif // _DOCXFOOTNOTES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
