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

#include "elementexport.hxx"

#include "strings.hxx"
#include <xmloff/xmlnmspe.hxx>
#include "eventexport.hxx"
#include "formenums.hxx"
#include "formcellbinding.hxx"
#include <xmloff/xformsexport.hxx>
#include "property_meta_data.hxx"

#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#include <com/sun/star/form/FormSubmitMethod.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/form/NavigationBarMode.hpp>
#include <com/sun/star/form/TabulatorCycle.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/awt/ImagePosition.hpp>

#include <sax/tools/converter.hxx>
#include <tools/gen.hxx>
#include <xmloff/txtprmap.hxx>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <tools/urlobj.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/XMLEventExport.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/maptype.hxx>
#include <tools/time.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/extract.hxx>
#include <sal/macros.h>
#include <sal/log.hxx>

#include <algorithm>

namespace xmloff
{

    #if OSL_DEBUG_LEVEL > 0
        #define RESET_BIT( bitfield, bit ) \
            bitfield = bitfield & ~bit
    #else
        #define RESET_BIT( bitfield, bit )
    #endif

    using namespace ::xmloff::token;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::table;
    using namespace ::com::sun::star::text;
    using namespace ::com::sun::star::form::binding;

    //= OElementExport
    OElementExport::OElementExport(IFormsExportContext& _rContext, const Reference< XPropertySet >& _rxProps,
        const Sequence< ScriptEventDescriptor >& _rEvents)
        :OPropertyExport(_rContext, _rxProps)
        ,m_aEvents(_rEvents)
    {
    }

    OElementExport::~OElementExport()
    {
    }

    void OElementExport::doExport()
    {
        // collect some general information about the element
        examine();

        // first add the attributes necessary for the element
        m_rContext.getGlobalContext().ClearAttrList();

        // add the attributes
        exportAttributes();

        // start the XML element
        implStartElement(getXMLElementName());

        // the sub elements (mostly control type dependent)
        exportSubTags();

        implEndElement();
    }

    void OElementExport::examine()
    {
        // nothing to do here
    }

    void OElementExport::exportAttributes()
    {
        // nothing to do here
    }

    void OElementExport::exportSubTags()
    {
        // the properties which where not exported 'til now
        exportRemainingProperties();

        // the script:events sub tags
        exportEvents();
    }

    void OElementExport::implStartElement(const sal_Char* _pName)
    {
        m_pXMLElement = std::make_unique<SvXMLElementExport>(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, _pName, true, true);
    }

    void OElementExport::implEndElement()
    {
        m_pXMLElement.reset();
    }

    void OElementExport::exportServiceNameAttribute()
    {
        Reference< XPersistObject > xPersistence(m_xProps, UNO_QUERY);
        if (!xPersistence.is())
        {
            OSL_FAIL("OElementExport::exportServiceNameAttribute: no XPersistObject!");
            return;
        }

        OUString sServiceName = xPersistence->getServiceName();
        // we don't want to write the old service name directly: it's a name used for compatibility reasons, but
        // as we start some kind of new file format here (with this xml export), we don't care about
        // compatibility ...
        // So we translate the old persistence service name into new ones, if possible

        OUString sToWriteServiceName = sServiceName;
#define CHECK_N_TRANSLATE( name )   \
        else if (sServiceName == SERVICE_PERSISTENT_COMPONENT_##name) \
            sToWriteServiceName = SERVICE_##name

        if (sServiceName == SERVICE_PERSISTENT_COMPONENT_EDIT)
        {
            // special handling for the edit field: we have two controls using this as persistence service name
            sToWriteServiceName = SERVICE_EDIT;
            Reference< XServiceInfo > xSI(m_xProps, UNO_QUERY);
            if (xSI.is() && xSI->supportsService(SERVICE_FORMATTEDFIELD))
                sToWriteServiceName = SERVICE_FORMATTEDFIELD;
        }
        CHECK_N_TRANSLATE( FORM );
        CHECK_N_TRANSLATE( LISTBOX );
        CHECK_N_TRANSLATE( COMBOBOX );
        CHECK_N_TRANSLATE( RADIOBUTTON );
        CHECK_N_TRANSLATE( GROUPBOX );
        CHECK_N_TRANSLATE( FIXEDTEXT );
        CHECK_N_TRANSLATE( COMMANDBUTTON );
        CHECK_N_TRANSLATE( CHECKBOX );
        CHECK_N_TRANSLATE( GRID );
        CHECK_N_TRANSLATE( IMAGEBUTTON );
        CHECK_N_TRANSLATE( FILECONTROL );
        CHECK_N_TRANSLATE( TIMEFIELD );
        CHECK_N_TRANSLATE( DATEFIELD );
        CHECK_N_TRANSLATE( NUMERICFIELD );
        CHECK_N_TRANSLATE( CURRENCYFIELD );
        CHECK_N_TRANSLATE( PATTERNFIELD );
        CHECK_N_TRANSLATE( HIDDENCONTROL );
        CHECK_N_TRANSLATE( IMAGECONTROL );
        CHECK_N_TRANSLATE( FORMATTEDFIELD );
#if OSL_DEBUG_LEVEL > 0
        Reference< XServiceInfo > xSI(m_xProps, UNO_QUERY);
        OSL_ENSURE(xSI.is() && xSI->supportsService(sToWriteServiceName),
            "OElementExport::exportServiceNameAttribute: wrong service name translation!");

#endif
        sToWriteServiceName =
            m_rContext.getGlobalContext().GetNamespaceMap().GetQNameByKey(
                XML_NAMESPACE_OOO, sToWriteServiceName );

        // now write this
        AddAttribute(
            OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::ServiceName),
            OAttributeMetaData::getCommonControlAttributeName(CCAFlags::ServiceName),
            sToWriteServiceName);
    }

    void OElementExport::exportEvents()
    {
        if (!m_aEvents.getLength())
            // nothing to do
            return;

        Reference< XNameReplace > xWrapper = new OEventDescriptorMapper(m_aEvents);
        m_rContext.getGlobalContext().GetEventExport().Export(xWrapper);
    }

    //= OControlExport
    OControlExport::OControlExport(IFormsExportContext& _rContext,  const Reference< XPropertySet >& _rxControl,
        const OUString& _rControlId, const OUString& _rReferringControls,
        const Sequence< ScriptEventDescriptor >& _rEvents)
        :OElementExport(_rContext, _rxControl, _rEvents)
        ,m_sControlId(_rControlId)
        ,m_sReferringControls(_rReferringControls)
        ,m_nClassId(FormComponentType::CONTROL)
        ,m_eType( UNKNOWN )
        ,m_nIncludeCommon(CCAFlags::NONE)
        ,m_nIncludeDatabase(DAFlags::NONE)
        ,m_nIncludeSpecial(SCAFlags::NONE)
        ,m_nIncludeEvents(EAFlags::NONE)
        ,m_nIncludeBindings(BAFlags::NONE)
    {
        OSL_ENSURE(m_xProps.is(), "OControlExport::OControlExport: invalid arguments!");
    }

