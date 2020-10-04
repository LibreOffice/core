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

#include "layerimport.hxx"
#include "formenums.hxx"
#include "elementimport.hxx"
#include "officeforms.hxx"
#include "strings.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlimp.hxx>
#include <XMLEventImportHelper.hxx>
#include <xmloff/xmlnumfi.hxx>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#include <com/sun/star/form/FormSubmitMethod.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/form/NavigationBarMode.hpp>
#include <com/sun/star/form/TabulatorCycle.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include "formevents.hxx"
#include "formcellbinding.hxx"
#include <xmloff/xformsimport.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <rtl/strbuf.hxx>
#include <tools/diagnose_ex.h>
#include <algorithm>

namespace xmloff
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::sdb;

//= OFormLayerXMLImport_Impl
OFormLayerXMLImport_Impl::OFormLayerXMLImport_Impl(SvXMLImport& _rImporter)
    :m_rImporter(_rImporter)
{
    // build the attribute2property map
    // string properties which are exported as attributes
    m_aAttributeMetaData.addStringProperty(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Name), PROPERTY_NAME);
    m_aAttributeMetaData.addStringProperty(
        OAttributeMetaData::getSpecialAttributeName(SCAFlags::GroupName), PROPERTY_GROUP_NAME);
    m_aAttributeMetaData.addStringProperty(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Label), PROPERTY_LABEL);
    m_aAttributeMetaData.addStringProperty(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::TargetLocation), PROPERTY_TARGETURL);
    m_aAttributeMetaData.addStringProperty(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Title), PROPERTY_TITLE);
    m_aAttributeMetaData.addStringProperty(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::TargetFrame), PROPERTY_TARGETFRAME);
    m_aAttributeMetaData.addStringProperty(
        OAttributeMetaData::getDatabaseAttributeName(DAFlags::DataField), PROPERTY_DATAFIELD);
    m_aAttributeMetaData.addStringProperty(
        OAttributeMetaData::getFormAttributeName(faCommand), PROPERTY_COMMAND);
    m_aAttributeMetaData.addStringProperty(
        OAttributeMetaData::getFormAttributeName(faDatasource), PROPERTY_DATASOURCENAME);
    m_aAttributeMetaData.addStringProperty(
        OAttributeMetaData::getFormAttributeName(faFilter), PROPERTY_FILTER);
    m_aAttributeMetaData.addStringProperty(
        OAttributeMetaData::getFormAttributeName(faOrder), PROPERTY_ORDER);

    // properties not added because they're already present in another form
    OSL_ENSURE(
        OUString::createFromAscii(OAttributeMetaData::getCommonControlAttributeName(CCAFlags::TargetLocation)).equalsAscii(
            OAttributeMetaData::getFormAttributeName(faAction)),
        "OFormLayerXMLImport_Impl::OFormLayerXMLImport_Impl: invalid attribute names (1)!");
        // if this fails, we would have to add a translation from faAction->PROPERTY_TARGETURL
        // We did not because we already have one CCAFlags::TargetLocation->PROPERTY_TARGETURL,
        // and CCAFlags::TargetLocation and faAction should be represented by the same attribute

    OSL_ENSURE(
        OUString::createFromAscii(OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Name)).equalsAscii(
            OAttributeMetaData::getFormAttributeName(faName)),
        "OFormLayerXMLImport_Impl::OFormLayerXMLImport_Impl: invalid attribute names (2)!");
        // the same for faName, CCAFlags::Name and PROPERTY_NAME

    // boolean properties which are exported as attributes
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::CurrentSelected), PROPERTY_STATE, false);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Disabled), PROPERTY_ENABLED, false, true);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Dropdown), PROPERTY_DROPDOWN, false);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Printable), PROPERTY_PRINTABLE, true);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::ReadOnly), PROPERTY_READONLY, false);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Selected), PROPERTY_DEFAULT_STATE, false);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::TabStop), PROPERTY_TABSTOP, true);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getDatabaseAttributeName(DAFlags::ConvertEmpty), PROPERTY_EMPTY_IS_NULL, false);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getSpecialAttributeName(SCAFlags::Validation), PROPERTY_STRICTFORMAT, false);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getSpecialAttributeName(SCAFlags::MultiLine), PROPERTY_MULTILINE, false);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getSpecialAttributeName(SCAFlags::AutoCompletion), PROPERTY_AUTOCOMPLETE, false);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getSpecialAttributeName(SCAFlags::Multiple), PROPERTY_MULTISELECTION, false);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getSpecialAttributeName(SCAFlags::DefaultButton), PROPERTY_DEFAULTBUTTON, false);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getSpecialAttributeName(SCAFlags::IsTristate), PROPERTY_TRISTATE, false);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getFormAttributeName(faAllowDeletes), PROPERTY_ALLOWDELETES, true);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getFormAttributeName(faAllowInserts), PROPERTY_ALLOWINSERTS, true);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getFormAttributeName(faAllowUpdates), PROPERTY_ALLOWUPDATES, true);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getFormAttributeName(faApplyFilter), PROPERTY_APPLYFILTER, false);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getFormAttributeName(faEscapeProcessing), PROPERTY_ESCAPEPROCESSING, true);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getFormAttributeName(faIgnoreResult), PROPERTY_IGNORERESULT, false);
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getSpecialAttributeName( SCAFlags::Toggle ), PROPERTY_TOGGLE, false );
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getSpecialAttributeName( SCAFlags::FocusOnClick ), PROPERTY_FOCUS_ON_CLICK, true );
    m_aAttributeMetaData.addBooleanProperty(
        OAttributeMetaData::getDatabaseAttributeName( DAFlags::InputRequired ), PROPERTY_INPUT_REQUIRED, false );

    // the int16 attributes
    m_aAttributeMetaData.addInt16Property(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::MaxLength), PROPERTY_MAXTEXTLENGTH);
    m_aAttributeMetaData.addInt16Property(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Size), PROPERTY_LINECOUNT);
    m_aAttributeMetaData.addInt16Property(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::TabIndex), PROPERTY_TABINDEX);
    m_aAttributeMetaData.addInt16Property(
        OAttributeMetaData::getDatabaseAttributeName(DAFlags::BoundColumn), PROPERTY_BOUNDCOLUMN);

    // the int32 attributes
    m_aAttributeMetaData.addInt32Property(
        OAttributeMetaData::getSpecialAttributeName( SCAFlags::PageStepSize ), PROPERTY_BLOCK_INCREMENT );

    // the enum attributes
    m_aAttributeMetaData.addEnumProperty(
        OAttributeMetaData::getCommonControlAttributeName( CCAFlags::VisualEffect ), PROPERTY_VISUAL_EFFECT,
        aVisualEffectMap,
        &::cppu::UnoType<sal_Int16>::get() );
    m_aAttributeMetaData.addEnumProperty(
        OAttributeMetaData::getCommonControlAttributeName( CCAFlags::Orientation ), PROPERTY_ORIENTATION,
        aOrientationMap,
        &::cppu::UnoType<sal_Int32>::get() );
    m_aAttributeMetaData.addEnumProperty(
        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::ButtonType), PROPERTY_BUTTONTYPE,
        aFormButtonTypeMap,
        &::cppu::UnoType<FormButtonType>::get());
    m_aAttributeMetaData.addEnumProperty(
        OAttributeMetaData::getDatabaseAttributeName(DAFlags::ListSource_TYPE), PROPERTY_LISTSOURCETYPE,
        aListSourceTypeMap,
        &::cppu::UnoType<ListSourceType>::get());
    m_aAttributeMetaData.addEnumProperty(
        OAttributeMetaData::getSpecialAttributeName(SCAFlags::State), PROPERTY_DEFAULT_STATE,
        aCheckStateMap,
        &::cppu::UnoType<sal_Int16>::get());
    m_aAttributeMetaData.addEnumProperty(
        OAttributeMetaData::getSpecialAttributeName(SCAFlags::CurrentState), PROPERTY_STATE,
        aCheckStateMap,
        &::cppu::UnoType<sal_Int16>::get());
    m_aAttributeMetaData.addEnumProperty(
        OAttributeMetaData::getFormAttributeName(faEnctype), PROPERTY_SUBMIT_ENCODING,
        aSubmitEncodingMap,
        &::cppu::UnoType<FormSubmitEncoding>::get());
    m_aAttributeMetaData.addEnumProperty(
        OAttributeMetaData::getFormAttributeName(faMethod), PROPERTY_SUBMIT_METHOD,
        aSubmitMethodMap,
        &::cppu::UnoType<FormSubmitMethod>::get());
    m_aAttributeMetaData.addEnumProperty(
        OAttributeMetaData::getFormAttributeName(faCommandType), PROPERTY_COMMAND_TYPE,
        aCommandTypeMap);
    m_aAttributeMetaData.addEnumProperty(
        OAttributeMetaData::getFormAttributeName(faNavigationMode), PROPERTY_NAVIGATION,
        aNavigationTypeMap,
        &::cppu::UnoType<NavigationBarMode>::get());
    m_aAttributeMetaData.addEnumProperty(
        OAttributeMetaData::getFormAttributeName(faTabbingCycle), PROPERTY_CYCLE,
        aTabulatorCycleMap,
        &::cppu::UnoType<TabulatorCycle>::get());

    // 'initialize'
    m_aCurrentPageIds = m_aControlIds.end();
}

