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

#include "layerimport.hxx"
#include "formenums.hxx"
#include "elementimport.hxx"
#include "officeforms.hxx"
#include "strings.hxx"
#include "formstyles.hxx"
#include "xmlimp.hxx"
#include "XMLEventImportHelper.hxx"
#include "xmlnumfi.hxx"
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#include <com/sun/star/form/FormSubmitMethod.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/form/NavigationBarMode.hpp>
#include <com/sun/star/form/TabulatorCycle.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <vcl/wintypes.hxx>		// for check states

#include <com/sun/star/form/XFormsSupplier.hpp>
#include "controlpropertyhdl.hxx"
#include "controlpropertymap.hxx"
#include "formevents.hxx"
#ifndef XMLOFF_FORMS_FORMCELLBINDING
#include "formcellbinding.hxx"
#endif
namespace binfilter {

SV_IMPL_REF( SvXMLStylesContext );

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::drawing;
    using namespace ::com::sun::star::xml;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::sdb;

    //=====================================================================
    //= OFormLayerXMLImport_Impl
    //=====================================================================
    //---------------------------------------------------------------------
    OFormLayerXMLImport_Impl::OFormLayerXMLImport_Impl(SvXMLImport& _rImporter)
        :m_rImporter(_rImporter)
        ,m_pAutoStyles(NULL)
    {
        // build the attribute2property map
        // string properties which are exported as attributes
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_NAME), PROPERTY_NAME);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_IMAGE_DATA), PROPERTY_IMAGEURL);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_LABEL), PROPERTY_LABEL);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_TARGET_LOCATION), PROPERTY_TARGETURL);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_TITLE), PROPERTY_TITLE);
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_TARGET_FRAME), PROPERTY_TARGETFRAME, "_blank");
        m_aAttributeMetaData.addStringProperty(
            OAttributeMetaData::getDatabaseAttributeName(DA_DATA_FIELD), PROPERTY_DATAFIELD);
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
            0 == ::rtl::OUString::createFromAscii(OAttributeMetaData::getCommonControlAttributeName(CCA_TARGET_LOCATION)).compareToAscii(
                OAttributeMetaData::getFormAttributeName(faAction)),
            "OFormLayerXMLImport_Impl::OFormLayerXMLImport_Impl: invalid attribute names (1)!");
            // if this fails, we would have to add a translation from faAction->PROPERTY_TARGETURL
            // We did not because we already have one CCA_TARGET_LOCATION->PROPERTY_TARGETURL,
            // and CCA_TARGET_LOCATION and faAction should be represented by the same attribute

        OSL_ENSURE(
            0 == ::rtl::OUString::createFromAscii(OAttributeMetaData::getCommonControlAttributeName(CCA_NAME)).compareToAscii(
                OAttributeMetaData::getFormAttributeName(faName)),
            "OFormLayerXMLImport_Impl::OFormLayerXMLImport_Impl: invalid attribute names (2)!");
            // the same for faName, CCA_NAME and PROPERTY_NAME

        // boolean properties which are exported as attributes
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_CURRENT_SELECTED), PROPERTY_STATE, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_DISABLED), PROPERTY_ENABLED, sal_False, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_DROPDOWN), PROPERTY_DROPDOWN, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_PRINTABLE), PROPERTY_PRINTABLE, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_READONLY), PROPERTY_READONLY, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_SELECTED), PROPERTY_DEFAULT_STATE, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_TAB_STOP), PROPERTY_TABSTOP, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getDatabaseAttributeName(DA_CONVERT_EMPTY), PROPERTY_EMPTY_IS_NULL, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_VALIDATION), PROPERTY_STRICTFORMAT, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_MULTI_LINE), PROPERTY_MULTILINE, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_AUTOMATIC_COMPLETION), PROPERTY_AUTOCOMPLETE, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_MULTIPLE), PROPERTY_MULTISELECTION, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_DEFAULT_BUTTON), PROPERTY_DEFAULTBUTTON, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_IS_TRISTATE), PROPERTY_TRISTATE, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getFormAttributeName(faAllowDeletes), PROPERTY_ALLOWDELETES, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getFormAttributeName(faAllowInserts), PROPERTY_ALLOWINSERTS, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getFormAttributeName(faAllowUpdates), PROPERTY_ALLOWUPDATES, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getFormAttributeName(faApplyFilter), PROPERTY_APPLYFILTER, sal_False);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getFormAttributeName(faEscapeProcessing), PROPERTY_ESCAPEPROCESSING, sal_True);
        m_aAttributeMetaData.addBooleanProperty(
            OAttributeMetaData::getFormAttributeName(faIgnoreResult), PROPERTY_IGNORERESULT, sal_False);

        // the int16 attributes
        m_aAttributeMetaData.addInt16Property(
            OAttributeMetaData::getCommonControlAttributeName(CCA_MAX_LENGTH), PROPERTY_MAXTEXTLENGTH, 0);
        m_aAttributeMetaData.addInt16Property(
            OAttributeMetaData::getCommonControlAttributeName(CCA_SIZE), PROPERTY_LINECOUNT, 5);
        m_aAttributeMetaData.addInt16Property(
            OAttributeMetaData::getCommonControlAttributeName(CCA_TAB_INDEX), PROPERTY_TABINDEX, 0);
        m_aAttributeMetaData.addInt16Property(
            OAttributeMetaData::getDatabaseAttributeName(DA_BOUND_COLUMN), PROPERTY_BOUNDCOLUMN, 0);

        // the enum attributes
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getCommonControlAttributeName(CCA_BUTTON_TYPE), PROPERTY_BUTTONTYPE,
            FormButtonType_PUSH, OEnumMapper::getEnumMap(OEnumMapper::epButtonType),
            &::getCppuType( static_cast<FormButtonType*>(NULL) ));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getDatabaseAttributeName(DA_LIST_SOURCE_TYPE), PROPERTY_LISTSOURCETYPE,
            ListSourceType_VALUELIST, OEnumMapper::getEnumMap(OEnumMapper::epListSourceType),
            &::getCppuType( static_cast<ListSourceType*>(NULL) ));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_STATE), PROPERTY_DEFAULT_STATE, STATE_NOCHECK,
            OEnumMapper::getEnumMap(OEnumMapper::epCheckState),
            &::getCppuType( static_cast< sal_Int16* >(NULL)));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getSpecialAttributeName(SCA_CURRENT_STATE), PROPERTY_STATE, STATE_NOCHECK,
            OEnumMapper::getEnumMap(OEnumMapper::epCheckState),
            &::getCppuType( static_cast< sal_Int16* >(NULL)));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getFormAttributeName(faEnctype), PROPERTY_SUBMIT_ENCODING,
            FormSubmitEncoding_URL, OEnumMapper::getEnumMap(OEnumMapper::epSubmitEncoding),
            &::getCppuType( static_cast<FormSubmitEncoding*>(NULL) ));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getFormAttributeName(faMethod), PROPERTY_SUBMIT_METHOD,
            FormSubmitMethod_GET, OEnumMapper::getEnumMap(OEnumMapper::epSubmitMethod),
            &::getCppuType( static_cast<FormSubmitMethod*>(NULL) ));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getFormAttributeName(faCommandType), PROPERTY_COMMAND_TYPE,
            CommandType::COMMAND, OEnumMapper::getEnumMap(OEnumMapper::epCommandType));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getFormAttributeName(faNavigationMode), PROPERTY_NAVIGATION,
            NavigationBarMode_NONE, OEnumMapper::getEnumMap(OEnumMapper::epNavigationType),
            &::getCppuType( static_cast<NavigationBarMode*>(NULL) ));
        m_aAttributeMetaData.addEnumProperty(
            OAttributeMetaData::getFormAttributeName(faTabbingCycle), PROPERTY_CYCLE,
            TabulatorCycle_RECORDS, OEnumMapper::getEnumMap(OEnumMapper::epTabCyle),
            &::getCppuType( static_cast<TabulatorCycle*>(NULL) ));

        // initialize our style map
        m_xPropertyHandlerFactory = new OControlPropertyHandlerFactory();
        ::rtl::Reference< XMLPropertySetMapper > xStylePropertiesMapper = new XMLPropertySetMapper(getControlStylePropertyMap(), m_xPropertyHandlerFactory.get());
        m_xImportMapper = new SvXMLImportPropertyMapper(xStylePropertiesMapper.get(), _rImporter);

        // 'initialize'
        m_aCurrentPageIds = m_aControlIds.end();
    }

    //---------------------------------------------------------------------
    OFormLayerXMLImport_Impl::~OFormLayerXMLImport_Impl()
    {
        // outlined to allow forward declaration of OAttribute2Property in the header

        if (m_pAutoStyles)
            m_pAutoStyles->ReleaseRef();
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::setAutoStyleContext(SvXMLStylesContext* _pNewContext)
    {
        OSL_ENSURE(!m_pAutoStyles, "OFormLayerXMLImport_Impl::setAutoStyleContext: not to be called twice!");
        m_pAutoStyles = _pNewContext;
        if (m_pAutoStyles)
            m_pAutoStyles->AddRef();
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::applyControlNumberStyle(const Reference< XPropertySet >& _rxControlModel, const ::rtl::OUString& _rControlNumerStyleName)
    {
        OSL_ENSURE(_rxControlModel.is() && (0 != _rControlNumerStyleName.getLength()),
            "OFormLayerXMLImport_Impl::applyControlNumberStyle: invalid arguments (this will crash)!");

        OSL_ENSURE(m_pAutoStyles, "OFormLayerXMLImport_Impl::applyControlNumberStyle: have no auto style context!");
        if (!m_pAutoStyles)
        {
            m_pAutoStyles = m_rImporter.GetShapeImport()->GetAutoStylesContext();
            if (m_pAutoStyles)
                m_pAutoStyles->AddRef();
        }

        if (m_pAutoStyles)
        {
            const SvXMLStyleContext* pStyle = m_pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_DATA_STYLE, _rControlNumerStyleName);
            if (pStyle)
            {
                const SvXMLNumFormatContext* pDataStyle = static_cast<const SvXMLNumFormatContext*>(pStyle);

                ::rtl::OUString sFormatDescription;
                Locale aFormatLocale;
                const_cast<SvXMLNumFormatContext*>(pDataStyle)->GetFormat(sFormatDescription, aFormatLocale);

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
                        sal_Int32 nFormatKey = xFormats->queryKey(sFormatDescription, aFormatLocale, sal_False);
                        if (-1 == nFormatKey)
                        {	// not yet available -> add it
                            nFormatKey = xFormats->addNew(sFormatDescription, aFormatLocale);
                        }

                        OSL_ENSURE(-1 != nFormatKey, "OFormLayerXMLImport_Impl::applyControlNumberStyle: could not obtain a format key!");
                        // set the format on the control model
                        _rxControlModel->setPropertyValue(PROPERTY_FORMATKEY, makeAny(nFormatKey));
                    }
                }
                catch(const Exception&)
                {
                    OSL_ENSURE(sal_False, "OFormLayerXMLImport_Impl::applyControlNumberStyle: couldn't set the format!");
                }
            }
            else
                OSL_ENSURE(sal_False, "OFormLayerXMLImport_Impl::applyControlNumberStyle: did not find the style with the given name!");
        }
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::registerCellValueBinding( const Reference< XPropertySet >& _rxControlModel, const ::rtl::OUString& _rCellAddress )
    {
        OSL_ENSURE( _rxControlModel.is() && _rCellAddress.getLength(),
            "OFormLayerXMLImport_Impl::registerCellValueBinding: invalid arguments!" );
        m_aCellValueBindings.push_back( ModelStringPair( _rxControlModel, _rCellAddress ) );
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::registerCellRangeListSource( const Reference< XPropertySet >& _rxControlModel, const ::rtl::OUString& _rCellRangeAddress )
    {
        OSL_ENSURE( _rxControlModel.is() && _rCellRangeAddress.getLength(),
            "OFormLayerXMLImport_Impl::registerCellRangeListSource: invalid arguments!" );
        m_aCellRangeListSources.push_back( ModelStringPair( _rxControlModel, _rCellRangeAddress ) );
    }

    //---------------------------------------------------------------------
    IControlIdMap& OFormLayerXMLImport_Impl::getControlIdMap()
    {
        return *this;
    }

    //---------------------------------------------------------------------
    OAttribute2Property& OFormLayerXMLImport_Impl::getAttributeMap()
    {
        return m_aAttributeMetaData;
    }

    //---------------------------------------------------------------------
    Reference< XMultiServiceFactory > OFormLayerXMLImport_Impl::getServiceFactory()
    {
        // #110680#
        // return ::comphelper::getProcessServiceFactory();
        return m_rImporter.getServiceFactory();
    }

    //---------------------------------------------------------------------
    SvXMLImport& OFormLayerXMLImport_Impl::getGlobalContext()
    {
        return m_rImporter;
    }

    //---------------------------------------------------------------------
    const SvXMLStyleContext* OFormLayerXMLImport_Impl::getStyleElement(const ::rtl::OUString& _rStyleName) const
    {
        OSL_ENSURE( m_pAutoStyles, "OFormLayerXMLImport_Impl::getStyleElement: have no auto style context!" );
            // did you use setAutoStyleContext?

        const SvXMLStyleContext* pControlStyle =
            m_pAutoStyles ? m_pAutoStyles->FindStyleChildContext( XML_STYLE_FAMILY_CONTROL_ID, _rStyleName ) : NULL;
        OSL_ENSURE( pControlStyle || !m_pAutoStyles,
                    ::rtl::OString( "OFormLayerXMLImport_Impl::getStyleElement: did not find the style named \"" )
                +=	::rtl::OString( _rStyleName.getStr(), _rStyleName.getLength(), RTL_TEXTENCODING_ASCII_US )
                +=	::rtl::OString( "\"!" ) );
        return pControlStyle;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::enterEventContext()
    {
        // install our own translation table. We need to disable the other tables because of name conflicts.
        m_rImporter.GetEventImport().PushTranslationTable();
        m_rImporter.GetEventImport().AddTranslationTable(g_pFormsEventTranslation);
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::leaveEventContext()
    {
        // install the original event tables.
        m_rImporter.GetEventImport().PopTranslationTable();
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::registerControlId(const Reference< XPropertySet >& _rxControl, const ::rtl::OUString& _rId)
    {
        OSL_ENSURE(m_aCurrentPageIds != m_aControlIds.end(), "OFormLayerXMLImport_Impl::registerControlId: no current page!");
        OSL_ENSURE(_rId.getLength(), "OFormLayerXMLImport_Impl::registerControlId: invalid (empty) control id!");

        OSL_ENSURE(m_aCurrentPageIds->second.end() == m_aCurrentPageIds->second.find(_rId), "OFormLayerXMLImport_Impl::registerControlId: control id already used!");
        m_aCurrentPageIds->second[_rId] = _rxControl;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::registerControlReferences(const Reference< XPropertySet >& _rxControl, const ::rtl::OUString& _rReferringControls)
    {
        OSL_ENSURE(_rReferringControls.getLength(), "OFormLayerXMLImport_Impl::registerControlReferences: invalid (empty) control id list!");
        OSL_ENSURE(_rxControl.is(), "OFormLayerXMLImport_Impl::registerControlReferences: invalid (NULL) control!");
        m_aControlReferences.push_back( ModelStringPair( _rxControl, _rReferringControls ) );
    }

    //---------------------------------------------------------------------
    ::rtl::Reference< SvXMLImportPropertyMapper > OFormLayerXMLImport_Impl::getStylePropertyMapper() const
    {
        return m_xImportMapper;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::startPage(const Reference< XDrawPage >& _rxDrawPage)
    {
        m_xForms.clear();

        OSL_ENSURE(_rxDrawPage.is(), "OFormLayerXMLImport_Impl::startPage: NULL page!");
        Reference< XFormsSupplier > xFormsSupp(_rxDrawPage, UNO_QUERY);
        OSL_ENSURE(xFormsSupp.is(), "OFormLayerXMLImport_Impl::startPage: invalid draw page (no XFormsSupplier)!");
        if (!xFormsSupp.is())
            return;

        m_xForms = Reference< XNameContainer >(xFormsSupp->getForms(), UNO_QUERY);
        OSL_ENSURE(m_xForms.is(), "OFormLayerXMLImport_Impl::startPage: invalid forms collection!");

        // add a new entry to our page map
        ::std::pair< MapDrawPage2MapIterator, bool > aPagePosition =
            m_aControlIds.insert(MapDrawPage2Map::value_type(_rxDrawPage, MapString2PropertySet()));
        OSL_ENSURE(aPagePosition.second, "OFormLayerXMLImport_Impl::startPage: already imported this page!");
        m_aCurrentPageIds = aPagePosition.first;
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::endPage()
    {
        OSL_ENSURE(m_xForms.is(), "OFormLayerXMLImport_Impl::endPage: sure you called startPage before?");

        // do some knittings for the controls which are referring to each other
        try
        {
            static const sal_Unicode s_nSeparator = ',';
            ::rtl::OUString sReferring;
            ::rtl::OUString sCurrentReferring;
            ::rtl::OUString sSeparator(&s_nSeparator, 1);
            Reference< XPropertySet > xCurrentReferring;
            sal_Int32 nSeparator, nPrevSep;
            for	(	::std::vector< ModelStringPair >::const_iterator aReferences = m_aControlReferences.begin();
                    aReferences != m_aControlReferences.end();
                    ++aReferences
                )
            {
                // the list of control ids is comma separated

                // in a list of n ids there are only n-1 separators ... have to catch this last id
                // -> normalize the list
                sReferring = aReferences->second;
                sReferring += sSeparator;

                nPrevSep = -1;
                while (-1 != (nSeparator = sReferring.indexOf(s_nSeparator, nPrevSep + 1)))
                {
                    sCurrentReferring = sReferring.copy(nPrevSep + 1, nSeparator - nPrevSep - 1);
                    xCurrentReferring = lookupControlId(sCurrentReferring);
                    if (xCurrentReferring.is())
                        // if this condition fails, this is an error, but lookupControlId should have asserted this ...
                        xCurrentReferring->setPropertyValue( PROPERTY_CONTROLLABEL, makeAny( aReferences->first ) );

                    nPrevSep = nSeparator;
                }
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OFormLayerXMLImport_Impl::endPage: unable to knit the control references (caught an exception)!");
        }

        // now that we have all children of the forms collection, attach the events
        Reference< XIndexAccess > xIndexContainer(m_xForms, UNO_QUERY);
        if (xIndexContainer.is())
            ODefaultEventAttacherManager::setEvents(xIndexContainer);

        // clear the structures for the control references.
        m_aControlReferences.clear();

        // and no we have no current page anymore
        m_aCurrentPageIds = m_aControlIds.end();
    }

    //---------------------------------------------------------------------
    Reference< XPropertySet > OFormLayerXMLImport_Impl::lookupControlId(const ::rtl::OUString& _rControlId)
    {
        OSL_ENSURE(m_aCurrentPageIds != m_aControlIds.end(), "OFormLayerXMLImport_Impl::lookupControlId: no current page!");
        Reference< XPropertySet > xReturn;
        if (m_aCurrentPageIds != m_aControlIds.end())
        {
            ConstMapString2PropertySetIterator aPos = m_aCurrentPageIds->second.find(_rControlId);
            if (m_aCurrentPageIds->second.end() != aPos)
                xReturn = aPos->second;
            else
                OSL_ENSURE(sal_False, "OFormLayerXMLImport_Impl::lookupControlId: invalid control id (did not find it)!");
        }
        return xReturn;
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OFormLayerXMLImport_Impl::createOfficeFormsContext(
        SvXMLImport& _rImport,
        sal_uInt16 _nPrefix,
        const ::rtl::OUString& _rLocalName)
    {
        return new OFormsRootImport( _rImport, _nPrefix, _rLocalName );
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OFormLayerXMLImport_Impl::createContext(const sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttribs)
    {
        OSL_ENSURE(m_xForms.is(), "OFormLayerXMLImport_Impl::createContext: have no forms collection (did you use startPage?)!");
        OSL_ENSURE(0 == _rLocalName.compareToAscii("form"), "OFormLayerXMLImport_Impl::createContext: don't know the element name (must be \"form\")!");

        if (!m_xForms.is() || (0 != _rLocalName.compareToAscii("form")))
        {
            return new SvXMLImportContext(m_rImporter, _nPrefix, _rLocalName);
        }

        return new OFormImport(*this, *this, _nPrefix, _rLocalName, m_xForms );
    }

    //---------------------------------------------------------------------
    XMLPropStyleContext* OFormLayerXMLImport_Impl::createControlStyleContext( sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttrList, SvXMLStylesContext& _rParentStyles,
        sal_uInt16 _nFamily, sal_Bool _bDefaultStyle )
    {
        return new OControlStyleContext( m_rImporter, _nPrefix, _rLocalName, _rxAttrList, _rParentStyles, _nFamily, _bDefaultStyle );
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport_Impl::documentDone( )
    {
        if ( ( getGlobalContext().getImportFlags() & IMPORT_CONTENT ) == 0 )
            return;

        // create (and bind) the spreadsheet cell bindings
        if  (   !m_aCellValueBindings.empty()
            &&  FormCellBindingHelper::isCellBindingAllowed( getGlobalContext().GetModel() )
            )
        {
            for (   ::std::vector< ModelStringPair >::const_iterator aCellBindings = m_aCellValueBindings.begin();
                    aCellBindings != m_aCellValueBindings.end();
                    ++aCellBindings
                )
            {
                try
                {
                    FormCellBindingHelper aHelper( aCellBindings->first, getGlobalContext().GetModel() );
                    OSL_ENSURE( aHelper.isCellBindingAllowed(), "OFormLayerXMLImport_Impl::documentDone: can't bind this control model!" );
                    if ( aHelper.isCellBindingAllowed() )
                    {
                        // There are special bindings for listboxes. See
                        // OListAndComboImport::doRegisterCellValueBinding for a comment on this HACK.
                        ::rtl::OUString sBoundCellAddress( aCellBindings->second );
                        sal_Int32 nIndicator = sBoundCellAddress.lastIndexOf( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":index" ) ) );

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
                    OSL_ENSURE( sal_False, "OFormLayerXMLImport_Impl::documentDone: caught an exception while binding to a cell!" );
                }
            }
            m_aCellValueBindings.clear();
        }

        // the same for the spreadsheet cell range list sources
        if  (   !m_aCellRangeListSources.empty()
            &&  FormCellBindingHelper::isListCellRangeAllowed( getGlobalContext().GetModel() )
            )
        {
            for (   ::std::vector< ModelStringPair >::const_iterator aRangeBindings = m_aCellRangeListSources.begin();
                    aRangeBindings != m_aCellRangeListSources.end();
                    ++aRangeBindings
                )
            {
                try
                {
                    FormCellBindingHelper aHelper( aRangeBindings->first, getGlobalContext().GetModel() );
                    OSL_ENSURE( aHelper.isListCellRangeAllowed(), "OFormLayerXMLImport_Impl::documentDone: can't bind this control model!" );
                    if ( aHelper.isListCellRangeAllowed() )
                    {
                        aHelper.setListSource( aHelper.createCellListSourceFromStringAddress( aRangeBindings->second ) );
                    }
                }
                catch( const Exception& )
                {
                    OSL_ENSURE( sal_False, "OFormLayerXMLImport_Impl::documentDone: caught an exception while binding to a cell range!" );
                }
            }
            m_aCellRangeListSources.clear();
        }
    }

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
