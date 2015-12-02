/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_INC_RDFHELPER_HXX
#define INCLUDED_SW_INC_RDFHELPER_HXX

#include <map>

#include <rtl/ustring.hxx>

#include <swdllapi.h>

class SwTextNode;

/// Provides access to RDF metadata on core objects.
class SW_DLLPUBLIC SwRDFHelper
{
public:
    /// Gets all (rTextNode, key, value) statements in RDF graphs of type rType.
    static std::map<OUString, OUString> getTextNodeStatements(const OUString& rType, SwTextNode& rTextNode);
};

#endif // INCLUDED_SW_INC_RDFHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
