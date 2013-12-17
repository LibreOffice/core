/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_DOCUMENTSTREAMACCESS_HXX
#define SC_DOCUMENTSTREAMACCESS_HXX

class ScDocument;

namespace sc {

struct DocumentStreamAccessImpl;

/**
 * Provides methods to allow direct shifting of document content without
 * broadcasting or shifting of broadcaster positions.
 */
class DocumentStreamAccess
{
    DocumentStreamAccessImpl* mpImpl;

    DocumentStreamAccess();

public:
    DocumentStreamAccess( ScDocument& rDoc );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
