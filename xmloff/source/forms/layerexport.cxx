/*************************************************************************
 *
 *  $RCSfile: layerexport.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:13:21 $
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

#include <stdio.h>

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
#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include "prhdlfac.hxx"
#endif
#ifndef _XMLOFF_ELEMENTEXPORT_HXX_
#include "elementexport.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif
#ifndef _XMLOFF_CONTEXTID_HXX_
#include "contextid.hxx"
#endif
#ifndef _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_
#include "controlpropertyhdl.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

// #110680#
//#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
//#include <comphelper/processfactory.hxx>
//#endif

#ifndef _XMLOFF_FORMS_CONTROLPROPERTYMAP_HXX_
#include "controlpropertymap.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _XMLOFF_FORMS_EVENTEXPORT_HXX_
#include "eventexport.hxx"
#endif
#ifndef _XMLOFF_XMLEVENTEXPORT_HXX
#include "XMLEventExport.hxx"
#endif
#ifndef _XMLOFF_FORMS_FORMEVENTS_HXX_
#include "formevents.hxx"
#endif
#ifndef _XMLOFF_XMLNUMFE_HXX
#include "xmlnumfe.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
/** === end UNO includes === **/

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::drawing;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::text;

    //=====================================================================
    //= OFormLayerXMLExport_Impl
    //=====================================================================
    //---------------------------------------------------------------------
    const ::rtl::OUString& OFormLayerXMLExport_Impl::getControlNumberStyleNamePrefix()
    {
        static const ::rtl::OUString s_sControlNumberStyleNamePrefix = ::rtl::OUString::createFromAscii("C");
        return s_sControlNumberStyleNamePrefix;
    }

    //---------------------------------------------------------------------
    OFormLayerXMLExport_Impl::OFormLayerXMLExport_Impl(SvXMLExport& _rContext)
        :m_rContext(_rContext)
        ,m_pControlNumberStyles(NULL)
    {
        initializePropertyMaps();

        // add our style family to the export context's style pool
        m_xPropertyHandlerFactory = new OControlPropertyHandlerFactory();
        ::vos::ORef< XMLPropertySetMapper > xStylePropertiesMapper = new XMLPropertySetMapper( getControlStylePropertyMap(), m_xPropertyHandlerFactory.getBodyPtr() );
        m_xExportMapper = new OFormExportPropertyMapper( xStylePropertiesMapper.getBodyPtr() );

        // our style family
        m_rContext.GetAutoStylePool()->AddFamily(
            XML_STYLE_FAMILY_CONTROL_ID, token::GetXMLToken(token::XML_PARAGRAPH),
            m_xExportMapper.getBodyPtr(),
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_CONTROL_PREFIX) )
        );

        // add our event translation table
        m_rContext.GetEventExport().AddTranslationTable(g_pFormsEventTranslation);

        clear();
    }

    //---------------------------------------------------------------------
    sal_Bool OFormLayerXMLExport_Impl::implCheckPage(const Reference< XDrawPage >& _rxDrawPage, Reference< XIndexAccess >& _rxForms)
    {
        Reference< XFormsSupplier > xFormsSupp(_rxDrawPage, UNO_QUERY);
        OSL_ENSURE(xFormsSupp.is(), "OFormLayerXMLExport_Impl::implCheckPage: invalid draw page (no XFormsSupplier)! Doin' nothing!");
        if (!xFormsSupp.is())
            return sal_False;

        _rxForms = Reference< XIndexAccess >(xFormsSupp->getForms(), UNO_QUERY);
        Reference< XServiceInfo > xSI(_rxForms, UNO_QUERY); // order is important!
        OSL_ENSURE(xSI.is(), "OFormLayerXMLExport_Impl::implCheckPage: invalid collection (must not be NULL and must have a ServiceInfo)!");
        if (!xSI.is())
            return sal_False;

        if (!xSI->supportsService(SERVICE_FORMSCOLLECTION))
        {
            OSL_ENSURE(sal_False, "OFormLayerXMLExport_Impl::implCheckPage: invalid collection (is no com.sun.star.form.Forms)!");
            // nothing to do
            return sal_False;
        }
        return sal_True;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::exportGridColumn(const Reference< XPropertySet >& _rxColumn,
        const Sequence< ScriptEventDescriptor >& _rEvents)
    {
        // do the exporting
        OColumnExport aExportImpl(*this, _rxColumn, _rEvents);
        aExportImpl.doExport();
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::exportControl(const Reference< XPropertySet >& _rxControl,
        const Sequence< ScriptEventDescriptor >& _rEvents)
    {
        // the list of the referring controls
        ::rtl::OUString sReferringControls;
        ConstMapPropertySet2StringIterator aReferring = m_aCurrentPageReferring->second.find(_rxControl);
        if (aReferring != m_aCurrentPageReferring->second.end())
            sReferringControls = aReferring->second;

        // the control id (should already have been created in examineForms)
        ::rtl::OUString sControlId;
        ConstMapPropertySet2StringIterator aControlId = m_aCurrentPageIds->second.find(_rxControl);
        OSL_ENSURE(aControlId != m_aCurrentPageIds->second.end(), "OFormLayerXMLExport_Impl::exportControl: could not find the control id!");
        if (aControlId != m_aCurrentPageIds->second.end())
            sControlId = aControlId->second;

        // do the exporting
        OControlExport aExportImpl(*this, _rxControl, sControlId, sReferringControls, _rEvents);
        aExportImpl.doExport();
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::exportForm(const Reference< XPropertySet >& _rxProps,
        const Sequence< ScriptEventDescriptor >& _rEvents)
    {
        OSL_ENSURE(_rxProps.is(), "OFormLayerXMLExport_Impl::exportForm: invalid property set!");
        OFormExport aAttributeHandler(*this, _rxProps, _rEvents);
        aAttributeHandler.doExport();
    }

    //---------------------------------------------------------------------
    ::vos::ORef< SvXMLExportPropertyMapper > OFormLayerXMLExport_Impl::getStylePropertyMapper()
    {
        return m_xExportMapper;
    }

    //---------------------------------------------------------------------
    SvXMLExport& OFormLayerXMLExport_Impl::getGlobalContext()
    {
        return m_rContext;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::exportCollectionElements(const Reference< XIndexAccess >& _rxCollection)
    {
        // step through all the elements of the collection
        sal_Int32 nElements = _rxCollection->getCount();

        Reference< XEventAttacherManager > xElementEventManager(_rxCollection, UNO_QUERY);
        Sequence< ScriptEventDescriptor > aElementEvents;

        Reference< XPropertySet > xCurrentProps;
        Reference< XPropertySetInfo > xPropsInfo;
        Reference< XIndexAccess > xCurrentContainer;
        for (sal_Int32 i=0; i<nElements; ++i)
        {
            try
            {
                // extract the current element
                ::cppu::extractInterface(xCurrentProps, _rxCollection->getByIndex(i));
                OSL_ENSURE(xCurrentProps.is(), "OFormLayerXMLExport_Impl::exportCollectionElements: invalid child element, skipping!");
                if (!xCurrentProps.is())
                    continue;

                // check if there is a ClassId property on the current element. If so, we assume it to be a control
                xPropsInfo = xCurrentProps->getPropertySetInfo();
                OSL_ENSURE(xPropsInfo.is(), "OFormLayerXMLExport_Impl::exportCollectionElements: no property set info!");
                if (!xPropsInfo.is())
                    // without this, a lot of stuff in the export routines may fail
                    continue;

                // if the element is part of a ignore list, we are not allowed to export it
                if ( m_aIgnoreList.end() != m_aIgnoreList.find( xCurrentProps ) )
                    continue;

                if (xElementEventManager.is())
                    aElementEvents = xElementEventManager->getScriptEvents(i);

                if (xPropsInfo->hasPropertyByName(PROPERTY_COLUMNSERVICENAME))
                {
                    exportGridColumn(xCurrentProps, aElementEvents);
                }
                else if (xPropsInfo->hasPropertyByName(PROPERTY_CLASSID))
                {
                    exportControl(xCurrentProps, aElementEvents);
                }
                else
                {
                    exportForm(xCurrentProps, aElementEvents);
                }
            }
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "OFormLayerXMLExport_Impl::exportCollectionElements: caught an exception ... skipping the current element!");
                continue;
            }
        }
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OFormLayerXMLExport_Impl::getObjectStyleName( const Reference< XPropertySet >& _rxObject )
    {
        ::rtl::OUString aObjectStyle;

        MapPropertySet2String::const_iterator aObjectStylePos = m_aGridColumnStyles.find( _rxObject );
        if ( m_aGridColumnStyles.end() != aObjectStylePos )
            aObjectStyle = aObjectStylePos->second;
        return aObjectStyle;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::clear()
    {
        m_aControlIds.clear();
        m_aReferringControls.clear();
        m_aCurrentPageIds = m_aControlIds.end();
        m_aCurrentPageReferring = m_aReferringControls.end();

        m_aControlNumberFormats.clear();
        m_aGridColumnStyles.clear();

        m_aIgnoreList.clear();
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::exportControlNumberStyles()
    {
        if (m_pControlNumberStyles)
            m_pControlNumberStyles->Export(sal_False);
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::exportAutoControlNumberStyles()
    {
        if ( m_pControlNumberStyles )
            m_pControlNumberStyles->Export( sal_True );
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::exportAutoStyles()
    {
        m_rContext.GetAutoStylePool()->exportXML(
            XML_STYLE_FAMILY_CONTROL_ID,
            m_rContext.GetDocHandler(),
            m_rContext.GetMM100UnitConverter(),
            m_rContext.GetNamespaceMap()
        );
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::exportForms(const Reference< XDrawPage >& _rxDrawPage)
    {
        // get the forms collection of the page
        Reference< XIndexAccess > xCollectionIndex;
        if (!implCheckPage(_rxDrawPage, xCollectionIndex))
            return;

#if OSL_DEBUG_LEVEL > 0
        sal_Bool bPageIsKnown =
#endif
            implMoveIterators(_rxDrawPage, sal_False);
        OSL_ENSURE(bPageIsKnown, "OFormLayerXMLExport_Impl::exportForms: exporting a page which has not been examined!");

        exportCollectionElements(xCollectionIndex);
    }

    //---------------------------------------------------------------------
    sal_Bool OFormLayerXMLExport_Impl::implMoveIterators(const Reference< XDrawPage >& _rxDrawPage, sal_Bool _bClear)
    {
        sal_Bool bKnownPage = sal_False;

        // the one for the ids
        m_aCurrentPageIds = m_aControlIds.find(_rxDrawPage);
        if (m_aControlIds.end() == m_aCurrentPageIds)
        {
            m_aControlIds[_rxDrawPage] = MapPropertySet2String();
            m_aCurrentPageIds = m_aControlIds.find(_rxDrawPage);
        }
        else
        {
            bKnownPage = sal_True;
            if (_bClear && m_aCurrentPageIds->second.size())
                m_aCurrentPageIds->second.clear();
        }

        // the one for the ids of the referring controls
        m_aCurrentPageReferring = m_aReferringControls.find(_rxDrawPage);
        if (m_aReferringControls.end() == m_aCurrentPageReferring)
        {
            m_aReferringControls[_rxDrawPage] = MapPropertySet2String();
            m_aCurrentPageReferring = m_aReferringControls.find(_rxDrawPage);
        }
        else
        {
            bKnownPage = sal_True;
            if (_bClear && m_aCurrentPageReferring->second.size())
                m_aCurrentPageReferring->second.clear();
        }
        return bKnownPage;
    }

    //---------------------------------------------------------------------
    sal_Bool OFormLayerXMLExport_Impl::seekPage(const Reference< XDrawPage >& _rxDrawPage)
    {
        return implMoveIterators(_rxDrawPage, sal_False);
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OFormLayerXMLExport_Impl::getControlId(const Reference< XPropertySet >& _rxControl)
    {
        OSL_ENSURE(m_aCurrentPageIds != m_aControlIds.end(), "OFormLayerXMLExport_Impl::getControlId: invalid current page!");
        OSL_ENSURE(m_aCurrentPageIds->second.end() != m_aCurrentPageIds->second.find(_rxControl),
            "OFormLayerXMLExport_Impl::getControlId: can not find the control!");
        return m_aCurrentPageIds->second[_rxControl];
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OFormLayerXMLExport_Impl::getImmediateNumberStyle( const Reference< XPropertySet >& _rxObject )
    {
        ::rtl::OUString sNumberStyle;

        sal_Int32 nOwnFormatKey = implExamineControlNumberFormat( _rxObject );
        if ( -1 != nOwnFormatKey )
            sNumberStyle = getControlNumberStyleExport()->GetStyleName( nOwnFormatKey );

        return sNumberStyle;
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OFormLayerXMLExport_Impl::getControlNumberStyle( const Reference< XPropertySet >& _rxControl )
    {
        ::rtl::OUString sNumberStyle;

        ConstMapPropertySet2IntIterator aControlFormatPos = m_aControlNumberFormats.find(_rxControl);
        if (m_aControlNumberFormats.end() != aControlFormatPos)
        {
            OSL_ENSURE(m_pControlNumberStyles, "OFormLayerXMLExport_Impl::getControlNumberStyle: have a control which has a format style, but no style exporter!");
            sNumberStyle = getControlNumberStyleExport()->GetStyleName(aControlFormatPos->second);
        }
        // it's allowed to ask for a control which does not have format information.
        // (This is for performance reasons)

        return sNumberStyle;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::examineForms(const Reference< XDrawPage >& _rxDrawPage)
    {
        // get the forms collection of the page
        Reference< XIndexAccess > xCollectionIndex;
        if (!implCheckPage(_rxDrawPage, xCollectionIndex))
            return;

        // move the iterator which specify the currently handled page
#if OSL_DEBUG_LEVEL > 0
        sal_Bool bPageIsKnown =
#endif
            implMoveIterators(_rxDrawPage, sal_True);
        OSL_ENSURE(!bPageIsKnown, "OFormLayerXMLExport_Impl::examineForms: examining a page twice!");

        ::std::stack< Reference< XIndexAccess > >   aContainerHistory;
        ::std::stack< sal_Int32 >                   aIndexHistory;

        Reference< XPropertySet >       xCurrent;

        Reference< XIndexAccess > xLoop = xCollectionIndex;
        sal_Int32 nChildPos = 0;
        do
        {
            if (nChildPos < xLoop->getCount())
            {
                ::cppu::extractInterface(xCurrent, xLoop->getByIndex(nChildPos));
                OSL_ENSURE(xCurrent.is(), "OFormLayerXMLExport_Impl::examineForms: invalid child object");
                if (!xCurrent.is())
                    continue;

                if (!checkExamineControl(xCurrent))
                {
                    // step down
                    Reference< XIndexAccess > xNextContainer(xCurrent, UNO_QUERY);
                    OSL_ENSURE(xNextContainer.is(), "OFormLayerXMLExport_Impl::examineForms: what the heck is this ... no control, no container?");
                    aContainerHistory.push(xLoop);
                    aIndexHistory.push(nChildPos);

                    xLoop = xNextContainer;
                    nChildPos = -1; // will be incremented below
                }
                ++nChildPos;
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

                    ++nChildPos;
                }
                if (nChildPos >= xLoop->getCount())
                    // exited the loop above because we have no history anymore (0 == aContainerHistory.size()),
                    // and on the current level there are no more children
                    // -> leave
                    break;
            }
        }
        while (xLoop.is());
    }

    //---------------------------------------------------------------------
    sal_Bool OFormLayerXMLExport_Impl::checkExamineControl(const Reference< XPropertySet >& _rxObject)
    {
        static const ::rtl::OUString sControlId(RTL_CONSTASCII_USTRINGPARAM("control"));

        Reference< XPropertySetInfo > xCurrentInfo = _rxObject->getPropertySetInfo();
        OSL_ENSURE(xCurrentInfo.is(), "OFormLayerXMLExport_Impl::checkExamineControl: no property set info");

        sal_Bool bIsControl = xCurrentInfo->hasPropertyByName( PROPERTY_CLASSID );
        if (bIsControl)
        {
            // ----------------------------------
            // generate a new control id

            // find a free id
            ::rtl::OUString sCurrentId = sControlId;
            sCurrentId += ::rtl::OUString::valueOf((sal_Int32)(m_aCurrentPageIds->second.size() + 1));
        #ifdef DBG_UTIL
            // Check if the id is already used. It shouldn't, as we currently have no mechanism for removing entries
            // from the map, so the approach used above (take the map size) should be sufficient. But if somebody
            // changes this (e.g. allows removing entries from the map), this assertion here probably will fail.
            for (   ConstMapPropertySet2StringIterator aCheck = m_aCurrentPageIds->second.begin();
                    aCheck != m_aCurrentPageIds->second.end();
                    ++aCheck
                )
                OSL_ENSURE(aCheck->second != sCurrentId,
                    "OFormLayerXMLExport_Impl::checkExamineControl: auto-generated control ID is already used!");
        #endif
            // add it to the map
            m_aCurrentPageIds->second[_rxObject] = sCurrentId;

            // ----------------------------------
            // check if this control has a "LabelControl" property referring another control
            if ( xCurrentInfo->hasPropertyByName( PROPERTY_CONTROLLABEL ) )
            {
                Reference< XPropertySet > xCurrentReference;
                ::cppu::extractInterface( xCurrentReference, _rxObject->getPropertyValue( PROPERTY_CONTROLLABEL ) );
                if (xCurrentReference.is())
                {
                    ::rtl::OUString& sReferencedBy = m_aCurrentPageReferring->second[xCurrentReference];
                    if (sReferencedBy.getLength())
                        // it's not the first _rxObject referring to the xCurrentReference
                        // -> separate the id
                        sReferencedBy += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
                    sReferencedBy += sCurrentId;
                }
            }

            // ----------------------------------
            // check if the control needs a number format style
            if ( xCurrentInfo->hasPropertyByName( PROPERTY_FORMATKEY ) )
            {
                examineControlNumberFormat(_rxObject);
            }

            // ----------------------------------
            // check if it's a control providing text
            Reference< XText > xControlText( _rxObject, UNO_QUERY );
            if ( xControlText.is() )
            {
                m_rContext.GetTextParagraphExport()->collectTextAutoStyles( xControlText );
            }

            // ----------------------------------
            // check if it is a grid control - in this case, we need special handling for the columns
            sal_Int16 nControlType = FormComponentType::CONTROL;
            _rxObject->getPropertyValue( PROPERTY_CLASSID ) >>= nControlType;
            if ( FormComponentType::GRIDCONTROL == nControlType )
            {
                collectGridAutoStyles( _rxObject );
            }
        }

        return bIsControl;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::collectGridAutoStyles( const Reference< XPropertySet >& _rxControl )
    {
        // loop through all columns of the grid
        try
        {
            Reference< XIndexAccess > xContainer( _rxControl, UNO_QUERY );
            OSL_ENSURE( xContainer.is(), "OFormLayerXMLExport_Impl::collectGridAutoStyles: grid control not being a container?!" );
            if ( xContainer.is() )
            {
                Reference< XPropertySet > xColumnProperties;
                Reference< XPropertySetInfo > xColumnPropertiesMeta;

                sal_Int32 nCount = xContainer->getCount();
                for ( sal_Int32 i=0; i<nCount; ++i )
                {
                    if ( xContainer->getByIndex( i ) >>= xColumnProperties )
                    {
                        xColumnPropertiesMeta = xColumnProperties->getPropertySetInfo();
                        // get the styles of the column
                        ::std::vector< XMLPropertyState > aPropertyStates = m_xExportMapper->Filter( xColumnProperties );

                        // care for the number format, additionally
                        ::rtl::OUString sColumnNumberStyle;
                        if ( xColumnPropertiesMeta.is() && xColumnPropertiesMeta->hasPropertyByName( PROPERTY_FORMATKEY ) )
                            sColumnNumberStyle = getImmediateNumberStyle( xColumnProperties );

                        if ( sColumnNumberStyle.getLength() )
                        {   // the column indeed has a formatting
                            sal_Int32 nStyleMapIndex = m_xExportMapper->getPropertySetMapper()->FindEntryIndex( CTF_FORMS_DATA_STYLE );
                                // TODO: move this to the ctor
                            OSL_ENSURE ( -1 != nStyleMapIndex, "XMLShapeExport::collectShapeAutoStyles: could not obtain the index for our context id!");

                            XMLPropertyState aNumberStyleState( nStyleMapIndex, makeAny( sColumnNumberStyle ) );
                            aPropertyStates.push_back( aNumberStyleState );
                        }

#if OSL_DEBUG_LEVEL > 0
                        ::std::vector< XMLPropertyState >::const_iterator aHaveALook = aPropertyStates.begin();
                        for ( ; aHaveALook != aPropertyStates.end(); ++aHaveALook )
                        {
                            sal_Int32 nDummy = 0;
                        }
#endif

                        if ( aPropertyStates.size() )
                        {   // add to the style pool
                            ::rtl::OUString sColumnStyleName = m_rContext.GetAutoStylePool()->Add( XML_STYLE_FAMILY_CONTROL_ID, aPropertyStates );

                            OSL_ENSURE( m_aGridColumnStyles.end() == m_aGridColumnStyles.find( xColumnProperties ),
                                "OFormLayerXMLExport_Impl::collectGridAutoStyles: already have a style for this column!" );

                            m_aGridColumnStyles.insert( MapPropertySet2String::value_type( xColumnProperties, sColumnStyleName ) );
                        }
                    }
                    else
                        OSL_ENSURE( sal_False, "OFormLayerXMLExport_Impl::collectGridAutoStyles: invalid grid column encountered!" );
                }
            }
        }
        catch( const Exception& e )
        {
            e;  // make compiler happy
            OSL_ENSURE( sal_False, "OFormLayerXMLExport_Impl::collectGridAutoStyles: error examining the grid colums!" );
        }
    }

    //---------------------------------------------------------------------
    sal_Int32 OFormLayerXMLExport_Impl::implExamineControlNumberFormat( const Reference< XPropertySet >& _rxObject )
    {
        // get the format key relative to our own formats supplier
        sal_Int32 nOwnFormatKey = ensureTranslateFormat( _rxObject );

        if ( -1 != nOwnFormatKey )
            // tell the exporter that we used this format
            getControlNumberStyleExport()->SetUsed( nOwnFormatKey );

        return nOwnFormatKey;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::examineControlNumberFormat( const Reference< XPropertySet >& _rxControl )
    {
        sal_Int32 nOwnFormatKey = implExamineControlNumberFormat( _rxControl );

        if ( -1 == nOwnFormatKey )
            // nothing to do, the number format of this control is void
            return;

        // remember the format key for this control (we'll be asked in getControlNumberStyle for this)
        OSL_ENSURE(m_aControlNumberFormats.end() == m_aControlNumberFormats.find(_rxControl),
            "OFormLayerXMLExport_Impl::examineControlNumberFormat: already handled this control!");
        m_aControlNumberFormats[_rxControl] = nOwnFormatKey;
    }

    //---------------------------------------------------------------------
    sal_Int32 OFormLayerXMLExport_Impl::ensureTranslateFormat(const Reference< XPropertySet >& _rxFormattedControl)
    {
        ensureControlNumberStyleExport();
        OSL_ENSURE(m_xControlNumberFormats.is(), "OFormLayerXMLExport_Impl::ensureTranslateFormat: no own formats supplier!");
            // (should have been created in ensureControlNumberStyleExport)

        sal_Int32 nOwnFormatKey = -1;

        // the format key (relative to the control's supplier)
        sal_Int32 nControlFormatKey = -1;
        Any aControlFormatKey = _rxFormattedControl->getPropertyValue(PROPERTY_FORMATKEY);
        if (aControlFormatKey >>= nControlFormatKey)
        {
            // the control's number format
            Reference< XNumberFormatsSupplier > xControlFormatsSupplier;
            _rxFormattedControl->getPropertyValue(PROPERTY_FORMATSSUPPLIER) >>= xControlFormatsSupplier;
            Reference< XNumberFormats > xControlFormats;
            if (xControlFormatsSupplier.is())
                xControlFormats = xControlFormatsSupplier->getNumberFormats();
            OSL_ENSURE(xControlFormats.is(), "OFormLayerXMLExport_Impl::ensureTranslateFormat: formatted control without supplier!");

            // obtain the persistent (does not depend on the formats supplier) representation of the control's format
            Locale aFormatLocale;
            ::rtl::OUString sFormatDescription;
            if (xControlFormats.is())
            {
                Reference< XPropertySet > xControlFormat = xControlFormats->getByKey(nControlFormatKey);

                xControlFormat->getPropertyValue(PROPERTY_LOCALE)       >>= aFormatLocale;
                xControlFormat->getPropertyValue(PROPERTY_FORMATSTRING) >>= sFormatDescription;
            }

            // check if our own formats collection already knows the format
            nOwnFormatKey = m_xControlNumberFormats->queryKey(sFormatDescription, aFormatLocale, sal_False);
            if (-1 == nOwnFormatKey)
            {   // no, we don't
                // -> create a new format
                nOwnFormatKey = m_xControlNumberFormats->addNew(sFormatDescription, aFormatLocale);
            }
            OSL_ENSURE(-1 != nOwnFormatKey, "OFormLayerXMLExport_Impl::ensureTranslateFormat: could not translate the controls format key!");
        }
        else
            OSL_ENSURE(!aControlFormatKey.hasValue(), "OFormLayerXMLExport_Impl::ensureTranslateFormat: invalid number format property value!");

        return nOwnFormatKey;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::ensureControlNumberStyleExport()
    {
        if (!m_pControlNumberStyles)
        {
            // create our number formats supplier (if necessary)
            Reference< XNumberFormatsSupplier > xFormatsSupplier;

            OSL_ENSURE(!m_xControlNumberFormats.is(), "OFormLayerXMLExport_Impl::getControlNumberStyleExport: inconsistence!");
                // the m_xControlNumberFormats and m_pControlNumberStyles should be maintained together

            try
            {
                // create it for en-US (does not really matter, as we will specify a locale for every
                // concrete language to use)
                Sequence< Any > aSupplierArgs(1);
                aSupplierArgs[0] <<= Locale (   ::rtl::OUString::createFromAscii("en"),
                                                ::rtl::OUString::createFromAscii("US"),
                                                ::rtl::OUString()
                                            );
                // #110680#
                //Reference< XInterface > xFormatsSupplierUntyped =
                //  ::comphelper::getProcessServiceFactory()->createInstanceWithArguments(
                //      SERVICE_NUMBERFORMATSSUPPLIER,
                //      aSupplierArgs
                //  );
                Reference< XInterface > xFormatsSupplierUntyped =
                    m_rContext.getServiceFactory()->createInstanceWithArguments(
                        SERVICE_NUMBERFORMATSSUPPLIER,
                        aSupplierArgs
                    );
                OSL_ENSURE(xFormatsSupplierUntyped.is(), "OFormLayerXMLExport_Impl::getControlNumberStyleExport: could not instantiate a number formats supplier!");

                xFormatsSupplier = Reference< XNumberFormatsSupplier >(xFormatsSupplierUntyped, UNO_QUERY);
                if (xFormatsSupplier.is())
                    m_xControlNumberFormats = xFormatsSupplier->getNumberFormats();
            }
            catch(const Exception&)
            {
            }

            OSL_ENSURE(m_xControlNumberFormats.is(), "OFormLayerXMLExport_Impl::getControlNumberStyleExport: could not obtain my default number formats!");

            // create the exporter
            m_pControlNumberStyles = new SvXMLNumFmtExport(m_rContext, xFormatsSupplier, getControlNumberStyleNamePrefix());
        }
    }

    //---------------------------------------------------------------------
    SvXMLNumFmtExport* OFormLayerXMLExport_Impl::getControlNumberStyleExport()
    {
        ensureControlNumberStyleExport();
        return m_pControlNumberStyles;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLExport_Impl::excludeFromExport( const Reference< XControlModel > _rxControl )
    {
        Reference< XPropertySet > xProps( _rxControl, UNO_QUERY );
        OSL_ENSURE( xProps.is(), "OFormLayerXMLExport_Impl::excludeFromExport: invalid control model!" );
#if OSL_DEBUG_LEVEL > 0
        ::std::pair< PropertySetBag::iterator, bool > aPos =
#endif
        m_aIgnoreList.insert( xProps );
        OSL_ENSURE( aPos.second, "OFormLayerXMLExport_Impl::excludeFromExport: element already exists in the ignore list!" );
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................


