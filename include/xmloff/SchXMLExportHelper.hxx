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
#ifndef INCLUDED_XMLOFF_SCHXMLEXPORTHELPER_HXX
#define INCLUDED_XMLOFF_SCHXMLEXPORTHELPER_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <rtl/ustring.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <memory>

class SvXMLAutoStylePoolP;
class SvXMLExport;
class SchXMLExportHelper_Impl;

/** With this class you can export a <chart:chart> element containing
    its data as <table:table> element or without internal table. In
    the latter case you have to provide a table address mapper if the
    cell addressing set at the document is not in XML format.
 */
class XMLOFF_DLLPUBLIC SchXMLExportHelper : public salhelper::SimpleReferenceObject
{
public:
    SchXMLExportHelper( SvXMLExport& rExport,
                        SvXMLAutoStylePoolP& rASPool );

    virtual ~SchXMLExportHelper() override;

    /// returns the string corresponding to the current FileFormat CLSID for Chart
    const OUString& getChartCLSID();

    void SetSourceShellID( const OUString& rShellID );
    void SetDestinationShellID( const OUString& rShellID );

private:
    SchXMLExportHelper(SchXMLExportHelper const &) = delete;
    SchXMLExportHelper& operator =(SchXMLExportHelper const &) = delete;

private:
    std::unique_ptr<SchXMLExportHelper_Impl> m_pImpl;
    friend class SchXMLExport;
};

#endif // INCLUDED_XMLOFF_SCHXMLEXPORTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
