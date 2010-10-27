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
#ifndef _XMLOFF_SCH_XMLEXPORTHELPER_HXX_
#define _XMLOFF_SCH_XMLEXPORTHELPER_HXX_

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"
#include <xmloff/uniref.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/util/XStringMapping.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <xmloff/xmlprmap.hxx>

#include <queue>
#include <vector>

class SvXMLAutoStylePoolP;
class SvXMLExport;
class SchXMLExportHelper_Impl;

/** With this class you can export a <chart:chart> element containing
    its data as <table:table> element or without internal table. In
    the latter case you have to provide a table address mapper if the
    cell addressing set at the document is not in XML format.
 */
class XMLOFF_DLLPUBLIC SchXMLExportHelper : public UniRefBase
{
public:
    SchXMLExportHelper( SvXMLExport& rExport,
                        SvXMLAutoStylePoolP& rASPool );

    virtual ~SchXMLExportHelper();

    /// returns the string corresponding to the current FileFormat CLSID for Chart
    const rtl::OUString& getChartCLSID();

private:
    SchXMLExportHelper(); // not defined
    SchXMLExportHelper(SchXMLExportHelper &); // not defined
    void operator =(SchXMLExportHelper &); // not defined

private:
    SchXMLExportHelper_Impl* m_pImpl;
    friend class SchXMLExport;
};

#endif  // _XMLOFF_SCH_XMLEXPORTHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
