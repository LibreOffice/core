/*************************************************************************
 *
 *  $RCSfile: formlayerexport.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-17 19:02:16 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#ifndef _XMLOFF_FORMLAYEREXPORT_HXX_
#include "formlayerexport.hxx"
#endif
#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif
#ifndef _XMLOFF_ELEMENTEXPORT_HXX_
#include "elementexport.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _XMLOFF_FORMS_LAYEREXPORT_HXX_
#include "layerexport.hxx"
#endif
#ifndef _XMLOFF_FORMS_PROPERTYEXPORT_HXX_
#include "propertyexport.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif


//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;

    //=====================================================================
    //= OFormLayerXMLExport
    //=====================================================================

    //---------------------------------------------------------------------
    OFormLayerXMLExport::OFormLayerXMLExport(SvXMLExport& _rContext)
        :m_rContext(_rContext)
        ,m_pImpl(new OFormLayerXMLExport_Impl(_rContext))
    {
    }

    //---------------------------------------------------------------------
    OFormLayerXMLExport::~OFormLayerXMLExport()
    {
        delete m_pImpl;
        m_pImpl = NULL;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport::exportStructure(const Reference< XNameAccess >& _rxFormsCollection)
    {
        Reference< XIndexAccess > xCollectionIndex(_rxFormsCollection, UNO_QUERY);
        Reference< XServiceInfo > xSI(xCollectionIndex, UNO_QUERY); // order is important!
        OSL_ENSHURE(xSI.is(), "OFormLayerXMLExport::exportStructure: invalid collection (must not be NULL and must have a ServiceInfo)!");
        if (!xSI.is())
            return;

        if (!xSI->supportsService(SERVICE_FORMSCOLLECTION))
        {
            OSL_ENSHURE(sal_False, "OFormLayerXMLExport::exportStructure: invalid collection (is no FormsCollection)!");
            // nothing to do
            return;
        }

        // let the control exporter create ids for the controls
        // They are needed at least for the references controls may have to other controls (e.g. when a
        // radio button referes to a group box as it's frame), so we would need at least the ids of all controls
        // which may be referring other controls. This way, we need to loop through the complete forms/controls
        // structure before doing any real export, just to collect _some_ ids. So why shouldn't we collect _all_
        // of them immediately ?
        try
        {
            m_pImpl->collectReferences(xCollectionIndex);
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OFormLayerXMLExport::exportStructure: could not collect the control ids!");
        }

        // let the impl method export this top-level collection
        m_pImpl->exportCollectionElements(xCollectionIndex);
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 13.11.00 14:58:17  fs
 ************************************************************************/

