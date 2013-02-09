/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_XML_IMPORTCONTEXT_HXX__
#define __SC_XML_IMPORTCONTEXT_HXX__

#include "xmloff/xmlictxt.hxx"
#include "xmloff/xmlimp.hxx"

class ScXMLImport;

/**
 * This class exists only to provide GetScImport() to its derived classes.
 */
class ScXMLImportContext : public SvXMLImportContext
{
public:
    ScXMLImportContext(
        SvXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName);

protected:
    ScXMLImport& GetScImport();
    const ScXMLImport& GetScImport() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#endif