OFormLayerXMLImport_Impl::~OFormLayerXMLImport_Impl()
{}

void OFormLayerXMLImport_Impl::setAutoStyleContext(SvXMLStylesContext* _pNewContext)
{
    OSL_ENSURE(!m_xAutoStyles.is(), "OFormLayerXMLImport_Impl::setAutoStyleContext: not to be called twice!");
    m_xAutoStyles.set(_pNewContext);
}

void OFormLayerXMLImport_Impl::applyControlNumberStyle(const Reference< XPropertySet >& _rxControlModel, const OUString& _rControlNumberStyleName)
{
    OSL_ENSURE(_rxControlModel.is() && (!_rControlNumberStyleName.isEmpty()),
        "OFormLayerXMLImport_Impl::applyControlNumberStyle: invalid arguments (this will crash)!");

    OSL_ENSURE(m_xAutoStyles.is(), "OFormLayerXMLImport_Impl::applyControlNumberStyle: have no auto style context!");
    if (!m_xAutoStyles.is())
    {
        m_xAutoStyles.set(m_rImporter.GetShapeImport()->GetAutoStylesContext());
    }

    if (!m_xAutoStyles.is())
        return;

    const SvXMLStyleContext* pStyle = m_xAutoStyles->FindStyleChildContext(XmlStyleFamily::DATA_STYLE, _rControlNumberStyleName);
    if (pStyle)
    {
        const SvXMLNumFormatContext* pDataStyle = static_cast<const SvXMLNumFormatContext*>(pStyle);

        // set this format at the control model
        try
        {
            // the models number format supplier and formats
            Reference< XNumberFormatsSupplier > xFormatsSupplier;
            _rxControlModel->getPropertyValue(PROPERTY_FORMATSSUPPLIER) >>= xFormatsSupplier;
            Reference< XNumberFormats > xFormats;
            if (xFormatsSupplier.is())
                xFormats = xFormatsSupplier->getNumberFormats();
            OSL_ENSURE(xFormats.is(), "OFormLayerXMLImport_Impl::applyControlNumberStyle: could not obtain the controls number formats!");

            // obtain a key
            if (xFormats.is())
            {
                sal_Int32 nFormatKey = const_cast<SvXMLNumFormatContext*>(pDataStyle)->CreateAndInsert( xFormatsSupplier );
                OSL_ENSURE(-1 != nFormatKey, "OFormLayerXMLImport_Impl::applyControlNumberStyle: could not obtain a format key!");

                // set the format on the control model
                _rxControlModel->setPropertyValue(PROPERTY_FORMATKEY, makeAny(nFormatKey));
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("OFormLayerXMLImport_Impl::applyControlNumberStyle: couldn't set the format!");
        }
    }
    else
        OSL_FAIL("OFormLayerXMLImport_Impl::applyControlNumberStyle: did not find the style with the given name!");
}

void OFormLayerXMLImport_Impl::registerCellValueBinding( const Reference< XPropertySet >& _rxControlModel, const OUString& _rCellAddress )
{
    OSL_ENSURE( _rxControlModel.is() && !_rCellAddress.isEmpty(),
        "OFormLayerXMLImport_Impl::registerCellValueBinding: invalid arguments!" );
    m_aCellValueBindings.emplace_back( _rxControlModel, _rCellAddress );
}

void OFormLayerXMLImport_Impl::registerXFormsValueBinding(
    const Reference< XPropertySet >& _rxControlModel,
    const OUString& _rBindingID )
{
    // TODO: is an empty binding name allowed?
    OSL_ENSURE( _rxControlModel.is(), "need  model" );

    m_aXFormsValueBindings.emplace_back( _rxControlModel, _rBindingID );
}

void OFormLayerXMLImport_Impl::registerXFormsListBinding(
    const Reference< XPropertySet >& _rxControlModel,
    const OUString& _rBindingID )
{
    // TODO: is an empty binding name allowed?
    OSL_ENSURE( _rxControlModel.is(), "need  model" );

    m_aXFormsListBindings.emplace_back( _rxControlModel, _rBindingID );
}

void OFormLayerXMLImport_Impl::registerXFormsSubmission(
    const Reference< XPropertySet >& _rxControlModel,
    const OUString& _rSubmissionID )
{
    // TODO: is an empty binding name allowed?
    OSL_ENSURE( _rxControlModel.is(), "need  model" );

    m_aXFormsSubmissions.emplace_back( _rxControlModel, _rSubmissionID );
}

void OFormLayerXMLImport_Impl::registerCellRangeListSource( const Reference< XPropertySet >& _rxControlModel, const OUString& _rCellRangeAddress )
{
    OSL_ENSURE( _rxControlModel.is() && !_rCellRangeAddress.isEmpty(),
        "OFormLayerXMLImport_Impl::registerCellRangeListSource: invalid arguments!" );
    m_aCellRangeListSources.emplace_back( _rxControlModel, _rCellRangeAddress );
}
const SvXMLStyleContext* OFormLayerXMLImport_Impl::getStyleElement(const OUString& _rStyleName) const
{
    OSL_ENSURE( m_xAutoStyles.is(), "OFormLayerXMLImport_Impl::getStyleElement: have no auto style context!" );
        // did you use setAutoStyleContext?

    const SvXMLStyleContext* pControlStyle =
        m_xAutoStyles.is() ? m_xAutoStyles->FindStyleChildContext( XmlStyleFamily::TEXT_PARAGRAPH, _rStyleName ) : nullptr;
    OSL_ENSURE( pControlStyle || !m_xAutoStyles.is(),
                OStringBuffer("OFormLayerXMLImport_Impl::getStyleElement: did not find the style named \"").append(OUStringToOString(_rStyleName, RTL_TEXTENCODING_ASCII_US)).append("\"!").getStr() );
    return pControlStyle;
}

void OFormLayerXMLImport_Impl::enterEventContext()
{
    // install our own translation table. We need to disable the other tables because of name conflicts.
    m_rImporter.GetEventImport().PushTranslationTable();
    m_rImporter.GetEventImport().AddTranslationTable(g_pFormsEventTranslation);
}

void OFormLayerXMLImport_Impl::leaveEventContext()
{
    // install the original event tables.
    m_rImporter.GetEventImport().PopTranslationTable();
}

void OFormLayerXMLImport_Impl::registerControlId(const Reference< XPropertySet >& _rxControl, const OUString& _rId)
{
    OSL_ENSURE(m_aCurrentPageIds != m_aControlIds.end(), "OFormLayerXMLImport_Impl::registerControlId: no current page!");
    OSL_ENSURE(!_rId.isEmpty(), "OFormLayerXMLImport_Impl::registerControlId: invalid (empty) control id!");

    OSL_ENSURE(m_aCurrentPageIds->second.end() == m_aCurrentPageIds->second.find(_rId), "OFormLayerXMLImport_Impl::registerControlId: control id already used!");
    m_aCurrentPageIds->second[_rId] = _rxControl;
}

void OFormLayerXMLImport_Impl::registerControlReferences(const Reference< XPropertySet >& _rxControl, const OUString& _rReferringControls)
{
    OSL_ENSURE(!_rReferringControls.isEmpty(), "OFormLayerXMLImport_Impl::registerControlReferences: invalid (empty) control id list!");
    OSL_ENSURE(_rxControl.is(), "OFormLayerXMLImport_Impl::registerControlReferences: invalid (NULL) control!");
    m_aControlReferences.emplace_back( _rxControl, _rReferringControls );
}

void OFormLayerXMLImport_Impl::startPage(const Reference< XDrawPage >& _rxDrawPage)
{
    m_xCurrentPageFormsSupp.clear();

    OSL_ENSURE(_rxDrawPage.is(), "OFormLayerXMLImport_Impl::startPage: NULL page!");
    m_xCurrentPageFormsSupp.set(_rxDrawPage, css::uno::UNO_QUERY);
    OSL_ENSURE( m_xCurrentPageFormsSupp.is(), "OFormLayerXMLImport_Impl::startPage: invalid draw page (no XFormsSupplier)!" );
    if ( !m_xCurrentPageFormsSupp.is() )
        return;

    // add a new entry to our page map
    ::std::pair< MapDrawPage2Map::iterator, bool > aPagePosition = m_aControlIds.emplace(_rxDrawPage, MapString2PropertySet());
    OSL_ENSURE(aPagePosition.second, "OFormLayerXMLImport_Impl::startPage: already imported this page!");
    m_aCurrentPageIds = aPagePosition.first;
}

void OFormLayerXMLImport_Impl::endPage()
{
    OSL_ENSURE( m_xCurrentPageFormsSupp.is(), "OFormLayerXMLImport_Impl::endPage: sure you called startPage before?" );

    // do some knittings for the controls which are referring to each other
    try
    {
        static const sal_Unicode s_nSeparator = ',';
        OUString sCurrentReferring;
        OUString sSeparator(&s_nSeparator, 1);
        Reference< XPropertySet > xCurrentReferring;
        sal_Int32 nSeparator, nPrevSep;
        for ( const auto& rReferences : m_aControlReferences )
        {
            // the list of control ids is comma separated

            // in a list of n ids there are only n-1 separators ... have to catch this last id
            // -> normalize the list
            OUString sReferring = rReferences.second + sSeparator;

            nPrevSep = -1;
            while (-1 != (nSeparator = sReferring.indexOf(s_nSeparator, nPrevSep + 1)))
            {
                sCurrentReferring = sReferring.copy(nPrevSep + 1, nSeparator - nPrevSep - 1);
                xCurrentReferring = lookupControlId(sCurrentReferring);
                if (xCurrentReferring.is())
                    // if this condition fails, this is an error, but lookupControlId should have asserted this ...
                    xCurrentReferring->setPropertyValue( PROPERTY_CONTROLLABEL, makeAny( rReferences.first ) );

                nPrevSep = nSeparator;
            }
        }
    }
    catch(Exception&)
    {
        TOOLS_WARN_EXCEPTION("xmloff.forms",
                             "unable to knit the control references (caught an exception)!");
    }

    // now that we have all children of the forms collection, attach the events
    Reference< XIndexAccess > xIndexContainer;
    if ( m_xCurrentPageFormsSupp.is() && m_xCurrentPageFormsSupp->hasForms() )
        xIndexContainer.set(m_xCurrentPageFormsSupp->getForms(), css::uno::UNO_QUERY);
    if ( xIndexContainer.is() )
        ODefaultEventAttacherManager::setEvents( xIndexContainer );

    // clear the structures for the control references.
    m_aControlReferences.clear();

    // and no we have no current page anymore
    m_aCurrentPageIds = m_aControlIds.end();
}

Reference< XPropertySet > OFormLayerXMLImport_Impl::lookupControlId(const OUString& _rControlId)
{
    OSL_ENSURE(m_aCurrentPageIds != m_aControlIds.end(), "OFormLayerXMLImport_Impl::lookupControlId: no current page!");
    Reference< XPropertySet > xReturn;
    if (m_aCurrentPageIds != m_aControlIds.end())
    {
        MapString2PropertySet::const_iterator aPos = m_aCurrentPageIds->second.find(_rControlId);
        if (m_aCurrentPageIds->second.end() != aPos)
            xReturn = aPos->second;
        else
            OSL_FAIL("OFormLayerXMLImport_Impl::lookupControlId: invalid control id (did not find it)!");
    }
    return xReturn;
}

SvXMLImportContext* OFormLayerXMLImport_Impl::createOfficeFormsContext(
    SvXMLImport& _rImport,
    sal_uInt16 _nPrefix,
    const OUString& _rLocalName)
{
    return new OFormsRootImport( _rImport, _nPrefix, _rLocalName );
}

SvXMLImportContext* OFormLayerXMLImport_Impl::createContext(const sal_uInt16 _nPrefix, const OUString& _rLocalName,
    const Reference< xml::sax::XAttributeList >&)
{
    SvXMLImportContext* pContext = nullptr;
    if ( _rLocalName == "form" )
    {
        if ( m_xCurrentPageFormsSupp.is() )
            pContext = new OFormImport(*this, *this, _nPrefix, _rLocalName, m_xCurrentPageFormsSupp->getForms() );
    }
    else if ( _nPrefix == XML_NAMESPACE_XFORMS
              && xmloff::token::IsXMLToken( _rLocalName, xmloff::token::XML_MODEL ) )
    {
        pContext = createXFormsModelContext( m_rImporter, _nPrefix, _rLocalName );
    }

    return pContext;
}

void OFormLayerXMLImport_Impl::documentDone( )
{
    SvXMLImport& rImport = getGlobalContext();
    if ( !( rImport.getImportFlags() & SvXMLImportFlags::CONTENT ) )
        return;

    // create (and bind) the spreadsheet cell bindings
    if  (   !m_aCellValueBindings.empty()
        &&  FormCellBindingHelper::isCellBindingAllowed( rImport.GetModel() )
        )
    {
        for ( const auto& rCellBindings : m_aCellValueBindings )
        {
            try
            {
                FormCellBindingHelper aHelper( rCellBindings.first, rImport.GetModel() );
                OSL_ENSURE( aHelper.isCellBindingAllowed(), "OFormLayerXMLImport_Impl::documentDone: can't bind this control model!" );
                if ( aHelper.isCellBindingAllowed() )
                {
                    // There are special bindings for listboxes. See
                    // OListAndComboImport::doRegisterCellValueBinding for a comment on this HACK.
                    OUString sBoundCellAddress( rCellBindings.second );
                    sal_Int32 nIndicator = sBoundCellAddress.lastIndexOf( ":index" );

                    bool bUseIndexBinding = false;
                    if ( nIndicator != -1 )
                    {
                        sBoundCellAddress = sBoundCellAddress.copy( 0, nIndicator );
                        bUseIndexBinding = true;
                    }

                    aHelper.setBinding( aHelper.createCellBindingFromStringAddress( sBoundCellAddress, bUseIndexBinding ) );
                }
            }
            catch( const Exception& )
            {
                TOOLS_WARN_EXCEPTION("xmloff.forms",
                                     "caught an exception while binding to a cell!");
            }
        }
        m_aCellValueBindings.clear();
    }

    // the same for the spreadsheet cell range list sources
    if  (   !m_aCellRangeListSources.empty()
        &&  FormCellBindingHelper::isListCellRangeAllowed( rImport.GetModel() )
        )
    {
        for ( const auto& rRangeBindings : m_aCellRangeListSources )
        {
            try
            {
                FormCellBindingHelper aHelper( rRangeBindings.first, rImport.GetModel() );
                OSL_ENSURE( aHelper.isListCellRangeAllowed(), "OFormLayerXMLImport_Impl::documentDone: can't bind this control model!" );
                if ( aHelper.isListCellRangeAllowed() )
                {
                    aHelper.setListSource( aHelper.createCellListSourceFromStringAddress( rRangeBindings.second ) );
                }
            }
            catch( const Exception& )
            {
                TOOLS_WARN_EXCEPTION("xmloff.forms",
                                     "caught an exception while binding to a cell range!");
            }
        }
        m_aCellRangeListSources.clear();
    }

    // process XForms-bindings; call registerXFormsValueBinding for each
    for (const auto& aXFormsValueBinding : m_aXFormsValueBindings)
        bindXFormsValueBinding(rImport.GetModel(), aXFormsValueBinding);
    // same for list bindings
    for (const auto& aXFormsListBindings : m_aXFormsListBindings)
        bindXFormsListBinding(rImport.GetModel(), aXFormsListBindings);
    // same for submissions
    for (const auto& aXFormsSubmission : m_aXFormsSubmissions)
        bindXFormsSubmission(rImport.GetModel(), aXFormsSubmission);
}

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
