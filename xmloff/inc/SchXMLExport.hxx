/*************************************************************************
 *
 *  $RCSfile: SchXMLExport.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: bm $ $Date: 2001-03-04 12:30:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef SCH_XMLEXPORT_HXX_
#define SCH_XMLEXPORT_HXX_

#ifndef _XMLOFF_SCH_XMLEXPORTHELPER_HXX_
#include "SchXMLExportHelper.hxx"
#endif
#ifndef _SCH_XMLAUTOSTYLEPOOLP_HXX_
#include "SchXMLAutoStylePoolP.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _UNIVERSALL_REFERENCE_HXX
#include "uniref.hxx"
#endif
#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include "prhdlfac.hxx"
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
    virtual void _ExportStyles( sal_Bool bUsed );
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void _ExportContent();

public:
    SchXMLExport( sal_uInt16 nExportFlags = EXPORT_ALL );
    virtual ~SchXMLExport();

    void SetProgress( sal_Int32 nPercentage );

    UniReference< XMLPropertySetMapper > GetPropertySetMapper() const { return maExportHelper.GetPropertySetMapper(); }
};

#endif  // SCH_XMLEXPORT_HXX_