    void OControlExport::exportOuterAttributes()
    {
        // the control id
        if (CCAFlags::Name & m_nIncludeCommon)
        {
            exportStringPropertyAttribute(
                OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::Name),
                OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Name),
                PROPERTY_NAME
                );
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCAFlags::Name;
        #endif
        }

        // the service name
        if (m_nIncludeCommon & CCAFlags::ServiceName)
        {
            exportServiceNameAttribute();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCAFlags::ServiceName;
        #endif
        }
    }

    void OControlExport::exportInnerAttributes()
    {
        // the control id
        if (CCAFlags::ControlId & m_nIncludeCommon)
        {
            OSL_ENSURE(!m_sControlId.isEmpty(), "OControlExport::exportInnerAttributes: have no control id for the control!");
            m_rContext.getGlobalContext().AddAttributeIdLegacy(
                XML_NAMESPACE_FORM, m_sControlId);
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCAFlags::ControlId;
        #endif
        }

        // "new-style" properties ...
        exportGenericHandlerAttributes();

        // common control attributes
        exportCommonControlAttributes();

        // common database attributes
        exportDatabaseAttributes();

        // attributes related to external bindings
        exportBindingAttributes();

        // attributes special to the respective control type
        exportSpecialAttributes();

        // add the style references to the attributes
        flagStyleProperties();
    }

    void OControlExport::exportAttributes()
    {
        exportOuterAttributes();
    }

    void OControlExport::exportSubTags()
    {
        // for the upcoming exportRemainingProperties:
        // if a control has the LabelControl property, this is not stored with the control itself, but instead with
        // the control which is referenced by this property. As the base class' exportRemainingProperties doesn't
        // know anything about this, we need to prevent that it tries to export this property
        exportedProperty(PROPERTY_CONTROLLABEL);

        // if it's a control supporting XText, then we need to declare all text-related properties
        // as "already exported". This prevents them from being exported as generic "form:property"-tags.
        // *If* we would export them this way, they would be completely superfluous, and sometimes even
        // disastrous, since they may, at import time, override paragraph properties which already have
        // been set before
        Reference< XText > xControlText( m_xProps, UNO_QUERY );
        if ( xControlText.is() )
        {
            const XMLPropertyMapEntry* pCharAttributeProperties = XMLTextPropertySetMapper::getPropertyMapForType( TextPropMap::TEXT );
            while ( pCharAttributeProperties->msApiName )
            {
                exportedProperty( OUString::createFromAscii( pCharAttributeProperties->msApiName ) );
                ++pCharAttributeProperties;
            }

            const XMLPropertyMapEntry* pParaAttributeProperties = XMLTextPropertySetMapper::getPropertyMapForType( TextPropMap::SHAPE_PARA );
            while ( pParaAttributeProperties->msApiName )
            {
                exportedProperty( OUString::createFromAscii( pParaAttributeProperties->msApiName ) );
                ++pParaAttributeProperties;
            }

            // the RichText property is not exported. The presence of the text:p element
            // will be used - upon reading - as indicator for the value of the RichText property
            exportedProperty( PROPERTY_RICH_TEXT );

            // strange thing: paragraphs support both a CharStrikeout and a CharCrossedOut property
            // The former is a short/enum value, the latter a boolean. The former has a real meaning
            // (the strikeout type), the latter hasn't. But, when the CharCrossedOut is exported and
            // later on imported, it overwrites anything which has previously been imported for
            // CharStrikeout.
            // #i27729#
            exportedProperty( "CharCrossedOut" );
        }

        if ( m_eType == LISTBOX )
        {
            // will be exported in exportListSourceAsElements:
            if ( controlHasUserSuppliedListEntries() )
                exportedProperty( PROPERTY_DEFAULT_SELECT_SEQ );

            // will not be exported in a generic way. Either exportListSourceAsElements cares
            // for them, or we don't need them
            exportedProperty( PROPERTY_STRING_ITEM_LIST );
            exportedProperty( PROPERTY_VALUE_SEQ );
            exportedProperty( PROPERTY_SELECT_SEQ );
            exportedProperty( PROPERTY_LISTSOURCE );
        }
        if ( m_eType == COMBOBOX )
            exportedProperty( PROPERTY_STRING_ITEM_LIST );

        // let the base class export the remaining properties and the events
        OElementExport::exportSubTags();

        // special sub tags for some controls
        switch (m_eType)
        {
            case LISTBOX:
                // don't export the list entries if the are not provided by the user, but obtained implicitly
                // from other sources
                // #i26944#
                if ( controlHasUserSuppliedListEntries() )
                    exportListSourceAsElements();
                break;
            case GRID:
            {   // a grid control requires us to store all columns as sub elements
                Reference< XIndexAccess > xColumnContainer(m_xProps, UNO_QUERY);
                OSL_ENSURE(xColumnContainer.is(), "OControlExport::exportSubTags: a grid control which is no IndexAccess?!!");
                if (xColumnContainer.is())
                    m_rContext.exportCollectionElements(xColumnContainer);
            }
            break;
            case COMBOBOX:
            {   // a combox box description has sub elements: the items
                DBG_CHECK_PROPERTY( PROPERTY_STRING_ITEM_LIST, Sequence< OUString > );

                // don't export the list entries if the are not provided by the user, but obtained implicitly
                // from other sources
                // #i26944#
                if ( controlHasUserSuppliedListEntries() )
                {
                    // get the item list
                    Sequence< OUString > aListItems;
                    m_xProps->getPropertyValue(PROPERTY_STRING_ITEM_LIST) >>= aListItems;
                    // loop through it and write the sub elements
                    const OUString* pListItems = aListItems.getConstArray();
                    for (sal_Int32 i=0; i<aListItems.getLength(); ++i, ++pListItems)
                    {
                        m_rContext.getGlobalContext().ClearAttrList();
                        AddAttribute(
                            OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::Label),
                            OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Label),
                            *pListItems);
                        SvXMLElementExport aFormElement(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, "item", true, true);
                    }
                }
            }
            break;

            case TEXT_AREA:
            {
                // if we act as rich text control, we need to export some text:p elements
                if ( xControlText.is() )
                {
                    bool bActingAsRichText = false;
                    if ( m_xPropertyInfo->hasPropertyByName( PROPERTY_RICH_TEXT ) )
                    {
                        OSL_VERIFY(m_xProps->getPropertyValue( PROPERTY_RICH_TEXT ) >>= bActingAsRichText );
                    }

                    if ( bActingAsRichText )
                        m_rContext.getGlobalContext().GetTextParagraphExport()->exportText( xControlText );
                }
            }
            break;
            default:
                // nothing do to
                break;
        }
    }

    void OControlExport::exportGenericHandlerAttributes()
    {
        const Sequence< Property > aProperties = m_xPropertyInfo->getProperties();
        for ( auto const & prop : aProperties )
        {
            try
            {
                // see if this property can already be handled with an IPropertyHandler (which, on the long
                // term, should be the case for most, if not all, properties)
                const PropertyDescription* propDescription = metadata::getPropertyDescription( prop.Name );
                if ( propDescription == nullptr )
                    continue;

                // let the factory provide the concrete handler. Note that caching, if desired, is the task
                // of the factory
                PPropertyHandler handler = (*propDescription->factory)( propDescription->propertyId );
                if ( !handler.get() )
                {
                    SAL_WARN( "xmloff.forms", "OControlExport::exportGenericHandlerAttributes: invalid property handler provided by the factory!" );
                    continue;
                }

                OUString attributeValue;
                if ( propDescription->propertyGroup == NO_GROUP )
                {
                    // that's a property which has a direct mapping to an attribute
                    if ( !shouldExportProperty( prop.Name ) )
                        // TODO: in the future, we surely need a more sophisticated approach to this, involving the property
                        // handler, or the property description
                    {
                        exportedProperty( prop.Name );
                        continue;
                    }

                    const Any propValue = m_xProps->getPropertyValue( prop.Name );
                    attributeValue = handler->getAttributeValue( propValue );
                }
                else
                {
                    // that's a property which is part of a group of properties, whose values, in their entity, comprise
                    // a single attribute value

                    // retrieve the descriptions of all other properties which add to the attribute value
                    PropertyDescriptionList descriptions;
                    metadata::getPropertyGroup( propDescription->propertyGroup, descriptions );

                    // retrieve the values for all those properties
                    PropertyValues aValues;
                    for ( const auto& desc : descriptions )
                    {
                        // TODO: XMultiPropertySet?
                        const Any propValue = m_xProps->getPropertyValue( desc->propertyName );
                        aValues[ desc->propertyId ] = propValue;
                    }

                    // let the handler translate into an XML attribute value
                    attributeValue = handler->getAttributeValue( aValues );
                }

                AddAttribute(
                    propDescription->attribute.namespacePrefix,
                    token::GetXMLToken( propDescription->attribute.attributeToken ),
                    attributeValue
                );

                exportedProperty( prop.Name );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("xmloff.forms");
            }
        }
    }

    void OControlExport::exportCommonControlAttributes()
    {
        size_t i=0;

        // I decided to handle all the properties here with some static arrays describing the property-attribute
        // relations. This leads to somewhat ugly code :), but the only alternative I can think of right now
        // would require maps and O(log n) searches, which seems somewhat expensive as this code is used
        // very frequently.

        // the extra indents for the respective blocks are to ensure that there is no copy'n'paste error, using
        // map identifiers from the wrong block

        // some string properties
        {
            // the attribute ids of all properties which are expected to be of type string
            static const CCAFlags nStringPropertyAttributeIds[] =
            {
                CCAFlags::Label, CCAFlags::Title
            };
            // the names of all properties which are expected to be of type string
            static const char * aStringPropertyNames[] =
            {
                PROPERTY_LABEL, PROPERTY_TITLE
            };
            OSL_ENSURE( SAL_N_ELEMENTS(aStringPropertyNames) ==
                        SAL_N_ELEMENTS(nStringPropertyAttributeIds),
                        "OControlExport::exportCommonControlAttributes: somebody tampered with the maps (1)!");

            for (i=0; i<SAL_N_ELEMENTS(nStringPropertyAttributeIds); ++i)
                if (nStringPropertyAttributeIds[i] & m_nIncludeCommon)
                {
                    exportStringPropertyAttribute(
                        OAttributeMetaData::getCommonControlAttributeNamespace(nStringPropertyAttributeIds[i]),
                        OAttributeMetaData::getCommonControlAttributeName(nStringPropertyAttributeIds[i]),
                        OUString::createFromAscii(aStringPropertyNames[i])
                        );
                #if OSL_DEBUG_LEVEL > 0
                    //  reset the bit for later checking
                    m_nIncludeCommon = m_nIncludeCommon & ~nStringPropertyAttributeIds[i];
                #endif
                }
        }

        // some boolean properties
        {
            static const CCAFlags nBooleanPropertyAttributeIds[] =
            {   // attribute flags
                CCAFlags::CurrentSelected, CCAFlags::Disabled, CCAFlags::Dropdown, CCAFlags::Printable, CCAFlags::ReadOnly, CCAFlags::Selected, CCAFlags::TabStop, CCAFlags::EnableVisible
            };
            static const char * pBooleanPropertyNames[] =
            {   // property names
                PROPERTY_STATE, PROPERTY_ENABLED,
                PROPERTY_DROPDOWN, PROPERTY_PRINTABLE,
                PROPERTY_READONLY, PROPERTY_DEFAULT_STATE,
                PROPERTY_TABSTOP, PROPERTY_ENABLEVISIBLE
            };
            static const BoolAttrFlags nBooleanPropertyAttrFlags[] =
            {   // attribute defaults
                BoolAttrFlags::DefaultFalse, BoolAttrFlags::DefaultFalse | BoolAttrFlags::InverseSemantics, BoolAttrFlags::DefaultFalse, BoolAttrFlags::DefaultTrue, BoolAttrFlags::DefaultFalse, BoolAttrFlags::DefaultFalse, BoolAttrFlags::DefaultVoid, BoolAttrFlags::DefaultFalse
            };
        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nIdCount = SAL_N_ELEMENTS(nBooleanPropertyAttributeIds);
            static const sal_Int32 nNameCount = SAL_N_ELEMENTS(pBooleanPropertyNames);
            static const sal_Int32 nFlagsCount = SAL_N_ELEMENTS(nBooleanPropertyAttrFlags);
            OSL_ENSURE((nIdCount == nNameCount) && (nNameCount == nFlagsCount),
                "OControlExport::exportCommonControlAttributes: somebody tampered with the maps (2)!");
        #endif
            for (i=0; i<SAL_N_ELEMENTS(nBooleanPropertyAttributeIds); ++i)
                if (nBooleanPropertyAttributeIds[i] & m_nIncludeCommon)
                {
                    exportBooleanPropertyAttribute(
                        OAttributeMetaData::getCommonControlAttributeNamespace(nBooleanPropertyAttributeIds[i]),
                        OAttributeMetaData::getCommonControlAttributeName(nBooleanPropertyAttributeIds[i]),
                        OUString::createFromAscii(pBooleanPropertyNames[i]),
                        nBooleanPropertyAttrFlags[i]);
        #if OSL_DEBUG_LEVEL > 0
                    //  reset the bit for later checking
                    m_nIncludeCommon = m_nIncludeCommon & ~nBooleanPropertyAttributeIds[i];
        #endif
                }
        }

        // some integer properties
        {
            // now the common handling
            static const CCAFlags nIntegerPropertyAttributeIds[] =
            {   // attribute flags
                CCAFlags::Size, CCAFlags::TabIndex
            };
            static const char * pIntegerPropertyNames[] =
            {   // property names
                PROPERTY_LINECOUNT, PROPERTY_TABINDEX
            };
            static const sal_Int16 nIntegerPropertyAttrDefaults[] =
            {   // attribute defaults
                5, 0
            };

            if ( m_nIncludeCommon & CCAFlags::MaxLength )
                exportedProperty(PROPERTY_MAXTEXTLENGTH);

        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nIdCount = SAL_N_ELEMENTS(nIntegerPropertyAttributeIds);
            static const sal_Int32 nNameCount = SAL_N_ELEMENTS(pIntegerPropertyNames);
            static const sal_Int32 nDefaultCount = SAL_N_ELEMENTS(nIntegerPropertyAttrDefaults);
            OSL_ENSURE((nIdCount == nNameCount) && (nNameCount == nDefaultCount),
                "OControlExport::exportCommonControlAttributes: somebody tampered with the maps (3)!");
        #endif
            for (i=0; i<SAL_N_ELEMENTS(nIntegerPropertyAttributeIds); ++i)
                if (nIntegerPropertyAttributeIds[i] & m_nIncludeCommon)
                {
                    exportInt16PropertyAttribute(
                        OAttributeMetaData::getCommonControlAttributeNamespace(nIntegerPropertyAttributeIds[i]),
                        OAttributeMetaData::getCommonControlAttributeName(nIntegerPropertyAttributeIds[i]),
                        OUString::createFromAscii(pIntegerPropertyNames[i]),
                        nIntegerPropertyAttrDefaults[i]);
        #if OSL_DEBUG_LEVEL > 0
                    //  reset the bit for later checking
                    m_nIncludeCommon = m_nIncludeCommon & ~nIntegerPropertyAttributeIds[i];
        #endif
                }

        }

        // some enum properties
        {
            if (m_nIncludeCommon & CCAFlags::ButtonType)
            {
                exportEnumPropertyAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::ButtonType),
                    OAttributeMetaData::getCommonControlAttributeName(CCAFlags::ButtonType),
                    PROPERTY_BUTTONTYPE,
                    aFormButtonTypeMap,
                    FormButtonType_PUSH);
        #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeCommon = m_nIncludeCommon & ~CCAFlags::ButtonType;
        #endif
            }
            if ( m_nIncludeCommon & CCAFlags::Orientation )
            {
                exportEnumPropertyAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace( CCAFlags::Orientation ),
                    OAttributeMetaData::getCommonControlAttributeName( CCAFlags::Orientation ),
                    PROPERTY_ORIENTATION,
                    aOrientationMap,
                    ScrollBarOrientation::HORIZONTAL
                );
        #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeCommon = m_nIncludeCommon & ~CCAFlags::Orientation;
        #endif
            }

            if ( m_nIncludeCommon & CCAFlags::VisualEffect )
            {
                exportEnumPropertyAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace( CCAFlags::VisualEffect ),
                    OAttributeMetaData::getCommonControlAttributeName( CCAFlags::VisualEffect ),
                    PROPERTY_VISUAL_EFFECT,
                    aVisualEffectMap,
                    VisualEffect::LOOK3D
                );
            #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeCommon = m_nIncludeCommon & ~CCAFlags::VisualEffect;
            #endif
            }
        }

        // some properties which require a special handling

        // the target frame
        if (m_nIncludeCommon & CCAFlags::TargetFrame)
        {
            exportTargetFrameAttribute();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCAFlags::TargetFrame;
        #endif
        }

        // max text length
        if ( m_nIncludeCommon & CCAFlags::MaxLength )
        {
            // normally, the respective property would be "MaxTextLen"
            // However, if the model has a property "PersistenceMaxTextLength", then we prefer this

            // determine the name of the property to export
            OUString sTextLenPropertyName( PROPERTY_MAXTEXTLENGTH );
            if ( m_xPropertyInfo->hasPropertyByName( PROPERTY_PERSISTENCE_MAXTEXTLENGTH ) )
                sTextLenPropertyName = PROPERTY_PERSISTENCE_MAXTEXTLENGTH;

            // export it
            exportInt16PropertyAttribute(
                OAttributeMetaData::getCommonControlAttributeNamespace( CCAFlags::MaxLength ),
                OAttributeMetaData::getCommonControlAttributeName( CCAFlags::MaxLength ),
                sTextLenPropertyName,
                0
            );

            // in either way, both properties count as "exported"
            exportedProperty( PROPERTY_MAXTEXTLENGTH );
            exportedProperty( PROPERTY_PERSISTENCE_MAXTEXTLENGTH );

        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCAFlags::MaxLength;
        #endif
        }

        if (m_nIncludeCommon & CCAFlags::TargetLocation)
        {
            exportTargetLocationAttribute(false);
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCAFlags::TargetLocation;
        #endif
        }

        // OJ #99721#
        if (m_nIncludeCommon & CCAFlags::ImageData)
        {
            exportImageDataAttribute();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCAFlags::ImageData;
        #endif
        }

        // the for attribute
        // the target frame
        if (m_nIncludeCommon & CCAFlags::For)
        {
            if (!m_sReferringControls.isEmpty())
            {   // there is at least one control referring to the one we're handling currently
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::For),
                    OAttributeMetaData::getCommonControlAttributeName(CCAFlags::For),
                    m_sReferringControls);
            }
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCAFlags::For;
        #endif
        }

        if ((CCAFlags::CurrentValue | CCAFlags::Value) & m_nIncludeCommon)
        {
            const sal_Char* pCurrentValuePropertyName = nullptr;
            const sal_Char* pValuePropertyName = nullptr;

            // get the property names
            getValuePropertyNames(m_eType, m_nClassId, pCurrentValuePropertyName, pValuePropertyName);

            // add the attributes if necessary and possible
            if (pCurrentValuePropertyName && (CCAFlags::CurrentValue & m_nIncludeCommon))
            {
                static const sal_Char* pCurrentValueAttributeName = OAttributeMetaData::getCommonControlAttributeName(CCAFlags::CurrentValue);
                // don't export the current-value if this value originates from a data binding
                // #i26944#
                if ( controlHasActiveDataBinding() )
                    exportedProperty( OUString::createFromAscii( pCurrentValuePropertyName ) );
                else
                {
                    static const sal_uInt16 nCurrentValueAttributeNamespaceKey = OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::CurrentValue);
                    exportGenericPropertyAttribute(
                        nCurrentValueAttributeNamespaceKey,
                        pCurrentValueAttributeName,
                        pCurrentValuePropertyName
                    );
                }
            }

            if (pValuePropertyName && (CCAFlags::Value & m_nIncludeCommon))
            {
                static const sal_Char* pValueAttributeName = OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Value);
                static const sal_uInt16 nValueAttributeNamespaceKey = OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::Value);
                exportGenericPropertyAttribute(
                    nValueAttributeNamespaceKey,
                    pValueAttributeName,
                    pValuePropertyName);
            }

            OSL_ENSURE((nullptr == pValuePropertyName) == (CCAFlags::NONE == (CCAFlags::Value & m_nIncludeCommon)),
                "OControlExport::exportCommonControlAttributes: no property found for the value attribute!");
            OSL_ENSURE((nullptr == pCurrentValuePropertyName ) == (CCAFlags::NONE == (CCAFlags::CurrentValue & m_nIncludeCommon)),
                "OControlExport::exportCommonControlAttributes: no property found for the current-value attribute!");

        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCAFlags(CCAFlags::CurrentValue | CCAFlags::Value);
        #endif
        }

        OSL_ENSURE(CCAFlags::NONE == m_nIncludeCommon,
            "OControlExport::exportCommonControlAttributes: forgot some flags!");
            // in the dbg_util version, we should have removed every bit we handled from the mask, so it should
            // be 0 now ...
    }

    void OControlExport::exportDatabaseAttributes()
    {
#if OSL_DEBUG_LEVEL > 0
        DAFlags nIncludeDatabase = m_nIncludeDatabase;
#endif
        // the only string property: DataField
        if (DAFlags::DataField & m_nIncludeDatabase)
        {
            exportStringPropertyAttribute(
                OAttributeMetaData::getDatabaseAttributeNamespace(),
                OAttributeMetaData::getDatabaseAttributeName(DAFlags::DataField),
                PROPERTY_DATAFIELD);
            RESET_BIT( nIncludeDatabase, DAFlags::DataField );
        }

        // InputRequired
        if ( DAFlags::InputRequired & m_nIncludeDatabase )
        {
            exportBooleanPropertyAttribute(
                OAttributeMetaData::getDatabaseAttributeNamespace(),
                OAttributeMetaData::getDatabaseAttributeName( DAFlags::InputRequired ),
                PROPERTY_INPUT_REQUIRED,
                BoolAttrFlags::DefaultFalse | BoolAttrFlags::DefaultVoid
            );
            RESET_BIT( nIncludeDatabase, DAFlags::InputRequired );
        }

        // the only int16 property: BoundColumn
        if (DAFlags::BoundColumn & m_nIncludeDatabase)
        {
            exportInt16PropertyAttribute(
                OAttributeMetaData::getDatabaseAttributeNamespace(),
                OAttributeMetaData::getDatabaseAttributeName(DAFlags::BoundColumn),
                PROPERTY_BOUNDCOLUMN,
                0,
                true);
            RESET_BIT( nIncludeDatabase, DAFlags::BoundColumn );
        }

        // ConvertEmptyToNull
        if (DAFlags::ConvertEmpty & m_nIncludeDatabase)
        {
            exportBooleanPropertyAttribute(
                OAttributeMetaData::getDatabaseAttributeNamespace(),
                OAttributeMetaData::getDatabaseAttributeName(DAFlags::ConvertEmpty),
                PROPERTY_EMPTY_IS_NULL,
                BoolAttrFlags::DefaultFalse
                );
            RESET_BIT( nIncludeDatabase, DAFlags::ConvertEmpty );
        }

        // the only enum property: ListSourceType
        if (DAFlags::ListSource_TYPE & m_nIncludeDatabase)
        {
            exportEnumPropertyAttribute(
                OAttributeMetaData::getDatabaseAttributeNamespace(),
                OAttributeMetaData::getDatabaseAttributeName(DAFlags::ListSource_TYPE),
                PROPERTY_LISTSOURCETYPE,
                aListSourceTypeMap,
                ListSourceType_VALUELIST
                );
            RESET_BIT( nIncludeDatabase, DAFlags::ListSource_TYPE );
        }

        if (m_nIncludeDatabase & DAFlags::ListSource)
        {
            exportListSourceAsAttribute();
            RESET_BIT( nIncludeDatabase, DAFlags::ListSource );
        }

#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE(DAFlags::NONE == nIncludeDatabase,
            "OControlExport::exportDatabaseAttributes: forgot some flags!");
            // in the dbg_util version, we should have removed every bit we handled from the mask, so it should
            // be 0 now ...
