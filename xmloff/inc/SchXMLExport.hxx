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
#ifndef SCH_XMLEXPORT_HXX_
#define SCH_XMLEXPORT_HXX_

#include <xmloff/SchXMLExportHelper.hxx>
#include "SchXMLAutoStylePoolP.hxx"
#include <xmloff/xmlexp.hxx>
#include <xmloff/uniref.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/prhdlfac.hxx>

namespace com { namespace sun { namespace star {
    namespace chart {
        class XDiagram;
        class XChartDocument;
        struct ChartSeriesAddress;
    }
    namespace drawing {
        class XShape;
    }
    namespace task {
        class XStatusIndicator;
    }
}}}

class SvXMLAutoStylePoolP;
class SvXMLUnitConverter;
class XMLChartExportPropertyMapper;

// ------------------------------------------
// export class for a complete chart document
// ------------------------------------------

class SchXMLExport : public SvXMLExport
{
private:
    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > mxStatusIndicator;
    SchXMLAutoStylePoolP maAutoStylePool;

    SchXMLExportHelper maExportHelper;

protected:
    virtual sal_uInt32 exportDoc( enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID );

    virtual void _ExportStyles( sal_Bool bUsed );
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void _ExportContent();

public:
    // #110680#
    SchXMLExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
        sal_uInt16 nExportFlags = EXPORT_ALL );
    virtual ~SchXMLExport();

    void SetProgress( sal_Int32 nPercentage );

    UniReference< XMLPropertySetMapper > GetPropertySetMapper() const;

    // XServiceInfo ( : SvXMLExport )
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
};

#endif  // SCH_XMLEXPORT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
