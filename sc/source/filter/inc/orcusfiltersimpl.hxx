/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SCFILT_ORCUSFILTERSIMPL_HXX__
#define __SCFILT_ORCUSFILTERSIMPL_HXX__

#include "orcusfilters.hxx"

#define __ORCUS_STATIC_LIB
#include <orcus/xml_namespace.hpp>

class ScOrcusFiltersImpl : public ScOrcusFilters
{
public:
    static OString toSystemPath(const OUString& rPath);

    virtual bool importCSV(ScDocument& rDoc, const OUString& rPath) const;
    virtual bool importGnumeric(ScDocument& rDoc, const OUString& rPath) const;

    virtual ScOrcusXMLContext* createXMLContext(ScDocument& rDoc, const OUString& rPath) const;
};

class ScOrcusXMLContextImpl : public ScOrcusXMLContext
{
    ScDocument& mrDoc;
    OUString maPath;

    orcus::xmlns_repository maNsRepo; /// XML namespace repository for this context.

public:
    ScOrcusXMLContextImpl(ScDocument& rDoc, const OUString& rPath);
    virtual ~ScOrcusXMLContextImpl();

    virtual bool loadXMLStructure(SvTreeListBox& rTreeCtrl, ScOrcusXMLTreeParam& rParam);

    virtual bool importXML(const ScOrcusImportXMLParam& rParam);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
