/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <memory>

class ScDocument;
struct ScOrcusXMLTreeParam;
struct ScOrcusImportXMLParam;
class ScOrcusXMLContext;
class SfxMedium;
namespace weld { class TreeView; }

/**
 * Collection of orcus filter wrappers.
 */
class ScOrcusFilters
{
public:
    virtual ~ScOrcusFilters() {}

    virtual bool importCSV(ScDocument& rDoc, SfxMedium& rMedium) const = 0;

    virtual bool importGnumeric(ScDocument& rDoc, SfxMedium& rMedium) const = 0;

    virtual bool importExcel2003XML(ScDocument& rDoc, SfxMedium& rMedium) const = 0;

    virtual bool importXLSX(ScDocument& rDoc, SfxMedium& rMedium) const = 0;

    virtual bool importODS(ScDocument& rDoc, SfxMedium& rMedium) const = 0;

    /**
     * Used to import just the styles from an xml file.
     */

    virtual bool importODS_Styles(ScDocument& rDoc, OUString& aFileName) const = 0;

    /**
     * Create a context for XML file.  The context object stores session
     * information for each unique XML file.  You must create a new context
     * for each XML file, and never to re-use the same context for multiple
     * XML files.
     *
     * The caller is responsible for deleting the instance returned from this
     * method when it's done.
     */
    virtual std::unique_ptr<ScOrcusXMLContext> createXMLContext(ScDocument& rDoc, const OUString& rPath) const = 0;
};

class ScOrcusXMLContext
{
public:
    virtual ~ScOrcusXMLContext() {}

    virtual void loadXMLStructure(weld::TreeView& rTreeCtrl, ScOrcusXMLTreeParam& rParam) = 0;

    virtual void importXML(const ScOrcusImportXMLParam& rParam) = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
