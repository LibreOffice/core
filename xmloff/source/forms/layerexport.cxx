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

#include <stdio.h>
#include "layerexport.hxx"
#include "strings.hxx"
#include <xmloff/xmlexp.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/prhdlfac.hxx>
#include "elementexport.hxx"
#include <xmloff/families.hxx>
#include <xmloff/contextid.hxx>
#include <xmloff/controlpropertyhdl.hxx>
#include <tools/diagnose_ex.h>
#include "controlpropertymap.hxx"
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/form/XFormsSupplier2.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/util/NumberFormatsSupplier.hpp>
#include "eventexport.hxx"
#include <xmloff/XMLEventExport.hxx>
#include "formevents.hxx"
#include <xmloff/xmlnumfe.hxx>
#include "xmloff/xformsexport.hxx"
#include "comphelper/processfactory.hxx"

#include <com/sun/star/text/XText.hpp>

#include <numeric>

namespace xmloff
{

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

    typedef ::com::sun::star::xforms::XFormsSupplier XXFormsSupplier;

    //= OFormLayerXMLExport_Impl
    const OUString& OFormLayerXMLExport_Impl::getControlNumberStyleNamePrefix()
    {
        static const OUString s_sControlNumberStyleNamePrefix("C");
        return s_sControlNumberStyleNamePrefix;
    }

    OFormLayerXMLExport_Impl::OFormLayerXMLExport_Impl(SvXMLExport& _rContext)
        :m_rContext(_rContext)
        ,m_pControlNumberStyles(NULL)
    {
        initializePropertyMaps();

        // add our style family to the export context's style pool
        m_xPropertyHandlerFactory = new OControlPropertyHandlerFactory();
        ::rtl::Reference< XMLPropertySetMapper > xStylePropertiesMapper = new XMLPropertySetMapper( getControlStylePropertyMap(), m_xPropertyHandlerFactory.get() );
        m_xStyleExportMapper = new OFormComponentStyleExportMapper( xStylePropertiesMapper.get() );

        // our style family
        m_rContext.GetAutoStylePool()->AddFamily(
            XML_STYLE_FAMILY_CONTROL_ID, token::GetXMLToken(token::XML_PARAGRAPH),
            m_xStyleExportMapper.get(),
            OUString(  XML_STYLE_FAMILY_CONTROL_PREFIX )
        );

        // add our event translation table
        m_rContext.GetEventExport().AddTranslationTable(g_pFormsEventTranslation);

        clear();
    }

    OFormLayerXMLExport_Impl::~OFormLayerXMLExport_Impl()
    {
    }

    sal_Bool OFormLayerXMLExport_Impl::impl_isFormPageContainingForms(const Reference< XDrawPage >& _rxDrawPage, Reference< XIndexAccess >& _rxForms)
    {
        Reference< XFormsSupplier2 > xFormsSupp(_rxDrawPage, UNO_QUERY);
        OSL_ENSURE(xFormsSupp.is(), "OFormLayerXMLExport_Impl::impl_isFormPageContainingForms: invalid draw page (no XFormsSupplier)! Doin' nothing!");
        if (!xFormsSupp.is())
            return sal_False;

        if ( !xFormsSupp->hasForms() )
            // nothing to do at all
            return sal_False;

        _rxForms = Reference< XIndexAccess >(xFormsSupp->getForms(), UNO_QUERY);
        Reference< XServiceInfo > xSI(_rxForms, UNO_QUERY); // order is important!
        OSL_ENSURE(xSI.is(), "OFormLayerXMLExport_Impl::impl_isFormPageContainingForms: invalid collection (must not be NULL and must have a ServiceInfo)!");
        if (!xSI.is())
            return sal_False;

        if (!xSI->supportsService("com.sun.star.form.Forms"))
        {
            OSL_FAIL("OFormLayerXMLExport_Impl::impl_isFormPageContainingForms: invalid collection (is no com.sun.star.form.Forms)!");
            // nothing to do
            return sal_False;
        }
        return sal_True;
    }

    void OFormLayerXMLExport_Impl::exportGridColumn(const Reference< XPropertySet >& _rxColumn,
        const Sequence< ScriptEventDescriptor >& _rEvents)
    {
        // do the exporting
        OColumnExport aExportImpl(*this, _rxColumn, getControlId( _rxColumn ), _rEvents);
        aExportImpl.doExport();
    }