#endif
    }

    void OControlExport::exportBindingAttributes()
    {
#if OSL_DEBUG_LEVEL > 0
        BAFlags nIncludeBinding = m_nIncludeBindings;
#endif

        if ( m_nIncludeBindings & BAFlags::LinkedCell )
        {
            exportCellBindingAttributes( bool(m_nIncludeBindings & BAFlags::ListLinkingType) );
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            nIncludeBinding = nIncludeBinding & ~BAFlags( BAFlags::LinkedCell | BAFlags::ListLinkingType );
        #endif
        }

        if ( m_nIncludeBindings & BAFlags::ListCellRange )
        {
            exportCellListSourceRange();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            nIncludeBinding = nIncludeBinding & ~BAFlags::ListCellRange;
        #endif
        }

        if ( m_nIncludeBindings & BAFlags::XFormsBind )
        {
            exportXFormsBindAttributes();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            nIncludeBinding = nIncludeBinding & ~BAFlags::XFormsBind;
        #endif
        }

        if ( m_nIncludeBindings & BAFlags::XFormsListBind )
        {
            exportXFormsListAttributes();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            nIncludeBinding = nIncludeBinding & ~BAFlags::XFormsListBind;
        #endif
        }

        if ( m_nIncludeBindings & BAFlags::XFormsSubmission )
        {
            exportXFormsSubmissionAttributes();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            nIncludeBinding = nIncludeBinding & ~BAFlags::XFormsSubmission;
        #endif
        }

        #if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( BAFlags::NONE == nIncludeBinding,
            "OControlExport::exportBindingAttributes: forgot some flags!");
            // in the debug version, we should have removed every bit we handled from the mask, so it should
            // be 0 now ...
        #endif
    }

    void OControlExport::exportSpecialAttributes()
    {
        sal_Int32 i=0;

        // the boolean properties
        {
            static const SCAFlags nBooleanPropertyAttributeIds[] =
            {   // attribute flags
                SCAFlags::Validation, SCAFlags::MultiLine, SCAFlags::AutoCompletion, SCAFlags::Multiple, SCAFlags::DefaultButton, SCAFlags::IsTristate,
                SCAFlags::Toggle, SCAFlags::FocusOnClick
            };
            static const char * pBooleanPropertyNames[] =
            {   // property names
                PROPERTY_STRICTFORMAT, PROPERTY_MULTILINE,
                PROPERTY_AUTOCOMPLETE,
                PROPERTY_MULTISELECTION,
                PROPERTY_DEFAULTBUTTON, PROPERTY_TRISTATE,
                PROPERTY_TOGGLE, PROPERTY_FOCUS_ON_CLICK
            };
            static const sal_Int32 nIdCount = SAL_N_ELEMENTS(nBooleanPropertyAttributeIds);
        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nNameCount = SAL_N_ELEMENTS(pBooleanPropertyNames);
            OSL_ENSURE((nIdCount == nNameCount),
                "OControlExport::exportSpecialAttributes: somebody tampered with the maps (1)!");
        #endif
            const SCAFlags* pAttributeId = nBooleanPropertyAttributeIds;
            for ( i = 0; i < nIdCount; ++i, ++pAttributeId )
            {
                if ( *pAttributeId & m_nIncludeSpecial)
                {
                    exportBooleanPropertyAttribute(
                        OAttributeMetaData::getSpecialAttributeNamespace( *pAttributeId ),
                        OAttributeMetaData::getSpecialAttributeName( *pAttributeId ),
                        OUString::createFromAscii(pBooleanPropertyNames[i]),
                        ( *pAttributeId == SCAFlags::FocusOnClick ) ? BoolAttrFlags::DefaultTrue : BoolAttrFlags::DefaultFalse
                    );
            #if OSL_DEBUG_LEVEL > 0
                    //  reset the bit for later checking
                    m_nIncludeSpecial = m_nIncludeSpecial & ~*pAttributeId;
            #endif
                }
            }
        }

        // the integer properties
        {
            static const SCAFlags nIntegerPropertyAttributeIds[] =
            {   // attribute flags
                SCAFlags::PageStepSize
            };
            static const char * pIntegerPropertyNames[] =
            {   // property names
                PROPERTY_BLOCK_INCREMENT
            };
            static const sal_Int32 nIntegerPropertyAttrDefaults[] =
            {   // attribute defaults (XML defaults, not runtime defaults!)
                10
            };

            static const sal_Int32 nIdCount = SAL_N_ELEMENTS( nIntegerPropertyAttributeIds );
        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nNameCount = SAL_N_ELEMENTS( pIntegerPropertyNames );
            OSL_ENSURE( ( nIdCount == nNameCount ),
                "OControlExport::exportSpecialAttributes: somebody tampered with the maps (2)!" );
            static const sal_Int32 nDefaultCount = SAL_N_ELEMENTS( nIntegerPropertyAttrDefaults );
            OSL_ENSURE( ( nIdCount == nDefaultCount ),
                "OControlExport::exportSpecialAttributes: somebody tampered with the maps (3)!" );
        #endif
            for ( i = 0; i < nIdCount; ++i )
                if ( nIntegerPropertyAttributeIds[i] & m_nIncludeSpecial )
                {
                    exportInt32PropertyAttribute(
                        OAttributeMetaData::getSpecialAttributeNamespace( nIntegerPropertyAttributeIds[i] ),
                        OAttributeMetaData::getSpecialAttributeName( nIntegerPropertyAttributeIds[i] ),
                        OUString::createFromAscii(pIntegerPropertyNames[i]),
                        nIntegerPropertyAttrDefaults[i]
                    );
            #if OSL_DEBUG_LEVEL > 0
                    //  reset the bit for later checking
                    m_nIncludeSpecial = m_nIncludeSpecial & ~nIntegerPropertyAttributeIds[i];
            #endif
                }

            if ( SCAFlags::StepSize & m_nIncludeSpecial )
            {
                OUString sPropertyName;
                if ( m_xPropertyInfo->hasPropertyByName( PROPERTY_LINE_INCREMENT ) )
                    sPropertyName = PROPERTY_LINE_INCREMENT;
                else if ( m_xPropertyInfo->hasPropertyByName( PROPERTY_SPIN_INCREMENT ) )
                    sPropertyName = PROPERTY_SPIN_INCREMENT;
                else
                    OSL_FAIL( "OControlExport::exportSpecialAttributes: not property which can be mapped to step-size attribute!" );

                if ( !sPropertyName.isEmpty() )
                    exportInt32PropertyAttribute(
                        OAttributeMetaData::getSpecialAttributeNamespace( SCAFlags::StepSize ),
                        OAttributeMetaData::getSpecialAttributeName( SCAFlags::StepSize ),
                        sPropertyName,
                        1
                    );

            #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCAFlags::StepSize;
            #endif
            }

        }

        // the enum properties
        {
            if (SCAFlags::State & m_nIncludeSpecial)
            {
                exportEnumPropertyAttribute(
                    OAttributeMetaData::getSpecialAttributeNamespace(SCAFlags::State),
                    OAttributeMetaData::getSpecialAttributeName(SCAFlags::State),
                    PROPERTY_DEFAULT_STATE,
                    aCheckStateMap,
                    TRISTATE_FALSE);
            #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCAFlags::State;
            #endif
            }

            if (SCAFlags::CurrentState & m_nIncludeSpecial)
            {
                exportEnumPropertyAttribute(
                    OAttributeMetaData::getSpecialAttributeNamespace(SCAFlags::CurrentState),
                    OAttributeMetaData::getSpecialAttributeName(SCAFlags::CurrentState),
                    PROPERTY_STATE,
                    aCheckStateMap,
                    TRISTATE_FALSE);
            #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCAFlags::CurrentState;
            #endif
            }
        }

        // some properties which require a special handling
        // the repeat delay
        {
            if ( m_nIncludeSpecial & SCAFlags::RepeatDelay )
            {
                DBG_CHECK_PROPERTY( PROPERTY_REPEAT_DELAY, sal_Int32 );

                sal_Int32 nRepeatDelay = 0;
                m_xProps->getPropertyValue( PROPERTY_REPEAT_DELAY ) >>= nRepeatDelay;
                tools::Time aTime( tools::Time::SYSTEM );
                aTime.MakeTimeFromMS( nRepeatDelay );
                util::Duration aDuration;
                aDuration.Hours   = aTime.GetHour();
                aDuration.Minutes = aTime.GetMin();
                aDuration.Seconds = aTime.GetSec();
                aDuration.NanoSeconds = (nRepeatDelay % 1000) * 1000000;

                OUStringBuffer buf;
                ::sax::Converter::convertDuration(buf, aDuration);
                AddAttribute(OAttributeMetaData::getSpecialAttributeNamespace( SCAFlags::RepeatDelay )
                            ,OAttributeMetaData::getSpecialAttributeName( SCAFlags::RepeatDelay )
                            ,buf.makeStringAndClear());

                exportedProperty( PROPERTY_REPEAT_DELAY );

            #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCAFlags::RepeatDelay;
            #endif
            }
        }

        // the EchoChar property needs special handling, cause it's a Int16, but must be stored as one-character-string
        {
            if (SCAFlags::EchoChar & m_nIncludeSpecial)
            {
                DBG_CHECK_PROPERTY( PROPERTY_ECHO_CHAR, sal_Int16 );
                sal_Int16 nValue(0);
                m_xProps->getPropertyValue(PROPERTY_ECHO_CHAR) >>= nValue;
                if (nValue)
                {
                    OUString sCharacter(reinterpret_cast<const sal_Unicode*>(&nValue), 1);
                    AddAttribute(
                        OAttributeMetaData::getSpecialAttributeNamespace(SCAFlags::EchoChar),
                        OAttributeMetaData::getSpecialAttributeName(SCAFlags::EchoChar),
                        sCharacter);
                }
                exportedProperty(PROPERTY_ECHO_CHAR);
            #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCAFlags::EchoChar;
            #endif
            }
        }

        // the string properties
        {
            static const SCAFlags nStringPropertyAttributeIds[] =
            {   // attribute flags
                SCAFlags::GroupName
            };
            static const OUStringLiteral pStringPropertyNames[] =
            {   // property names
                PROPERTY_GROUP_NAME
            };

            static const sal_Int32 nIdCount = SAL_N_ELEMENTS( nStringPropertyAttributeIds );
        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nNameCount = SAL_N_ELEMENTS( pStringPropertyNames );
            OSL_ENSURE( ( nIdCount == nNameCount ),
                "OControlExport::exportSpecialAttributes: somebody tampered with the maps (2)!" );
        #endif
            for ( i = 0; i < nIdCount; ++i )
                if ( nStringPropertyAttributeIds[i] & m_nIncludeSpecial )
                {
                    exportStringPropertyAttribute(
                        OAttributeMetaData::getSpecialAttributeNamespace( nStringPropertyAttributeIds[i] ),
                        OAttributeMetaData::getSpecialAttributeName( nStringPropertyAttributeIds[i] ),
                        pStringPropertyNames[i]
                    );
            #if OSL_DEBUG_LEVEL > 0
                    //  reset the bit for later checking
                    m_nIncludeSpecial = m_nIncludeSpecial & ~nStringPropertyAttributeIds[i];
            #endif
                }
        }

        if ((SCAFlags::MinValue | SCAFlags::MaxValue) & m_nIncludeSpecial)
        {
            // need to export the min value and the max value as attributes
            // It depends on the real type (FormComponentType) of the control, which properties hold these
            // values
            const sal_Char* pMinValuePropertyName = nullptr;
            const sal_Char* pMaxValuePropertyName = nullptr;
            getValueLimitPropertyNames(m_nClassId, pMinValuePropertyName, pMaxValuePropertyName);

            OSL_ENSURE((nullptr == pMinValuePropertyName) == (SCAFlags::NONE == (SCAFlags::MinValue & m_nIncludeSpecial)),
                "OControlExport::exportCommonControlAttributes: no property found for the min value attribute!");
            OSL_ENSURE((nullptr == pMaxValuePropertyName) == (SCAFlags::NONE == (SCAFlags::MaxValue & m_nIncludeSpecial)),
                "OControlExport::exportCommonControlAttributes: no property found for the max value attribute!");

            // add the two attributes
            static const sal_Char* pMinValueAttributeName = OAttributeMetaData::getSpecialAttributeName(SCAFlags::MinValue);
            static const sal_Char* pMaxValueAttributeName = OAttributeMetaData::getSpecialAttributeName(SCAFlags::MaxValue);
            static const sal_uInt16 nMinValueNamespaceKey = OAttributeMetaData::getSpecialAttributeNamespace(SCAFlags::MinValue);
            static const sal_uInt16 nMaxValueNamespaceKey = OAttributeMetaData::getSpecialAttributeNamespace(SCAFlags::MaxValue);

            if (pMinValuePropertyName && (SCAFlags::MinValue & m_nIncludeSpecial))
                exportGenericPropertyAttribute(
                    nMinValueNamespaceKey,
                    pMinValueAttributeName,
                    pMinValuePropertyName);

            if (pMaxValuePropertyName && (SCAFlags::MaxValue & m_nIncludeSpecial))
                exportGenericPropertyAttribute(
                    nMaxValueNamespaceKey,
                    pMaxValueAttributeName,
                    pMaxValuePropertyName);
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeSpecial = m_nIncludeSpecial & ~SCAFlags(SCAFlags::MinValue | SCAFlags::MaxValue);
        #endif
        }

        if ( SCAFlags::ImagePosition & m_nIncludeSpecial )
        {
            exportImagePositionAttributes();
            RESET_BIT( m_nIncludeSpecial, SCAFlags::ImagePosition );
        }

        OSL_ENSURE(SCAFlags::NONE == m_nIncludeSpecial,
            "OControlExport::exportSpecialAttributes: forgot some flags!");
            // in the dbg_util version, we should have removed every bit we handled from the mask, so it should
            // be 0 now ...
    }

    OUString OControlExport::getScalarListSourceValue() const
    {
        OUString sListSource;
        Any aListSource = m_xProps->getPropertyValue( PROPERTY_LISTSOURCE );
        if ( !( aListSource >>= sListSource ) )
        {
            Sequence< OUString > aListSourceSequence;
            aListSource >>= aListSourceSequence;
            if ( aListSourceSequence.getLength() )
                sListSource = aListSourceSequence[ 0 ];
        }
        return sListSource;
    }

    void OControlExport::exportListSourceAsAttribute()
    {
        // DAFlags::ListSource needs some special handling
        DBG_CHECK_PROPERTY_NO_TYPE( PROPERTY_LISTSOURCE );

        OUString sListSource = getScalarListSourceValue();
        if ( !sListSource.isEmpty() )
        {   // the ListSource property needs to be exported as attribute, and it is not empty
            AddAttribute(
                OAttributeMetaData::getDatabaseAttributeNamespace(),
                OAttributeMetaData::getDatabaseAttributeName(DAFlags::ListSource),
                sListSource);
        }

        exportedProperty( PROPERTY_LISTSOURCE );
    }

    void OControlExport::getSequenceInt16PropertyAsSet(const OUString& _rPropertyName, Int16Set& _rOut)
    {
        Sequence< sal_Int16 > aValueSequence;
        DBG_CHECK_PROPERTY(_rPropertyName, Sequence< sal_Int16 >);
        m_xProps->getPropertyValue(_rPropertyName) >>= aValueSequence;

        const sal_Int16* pValues = aValueSequence.getConstArray();
        for (sal_Int32 i=0; i<aValueSequence.getLength(); ++i, ++pValues)
            _rOut.insert(*pValues);
    }

    void OControlExport::exportListSourceAsElements()
    {
        // the string lists
        Sequence< OUString > aItems, aValues;
        DBG_CHECK_PROPERTY( PROPERTY_STRING_ITEM_LIST, Sequence< OUString > );
        m_xProps->getPropertyValue(PROPERTY_STRING_ITEM_LIST) >>= aItems;

        DBG_CHECK_PROPERTY( PROPERTY_LISTSOURCE, Sequence< OUString > );
        if ( DAFlags::NONE == ( m_nIncludeDatabase & DAFlags::ListSource ) )
            m_xProps->getPropertyValue(PROPERTY_LISTSOURCE) >>= aValues;
        // if we exported the list source as attribute, we do not repeat it as sub elements

        // the selection lists
        Int16Set aSelection, aDefaultSelection;
        getSequenceInt16PropertyAsSet(PROPERTY_SELECT_SEQ, aSelection);
        getSequenceInt16PropertyAsSet(PROPERTY_DEFAULT_SELECT_SEQ, aDefaultSelection);

        // the string for "true"
        OUString sTrue;
        OUStringBuffer sBuffer;
        ::sax::Converter::convertBool(sBuffer, true);
        sTrue = sBuffer.makeStringAndClear();

        // loop through both lists ('til the maximum of both lengths)
        const OUString* pItems = aItems.getConstArray();
        const OUString* pValues = aValues.getConstArray();

        sal_Int32 nItems = aItems.getLength();
        sal_Int32 nValues = aValues.getLength();

        sal_Int16 nMaxLen = static_cast<sal_Int16>(std::max(nItems, nValues));

        for (sal_Int16 i=0; i<nMaxLen; ++i )
        {
            m_rContext.getGlobalContext().ClearAttrList();
            if (i < nItems)
            {
                // there is an item at this position
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::Label),
                    OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Label),
                    *pItems);
                ++pItems;
            }
            if (i < nValues)
            {
                // there is an value at this position
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::Value),
                    OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Value),
                    *pValues);
                ++pValues;
            }

            Int16Set::iterator aSelectedPos = aSelection.find(i);
            if (aSelection.end() != aSelectedPos)
            {   // the item at this position is selected
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::CurrentSelected),
                    OAttributeMetaData::getCommonControlAttributeName(CCAFlags::CurrentSelected),
                    sTrue
                    );
                aSelection.erase(aSelectedPos);
            }

            Int16Set::iterator aDefaultSelectedPos = aDefaultSelection.find(i);
            if (aDefaultSelection.end() != aDefaultSelectedPos)
            {   // the item at this position is selected as default
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::Selected),
                    OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Selected),
                    sTrue
                    );
                aDefaultSelection.erase(aDefaultSelectedPos);
            }
            SvXMLElementExport aFormElement(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, "option", true, true);
        }

        // There may be more "selected" or "default-selected" items than there are in the lists in real,
        // so we need to store some additional "form:option" items which have no name and no label, but
        // one or both of the selected flags.
        // 21.05.2001 - 85388 - frank.schoenheit@germany.sun.com

        if ( !aSelection.empty() || !aDefaultSelection.empty() )
        {
            sal_Int16 nLastSelected = -1;
            if ( !aSelection.empty() )
                nLastSelected = *(--aSelection.end());

            sal_Int16 nLastDefaultSelected = -1;
            if ( !aDefaultSelection.empty() )
                nLastDefaultSelected = *(--aDefaultSelection.end());

            // the maximum element in both sets
            sal_Int16 nLastReferredEntry = std::max(nLastSelected, nLastDefaultSelected);
            OSL_ENSURE(nLastReferredEntry >= nMaxLen, "OControlExport::exportListSourceAsElements: inconsistence!");
                // if the maximum (selected or default selected) entry number is less than the maximum item count
                // in both lists, the entry number should have been removed from the set

            for (sal_Int16 i=nMaxLen; i<=nLastReferredEntry; ++i)
            {
                if (aSelection.end() != aSelection.find(i))
                {   // the (not existent) item at this position is selected
                    AddAttribute(
                        OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::CurrentSelected),
                        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::CurrentSelected),
                        sTrue
                        );
                }

                if (aDefaultSelection.end() != aDefaultSelection.find(i))
                {   // the (not existent) item at this position is selected as default
                    AddAttribute(
                        OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::Selected),
                        OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Selected),
                        sTrue
                        );
                }
                SvXMLElementExport aFormElement(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, "option", true, true);
            }
        }
    }

    void OControlExport::implStartElement(const sal_Char* _pName)
    {
        // before we let the base class start it's outer element, we add a wrapper element
        const sal_Char *pOuterElementName = getOuterXMLElementName();
        if (pOuterElementName)
            m_pOuterElement = std::make_unique<SvXMLElementExport>(
                                        m_rContext.getGlobalContext(),
                                        XML_NAMESPACE_FORM,
                                        pOuterElementName, true,
                                        true);

        // add the attributes for the inner element
        exportInnerAttributes();

        // and start the inner element
        OElementExport::implStartElement(_pName);
    }

    void OControlExport::implEndElement()
    {
        // end the inner element
        OElementExport::implEndElement();

        // end the outer element if it exists
        m_pOuterElement.reset();
    }

    const sal_Char* OControlExport::getOuterXMLElementName() const
    {
        return nullptr;
    }

    const sal_Char* OControlExport::getXMLElementName() const
    {
        return getElementName(m_eType);
    }

    void OControlExport::examine()
    {
        OSL_ENSURE( ( m_nIncludeCommon == CCAFlags::NONE ) && ( m_nIncludeSpecial == SCAFlags::NONE ) && ( m_nIncludeDatabase == DAFlags::NONE )
                 && ( m_nIncludeEvents == EAFlags::NONE ) && ( m_nIncludeBindings == BAFlags::NONE),
                 "OControlExport::examine: called me twice? Not initialized?" );

        // get the class id to decide which kind of element we need in the XML stream
        m_nClassId = FormComponentType::CONTROL;
        DBG_CHECK_PROPERTY( PROPERTY_CLASSID, sal_Int16 );
        m_xProps->getPropertyValue(PROPERTY_CLASSID) >>= m_nClassId;
        bool knownType = false;
        switch (m_nClassId)
        {
            case FormComponentType::DATEFIELD:
                m_eType = DATE;
                knownType = true;
                [[fallthrough]];
            case FormComponentType::TIMEFIELD:
                if ( !knownType )
                {
                    m_eType = TIME;
                    knownType = true;
                }
                m_nIncludeSpecial |= SCAFlags::Validation;
                [[fallthrough]];
            case FormComponentType::NUMERICFIELD:
            case FormComponentType::CURRENCYFIELD:
            case FormComponentType::PATTERNFIELD:
                if ( !knownType )
                {
                    m_eType = FORMATTED_TEXT;
                    knownType = true;
                }
                [[fallthrough]];
            case FormComponentType::TEXTFIELD:
            {   // it's some kind of edit. To know which type we need further investigation

                if ( !knownType )
                {
                    // check if it's a formatted field
                    if (m_xPropertyInfo->hasPropertyByName(PROPERTY_FORMATKEY))
                    {
                        m_eType = FORMATTED_TEXT;
                    }
                    else
                    {
                        // all other controls are represented by an ordinary edit control, but which XML control type
                        // it is depends on the current values of some properties

                        // if the EchoChar string is not empty, it is a password field
                        sal_Int16 nEchoChar = 0;
                        if (m_xPropertyInfo->hasPropertyByName(PROPERTY_ECHOCHAR))
                            // grid columns do not have this property ....
                            m_xProps->getPropertyValue(PROPERTY_ECHOCHAR) >>= nEchoChar;
                        if (nEchoChar)
                        {
                            m_eType = PASSWORD;
                            m_nIncludeSpecial |= SCAFlags::EchoChar;
                        }
                        else
                        {
                            // if the MultiLine property is sal_True, it is a TextArea
                            bool bMultiLine = false;
                            if (m_xPropertyInfo->hasPropertyByName(PROPERTY_MULTILINE))
                                // grid columns do not have this property ....
                                bMultiLine = ::cppu::any2bool(m_xProps->getPropertyValue(PROPERTY_MULTILINE));

                            if ( bMultiLine )
                                m_eType = TEXT_AREA;
                            else
                                // the only case left is represented by a Text element
                                m_eType = TEXT;
                        }
                    }
                }

                // attributes which are common to all the types:
                // common attributes
                m_nIncludeCommon =
                    CCAFlags::Name | CCAFlags::ServiceName | CCAFlags::Disabled |
                    CCAFlags::Printable | CCAFlags::TabIndex | CCAFlags::TabStop | CCAFlags::Title;

                if  (   ( m_nClassId != FormComponentType::DATEFIELD )
                    &&  ( m_nClassId != FormComponentType::TIMEFIELD )
                    )
                    // date and time field values are handled differently nowadays
                    m_nIncludeCommon |= CCAFlags::Value;

                // database attributes
                m_nIncludeDatabase = DAFlags::DataField | DAFlags::InputRequired;

                // event attributes
                m_nIncludeEvents = EAFlags::ControlEvents | EAFlags::OnChange | EAFlags::OnSelect;

                // only text and pattern fields have a ConvertEmptyToNull property
                if  (   ( m_nClassId == FormComponentType::TEXTFIELD )
                    ||  ( m_nClassId == FormComponentType::PATTERNFIELD )
                    )
                    m_nIncludeDatabase |= DAFlags::ConvertEmpty;

                // all controls but the file control fields have a readonly property
                if ( m_nClassId != FormComponentType::FILECONTROL )
                    m_nIncludeCommon |= CCAFlags::ReadOnly;

                // a text field has a max text len
                if ( m_nClassId == FormComponentType::TEXTFIELD )
                    m_nIncludeCommon |= CCAFlags::MaxLength;

                // max and min values and validation:
                if (FORMATTED_TEXT == m_eType)
                {   // in general all controls represented as formatted-text have these props
                    if  ( FormComponentType::PATTERNFIELD != m_nClassId )   // except the PatternField
                        m_nIncludeSpecial |= SCAFlags::MaxValue | SCAFlags::MinValue;

                    if (FormComponentType::TEXTFIELD != m_nClassId)
                        // and the FormattedField does not have a validation flag
                        m_nIncludeSpecial |= SCAFlags::Validation;
                }

                // if it's not a password field or rich text control, the CurrentValue needs to be stored, too
                if  (   ( PASSWORD != m_eType )
                    &&  ( DATE != m_eType )
                    &&  ( TIME != m_eType )
                    )
                {
                    m_nIncludeCommon |= CCAFlags::CurrentValue;
                }
            }
            break;

            case FormComponentType::FILECONTROL:
                m_eType = FILE;
                m_nIncludeCommon =
                    CCAFlags::Name | CCAFlags::ServiceName | CCAFlags::CurrentValue | CCAFlags::Disabled |
                    CCAFlags::Printable | CCAFlags::TabIndex | CCAFlags::TabStop | CCAFlags::Title |
                    CCAFlags::Value;
                m_nIncludeEvents = EAFlags::ControlEvents | EAFlags::OnChange | EAFlags::OnSelect;
                break;

            case FormComponentType::FIXEDTEXT:
                m_eType = FIXED_TEXT;
                m_nIncludeCommon =
                    CCAFlags::Name | CCAFlags::ServiceName | CCAFlags::Disabled | CCAFlags::Label |
                    CCAFlags::Printable | CCAFlags::Title | CCAFlags::For;
                m_nIncludeSpecial = SCAFlags::MultiLine;
                m_nIncludeEvents = EAFlags::ControlEvents;
                break;

            case FormComponentType::COMBOBOX:
                m_eType = COMBOBOX;
                m_nIncludeCommon =
                    CCAFlags::Name | CCAFlags::ServiceName | CCAFlags::CurrentValue |
                    CCAFlags::Disabled | CCAFlags::Dropdown | CCAFlags::MaxLength | CCAFlags::Printable | CCAFlags::ReadOnly | CCAFlags::Size |
                    CCAFlags::TabIndex | CCAFlags::TabStop | CCAFlags::Title | CCAFlags::Value;
                m_nIncludeSpecial = SCAFlags::AutoCompletion;
                m_nIncludeDatabase = DAFlags::ConvertEmpty | DAFlags::DataField | DAFlags::InputRequired | DAFlags::ListSource | DAFlags::ListSource_TYPE;
                m_nIncludeEvents = EAFlags::ControlEvents | EAFlags::OnChange | EAFlags::OnSelect;
                break;

            case FormComponentType::LISTBOX:
                m_eType = LISTBOX;
                m_nIncludeCommon =
                    CCAFlags::Name | CCAFlags::ServiceName | CCAFlags::Disabled | CCAFlags::Dropdown |
                    CCAFlags::Printable | CCAFlags::Size | CCAFlags::TabIndex | CCAFlags::TabStop | CCAFlags::Title;
                m_nIncludeSpecial = SCAFlags::Multiple;
                m_nIncludeDatabase = DAFlags::BoundColumn | DAFlags::DataField | DAFlags::InputRequired | DAFlags::ListSource_TYPE;
                m_nIncludeEvents = EAFlags::ControlEvents | EAFlags::OnChange | EAFlags::OnClick | EAFlags::OnDoubleClick;
                // check if we need to export the ListSource as attribute
                {
                    // for a list box, if the ListSourceType is VALUE_LIST, no ListSource is stored, but instead
                    // a sequence of pairs which is build from the StringItemList and the ValueList
                    ListSourceType eListSourceType = ListSourceType_VALUELIST;
                    bool bSuccess =
                        m_xProps->getPropertyValue(PROPERTY_LISTSOURCETYPE) >>= eListSourceType;
                    OSL_ENSURE(bSuccess, "OControlExport::examineControl: could not retrieve the ListSourceType!");
                    if (ListSourceType_VALUELIST != eListSourceType)
                    {
                        m_nIncludeDatabase |= DAFlags::ListSource;
                    }
                }

                break;

            case FormComponentType::COMMANDBUTTON:
                m_eType = BUTTON;
                m_nIncludeCommon |= CCAFlags::TabStop | CCAFlags::Label;
                m_nIncludeSpecial = SCAFlags::DefaultButton | SCAFlags::Toggle | SCAFlags::FocusOnClick | SCAFlags::ImagePosition | SCAFlags::RepeatDelay;
                [[fallthrough]];
            case FormComponentType::IMAGEBUTTON:
                if (BUTTON != m_eType)
                {
                    // not coming from the previous case
                    m_eType = IMAGE;
                }
                m_nIncludeCommon |=
                    CCAFlags::Name | CCAFlags::ServiceName | CCAFlags::ButtonType | CCAFlags::Disabled |
                    CCAFlags::ImageData | CCAFlags::Printable | CCAFlags::TabIndex | CCAFlags::TargetFrame |
                    CCAFlags::TargetLocation | CCAFlags::Title;
                m_nIncludeEvents = EAFlags::ControlEvents | EAFlags::OnClick  | EAFlags::OnDoubleClick;
                break;

            case FormComponentType::CHECKBOX:
                m_eType = CHECKBOX;
                m_nIncludeSpecial = SCAFlags::CurrentState | SCAFlags::IsTristate | SCAFlags::State;
                [[fallthrough]];
            case FormComponentType::RADIOBUTTON:
                m_nIncludeCommon =
                    CCAFlags::Name | CCAFlags::ServiceName | CCAFlags::Disabled | CCAFlags::Label | CCAFlags::Printable |
                    CCAFlags::TabIndex | CCAFlags::TabStop | CCAFlags::Title | CCAFlags::Value | CCAFlags::VisualEffect;
                if (CHECKBOX != m_eType)
                {   // not coming from the previous case
                    m_eType = RADIO;
                    m_nIncludeCommon |= CCAFlags::CurrentSelected | CCAFlags::Selected;
                }
                if ( m_xPropertyInfo->hasPropertyByName( PROPERTY_IMAGE_POSITION ) )
                    m_nIncludeSpecial |= SCAFlags::ImagePosition;
                if ( m_xPropertyInfo->hasPropertyByName( PROPERTY_GROUP_NAME ) )
                    m_nIncludeSpecial |= SCAFlags::GroupName;
                m_nIncludeDatabase = DAFlags::DataField | DAFlags::InputRequired;
                m_nIncludeEvents = EAFlags::ControlEvents | EAFlags::OnChange;
                break;

            case FormComponentType::GROUPBOX:
                m_eType = FRAME;
                m_nIncludeCommon =
                    CCAFlags::Name | CCAFlags::ServiceName | CCAFlags::Disabled | CCAFlags::Label |
                    CCAFlags::Printable | CCAFlags::Title | CCAFlags::For;
                m_nIncludeEvents = EAFlags::ControlEvents;
                break;

            case FormComponentType::IMAGECONTROL:
                m_eType = IMAGE_FRAME;
                m_nIncludeCommon =
                    CCAFlags::Name | CCAFlags::ServiceName | CCAFlags::Disabled | CCAFlags::ImageData |
                    CCAFlags::Printable | CCAFlags::ReadOnly | CCAFlags::Title;
                m_nIncludeDatabase = DAFlags::DataField | DAFlags::InputRequired;
                m_nIncludeEvents = EAFlags::ControlEvents;
                break;

            case FormComponentType::HIDDENCONTROL:
                m_eType = HIDDEN;
                m_nIncludeCommon =
                    CCAFlags::Name | CCAFlags::ServiceName | CCAFlags::Value;
                break;

            case FormComponentType::GRIDCONTROL:
                m_eType = GRID;
                m_nIncludeCommon =
                    CCAFlags::Name | CCAFlags::ServiceName | CCAFlags::Disabled | CCAFlags::Printable |
                    CCAFlags::TabIndex | CCAFlags::TabStop | CCAFlags::Title;
                m_nIncludeEvents = EAFlags::ControlEvents;
                break;

            case FormComponentType::SCROLLBAR:
            case FormComponentType::SPINBUTTON:
                m_eType = VALUERANGE;
                m_nIncludeCommon =
                    CCAFlags::Name | CCAFlags::ServiceName | CCAFlags::Disabled | CCAFlags::Printable |
                    CCAFlags::Title | CCAFlags::CurrentValue | CCAFlags::Value | CCAFlags::Orientation;
                m_nIncludeSpecial = SCAFlags::MaxValue | SCAFlags::StepSize | SCAFlags::MinValue | SCAFlags::RepeatDelay;

                if ( m_nClassId == FormComponentType::SCROLLBAR )
                    m_nIncludeSpecial |= SCAFlags::PageStepSize ;

                m_nIncludeEvents = EAFlags::ControlEvents;
                break;

            default:
                OSL_FAIL("OControlExport::examineControl: unknown control type (class id)!");
                [[fallthrough]];

            case FormComponentType::NAVIGATIONBAR:
                // TODO: should we have an own file format for this?
                // NO break

            case FormComponentType::CONTROL:
                m_eType = GENERIC_CONTROL;
                // unknown control type
                m_nIncludeCommon = CCAFlags::Name | CCAFlags::ServiceName;
                    // at least a name should be there, 'cause without a name the control could never have been
                    // inserted into its parent container
                    // In addition, the service name is absolutely necessary to create the control upon reading.
                m_nIncludeEvents = EAFlags::ControlEvents;
                    // we always should be able to export events - this is not control type dependent
                break;
        }

        // in general, all control types need to export the control id
        m_nIncludeCommon |= CCAFlags::ControlId;

        // is it a control bound to a calc cell?
        if ( FormCellBindingHelper::livesInSpreadsheetDocument( m_xProps ) )
        {
            FormCellBindingHelper aHelper( m_xProps, nullptr );
            {
                if ( FormCellBindingHelper::isCellBinding( aHelper.getCurrentBinding( ) ) )
                {
                    m_nIncludeBindings |= BAFlags::LinkedCell;
                    if ( m_nClassId == FormComponentType::LISTBOX )
                        m_nIncludeBindings |= BAFlags::ListLinkingType;
                }
            }

            // is it a list-like control which uses a calc cell range as list source?
            {
                if ( FormCellBindingHelper::isCellRangeListSource( aHelper.getCurrentListSource( ) ) )
                    m_nIncludeBindings |= BAFlags::ListCellRange;
            }
        }

        // is control bound to XForms?
        if( !getXFormsBindName( m_xProps ).isEmpty() )
        {
            m_nIncludeBindings |= BAFlags::XFormsBind;
        }

        // is (list-)control bound to XForms list?
        if( !getXFormsListBindName( m_xProps ).isEmpty() )
        {
            m_nIncludeBindings |= BAFlags::XFormsListBind;
        }

        // does the control have an XForms submission?
        if( !getXFormsSubmissionName( m_xProps ).isEmpty() )
        {
            m_nIncludeBindings |= BAFlags::XFormsSubmission;
        }
    }

    void OControlExport::exportCellBindingAttributes( bool _bIncludeListLinkageType )
    {
        try
        {
            FormCellBindingHelper aHelper( m_xProps, nullptr );
            Reference< XValueBinding > xBinding( aHelper.getCurrentBinding() );
            OSL_ENSURE( xBinding.is(), "OControlExport::exportCellBindingAttributes: invalid bindable or invalid binding!" );
            if ( xBinding.is() )
            {
                AddAttribute(
                    OAttributeMetaData::getBindingAttributeNamespace(),
                    OAttributeMetaData::getBindingAttributeName( BAFlags::LinkedCell ),
                    aHelper.getStringAddressFromCellBinding( xBinding )
                );

                if ( _bIncludeListLinkageType )
                {
                    sal_Int16 nLinkageType = FormCellBindingHelper::isCellIntegerBinding( xBinding ) ? 1 : 0;

                    OUStringBuffer sBuffer;
                    SvXMLUnitConverter::convertEnum(
                        sBuffer,
                        nLinkageType,
                        aListLinkageMap
                    );

                    AddAttribute(
                        OAttributeMetaData::getBindingAttributeNamespace(),
                        OAttributeMetaData::getBindingAttributeName( BAFlags::ListLinkingType ),
                        sBuffer.makeStringAndClear()
                    );
                }

            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("xmloff.forms");
            OSL_FAIL( "OControlExport::exportCellBindingAttributes: caught an exception!" );
        }
    }

    void OControlExport::exportXFormsBindAttributes()
    {
        OUString sBindName = getXFormsBindName( m_xProps );
        AddAttribute( XML_NAMESPACE_XFORMS, XML_BIND, sBindName );
    }
    void OControlExport::exportXFormsListAttributes()
    {
        OUString sBindName = getXFormsListBindName( m_xProps );
        AddAttribute( XML_NAMESPACE_FORM, XML_XFORMS_LIST_SOURCE, sBindName );
    }
    void OControlExport::exportXFormsSubmissionAttributes()
    {
        OUString sSubmission = getXFormsSubmissionName( m_xProps );
        AddAttribute( XML_NAMESPACE_FORM, XML_XFORMS_SUBMISSION, sSubmission );
    }
    void OControlExport::exportCellListSourceRange( )
    {
        try
        {
            Reference< XListEntrySink > xSink( m_xProps, UNO_QUERY );
            Reference< XListEntrySource > xSource;
            if ( xSink.is() )
                xSource.set(xSink->getListEntrySource(), css::uno::UNO_QUERY);
            OSL_ENSURE( xSource.is(), "OControlExport::exportCellListSourceRange: list source or sink!" );
            if ( xSource.is() )
            {
                FormCellBindingHelper aHelper( m_xProps, nullptr );

                AddAttribute(
                    OAttributeMetaData::getBindingAttributeNamespace(),
                    OAttributeMetaData::getBindingAttributeName( BAFlags::ListCellRange ),
                    aHelper.getStringAddressFromCellListSource( xSource )
                );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("xmloff.forms");
            OSL_FAIL( "OControlExport::exportCellListSourceRange: caught an exception!" );
        }
    }

    void OControlExport::exportImagePositionAttributes()
    {
        try
        {
            sal_Int16 nImagePosition = ImagePosition::Centered;
            OSL_VERIFY( m_xProps->getPropertyValue( PROPERTY_IMAGE_POSITION ) >>= nImagePosition );
            OSL_ENSURE( ( nImagePosition >= ImagePosition::LeftTop ) && ( nImagePosition <= ImagePosition::Centered ),
                "OControlExport::exportImagePositionAttributes: don't know this image position!" );

            if ( ( nImagePosition < ImagePosition::LeftTop ) || ( nImagePosition > ImagePosition::Centered ) )
                // this is important to prevent potential buffer overflows below, so don't optimize
                nImagePosition = ImagePosition::Centered;

            if ( nImagePosition == ImagePosition::Centered )
            {
                AddAttribute( XML_NAMESPACE_FORM, GetXMLToken( XML_IMAGE_POSITION ), GetXMLToken( XML_CENTER ) );
            }
            else
            {
                const XMLTokenEnum eXmlImagePositions[] =
                {
                    XML_START, XML_END, XML_TOP, XML_BOTTOM
                };
                const XMLTokenEnum eXmlImageAligns[] =
                {
                    XML_START, XML_CENTER, XML_END
                };

                XMLTokenEnum eXmlImagePosition = eXmlImagePositions[ nImagePosition / 3 ];
                XMLTokenEnum eXmlImageAlign    = eXmlImageAligns   [ nImagePosition % 3 ];

                AddAttribute( XML_NAMESPACE_FORM, GetXMLToken( XML_IMAGE_POSITION ), GetXMLToken( eXmlImagePosition ) );
                AddAttribute( XML_NAMESPACE_FORM, GetXMLToken( XML_IMAGE_ALIGN    ), GetXMLToken( eXmlImageAlign    ) );
            }

            exportedProperty( PROPERTY_IMAGE_POSITION );
            // some of the controls which have an ImagePosition also have an ImageAlign for compatibility
            // reasons. Since the ImageAlign values simply represent a sub set of the ImagePosition values,
            // we don't need to export ImageAlign anymore
            exportedProperty( PROPERTY_IMAGE_ALIGN );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("xmloff.forms");
        }
    }

    bool OControlExport::controlHasActiveDataBinding() const
    {
        try
        {
            // currently exchanging the data with a database column?
            OUString sBoundFieldPropertyName( "BoundField" );
            if ( m_xPropertyInfo.is() && m_xPropertyInfo->hasPropertyByName( sBoundFieldPropertyName ) )
            {
                Reference< XPropertySet > xBoundField;
                m_xProps->getPropertyValue( sBoundFieldPropertyName ) >>= xBoundField;
                if ( xBoundField.is() )
                    return true;
            }

            // currently exchanging data with an external binding?
            Reference< XBindableValue > xBindable( m_xProps, UNO_QUERY );
            if ( xBindable.is() && xBindable->getValueBinding().is() )
                return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("xmloff.forms");
            OSL_FAIL( "OColumnExport::controlHasActiveDataBinding: caught an exception!" );
        }

        return false;
    }

    bool OControlExport::controlHasUserSuppliedListEntries() const
    {
        try
        {
            // an external list source?
            Reference< XListEntrySink > xEntrySink( m_xProps, UNO_QUERY );
            if ( xEntrySink.is() && xEntrySink->getListEntrySource().is() )
                return false;

            if ( m_xPropertyInfo.is() && m_xPropertyInfo->hasPropertyByName( PROPERTY_LISTSOURCETYPE ) )
            {
                ListSourceType eListSourceType = ListSourceType_VALUELIST;
                OSL_VERIFY( m_xProps->getPropertyValue( PROPERTY_LISTSOURCETYPE ) >>= eListSourceType );
                if ( eListSourceType == ListSourceType_VALUELIST )
                    // for value lists, the list entries as entered by the user are used
                    return true;

                // for every other type, the list entries are filled with some data obtained
                // from a database - if and only if the ListSource property is not empty
                return getScalarListSourceValue().isEmpty();
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("xmloff.forms");
        }

        OSL_FAIL( "OControlExport::controlHasUserSuppliedListEntries: unreachable code!" );
            // this method should be called for list and combo boxes only
        return true;
    }

    //= OColumnExport
    OColumnExport::OColumnExport(IFormsExportContext& _rContext, const Reference< XPropertySet >& _rxControl, const OUString& _rControlId,
        const Sequence< ScriptEventDescriptor >& _rEvents)
        :OControlExport(_rContext, _rxControl, _rControlId, OUString(), _rEvents)
    {
    }

    OColumnExport::~OColumnExport()
    {
    }

    void OColumnExport::exportServiceNameAttribute()
    {
        // the attribute "service name" (which has a slightly different meaning for columns
        DBG_CHECK_PROPERTY( PROPERTY_COLUMNSERVICENAME, OUString );
        OUString sColumnServiceName;
        m_xProps->getPropertyValue(PROPERTY_COLUMNSERVICENAME) >>= sColumnServiceName;
        // the service name is a full qualified one (i.e. com.sun.star.form.TextField), but the
        // real service name for the column (for use with the XGridColumnFactory) is only the last
        // token of this complete name.
        sal_Int32 nLastSep = sColumnServiceName.lastIndexOf('.');
        OSL_ENSURE(-1 != nLastSep, "OColumnExport::startExportElement: invalid service name!");
        sColumnServiceName = sColumnServiceName.copy(nLastSep + 1);
        sColumnServiceName =
            m_rContext.getGlobalContext().GetNamespaceMap().GetQNameByKey(
                XML_NAMESPACE_OOO, sColumnServiceName );
        // add the attribute
        AddAttribute( OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::ServiceName)
                    , OAttributeMetaData::getCommonControlAttributeName(CCAFlags::ServiceName)
                    , sColumnServiceName);
        // flag the property as "handled"
        exportedProperty(PROPERTY_COLUMNSERVICENAME);

    }

    const sal_Char* OColumnExport::getOuterXMLElementName() const
    {
        return "column";
    }

    void OColumnExport::exportAttributes()
    {
        OControlExport::exportAttributes();

        // the attribute "label"
        exportStringPropertyAttribute(
            OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::Label),
            OAttributeMetaData::getCommonControlAttributeName(CCAFlags::Label),
            PROPERTY_LABEL);

        // the style attribute
        OUString sStyleName = m_rContext.getObjectStyleName( m_xProps );
        if ( !sStyleName.isEmpty() )
        {
            AddAttribute(
                OAttributeMetaData::getSpecialAttributeNamespace( SCAFlags::ColumnStyleName ),
                OAttributeMetaData::getSpecialAttributeName( SCAFlags::ColumnStyleName ),
                sStyleName
            );
        }
    }

    void OColumnExport::examine()
    {
        OControlExport::examine();

        // grid columns miss some properties of the controls they're representing
        m_nIncludeCommon &= ~CCAFlags(CCAFlags::For | CCAFlags::Printable | CCAFlags::TabIndex | CCAFlags::TabStop | CCAFlags::Label);
        m_nIncludeSpecial &= ~SCAFlags(SCAFlags::EchoChar | SCAFlags::AutoCompletion | SCAFlags::Multiple | SCAFlags::MultiLine);

        if (FormComponentType::DATEFIELD != m_nClassId)
            // except date fields, no column has the DropDown property
            m_nIncludeCommon &= ~CCAFlags::Dropdown;
    }

    //= OFormExport
    OFormExport::OFormExport(IFormsExportContext& _rContext, const Reference< XPropertySet >& _rxForm,
        const Sequence< ScriptEventDescriptor >& _rEvents)
        :OElementExport(_rContext, _rxForm, _rEvents)
        ,m_bCreateConnectionResourceElement(false)
    {
        OSL_ENSURE(m_xProps.is(), "OFormExport::OFormExport: invalid arguments!");
    }

    const sal_Char* OFormExport::getXMLElementName() const
    {
        return "form";
    }

    void OFormExport::exportSubTags()
    {
        if ( m_bCreateConnectionResourceElement && m_xProps.is() )
        {
            m_rContext.getGlobalContext().ClearAttrList();
            OUString sPropValue;
            m_xProps->getPropertyValue( PROPERTY_DATASOURCENAME ) >>= sPropValue; // if set it is a file url
            if ( sPropValue.isEmpty() )
                m_xProps->getPropertyValue( PROPERTY_URL ) >>= sPropValue;
            if ( !sPropValue.isEmpty() )
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::TargetLocation),
                    OAttributeMetaData::getCommonControlAttributeName(CCAFlags::TargetLocation),
                    m_rContext.getGlobalContext().GetRelativeReference(sPropValue));
            if ( m_rContext.getGlobalContext().GetAttrList().getLength() )
            {
                SvXMLElementExport aFormElement(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, xmloff::token::XML_CONNECTION_RESOURCE, true, true);
            }
        }

        // let the base class export the remaining properties and the events
        OElementExport::exportSubTags();
        // loop through all children
        Reference< XIndexAccess > xCollection(m_xProps, UNO_QUERY);
        OSL_ENSURE(xCollection.is(), "OFormLayerXMLExport::implExportForm: a form which is not an index access? Suspicious!");

        if (xCollection.is())
            m_rContext.exportCollectionElements(xCollection);
    }

    void OFormExport::exportAttributes()
    {
        sal_Int32 i=0;

        // the string properties
        {
            static const FormAttributes eStringPropertyIds[] =
            {
                faName, /*faAction,*/ faCommand, faFilter, faOrder
            };
            static const char * aStringPropertyNames[] =
            {
                PROPERTY_NAME, /*PROPERTY_TARGETURL,*/ PROPERTY_COMMAND, PROPERTY_FILTER, PROPERTY_ORDER
            };
            static const sal_Int32 nIdCount = SAL_N_ELEMENTS(eStringPropertyIds);
        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nNameCount = SAL_N_ELEMENTS(aStringPropertyNames);
            OSL_ENSURE((nIdCount == nNameCount),
                "OFormExport::exportAttributes: somebody tampered with the maps (1)!");
        #endif
            for (i=0; i<nIdCount; ++i)
                exportStringPropertyAttribute(
                    OAttributeMetaData::getFormAttributeNamespace(eStringPropertyIds[i]),
                    OAttributeMetaData::getFormAttributeName(eStringPropertyIds[i]),
                    OUString::createFromAscii(aStringPropertyNames[i]));

            // #i112082# xlink:type is added as part of exportTargetLocationAttribute

            // now export the data source name or databaselocation or connection resource
            OUString sPropValue;
            m_xProps->getPropertyValue( PROPERTY_DATASOURCENAME ) >>= sPropValue;
            m_bCreateConnectionResourceElement = sPropValue.isEmpty();
            if ( !m_bCreateConnectionResourceElement )
            {
                INetURLObject aURL(sPropValue);
                m_bCreateConnectionResourceElement = ( aURL.GetProtocol() == INetProtocol::File );
                if ( !m_bCreateConnectionResourceElement )
                    exportStringPropertyAttribute(
                        OAttributeMetaData::getFormAttributeNamespace(faDatasource),
                        OAttributeMetaData::getFormAttributeName(faDatasource),
                        PROPERTY_DATASOURCENAME);
            }
            else
                exportedProperty(PROPERTY_URL);
            if ( m_bCreateConnectionResourceElement )
                exportedProperty(PROPERTY_DATASOURCENAME);
        }

        // the boolean properties
        {
            static const FormAttributes eBooleanPropertyIds[] =
            {
                faAllowDeletes, faAllowInserts, faAllowUpdates, faApplyFilter, faEscapeProcessing, faIgnoreResult
            };
            static const char * pBooleanPropertyNames[] =
            {
                PROPERTY_ALLOWDELETES,
                PROPERTY_ALLOWINSERTS,
                PROPERTY_ALLOWUPDATES,
                PROPERTY_APPLYFILTER,
                PROPERTY_ESCAPEPROCESSING,
                PROPERTY_IGNORERESULT
            };
            static const BoolAttrFlags nBooleanPropertyAttrFlags[] =
            {
                BoolAttrFlags::DefaultTrue, BoolAttrFlags::DefaultTrue, BoolAttrFlags::DefaultTrue, BoolAttrFlags::DefaultFalse, BoolAttrFlags::DefaultTrue, BoolAttrFlags::DefaultFalse
            };
            static const sal_Int32 nIdCount = SAL_N_ELEMENTS(eBooleanPropertyIds);
        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nNameCount = SAL_N_ELEMENTS(pBooleanPropertyNames);
            static const sal_Int32 nFlagsCount = SAL_N_ELEMENTS(nBooleanPropertyAttrFlags);
            OSL_ENSURE((nIdCount == nNameCount) && (nNameCount == nFlagsCount),
                "OFormExport::exportAttributes: somebody tampered with the maps (2)!");
        #endif
            for (i=0; i<nIdCount; ++i)
                exportBooleanPropertyAttribute(
                    OAttributeMetaData::getFormAttributeNamespace(eBooleanPropertyIds[i]),
                    OAttributeMetaData::getFormAttributeName(eBooleanPropertyIds[i]),
                    OUString::createFromAscii(pBooleanPropertyNames[i]),
                    nBooleanPropertyAttrFlags[i]
                );
        }

        // the enum properties
        {
            exportEnumPropertyAttribute(
                OAttributeMetaData::getFormAttributeNamespace(faEnctype),
                OAttributeMetaData::getFormAttributeName(faEnctype),
                PROPERTY_SUBMIT_ENCODING,
                aSubmitEncodingMap,
                FormSubmitEncoding_URL,
                false
            );
            exportEnumPropertyAttribute(
                OAttributeMetaData::getFormAttributeNamespace(faMethod),
                OAttributeMetaData::getFormAttributeName(faMethod),
                PROPERTY_SUBMIT_METHOD,
                aSubmitMethodMap,
                FormSubmitMethod_GET,
                false
            );
            exportEnumPropertyAttribute(
                OAttributeMetaData::getFormAttributeNamespace(faCommandType),
                OAttributeMetaData::getFormAttributeName(faCommandType),
                PROPERTY_COMMAND_TYPE,
                aCommandTypeMap,
                CommandType::COMMAND,
                false
            );
            exportEnumPropertyAttribute(
                OAttributeMetaData::getFormAttributeNamespace(faNavigationMode),
                OAttributeMetaData::getFormAttributeName(faNavigationMode),
                PROPERTY_NAVIGATION,
                aNavigationTypeMap,
                NavigationBarMode_CURRENT,
                false
            );
            exportEnumPropertyAttribute(
                OAttributeMetaData::getFormAttributeNamespace(faTabbingCycle),
                OAttributeMetaData::getFormAttributeName(faTabbingCycle),
                PROPERTY_CYCLE,
                aTabulatorCycleMap,
                TabulatorCycle_RECORDS,
                true
            );
        }

        // the service name
        exportServiceNameAttribute();
        // the target frame
        exportTargetFrameAttribute();
        // the target URL
        exportTargetLocationAttribute(true);    // #i110911# add type attribute (for form, but not for control)

        // master fields
        exportStringSequenceAttribute(
            OAttributeMetaData::getFormAttributeNamespace(faMasterFields),
            OAttributeMetaData::getFormAttributeName(faMasterFields),
            PROPERTY_MASTERFIELDS);
        // detail fields
        exportStringSequenceAttribute(
            OAttributeMetaData::getFormAttributeNamespace(faDetailFiels),
            OAttributeMetaData::getFormAttributeName(faDetailFiels),
            PROPERTY_DETAILFIELDS);
    }
}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
