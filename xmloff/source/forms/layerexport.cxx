/*************************************************************************
 *
 *  $RCSfile: layerexport.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-17 19:02:29 $
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

#ifndef _XMLOFF_FORMS_LAYEREXPORT_HXX_
#include "layerexport.hxx"
#endif
#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_ELEMENTEXPORT_HXX_
#include "elementexport.hxx"
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;

    //=====================================================================
    //= OFormLayerXMLExport_Impl
    //=====================================================================
    //---------------------------------------------------------------------
    OFormLayerXMLExport_Impl::OFormLayerXMLExport_Impl(SvXMLExport& _rContext)
        :m_rContext(_rContext)
    {
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::exportControl(const Reference< XPropertySet >& _rxControl)
    {
        // the list of the referring controls
        ::rtl::OUString sReferringControls;
        ConstMapPropertySet2StringIterator aReferring = m_aReferringControls.find(_rxControl);
        if (aReferring != m_aReferringControls.end())
            sReferringControls = aReferring->second;

        // the control id (should already have been created in collectReferences)
        ::rtl::OUString sControlId;
        ConstMapPropertySet2StringIterator aControlId = m_aControlIds.find(_rxControl);
        OSL_ENSHURE(aControlId != m_aControlIds.end(), "OFormLayerXMLExport_Impl::exportControl: could not find the control!");
        if (aControlId != m_aControlIds.end())
            sControlId = aControlId->second;

        // do the exporting
        OControlExport aExportImpl(m_rContext, _rxControl, sControlId, sReferringControls);
            // (done by the ctor and the dtor of the OControlExport object)
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::exportForm(const Reference< XPropertySet >& _rxProps) throw (Exception)
    {
        OSL_ENSHURE(_rxProps.is(), "OFormLayerXMLExport_Impl::exportForm: invalid property set!");
        OFormExport aAttributeHandler(m_rContext, this, _rxProps);
            // this object will do everything necessary ...
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::exportCollectionElements(const Reference< XIndexAccess >& _rxCollection)
    {
        // step through all the elements of the collection
        sal_Int32 nElements = _rxCollection->getCount();

        Reference< XPropertySet > xCurrentProps;
        Reference< XPropertySetInfo > xPropsInfo;
        Reference< XIndexAccess > xCurrentContainer;
        for (sal_Int32 i=0; i<nElements; ++i)
        {
            try
            {
                // extract the current element
                ::cppu::extractInterface(xCurrentProps, _rxCollection->getByIndex(i));
                OSL_ENSHURE(xCurrentProps.is(), "OFormLayerXMLExport_Impl::exportCollectionElements: invalid child element, skipping!");
                if (!xCurrentProps.is())
                    continue;

                // check if there is a ClassId property on the current element. If so, we assume it to be a control
                xPropsInfo = xCurrentProps->getPropertySetInfo();
                if (xPropsInfo.is() && xPropsInfo->hasPropertyByName(PROPERTY_CLASSID))
                {
                    exportControl(xCurrentProps);
                }
                else
                {
                    exportForm(xCurrentProps);
                }
            }
            catch(Exception&)
            {
                OSL_ENSHURE(sal_False, "OFormLayerXMLExport_Impl::exportCollectionElements: caught an exception ... skipping the current element!");
                continue;
            }
        }
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::collectReferences(const Reference< XIndexAccess >& _rxStart)
    {
        m_aControlIds.clear();
        m_aReferringControls.clear();

        ::std::stack< Reference< XIndexAccess > >   aContainerHistory;
        ::std::stack< sal_Int32 >                   aIndexHistory;

        Reference< XPropertySet >       xCurrent;
        Reference< XPropertySetInfo >   xCurrentInfo;
        Reference< XPropertySet >       xCurrentReference;

        const ::rtl::OUString sControlCheck(PROPERTY_CLASSID);
        const ::rtl::OUString sReferenceCheck(PROPERTY_CONTROLLABEL);
        const ::rtl::OUString sControlId(RTL_CONSTASCII_USTRINGPARAM("control"));

        Reference< XIndexAccess > xLoop = _rxStart;
        sal_Int32 nChildPos = 0;
        do
        {
            if (nChildPos < xLoop->getCount())
            {
                ::cppu::extractInterface(xCurrent, xLoop->getByIndex(nChildPos));
                OSL_ENSURE(xCurrent.is(), "OFormLayerXMLExport_Impl::collectReferences: invalid child object");
                if (!xCurrent.is())
                    continue;

                xCurrentInfo = xCurrent->getPropertySetInfo();
                OSL_ENSURE(xCurrentInfo.is(), "OFormLayerXMLExport_Impl::collectReferences: no property set info");
                if (xCurrentInfo->hasPropertyByName(sControlCheck))
                {
                    // generate a new control id

                    // find a free id
                    ::rtl::OUString sCurrentId = sControlId;
                    sCurrentId += ::rtl::OUString::valueOf((sal_Int32)(m_aControlIds.size() + 1));
                #ifdef DBG_UTIL
                    // Check if the id is already used. It shouldn't, as we currently have no mechanism for removing entries
                    // from the map, so the approach used above (take the map size) should be sufficient. But if somebody
                    // changes this (e.g. allows removing entries from the map), this assertion here probably will fail.
                    for (   ConstMapPropertySet2StringIterator aCheck = m_aControlIds.begin();
                            aCheck != m_aControlIds.end();
                            ++aCheck
                        )
                        OSL_ENSURE(aCheck->second != sCurrentId,
                            "OFormLayerXMLExport_Impl::collectReferences: auto-generated control ID is already used!");
                #endif
                    // add it to the map
                    m_aControlIds[xCurrent] = sCurrentId;

                    // check if this control has a "LabelControl" property referring another control
                    if (xCurrentInfo->hasPropertyByName(sReferenceCheck))
                    {
                        ::cppu::extractInterface(xCurrentReference, xCurrent->getPropertyValue(sReferenceCheck));
                        if (xCurrentReference.is())
                        {
                            ::rtl::OUString& sReferencedBy = m_aReferringControls[xCurrentReference];
                            if (sReferencedBy.getLength())
                                // it's not the first xCurrent referring to the xCurrentReference
                                // -> separate the id
                                sReferencedBy += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
                            sReferencedBy += sCurrentId;
                        }
                    }
                }
                else
                {
                    // step down
                    Reference< XIndexAccess > xNextContainer(xCurrent, UNO_QUERY);
                    OSL_ENSURE(xNextContainer.is(), "OFormLayerXMLExport_Impl::collectReferences: what the heck is this ... no control, no container?");
                    aContainerHistory.push(xLoop);
                    aIndexHistory.push(nChildPos);

                    xLoop = xNextContainer;
                    nChildPos = -1; // will be incremented below
                }
            }
            else
            {
                // step up
                while ((nChildPos >= xLoop->getCount()) && aContainerHistory.size())
                {
                    xLoop = aContainerHistory.top();
                    aContainerHistory.pop();
                    nChildPos = aIndexHistory.top();
                    aIndexHistory.pop();
                }
            }
            ++nChildPos;
        }
        while (xLoop.is() && aContainerHistory.size());
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 17.11.00 17:22:45  fs
 ************************************************************************/