    void OFormLayerXMLExport_Impl::exportControl(const Reference< XPropertySet >& _rxControl,
        const Sequence< ScriptEventDescriptor >& _rEvents)
    {
        // the list of the referring controls
        OUString sReferringControls;
        MapPropertySet2String::const_iterator aReferring = m_aCurrentPageReferring->second.find(_rxControl);
        if (aReferring != m_aCurrentPageReferring->second.end())
            sReferringControls = aReferring->second;

        // the control id (should already have been created in examineForms)
        OUString sControlId( getControlId( _rxControl ) );

        // do the exporting
        OControlExport aExportImpl(*this, _rxControl, sControlId, sReferringControls, _rEvents);
        aExportImpl.doExport();
    }

    void OFormLayerXMLExport_Impl::exportForm(const Reference< XPropertySet >& _rxProps,
        const Sequence< ScriptEventDescriptor >& _rEvents)
    {
        OSL_ENSURE(_rxProps.is(), "OFormLayerXMLExport_Impl::exportForm: invalid property set!");
        OFormExport aAttributeHandler(*this, _rxProps, _rEvents);
        aAttributeHandler.doExport();
    }

    ::rtl::Reference< SvXMLExportPropertyMapper > OFormLayerXMLExport_Impl::getStylePropertyMapper()
    {
        return m_xStyleExportMapper;
    }

    SvXMLExport& OFormLayerXMLExport_Impl::getGlobalContext()
    {
        return m_rContext;
    }

    void OFormLayerXMLExport_Impl::exportCollectionElements(const Reference< XIndexAccess >& _rxCollection)
    {
        // step through all the elements of the collection
        sal_Int32 nElements = _rxCollection->getCount();

        Reference< XEventAttacherManager > xElementEventManager(_rxCollection, UNO_QUERY);
        Sequence< ScriptEventDescriptor > aElementEvents;

        Reference< XPropertySetInfo > xPropsInfo;
        Reference< XIndexAccess > xCurrentContainer;
        for (sal_Int32 i=0; i<nElements; ++i)
        {
            try
            {
                // extract the current element
                Reference< XPropertySet > xCurrentProps( _rxCollection->getByIndex(i), UNO_QUERY );
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
                OSL_FAIL("OFormLayerXMLExport_Impl::exportCollectionElements: caught an exception ... skipping the current element!");
                continue;
            }
        }
    }

    OUString OFormLayerXMLExport_Impl::getObjectStyleName( const Reference< XPropertySet >& _rxObject )
    {
        OUString aObjectStyle;

        MapPropertySet2String::const_iterator aObjectStylePos = m_aGridColumnStyles.find( _rxObject );
        if ( m_aGridColumnStyles.end() != aObjectStylePos )
            aObjectStyle = aObjectStylePos->second;
        return aObjectStyle;
    }

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

    void OFormLayerXMLExport_Impl::exportAutoControlNumberStyles()
    {
        if ( m_pControlNumberStyles )
            m_pControlNumberStyles->Export( sal_True );
    }

    void OFormLayerXMLExport_Impl::exportAutoStyles()
    {
        m_rContext.GetAutoStylePool()->exportXML(
            XML_STYLE_FAMILY_CONTROL_ID,
            m_rContext.GetDocHandler(),
            m_rContext.GetMM100UnitConverter(),
            m_rContext.GetNamespaceMap()
        );
    }

    void OFormLayerXMLExport_Impl::exportForms(const Reference< XDrawPage >& _rxDrawPage)
    {
        // get the forms collection of the page
        Reference< XIndexAccess > xCollectionIndex;
        if (!impl_isFormPageContainingForms(_rxDrawPage, xCollectionIndex))
            return;

#if OSL_DEBUG_LEVEL > 0
        sal_Bool bPageIsKnown =
#endif
            implMoveIterators(_rxDrawPage, sal_False);
        OSL_ENSURE(bPageIsKnown, "OFormLayerXMLExport_Impl::exportForms: exporting a page which has not been examined!");

        // export forms collection
        exportCollectionElements(xCollectionIndex);
    }

    void OFormLayerXMLExport_Impl::exportXForms() const
    {
        // export XForms models
        ::exportXForms( m_rContext );
    }

