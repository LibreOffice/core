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

#ifndef _XMLOFF_SCH_XMLEXPORTHELPER_HXX_
#include <bf_xmloff/SchXMLExportHelper.hxx>
#endif
#ifndef _SCH_XMLAUTOSTYLEPOOLP_HXX_
#include "SchXMLAutoStylePoolP.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include <bf_xmloff/xmlexp.hxx>
#endif
#ifndef _UNIVERSALL_REFERENCE_HXX
#include <bf_xmloff/uniref.hxx>
#endif
#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include <bf_xmloff/xmlprmap.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include <bf_xmloff/prhdlfac.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace chart {
        class XDiagram;
        class XChartDocument;
        class XChartDataArray;
        struct ChartSeriesAddress;
    }
    namespace drawing {
        class XShape;
    }
    namespace task {
        class XStatusIndicator;
    }
}}}
namespace binfilter {

class SvXMLAutoStylePoolP;
class SvXMLUnitConverter;
class XMLChartExportPropertyMapper;

// ------------------------------------------
// export class for a complete chart document
// ------------------------------------------

class SchXMLExport : public SvXMLExport
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > mxStatusIndicator;
    SchXMLAutoStylePoolP maAutoStylePool;

    SchXMLExportHelper maExportHelper;

protected:
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

    UniReference< XMLPropertySetMapper > GetPropertySetMapper() const { return maExportHelper.GetPropertySetMapper(); }

    // XServiceInfo ( : SvXMLExport )
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
};

}//end of namespace binfilter
#endif	// SCH_XMLEXPORT_HXX_