    bool OFormLayerXMLExport_Impl::pageContainsForms( const Reference< XDrawPage >& _rxDrawPage ) const
    {
        Reference< XFormsSupplier2 > xFormsSupp( _rxDrawPage, UNO_QUERY );
        DBG_ASSERT( xFormsSupp.is(), "OFormLayerXMLExport_Impl::pageContainsForms: no XFormsSupplier2!" );
        return xFormsSupp.is() && xFormsSupp->hasForms();
    }

    bool OFormLayerXMLExport_Impl::documentContainsXForms() const
    {
        Reference< XXFormsSupplier > xXFormSupp( m_rContext.GetModel(), UNO_QUERY );
        Reference< XNameContainer > xForms;
        if ( xXFormSupp.is() )
            xForms = xXFormSupp->getXForms();
        return xForms.is() && xForms->hasElements();
    }

    sal_Bool OFormLayerXMLExport_Impl::implMoveIterators(const Reference< XDrawPage >& _rxDrawPage, sal_Bool _bClear)
    {
        if (!_rxDrawPage.is())
            return false;

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
            if (_bClear && !m_aCurrentPageIds->second.empty() )
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
            if (_bClear && !m_aCurrentPageReferring->second.empty() )
                m_aCurrentPageReferring->second.clear();
        }
        return bKnownPage;
    }

    sal_Bool OFormLayerXMLExport_Impl::seekPage(const Reference< XDrawPage >& _rxDrawPage)
    {
        sal_Bool bKnownPage = implMoveIterators( _rxDrawPage, sal_False );
        if ( bKnownPage )
            return sal_True;

        // if the page is not yet known, this does not automatically mean that it has
        // not been examined. Instead, examineForms returns silently and successfully
        // if a page is a XFormsPageSupplier2, but does not have a forms collection
        // (This behaviour of examineForms is a performance optimization, to not force
        // the page to create a forms container just to see that it's empty.)

        // So, in such a case, seekPage is considered to be successful, too, though the
        // page was not yet known
        Reference< XFormsSupplier2 > xFormsSupp( _rxDrawPage, UNO_QUERY );
        if ( xFormsSupp.is() && !xFormsSupp->hasForms() )
            return sal_True;

        // anything else means that the page has not been examined before, or it's no
        // valid form page. Both cases are Bad (TM).
        return sal_False;
    }

    OUString OFormLayerXMLExport_Impl::getControlId(const Reference< XPropertySet >& _rxControl)
    {
        if (m_aCurrentPageIds == m_aControlIds.end())
            return OUString();

        OSL_ENSURE(m_aCurrentPageIds->second.end() != m_aCurrentPageIds->second.find(_rxControl),
            "OFormLayerXMLExport_Impl::getControlId: can not find the control!");
        return m_aCurrentPageIds->second[_rxControl];
    }

    OUString OFormLayerXMLExport_Impl::getImmediateNumberStyle( const Reference< XPropertySet >& _rxObject )
    {
        OUString sNumberStyle;

        sal_Int32 nOwnFormatKey = implExamineControlNumberFormat( _rxObject );
        if ( -1 != nOwnFormatKey )
            sNumberStyle = getControlNumberStyleExport()->GetStyleName( nOwnFormatKey );

        return sNumberStyle;
    }

    OUString OFormLayerXMLExport_Impl::getControlNumberStyle( const Reference< XPropertySet >& _rxControl )
    {
        OUString sNumberStyle;

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

    void OFormLayerXMLExport_Impl::examineForms(const Reference< XDrawPage >& _rxDrawPage)
    {
        // get the forms collection of the page
        Reference< XIndexAccess > xCollectionIndex;
        if (!impl_isFormPageContainingForms(_rxDrawPage, xCollectionIndex))
            return;

        // move the iterator which specify the currently handled page
#if OSL_DEBUG_LEVEL > 0
        sal_Bool bPageIsKnown =
#endif
            implMoveIterators(_rxDrawPage, sal_True);
        OSL_ENSURE(!bPageIsKnown, "OFormLayerXMLExport_Impl::examineForms: examining a page twice!");

        ::std::stack< Reference< XIndexAccess > >   aContainerHistory;
        ::std::stack< sal_Int32 >                   aIndexHistory;

        Reference< XIndexAccess > xLoop = xCollectionIndex;
        sal_Int32 nChildPos = 0;
        do
        {
            if (nChildPos < xLoop->getCount())
            {
                Reference< XPropertySet > xCurrent( xLoop->getByIndex( nChildPos ), UNO_QUERY );
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
                while ((nChildPos >= xLoop->getCount()) && !aContainerHistory.empty() )
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

    namespace
    {
        struct AccumulateSize : public ::std::binary_function< size_t, MapPropertySet2Map::value_type, size_t >
        {
            size_t operator()( size_t _size, const MapPropertySet2Map::value_type& _map ) const
            {
                return _size + _map.second.size();
            }
        };

        OUString lcl_findFreeControlId( const MapPropertySet2Map& _rAllPagesControlIds )
        {
            static const OUString sControlIdBase(  "control"  );
            OUString sControlId = sControlIdBase;

            size_t nKnownControlCount = ::std::accumulate( _rAllPagesControlIds.begin(), _rAllPagesControlIds.end(), (size_t)0, AccumulateSize() );
            sControlId += OUString::valueOf( (sal_Int32)nKnownControlCount + 1 );

        #ifdef DBG_UTIL
            // Check if the id is already used. It shouldn't, as we currently have no mechanism for removing entries
            // from the map, so the approach used above (take the accumulated map size) should be sufficient. But if
            // somebody changes this (e.g. allows removing entries from the map), the assertion below probably will fail.
            for (   MapPropertySet2Map::const_iterator outer = _rAllPagesControlIds.begin();
                    outer != _rAllPagesControlIds.end();
                    ++outer
                )
                for (   MapPropertySet2String::const_iterator inner = outer->second.begin();
                        inner != outer->second.end();
                        ++inner
                    )
                {
                    OSL_ENSURE( inner->second != sControlId,
                        "lcl_findFreeControlId: auto-generated control ID is already used!" );
                }
        #endif
            return sControlId;
        }
    }

    sal_Bool OFormLayerXMLExport_Impl::checkExamineControl(const Reference< XPropertySet >& _rxObject)
    {
        Reference< XPropertySetInfo > xCurrentInfo = _rxObject->getPropertySetInfo();
        OSL_ENSURE(xCurrentInfo.is(), "OFormLayerXMLExport_Impl::checkExamineControl: no property set info");

        sal_Bool bIsControl = xCurrentInfo->hasPropertyByName( PROPERTY_CLASSID );
        if (bIsControl)
        {
            // generate a new control id

            // find a free id
            OUString sCurrentId = lcl_findFreeControlId( m_aControlIds );
            // add it to the map
            m_aCurrentPageIds->second[_rxObject] = sCurrentId;

            // check if this control has a "LabelControl" property referring another control
            if ( xCurrentInfo->hasPropertyByName( PROPERTY_CONTROLLABEL ) )
            {
                Reference< XPropertySet > xCurrentReference( _rxObject->getPropertyValue( PROPERTY_CONTROLLABEL ), UNO_QUERY );
                if (xCurrentReference.is())
                {
                    OUString& sReferencedBy = m_aCurrentPageReferring->second[xCurrentReference];
                    if (!sReferencedBy.isEmpty())
                        // it's not the first _rxObject referring to the xCurrentReference
                        // -> separate the id
                        sReferencedBy += OUString(",");
                    sReferencedBy += sCurrentId;
                }
            }

            // check if the control needs a number format style
            if ( xCurrentInfo->hasPropertyByName( PROPERTY_FORMATKEY ) )
            {
                examineControlNumberFormat(_rxObject);
            }

            // check if it's a control providing text
            Reference< XText > xControlText( _rxObject, UNO_QUERY );
            if ( xControlText.is() )
            {
                m_rContext.GetTextParagraphExport()->collectTextAutoStyles( xControlText );
            }

            // check if it is a grid control - in this case, we need special handling for the columns
            sal_Int16 nControlType = FormComponentType::CONTROL;
            _rxObject->getPropertyValue( PROPERTY_CLASSID ) >>= nControlType;
            if ( FormComponentType::GRIDCONTROL == nControlType )
            {
                collectGridColumnStylesAndIds( _rxObject );
            }
        }

        return bIsControl;
    }

    void OFormLayerXMLExport_Impl::collectGridColumnStylesAndIds( const Reference< XPropertySet >& _rxControl )
    {
        // loop through all columns of the grid
        try
        {
            Reference< XIndexAccess > xContainer( _rxControl, UNO_QUERY );
            OSL_ENSURE( xContainer.is(), "OFormLayerXMLExport_Impl::collectGridColumnStylesAndIds: grid control not being a container?!" );
            if ( !xContainer.is() )
                return;

            Reference< XPropertySetInfo > xColumnPropertiesMeta;

            sal_Int32 nCount = xContainer->getCount();
            for ( sal_Int32 i=0; i<nCount; ++i )
            {
                Reference< XPropertySet > xColumnProperties( xContainer->getByIndex( i ), UNO_QUERY );
                OSL_ENSURE( xColumnProperties.is(), "OFormLayerXMLExport_Impl::collectGridColumnStylesAndIds: invalid grid column encountered!" );
                if ( !xColumnProperties.is() )
                    continue;

                // generate a new control id

                // find a free id
                OUString sCurrentId = lcl_findFreeControlId( m_aControlIds );
                // add it to the map
                m_aCurrentPageIds->second[ xColumnProperties ] = sCurrentId;

                // determine a number style, if needed
                xColumnPropertiesMeta = xColumnProperties->getPropertySetInfo();
                // get the styles of the column
                ::std::vector< XMLPropertyState > aPropertyStates = m_xStyleExportMapper->Filter( xColumnProperties );

                // care for the number format, additionally
                OUString sColumnNumberStyle;
                if ( xColumnPropertiesMeta.is() && xColumnPropertiesMeta->hasPropertyByName( PROPERTY_FORMATKEY ) )
                    sColumnNumberStyle = getImmediateNumberStyle( xColumnProperties );

                if ( !sColumnNumberStyle.isEmpty() )
                {   // the column indeed has a formatting
                    sal_Int32 nStyleMapIndex = m_xStyleExportMapper->getPropertySetMapper()->FindEntryIndex( CTF_FORMS_DATA_STYLE );
                        // TODO: move this to the ctor
                    OSL_ENSURE ( -1 != nStyleMapIndex, "XMLShapeExport::collectShapeAutoStyles: could not obtain the index for our context id!");

                    XMLPropertyState aNumberStyleState( nStyleMapIndex, makeAny( sColumnNumberStyle ) );
                    aPropertyStates.push_back( aNumberStyleState );
                }

#if OSL_DEBUG_LEVEL > 0
                ::std::vector< XMLPropertyState >::const_iterator aHaveALook = aPropertyStates.begin();
                for ( ; aHaveALook != aPropertyStates.end(); ++aHaveALook )
                {
                    (void)aHaveALook;
                }
#endif

                // determine the column style

                if ( !aPropertyStates.empty() )
                {   // add to the style pool
                    OUString sColumnStyleName = m_rContext.GetAutoStylePool()->Add( XML_STYLE_FAMILY_CONTROL_ID, aPropertyStates );

                    OSL_ENSURE( m_aGridColumnStyles.end() == m_aGridColumnStyles.find( xColumnProperties ),
                        "OFormLayerXMLExport_Impl::collectGridColumnStylesAndIds: already have a style for this column!" );

                    m_aGridColumnStyles.insert( MapPropertySet2String::value_type( xColumnProperties, sColumnStyleName ) );
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    sal_Int32 OFormLayerXMLExport_Impl::implExamineControlNumberFormat( const Reference< XPropertySet >& _rxObject )
    {
        // get the format key relative to our own formats supplier
        sal_Int32 nOwnFormatKey = ensureTranslateFormat( _rxObject );

        if ( -1 != nOwnFormatKey )
            // tell the exporter that we used this format
            getControlNumberStyleExport()->SetUsed( nOwnFormatKey );

        return nOwnFormatKey;
    }

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
            OUString sFormatDescription;
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
                Locale aLocale (  OUString("en"),
                                                 OUString("US"),
                                                 OUString()
                                             );
                xFormatsSupplier = NumberFormatsSupplier::createWithLocale( m_rContext.getComponentContext(), aLocale );
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

    SvXMLNumFmtExport* OFormLayerXMLExport_Impl::getControlNumberStyleExport()
    {
        ensureControlNumberStyleExport();
        return m_pControlNumberStyles;
    }

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

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
