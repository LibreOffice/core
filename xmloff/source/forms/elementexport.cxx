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
#include "xmloff/xmlnmspe.hxx"
#include "eventexport.hxx"
#include "formenums.hxx"
#include "formcellbinding.hxx"
#include "xmloff/xformsexport.hxx"
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
#include <tools/wintypes.hxx>       // for check states
#include <xmloff/txtprmap.hxx>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <tools/urlobj.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/XMLEventExport.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <tools/time.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/extract.hxx>
#include <sal/macros.h>

#include <stdio.h>
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
        ,m_pXMLElement(NULL)
    {
    }

    OElementExport::~OElementExport()
    {
        implEndElement();
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
        m_pXMLElement = new SvXMLElementExport(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, _pName, sal_True, sal_True);
    }

    void OElementExport::implEndElement()
    {
        delete m_pXMLElement;
        m_pXMLElement = NULL;
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
        else if (sServiceName.equals(SERVICE_PERSISTENT_COMPONENT_##name)) \
            sToWriteServiceName = SERVICE_##name

        if (sal_False)
            ;
        CHECK_N_TRANSLATE( FORM );
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
        else if (sServiceName.equals(SERVICE_PERSISTENT_COMPONENT_EDIT))
        {
            // special handling for the edit field: we have two controls using this as persistence service name
            sToWriteServiceName = SERVICE_EDIT;
            Reference< XServiceInfo > xSI(m_xProps, UNO_QUERY);
            if (xSI.is() && xSI->supportsService(SERVICE_FORMATTEDFIELD))
                sToWriteServiceName = SERVICE_FORMATTEDFIELD;
        }
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
            OAttributeMetaData::getCommonControlAttributeNamespace(CCA_SERVICE_NAME),
            OAttributeMetaData::getCommonControlAttributeName(CCA_SERVICE_NAME),
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
        ,m_nIncludeCommon(0)
        ,m_nIncludeDatabase(0)
        ,m_nIncludeSpecial(0)
        ,m_nIncludeEvents(0)
        ,m_nIncludeBindings(0)
        ,m_pOuterElement(NULL)
    {
        OSL_ENSURE(m_xProps.is(), "OControlExport::OControlExport: invalid arguments!");
    }

    OControlExport::~OControlExport()
    {
        implEndElement();
    }

    void OControlExport::exportOuterAttributes()
    {
        // the control id
        if (CCA_NAME & m_nIncludeCommon)
        {
            exportStringPropertyAttribute(
                OAttributeMetaData::getCommonControlAttributeNamespace(CCA_NAME),
                OAttributeMetaData::getCommonControlAttributeName(CCA_NAME),
                PROPERTY_NAME
                );
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_NAME;
        #endif
        }

        // the service name
        if (m_nIncludeCommon & CCA_SERVICE_NAME)
        {
            exportServiceNameAttribute();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_SERVICE_NAME;
        #endif
        }
    }

    void OControlExport::exportInnerAttributes()
    {
        // the control id
        if (CCA_CONTROL_ID & m_nIncludeCommon)
        {
            OSL_ENSURE(!m_sControlId.isEmpty(), "OControlExport::exportInnerAttributes: have no control id for the control!");
            m_rContext.getGlobalContext().AddAttributeIdLegacy(
                XML_NAMESPACE_FORM, m_sControlId);
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_CONTROL_ID;
        #endif
        }

        // "new-style" properties ...
        exportGenericHandlerAttributes();

        // common control attributes
        exportCommonControlAttributes();

        // common database attributes
        exportDatabaseAttributes();

        // attributes related to external bindings
        exportBindingAtributes();

        // attributes special to the respective control type
        exportSpecialAttributes();

        // add the style references to the attributes
        flagStyleProperties();
    }

    void OControlExport::exportAttributes()
    {
        exportOuterAttributes();
    }

    void OControlExport::exportSubTags() throw (Exception)
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
            const XMLPropertyMapEntry* pCharAttributeProperties = XMLTextPropertySetMapper::getPropertyMapForType( TEXT_PROP_MAP_TEXT );
            while ( pCharAttributeProperties->msApiName )
            {
                exportedProperty( OUString::createFromAscii( pCharAttributeProperties->msApiName ) );
                ++pCharAttributeProperties;
            }

            const XMLPropertyMapEntry* pParaAttributeProperties = XMLTextPropertySetMapper::getPropertyMapForType( TEXT_PROP_MAP_SHAPE_PARA );
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
            exportedProperty( OUString( "CharCrossedOut" ) );
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
                            OAttributeMetaData::getCommonControlAttributeNamespace(CCA_LABEL),
                            OAttributeMetaData::getCommonControlAttributeName(CCA_LABEL),
                            *pListItems);
                        SvXMLElementExport aFormElement(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, "item", sal_True, sal_True);
                    }
                }
            }
            break;

            case TEXT_AREA:
            {
                // if we act as rich text control, we need to export some text:p elements
                if ( xControlText.is() )
                {
                    sal_Bool bActingAsRichText = sal_False;
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
        for (   const Property* prop = aProperties.getConstArray();
                prop != aProperties.getConstArray() + aProperties.getLength();
                ++prop
            )
        {
            try
            {
                // see if this property can already be handled with an IPropertyHandler (which, on the long
                // term, should be the case for most, if not all, properties)
                const PropertyDescription* propDescription = metadata::getPropertyDescription( prop->Name );
                if ( propDescription == NULL )
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
                    if ( !shouldExportProperty( prop->Name ) )
                        // TODO: in the future, we surely need a more sophisticated approach to this, involving the property
                        // handler, or the property description
                    {
                        exportedProperty( prop->Name );
                        continue;
                    }

                    const Any propValue = m_xProps->getPropertyValue( prop->Name );
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
                    for (   PropertyDescriptionList::iterator desc = descriptions.begin();
                            desc != descriptions.end();
                            ++desc
                        )
                    {
                        // TODO: XMultiPropertySet?
                        const Any propValue = m_xProps->getPropertyValue( (*desc)->propertyName );
                        aValues[ (*desc)->propertyId ] = propValue;
                    }

                    // let the handler translate into an XML attribute value
                    attributeValue = handler->getAttributeValue( aValues );
                }

                AddAttribute(
                    propDescription->attribute.namespacePrefix,
                    token::GetXMLToken( propDescription->attribute.attributeToken ),
                    attributeValue
                );

                exportedProperty( prop->Name );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
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
            static const sal_Int32 nStringPropertyAttributeIds[] =
            {
                CCA_LABEL, CCA_TITLE
            };
            // the names of all properties which are expected to be of type string
            static const OUString aStringPropertyNames[] =
            {
                OUString(PROPERTY_LABEL), OUString(PROPERTY_TITLE)
            };
            OSL_ENSURE( sizeof(aStringPropertyNames)/sizeof(aStringPropertyNames[0]) ==
                        sizeof(nStringPropertyAttributeIds)/sizeof(nStringPropertyAttributeIds[0]),
                        "OControlExport::exportCommonControlAttributes: somebody tampered with the maps (1)!");

            for (i=0; i<sizeof(nStringPropertyAttributeIds)/sizeof(nStringPropertyAttributeIds[0]); ++i)
                if (nStringPropertyAttributeIds[i] & m_nIncludeCommon)
                {
                    exportStringPropertyAttribute(
                        OAttributeMetaData::getCommonControlAttributeNamespace(nStringPropertyAttributeIds[i]),
                        OAttributeMetaData::getCommonControlAttributeName(nStringPropertyAttributeIds[i]),
                        aStringPropertyNames[i]
                        );
                #if OSL_DEBUG_LEVEL > 0
                    //  reset the bit for later checking
                    m_nIncludeCommon = m_nIncludeCommon & ~nStringPropertyAttributeIds[i];
                #endif
                }
        }

        // some boolean properties
        {
            static const sal_Int32 nBooleanPropertyAttributeIds[] =
            {   // attribute flags
                CCA_CURRENT_SELECTED, CCA_DISABLED, CCA_DROPDOWN, CCA_PRINTABLE, CCA_READONLY, CCA_SELECTED, CCA_TAB_STOP, CCA_ENABLEVISIBLE
            };
            static const OUString pBooleanPropertyNames[] =
            {   // property names
                OUString(PROPERTY_STATE), OUString(PROPERTY_ENABLED),
                OUString(PROPERTY_DROPDOWN), OUString(PROPERTY_PRINTABLE),
                OUString(PROPERTY_READONLY), OUString(PROPERTY_DEFAULT_STATE),
                OUString(PROPERTY_TABSTOP), OUString(PROPERTY_ENABLEVISIBLE)
            };
            static const sal_Bool nBooleanPropertyAttrFlags[] =
            {   // attribute defaults
                BOOLATTR_DEFAULT_FALSE, BOOLATTR_DEFAULT_FALSE | BOOLATTR_INVERSE_SEMANTICS, BOOLATTR_DEFAULT_FALSE, BOOLATTR_DEFAULT_TRUE, BOOLATTR_DEFAULT_FALSE, BOOLATTR_DEFAULT_FALSE, BOOLATTR_DEFAULT_VOID, BOOLATTR_DEFAULT_FALSE
            };
        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nIdCount = sizeof(nBooleanPropertyAttributeIds) / sizeof(nBooleanPropertyAttributeIds[0]);
            static const sal_Int32 nNameCount = sizeof(pBooleanPropertyNames) / sizeof(pBooleanPropertyNames[0]);
            static const sal_Int32 nFlagsCount = sizeof(nBooleanPropertyAttrFlags) / sizeof(nBooleanPropertyAttrFlags[0]);
            OSL_ENSURE((nIdCount == nNameCount) && (nNameCount == nFlagsCount),
                "OControlExport::exportCommonControlAttributes: somebody tampered with the maps (2)!");
        #endif
            for (i=0; i<sizeof(nBooleanPropertyAttributeIds)/sizeof(nBooleanPropertyAttributeIds[0]); ++i)
                if (nBooleanPropertyAttributeIds[i] & m_nIncludeCommon)
                {
                    exportBooleanPropertyAttribute(
                        OAttributeMetaData::getCommonControlAttributeNamespace(nBooleanPropertyAttributeIds[i]),
                        OAttributeMetaData::getCommonControlAttributeName(nBooleanPropertyAttributeIds[i]),
                        pBooleanPropertyNames[i],
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
            static sal_Int32 nIntegerPropertyAttributeIds[] =
            {   // attribute flags
                CCA_SIZE, CCA_TAB_INDEX
            };
            static const OUString pIntegerPropertyNames[] =
            {   // property names
                OUString(PROPERTY_LINECOUNT), OUString(PROPERTY_TABINDEX)
            };
            static const sal_Int16 nIntegerPropertyAttrDefaults[] =
            {   // attribute defaults
                5, 0
            };

            if ( m_nIncludeCommon & CCA_MAX_LENGTH )
                exportedProperty(PROPERTY_MAXTEXTLENGTH);

        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nIdCount = sizeof(nIntegerPropertyAttributeIds) / sizeof(nIntegerPropertyAttributeIds[0]);
            static const sal_Int32 nNameCount = sizeof(pIntegerPropertyNames) / sizeof(pIntegerPropertyNames[0]);
            static const sal_Int32 nDefaultCount = sizeof(nIntegerPropertyAttrDefaults) / sizeof(nIntegerPropertyAttrDefaults[0]);
            OSL_ENSURE((nIdCount == nNameCount) && (nNameCount == nDefaultCount),
                "OControlExport::exportCommonControlAttributes: somebody tampered with the maps (3)!");
        #endif
            for (i=0; i<sizeof(nIntegerPropertyAttributeIds)/sizeof(nIntegerPropertyAttributeIds[0]); ++i)
                if (nIntegerPropertyAttributeIds[i] & m_nIncludeCommon)
                {
                    exportInt16PropertyAttribute(
                        OAttributeMetaData::getCommonControlAttributeNamespace(nIntegerPropertyAttributeIds[i]),
                        OAttributeMetaData::getCommonControlAttributeName(nIntegerPropertyAttributeIds[i]),
                        pIntegerPropertyNames[i],
                        nIntegerPropertyAttrDefaults[i]);
        #if OSL_DEBUG_LEVEL > 0
                    //  reset the bit for later checking
                    m_nIncludeCommon = m_nIncludeCommon & ~nIntegerPropertyAttributeIds[i];
        #endif
                }

        }

        // some enum properties
        {
            if (m_nIncludeCommon & CCA_BUTTON_TYPE)
            {
                exportEnumPropertyAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCA_BUTTON_TYPE),
                    OAttributeMetaData::getCommonControlAttributeName(CCA_BUTTON_TYPE),
                    PROPERTY_BUTTONTYPE,
                    OEnumMapper::getEnumMap(OEnumMapper::epButtonType),
                    FormButtonType_PUSH);
        #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeCommon = m_nIncludeCommon & ~CCA_BUTTON_TYPE;
        #endif
            }
            if ( m_nIncludeCommon & CCA_ORIENTATION )
            {
                exportEnumPropertyAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace( CCA_ORIENTATION ),
                    OAttributeMetaData::getCommonControlAttributeName( CCA_ORIENTATION ),
                    PROPERTY_ORIENTATION,
                    OEnumMapper::getEnumMap( OEnumMapper::epOrientation ),
                    ScrollBarOrientation::HORIZONTAL
                );
        #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeCommon = m_nIncludeCommon & ~CCA_ORIENTATION;
        #endif
            }

            if ( m_nIncludeCommon & CCA_VISUAL_EFFECT )
            {
                exportEnumPropertyAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace( CCA_VISUAL_EFFECT ),
                    OAttributeMetaData::getCommonControlAttributeName( CCA_VISUAL_EFFECT ),
                    PROPERTY_VISUAL_EFFECT,
                    OEnumMapper::getEnumMap( OEnumMapper::epVisualEffect ),
                    VisualEffect::LOOK3D
                );
            #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeCommon = m_nIncludeCommon & ~CCA_VISUAL_EFFECT;
            #endif
            }
        }

        // some properties which require a special handling

        // the target frame
        if (m_nIncludeCommon & CCA_TARGET_FRAME)
        {
            exportTargetFrameAttribute();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_TARGET_FRAME;
        #endif
        }

        // max text length
        if ( m_nIncludeCommon & CCA_MAX_LENGTH )
        {
            // normally, the respective property would be "MaxTextLen"
            // However, if the model has a property "PersistenceMaxTextLength", then we prefer this

            // determine the name of the property to export
            OUString sTextLenPropertyName( PROPERTY_MAXTEXTLENGTH );
            if ( m_xPropertyInfo->hasPropertyByName( PROPERTY_PERSISTENCE_MAXTEXTLENGTH ) )
                sTextLenPropertyName = PROPERTY_PERSISTENCE_MAXTEXTLENGTH;

            // export it
            exportInt16PropertyAttribute(
                OAttributeMetaData::getCommonControlAttributeNamespace( CCA_MAX_LENGTH ),
                OAttributeMetaData::getCommonControlAttributeName( CCA_MAX_LENGTH ),
                sTextLenPropertyName,
                0
            );

            // in either way, both properties count as "exported"
            exportedProperty( PROPERTY_MAXTEXTLENGTH );
            exportedProperty( PROPERTY_PERSISTENCE_MAXTEXTLENGTH );

        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_MAX_LENGTH;
        #endif
        }

        if (m_nIncludeCommon & CCA_TARGET_LOCATION)
        {
            exportTargetLocationAttribute(false);
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_TARGET_LOCATION;
        #endif
        }

        // OJ #99721#
        if (m_nIncludeCommon & CCA_IMAGE_DATA)
        {
            exportImageDataAttribute();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_IMAGE_DATA;
        #endif
        }

        // the for attribute
        // the target frame
        if (m_nIncludeCommon & CCA_FOR)
        {
            if (!m_sReferringControls.isEmpty())
            {   // there is at least one control referring to the one we're handling currently
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCA_FOR),
                    OAttributeMetaData::getCommonControlAttributeName(CCA_FOR),
                    m_sReferringControls);
            }
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_FOR;
        #endif
        }

        if ((CCA_CURRENT_VALUE | CCA_VALUE) & m_nIncludeCommon)
        {
            const sal_Char* pCurrentValuePropertyName = NULL;
            const sal_Char* pValuePropertyName = NULL;

            // get the property names
            getValuePropertyNames(m_eType, m_nClassId, pCurrentValuePropertyName, pValuePropertyName);

            static const sal_Char* pCurrentValueAttributeName = OAttributeMetaData::getCommonControlAttributeName(CCA_CURRENT_VALUE);
            static const sal_Char* pValueAttributeName = OAttributeMetaData::getCommonControlAttributeName(CCA_VALUE);
            static const sal_uInt16 nCurrentValueAttributeNamespaceKey = OAttributeMetaData::getCommonControlAttributeNamespace(CCA_CURRENT_VALUE);
            static const sal_uInt16 nValueAttributeNamespaceKey = OAttributeMetaData::getCommonControlAttributeNamespace(CCA_VALUE);

            // add the atrtributes if necessary and possible
            if (pCurrentValuePropertyName && (CCA_CURRENT_VALUE & m_nIncludeCommon))
            {
                // don't export the current-value if this value originates from a data binding
                // #i26944#
                if ( controlHasActiveDataBinding() )
                    exportedProperty( OUString::createFromAscii( pCurrentValuePropertyName ) );
                else
                    exportGenericPropertyAttribute(
                        nCurrentValueAttributeNamespaceKey,
                        pCurrentValueAttributeName,
                        pCurrentValuePropertyName
                    );
            }

            if (pValuePropertyName && (CCA_VALUE & m_nIncludeCommon))
                exportGenericPropertyAttribute(
                    nValueAttributeNamespaceKey,
                    pValueAttributeName,
                    pValuePropertyName);

            OSL_ENSURE((NULL == pValuePropertyName) == (0 == (CCA_VALUE & m_nIncludeCommon)),
                "OControlExport::exportCommonControlAttributes: no property found for the value attribute!");
            OSL_ENSURE((NULL == pCurrentValuePropertyName ) == (0 == (CCA_CURRENT_VALUE & m_nIncludeCommon)),
                "OControlExport::exportCommonControlAttributes: no property found for the current-value attribute!");

        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeCommon = m_nIncludeCommon & ~(CCA_CURRENT_VALUE | CCA_VALUE);
        #endif
        }

        OSL_ENSURE(0 == m_nIncludeCommon,
            "OControlExport::exportCommonControlAttributes: forgot some flags!");
            // in the dbg_util version, we should have removed every bit we handled from the mask, so it should
            // be 0 now ...
    }

    void OControlExport::exportDatabaseAttributes()
    {
#if OSL_DEBUG_LEVEL > 0
        sal_Int32 nIncludeDatabase = m_nIncludeDatabase;
#endif
        // the only string property: DataField
        if (DA_DATA_FIELD & m_nIncludeDatabase)
        {
            exportStringPropertyAttribute(
                OAttributeMetaData::getDatabaseAttributeNamespace(DA_DATA_FIELD),
                OAttributeMetaData::getDatabaseAttributeName(DA_DATA_FIELD),
                PROPERTY_DATAFIELD);
            RESET_BIT( nIncludeDatabase, DA_DATA_FIELD );
        }

        // InputRequired
        if ( DA_INPUT_REQUIRED & m_nIncludeDatabase )
        {
            exportBooleanPropertyAttribute(
                OAttributeMetaData::getDatabaseAttributeNamespace( DA_INPUT_REQUIRED ),
                OAttributeMetaData::getDatabaseAttributeName( DA_INPUT_REQUIRED ),
                PROPERTY_INPUT_REQUIRED,
                BOOLATTR_DEFAULT_TRUE
            );
            RESET_BIT( nIncludeDatabase, DA_INPUT_REQUIRED );
        }

        // the only int16 property: BoundColumn
        if (DA_BOUND_COLUMN & m_nIncludeDatabase)
        {
            exportInt16PropertyAttribute(
                OAttributeMetaData::getDatabaseAttributeNamespace(DA_BOUND_COLUMN),
                OAttributeMetaData::getDatabaseAttributeName(DA_BOUND_COLUMN),
                PROPERTY_BOUNDCOLUMN,
                0,
                true);
            RESET_BIT( nIncludeDatabase, DA_BOUND_COLUMN );
        }

        // ConvertEmptyToNull
        if (DA_CONVERT_EMPTY & m_nIncludeDatabase)
        {
            exportBooleanPropertyAttribute(
                OAttributeMetaData::getDatabaseAttributeNamespace(DA_CONVERT_EMPTY),
                OAttributeMetaData::getDatabaseAttributeName(DA_CONVERT_EMPTY),
                PROPERTY_EMPTY_IS_NULL,
                BOOLATTR_DEFAULT_FALSE
                );
            RESET_BIT( nIncludeDatabase, DA_CONVERT_EMPTY );
        }

        // the only enum property: ListSourceType
        if (DA_LIST_SOURCE_TYPE & m_nIncludeDatabase)
        {
            exportEnumPropertyAttribute(
                OAttributeMetaData::getDatabaseAttributeNamespace(DA_LIST_SOURCE_TYPE),
                OAttributeMetaData::getDatabaseAttributeName(DA_LIST_SOURCE_TYPE),
                PROPERTY_LISTSOURCETYPE,
                OEnumMapper::getEnumMap(OEnumMapper::epListSourceType),
                ListSourceType_VALUELIST
                );
            RESET_BIT( nIncludeDatabase, DA_LIST_SOURCE_TYPE );
        }

        if (m_nIncludeDatabase & DA_LIST_SOURCE)
        {
            exportListSourceAsAttribute();
            RESET_BIT( nIncludeDatabase, DA_LIST_SOURCE );
        }

#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE(0 == nIncludeDatabase,
            "OControlExport::exportDatabaseAttributes: forgot some flags!");
            // in the dbg_util version, we should have removed every bit we handled from the mask, so it should
            // be 0 now ...
#endif
    }

    void OControlExport::exportBindingAtributes()
    {
#if OSL_DEBUG_LEVEL > 0
        sal_Int32 nIncludeBinding = m_nIncludeBindings;
#endif

        if ( m_nIncludeBindings & BA_LINKED_CELL )
        {
            exportCellBindingAttributes( ( m_nIncludeBindings & BA_LIST_LINKING_TYPE ) != 0 );
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            nIncludeBinding = nIncludeBinding & ~( BA_LINKED_CELL | BA_LIST_LINKING_TYPE );
        #endif
        }

        if ( m_nIncludeBindings & BA_LIST_CELL_RANGE )
        {
            exportCellListSourceRange();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            nIncludeBinding = nIncludeBinding & ~BA_LIST_CELL_RANGE;
        #endif
        }

        if ( m_nIncludeBindings & BA_XFORMS_BIND )
        {
            exportXFormsBindAttributes();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            nIncludeBinding = nIncludeBinding & ~BA_XFORMS_BIND;
        #endif
        }

        if ( m_nIncludeBindings & BA_XFORMS_LISTBIND )
        {
            exportXFormsListAttributes();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            nIncludeBinding = nIncludeBinding & ~BA_XFORMS_LISTBIND;
        #endif
        }

        if ( m_nIncludeBindings & BA_XFORMS_SUBMISSION )
        {
            exportXFormsSubmissionAttributes();
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            nIncludeBinding = nIncludeBinding & ~BA_XFORMS_SUBMISSION;
        #endif
        }

        OSL_ENSURE( 0 == nIncludeBinding,
            "OControlExport::exportBindingAtributes: forgot some flags!");
            // in the debug version, we should have removed every bit we handled from the mask, so it should
            // be 0 now ...
    }

    void OControlExport::exportSpecialAttributes()
    {
        sal_Int32 i=0;

        // the boolean properties
        {
            static const sal_Int32 nBooleanPropertyAttributeIds[] =
            {   // attribute flags
                SCA_VALIDATION, SCA_MULTI_LINE, SCA_AUTOMATIC_COMPLETION, SCA_MULTIPLE, SCA_DEFAULT_BUTTON, SCA_IS_TRISTATE,
                SCA_TOGGLE, SCA_FOCUS_ON_CLICK
            };
            static const OUString pBooleanPropertyNames[] =
            {   // property names
                OUString(PROPERTY_STRICTFORMAT), OUString(PROPERTY_MULTILINE),
                OUString(PROPERTY_AUTOCOMPLETE),
                OUString(PROPERTY_MULTISELECTION),
                OUString(PROPERTY_DEFAULTBUTTON), OUString(PROPERTY_TRISTATE),
                OUString(PROPERTY_TOGGLE), OUString(PROPERTY_FOCUS_ON_CLICK)
            };
            static const sal_Int32 nIdCount = sizeof(nBooleanPropertyAttributeIds) / sizeof(nBooleanPropertyAttributeIds[0]);
        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nNameCount = sizeof(pBooleanPropertyNames) / sizeof(pBooleanPropertyNames[0]);
            OSL_ENSURE((nIdCount == nNameCount),
                "OControlExport::exportSpecialAttributes: somebody tampered with the maps (1)!");
        #endif
            const sal_Int32* pAttributeId = nBooleanPropertyAttributeIds;
            for ( i = 0; i < nIdCount; ++i, ++pAttributeId )
            {
                if ( *pAttributeId & m_nIncludeSpecial)
                {
                    exportBooleanPropertyAttribute(
                        OAttributeMetaData::getSpecialAttributeNamespace( *pAttributeId ),
                        OAttributeMetaData::getSpecialAttributeName( *pAttributeId ),
                        pBooleanPropertyNames[i],
                        ( *pAttributeId == SCA_FOCUS_ON_CLICK ) ? BOOLATTR_DEFAULT_TRUE : BOOLATTR_DEFAULT_FALSE
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
            static sal_Int32 nIntegerPropertyAttributeIds[] =
            {   // attribute flags
                SCA_PAGE_STEP_SIZE
            };
            static const OUString pIntegerPropertyNames[] =
            {   // property names
                OUString(PROPERTY_BLOCK_INCREMENT)
            };
            static const sal_Int32 nIntegerPropertyAttrDefaults[] =
            {   // attribute defaults (XML defaults, not runtime defaults!)
                10
            };

            static const sal_Int32 nIdCount = sizeof( nIntegerPropertyAttributeIds ) / sizeof( nIntegerPropertyAttributeIds[0] );
        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nNameCount = sizeof( pIntegerPropertyNames ) / sizeof( pIntegerPropertyNames[0] );
            OSL_ENSURE( ( nIdCount == nNameCount ),
                "OControlExport::exportSpecialAttributes: somebody tampered with the maps (2)!" );
            static const sal_Int32 nDefaultCount = sizeof( nIntegerPropertyAttrDefaults ) / sizeof( nIntegerPropertyAttrDefaults[0] );
            OSL_ENSURE( ( nIdCount == nDefaultCount ),
                "OControlExport::exportSpecialAttributes: somebody tampered with the maps (3)!" );
        #endif
            for ( i = 0; i < nIdCount; ++i )
                if ( nIntegerPropertyAttributeIds[i] & m_nIncludeSpecial )
                {
                    exportInt32PropertyAttribute(
                        OAttributeMetaData::getSpecialAttributeNamespace( nIntegerPropertyAttributeIds[i] ),
                        OAttributeMetaData::getSpecialAttributeName( nIntegerPropertyAttributeIds[i] ),
                        pIntegerPropertyNames[i],
                        nIntegerPropertyAttrDefaults[i]
                    );
            #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeSpecial = m_nIncludeSpecial & ~nIntegerPropertyAttributeIds[i];
            #endif
                }

            if ( SCA_STEP_SIZE & m_nIncludeSpecial )
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
                        OAttributeMetaData::getSpecialAttributeNamespace( SCA_STEP_SIZE ),
                        OAttributeMetaData::getSpecialAttributeName( SCA_STEP_SIZE ),
                        sPropertyName,
                        1
                    );

            #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCA_STEP_SIZE;
            #endif
            }

        }

        // the enum properties
        {
            if (SCA_STATE & m_nIncludeSpecial)
            {
                exportEnumPropertyAttribute(
                    OAttributeMetaData::getSpecialAttributeNamespace(SCA_STATE),
                    OAttributeMetaData::getSpecialAttributeName(SCA_STATE),
                    PROPERTY_DEFAULT_STATE,
                    OEnumMapper::getEnumMap(OEnumMapper::epCheckState),
                    STATE_NOCHECK);
            #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCA_STATE;
            #endif
            }

            if (SCA_CURRENT_STATE & m_nIncludeSpecial)
            {
                exportEnumPropertyAttribute(
                    OAttributeMetaData::getSpecialAttributeNamespace(SCA_CURRENT_STATE),
                    OAttributeMetaData::getSpecialAttributeName(SCA_CURRENT_STATE),
                    PROPERTY_STATE,
                    OEnumMapper::getEnumMap(OEnumMapper::epCheckState),
                    STATE_NOCHECK);
            #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCA_CURRENT_STATE;
            #endif
            }
        }

        // some properties which require a special handling
        // the repeat delay
        {
            if ( m_nIncludeSpecial & SCA_REPEAT_DELAY )
            {
                DBG_CHECK_PROPERTY( PROPERTY_REPEAT_DELAY, sal_Int32 );

                sal_Int32 nRepeatDelay = 0;
                m_xProps->getPropertyValue( PROPERTY_REPEAT_DELAY ) >>= nRepeatDelay;
                Time aTime( Time::SYSTEM );
                aTime.MakeTimeFromMS( nRepeatDelay );
                util::Duration aDuration;
                aDuration.Hours   = aTime.GetHour();
                aDuration.Minutes = aTime.GetMin();
                aDuration.Seconds = aTime.GetSec();
                aDuration.NanoSeconds = (nRepeatDelay % 1000) * 1000000;

                OUStringBuffer buf;
                ::sax::Converter::convertDuration(buf, aDuration);
                AddAttribute(OAttributeMetaData::getSpecialAttributeNamespace( SCA_REPEAT_DELAY )
                            ,OAttributeMetaData::getSpecialAttributeName( SCA_REPEAT_DELAY )
                            ,buf.makeStringAndClear());

                exportedProperty( PROPERTY_REPEAT_DELAY );

            #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCA_REPEAT_DELAY;
            #endif
            }
        }

        // the EchoChar property needs special handling, cause it's a Int16, but must be stored as one-character-string
        {
            if (SCA_ECHO_CHAR & m_nIncludeSpecial)
            {
                DBG_CHECK_PROPERTY( PROPERTY_ECHO_CHAR, sal_Int16 );
                sal_Int16 nValue(0);
                m_xProps->getPropertyValue(PROPERTY_ECHO_CHAR) >>= nValue;
                if (nValue)
                {
                    OUString sCharacter(reinterpret_cast<const sal_Unicode*>(&nValue), 1);
                    AddAttribute(
                        OAttributeMetaData::getSpecialAttributeNamespace(SCA_ECHO_CHAR),
                        OAttributeMetaData::getSpecialAttributeName(SCA_ECHO_CHAR),
                        sCharacter);
                }
                exportedProperty(PROPERTY_ECHO_CHAR);
            #if OSL_DEBUG_LEVEL > 0
                //  reset the bit for later checking
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCA_ECHO_CHAR;
            #endif
            }
        }

        // the string properties
        {
            static const sal_Int32 nStringPropertyAttributeIds[] =
            {   // attribute flags
                SCA_GROUP_NAME
            };
            static const OUString pStringPropertyNames[] =
            {   // property names
                OUString(PROPERTY_GROUP_NAME)
            };

            static const sal_Int32 nIdCount = sizeof( nStringPropertyAttributeIds ) / sizeof( nStringPropertyAttributeIds[0] );
        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nNameCount = sizeof( pStringPropertyNames ) / sizeof( pStringPropertyNames[0] );
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

        if ((SCA_MIN_VALUE | SCA_MAX_VALUE) & m_nIncludeSpecial)
        {
            // need to export the min value and the max value as attributes
            // It depends on the real type (FormComponentType) of the control, which properties hold these
            // values
            const sal_Char* pMinValuePropertyName = NULL;
            const sal_Char* pMaxValuePropertyName = NULL;
            getValueLimitPropertyNames(m_nClassId, pMinValuePropertyName, pMaxValuePropertyName);

            OSL_ENSURE((NULL == pMinValuePropertyName) == (0 == (SCA_MIN_VALUE & m_nIncludeSpecial)),
                "OControlExport::exportCommonControlAttributes: no property found for the min value attribute!");
            OSL_ENSURE((NULL == pMaxValuePropertyName) == (0 == (SCA_MAX_VALUE & m_nIncludeSpecial)),
                "OControlExport::exportCommonControlAttributes: no property found for the max value attribute!");

            // add the two attributes
            static const sal_Char* pMinValueAttributeName = OAttributeMetaData::getSpecialAttributeName(SCA_MIN_VALUE);
            static const sal_Char* pMaxValueAttributeName = OAttributeMetaData::getSpecialAttributeName(SCA_MAX_VALUE);
            static const sal_uInt16 nMinValueNamespaceKey = OAttributeMetaData::getSpecialAttributeNamespace(SCA_MIN_VALUE);
            static const sal_uInt16 nMaxValueNamespaceKey = OAttributeMetaData::getSpecialAttributeNamespace(SCA_MAX_VALUE);

            if (pMinValuePropertyName && (SCA_MIN_VALUE & m_nIncludeSpecial))
                exportGenericPropertyAttribute(
                    nMinValueNamespaceKey,
                    pMinValueAttributeName,
                    pMinValuePropertyName);

            if (pMaxValuePropertyName && (SCA_MAX_VALUE & m_nIncludeSpecial))
                exportGenericPropertyAttribute(
                    nMaxValueNamespaceKey,
                    pMaxValueAttributeName,
                    pMaxValuePropertyName);
        #if OSL_DEBUG_LEVEL > 0
            //  reset the bit for later checking
            m_nIncludeSpecial = m_nIncludeSpecial & ~(SCA_MIN_VALUE | SCA_MAX_VALUE);
        #endif
        }

        if ( SCA_IMAGE_POSITION & m_nIncludeSpecial )
        {
            exportImagePositionAttributes();
            RESET_BIT( m_nIncludeSpecial, SCA_IMAGE_POSITION );
        }

        OSL_ENSURE(0 == m_nIncludeSpecial,
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
        // DA_LIST_SOURCE needs some special handling
        DBG_CHECK_PROPERTY_NO_TYPE( PROPERTY_LISTSOURCE );

        OUString sListSource = getScalarListSourceValue();
        if ( !sListSource.isEmpty() )
        {   // the ListSource property needs to be exported as attribute, and it is not empty
            AddAttribute(
                OAttributeMetaData::getDatabaseAttributeNamespace(DA_LIST_SOURCE),
                OAttributeMetaData::getDatabaseAttributeName(DA_LIST_SOURCE),
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
        if ( 0 == ( m_nIncludeDatabase & DA_LIST_SOURCE ) )
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

        sal_Int16 nMaxLen = (sal_Int16)std::max(nItems, nValues);

        for (sal_Int16 i=0; i<nMaxLen; ++i )
        {
            m_rContext.getGlobalContext().ClearAttrList();
            if (i < nItems)
            {
                // there is an item at this position
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCA_LABEL),
                    OAttributeMetaData::getCommonControlAttributeName(CCA_LABEL),
                    *pItems);
                ++pItems;
            }
            if (i < nValues)
            {
                // there is an value at this position
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCA_VALUE),
                    OAttributeMetaData::getCommonControlAttributeName(CCA_VALUE),
                    *pValues);
                ++pValues;
            }

            Int16SetIterator aSelectedPos = aSelection.find(i);
            if (aSelection.end() != aSelectedPos)
            {   // the item at this position is selected
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCA_CURRENT_SELECTED),
                    OAttributeMetaData::getCommonControlAttributeName(CCA_CURRENT_SELECTED),
                    sTrue
                    );
                aSelection.erase(aSelectedPos);
            }

            Int16SetIterator aDefaultSelectedPos = aDefaultSelection.find(i);
            if (aDefaultSelection.end() != aDefaultSelectedPos)
            {   // the item at this position is selected as default
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCA_SELECTED),
                    OAttributeMetaData::getCommonControlAttributeName(CCA_SELECTED),
                    sTrue
                    );
                aDefaultSelection.erase(aDefaultSelectedPos);
            }
            SvXMLElementExport aFormElement(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, "option", sal_True, sal_True);
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
                        OAttributeMetaData::getCommonControlAttributeNamespace(CCA_CURRENT_SELECTED),
                        OAttributeMetaData::getCommonControlAttributeName(CCA_CURRENT_SELECTED),
                        sTrue
                        );
                }

                if (aDefaultSelection.end() != aDefaultSelection.find(i))
                {   // the (not existent) item at this position is selected as default
                    AddAttribute(
                        OAttributeMetaData::getCommonControlAttributeNamespace(CCA_SELECTED),
                        OAttributeMetaData::getCommonControlAttributeName(CCA_SELECTED),
                        sTrue
                        );
                }
                SvXMLElementExport aFormElement(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, "option", sal_True, sal_True);
            }
        }
    }

    void OControlExport::implStartElement(const sal_Char* _pName)
    {
        // before we let the base class start it's outer element, we add a wrapper element
        const sal_Char *pOuterElementName = getOuterXMLElementName();
        m_pOuterElement = pOuterElementName
                               ? new SvXMLElementExport(
                                        m_rContext.getGlobalContext(),
                                        XML_NAMESPACE_FORM,
                                        pOuterElementName, sal_True,
                                        sal_True)
                            : 0;

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
        delete m_pOuterElement;
        m_pOuterElement = NULL;
    }

    const sal_Char* OControlExport::getOuterXMLElementName() const
    {
        return 0;
    }

    const sal_Char* OControlExport::getXMLElementName() const
    {
        return getElementName(m_eType);
    }

    void OControlExport::examine()
    {
        OSL_ENSURE( ( m_nIncludeCommon == 0 ) && ( m_nIncludeSpecial == 0 ) && ( m_nIncludeDatabase == 0 )
                 && ( m_nIncludeEvents == 0 ) && ( m_nIncludeBindings == 0),
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
                // NO BREAK
            case FormComponentType::TIMEFIELD:
                if ( !knownType )
                {
                    m_eType = TIME;
                    knownType = true;
                }
                m_nIncludeSpecial |= SCA_VALIDATION;
                // NO BREAK
            case FormComponentType::NUMERICFIELD:
            case FormComponentType::CURRENCYFIELD:
            case FormComponentType::PATTERNFIELD:
                if ( !knownType )
                {
                    m_eType = FORMATTED_TEXT;
                    knownType = true;
                }
                // NO BREAK
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
                            m_nIncludeSpecial |= SCA_ECHO_CHAR;
                        }
                        else
                        {
                            // if the MultiLine property is sal_True, it is a TextArea
                            sal_Bool bMultiLine = sal_False;
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
                    knownType = true;
                }

                // attributes which are common to all the types:
                // common attributes
                m_nIncludeCommon =
                    CCA_NAME | CCA_SERVICE_NAME | CCA_DISABLED |
                    CCA_PRINTABLE | CCA_TAB_INDEX | CCA_TAB_STOP | CCA_TITLE;

                if  (   ( m_nClassId != FormComponentType::DATEFIELD )
                    &&  ( m_nClassId != FormComponentType::TIMEFIELD )
                    )
                    // date and time field values are handled differently nowadays
                    m_nIncludeCommon |= CCA_VALUE;

                // database attributes
                m_nIncludeDatabase = DA_DATA_FIELD | DA_INPUT_REQUIRED;

                // event attributes
                m_nIncludeEvents = EA_CONTROL_EVENTS | EA_ON_CHANGE | EA_ON_SELECT;

                // only text and pattern fields have a ConvertEmptyToNull property
                if  (   ( m_nClassId == FormComponentType::TEXTFIELD )
                    ||  ( m_nClassId == FormComponentType::PATTERNFIELD )
                    )
                    m_nIncludeDatabase |= DA_CONVERT_EMPTY;

                // all controls but the file control fields have a readonly property
                if ( m_nClassId != FormComponentType::FILECONTROL )
                    m_nIncludeCommon |= CCA_READONLY;

                // a text field has a max text len
                if ( m_nClassId == FormComponentType::TEXTFIELD )
                    m_nIncludeCommon |= CCA_MAX_LENGTH;

                // max and min values and validation:
                if (FORMATTED_TEXT == m_eType)
                {   // in general all controls represented as formatted-text have these props
                    if  ( FormComponentType::PATTERNFIELD != m_nClassId )   // except the PatternField
                        m_nIncludeSpecial |= SCA_MAX_VALUE | SCA_MIN_VALUE;

                    if (FormComponentType::TEXTFIELD != m_nClassId)
                        // and the FormattedField does not have a validation flag
                        m_nIncludeSpecial |= SCA_VALIDATION;
                }

                // if it's not a password field or rich text control, the CurrentValue needs to be stored, too
                if  (   ( PASSWORD != m_eType )
                    &&  ( DATE != m_eType )
                    &&  ( TIME != m_eType )
                    )
                {
                    m_nIncludeCommon |= CCA_CURRENT_VALUE;
                }
            }
            break;

            case FormComponentType::FILECONTROL:
                m_eType = FILE;
                m_nIncludeCommon =
                    CCA_NAME | CCA_SERVICE_NAME | CCA_CURRENT_VALUE | CCA_DISABLED |
                    CCA_PRINTABLE | CCA_TAB_INDEX | CCA_TAB_STOP | CCA_TITLE |
                    CCA_VALUE;
                m_nIncludeEvents = EA_CONTROL_EVENTS | EA_ON_CHANGE | EA_ON_SELECT;
                break;

            case FormComponentType::FIXEDTEXT:
                m_eType = FIXED_TEXT;
                m_nIncludeCommon =
                    CCA_NAME | CCA_SERVICE_NAME | CCA_DISABLED | CCA_LABEL |
                    CCA_PRINTABLE | CCA_TITLE | CCA_FOR;
                m_nIncludeSpecial = SCA_MULTI_LINE;
                m_nIncludeEvents = EA_CONTROL_EVENTS;
                break;

            case FormComponentType::COMBOBOX:
                m_eType = COMBOBOX;
                m_nIncludeCommon =
                    CCA_NAME | CCA_SERVICE_NAME | CCA_CURRENT_VALUE |
                    CCA_DISABLED | CCA_DROPDOWN | CCA_MAX_LENGTH | CCA_PRINTABLE | CCA_READONLY | CCA_SIZE |
                    CCA_TAB_INDEX | CCA_TAB_STOP | CCA_TITLE | CCA_VALUE;
                m_nIncludeSpecial = SCA_AUTOMATIC_COMPLETION;
                m_nIncludeDatabase = DA_CONVERT_EMPTY | DA_DATA_FIELD | DA_INPUT_REQUIRED | DA_LIST_SOURCE | DA_LIST_SOURCE_TYPE;
                m_nIncludeEvents = EA_CONTROL_EVENTS | EA_ON_CHANGE | EA_ON_SELECT;
                break;

            case FormComponentType::LISTBOX:
                m_eType = LISTBOX;
                m_nIncludeCommon =
                    CCA_NAME | CCA_SERVICE_NAME | CCA_DISABLED | CCA_DROPDOWN |
                    CCA_PRINTABLE | CCA_SIZE | CCA_TAB_INDEX | CCA_TAB_STOP | CCA_TITLE;
                m_nIncludeSpecial = SCA_MULTIPLE;
                m_nIncludeDatabase = DA_BOUND_COLUMN | DA_DATA_FIELD | DA_INPUT_REQUIRED | DA_LIST_SOURCE_TYPE;
                m_nIncludeEvents = EA_CONTROL_EVENTS | EA_ON_CHANGE | EA_ON_CLICK | EA_ON_DBLCLICK;
                // check if we need to export the ListSource as attribute
                {
                    // for a list box, if the ListSourceType is VALUE_LIST, no ListSource is stored, but instead
                    // a sequence of pairs which is build from the StringItemList and the ValueList
                    ListSourceType eListSourceType = ListSourceType_VALUELIST;
                #if OSL_DEBUG_LEVEL > 0
                    sal_Bool bSuccess =
                #endif
                    m_xProps->getPropertyValue(PROPERTY_LISTSOURCETYPE) >>= eListSourceType;
                    OSL_ENSURE(bSuccess, "OControlExport::examineControl: could not retrieve the ListSourceType!");
                    if (ListSourceType_VALUELIST != eListSourceType)
                    {
                        m_nIncludeDatabase |= DA_LIST_SOURCE;
                    }
                }

                break;

            case FormComponentType::COMMANDBUTTON:
                m_eType = BUTTON;
                m_nIncludeCommon |= CCA_TAB_STOP | CCA_LABEL;
                m_nIncludeSpecial = SCA_DEFAULT_BUTTON | SCA_TOGGLE | SCA_FOCUS_ON_CLICK | SCA_IMAGE_POSITION | SCA_REPEAT_DELAY;
                // NO BREAK !
            case FormComponentType::IMAGEBUTTON:
                if (BUTTON != m_eType)
                {
                    // not coming from the previous case
                    m_eType = IMAGE;
                }
                m_nIncludeCommon |=
                    CCA_NAME | CCA_SERVICE_NAME | CCA_BUTTON_TYPE | CCA_DISABLED |
                    CCA_IMAGE_DATA | CCA_PRINTABLE | CCA_TAB_INDEX | CCA_TARGET_FRAME |
                    CCA_TARGET_LOCATION | CCA_TITLE;
                m_nIncludeEvents = EA_CONTROL_EVENTS | EA_ON_CLICK  | EA_ON_DBLCLICK;
                break;

            case FormComponentType::CHECKBOX:
                m_eType = CHECKBOX;
                m_nIncludeSpecial = SCA_CURRENT_STATE | SCA_IS_TRISTATE | SCA_STATE;
                // NO BREAK !
            case FormComponentType::RADIOBUTTON:
                m_nIncludeCommon =
                    CCA_NAME | CCA_SERVICE_NAME | CCA_DISABLED | CCA_LABEL | CCA_PRINTABLE |
                    CCA_TAB_INDEX | CCA_TAB_STOP | CCA_TITLE | CCA_VALUE | CCA_VISUAL_EFFECT;
                if (CHECKBOX != m_eType)
                {   // not coming from the previous case
                    m_eType = RADIO;
                    m_nIncludeCommon |= CCA_CURRENT_SELECTED | CCA_SELECTED;
                }
                if ( m_xPropertyInfo->hasPropertyByName( PROPERTY_IMAGE_POSITION ) )
                    m_nIncludeSpecial |= SCA_IMAGE_POSITION;
                if ( m_xPropertyInfo->hasPropertyByName( PROPERTY_GROUP_NAME ) )
                    m_nIncludeSpecial |= SCA_GROUP_NAME;
                m_nIncludeDatabase = DA_DATA_FIELD | DA_INPUT_REQUIRED;
                m_nIncludeEvents = EA_CONTROL_EVENTS | EA_ON_CHANGE;
                break;

            case FormComponentType::GROUPBOX:
                m_eType = FRAME;
                m_nIncludeCommon =
                    CCA_NAME | CCA_SERVICE_NAME | CCA_DISABLED | CCA_LABEL |
                    CCA_PRINTABLE | CCA_TITLE | CCA_FOR;
                m_nIncludeEvents = EA_CONTROL_EVENTS;
                break;

            case FormComponentType::IMAGECONTROL:
                m_eType = IMAGE_FRAME;
                m_nIncludeCommon =
                    CCA_NAME | CCA_SERVICE_NAME | CCA_DISABLED | CCA_IMAGE_DATA |
                    CCA_PRINTABLE | CCA_READONLY | CCA_TITLE;
                m_nIncludeDatabase = DA_DATA_FIELD | DA_INPUT_REQUIRED;
                m_nIncludeEvents = EA_CONTROL_EVENTS;
                break;

            case FormComponentType::HIDDENCONTROL:
                m_eType = HIDDEN;
                m_nIncludeCommon =
                    CCA_NAME | CCA_SERVICE_NAME | CCA_VALUE;
                break;

            case FormComponentType::GRIDCONTROL:
                m_eType = GRID;
                m_nIncludeCommon =
                    CCA_NAME | CCA_SERVICE_NAME | CCA_DISABLED | CCA_PRINTABLE |
                    CCA_TAB_INDEX | CCA_TAB_STOP | CCA_TITLE;
                m_nIncludeEvents = EA_CONTROL_EVENTS;
                break;

            case FormComponentType::SCROLLBAR:
            case FormComponentType::SPINBUTTON:
                m_eType = VALUERANGE;
                m_nIncludeCommon =
                    CCA_NAME | CCA_SERVICE_NAME | CCA_DISABLED | CCA_PRINTABLE |
                    CCA_TITLE | CCA_CURRENT_VALUE | CCA_VALUE | CCA_ORIENTATION;
                m_nIncludeSpecial = SCA_MAX_VALUE | SCA_STEP_SIZE | SCA_MIN_VALUE | SCA_REPEAT_DELAY;

                if ( m_nClassId == FormComponentType::SCROLLBAR )
                    m_nIncludeSpecial |= SCA_PAGE_STEP_SIZE ;

                m_nIncludeEvents = EA_CONTROL_EVENTS;
                break;

            default:
                OSL_FAIL("OControlExport::examineControl: unknown control type (class id)!");
                // NO break!

            case FormComponentType::NAVIGATIONBAR:
                // TODO: should we have an own file format for this?
                // NO break

            case FormComponentType::CONTROL:
                m_eType = GENERIC_CONTROL;
                // unknown control type
                m_nIncludeCommon = CCA_NAME | CCA_SERVICE_NAME;
                    // at least a name should be there, 'cause without a name the control could never have been
                    // inserted into it's parent container
                    // In addition, the service name is absolutely necessary to create the control upon reading.
                m_nIncludeEvents = EA_CONTROL_EVENTS;
                    // we always should be able to export events - this is not control type dependent
                break;
        }

        // in general, all control types need to export the control id
        m_nIncludeCommon |= CCA_CONTROL_ID;

        // is is a control bound to a calc cell?
        if ( FormCellBindingHelper::livesInSpreadsheetDocument( m_xProps ) )
        {
            FormCellBindingHelper aHelper( m_xProps, NULL );
            {
                if ( aHelper.isCellBinding( aHelper.getCurrentBinding( ) ) )
                {
                    m_nIncludeBindings |= BA_LINKED_CELL;
                    if ( m_nClassId == FormComponentType::LISTBOX )
                        m_nIncludeBindings |= BA_LIST_LINKING_TYPE;
                }
            }

            // is it a list-like control which uses a calc cell range as list source?
            {
                if ( aHelper.isCellRangeListSource( aHelper.getCurrentListSource( ) ) )
                    m_nIncludeBindings |= BA_LIST_CELL_RANGE;
            }
        }

        // is control bound to XForms?
        if( !getXFormsBindName( m_xProps ).isEmpty() )
        {
            m_nIncludeBindings |= BA_XFORMS_BIND;
        }

        // is (list-)control bound to XForms list?
        if( !getXFormsListBindName( m_xProps ).isEmpty() )
        {
            m_nIncludeBindings |= BA_XFORMS_LISTBIND;
        }

        // does the control have an XForms submission?
        if( !getXFormsSubmissionName( m_xProps ).isEmpty() )
        {
            m_nIncludeBindings |= BA_XFORMS_SUBMISSION;
        }
    }

    void OControlExport::exportCellBindingAttributes( bool _bIncludeListLinkageType )
    {
        try
        {
            FormCellBindingHelper aHelper( m_xProps, NULL );
            Reference< XValueBinding > xBinding( aHelper.getCurrentBinding() );
            OSL_ENSURE( xBinding.is(), "OControlExport::exportCellBindingAttributes: invalid bindable or invalid binding!" );
            if ( xBinding.is() )
            {
                AddAttribute(
                    OAttributeMetaData::getBindingAttributeNamespace( BA_LINKED_CELL ),
                    OAttributeMetaData::getBindingAttributeName( BA_LINKED_CELL ),
                    aHelper.getStringAddressFromCellBinding( xBinding )
                );

                if ( _bIncludeListLinkageType )
                {
                    sal_Int16 nLinkageType = aHelper.isCellIntegerBinding( xBinding ) ? 1 : 0;

                    OUStringBuffer sBuffer;
                    m_rContext.getGlobalContext().GetMM100UnitConverter().convertEnum(
                        sBuffer,
                        (sal_uInt16)nLinkageType,
                        OEnumMapper::getEnumMap( OEnumMapper::epListLinkageType )
                    );

                    AddAttribute(
                        OAttributeMetaData::getBindingAttributeNamespace( BA_LIST_LINKING_TYPE ),
                        OAttributeMetaData::getBindingAttributeName( BA_LIST_LINKING_TYPE ),
                        sBuffer.makeStringAndClear()
                    );
                }

            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OControlExport::exportCellBindingAttributes: caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
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
                xSource = xSource.query( xSink->getListEntrySource() );
            OSL_ENSURE( xSource.is(), "OControlExport::exportCellListSourceRange: list source or sink!" );
            if ( xSource.is() )
            {
                FormCellBindingHelper aHelper( m_xProps, NULL );

                AddAttribute(
                    OAttributeMetaData::getBindingAttributeNamespace( BA_LIST_CELL_RANGE ),
                    OAttributeMetaData::getBindingAttributeName( BA_LIST_CELL_RANGE ),
                    aHelper.getStringAddressFromCellListSource( xSource )
                );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OControlExport::exportCellListSourceRange: caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
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
                XMLTokenEnum eXmlImagePositions[] =
                {
                    XML_START, XML_END, XML_TOP, XML_BOTTOM
                };
                XMLTokenEnum eXmlImageAligns[] =
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
            DBG_UNHANDLED_EXCEPTION();
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
            OSL_FAIL( "OColumnExport::controlHasActiveDataBinding: caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
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
            OSL_FAIL( "OControlExport::controlHasUserSuppliedListEntries: caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
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
        implEndElement();
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
        AddAttribute( OAttributeMetaData::getCommonControlAttributeNamespace(CCA_SERVICE_NAME)
                    , OAttributeMetaData::getCommonControlAttributeName(CCA_SERVICE_NAME)
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
            OAttributeMetaData::getCommonControlAttributeNamespace(CCA_LABEL),
            OAttributeMetaData::getCommonControlAttributeName(CCA_LABEL),
            PROPERTY_LABEL);

        // the style attribute
        OUString sStyleName = m_rContext.getObjectStyleName( m_xProps );
        if ( !sStyleName.isEmpty() )
        {
            AddAttribute(
                OAttributeMetaData::getSpecialAttributeNamespace( SCA_COLUMN_STYLE_NAME ),
                OAttributeMetaData::getSpecialAttributeName( SCA_COLUMN_STYLE_NAME ),
                sStyleName
            );
        }
    }

    void OColumnExport::examine()
    {
        OControlExport::examine();

        // grid columns miss some properties of the controls they're representing
        m_nIncludeCommon &= ~(CCA_FOR | CCA_PRINTABLE | CCA_TAB_INDEX | CCA_TAB_STOP | CCA_LABEL);
        m_nIncludeSpecial &= ~(SCA_ECHO_CHAR | SCA_AUTOMATIC_COMPLETION | SCA_MULTIPLE | SCA_MULTI_LINE);

        if (FormComponentType::DATEFIELD != m_nClassId)
            // except date fields, no column has the DropDown property
            m_nIncludeCommon &= ~CCA_DROPDOWN;
    }

    //= OFormExport
    OFormExport::OFormExport(IFormsExportContext& _rContext, const Reference< XPropertySet >& _rxForm,
        const Sequence< ScriptEventDescriptor >& _rEvents)
        :OElementExport(_rContext, _rxForm, _rEvents)
        ,m_bCreateConnectionResourceElement(sal_False)
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
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCA_TARGET_LOCATION),
                    OAttributeMetaData::getCommonControlAttributeName(CCA_TARGET_LOCATION),
                    sPropValue);
            if ( m_rContext.getGlobalContext().GetAttrList().getLength() )
            {
                SvXMLElementExport aFormElement(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, xmloff::token::XML_CONNECTION_RESOURCE, sal_True, sal_True);
            }
        }

        // let the base class export the remaining properties and the events
        OElementExport::exportSubTags();
        // loop through all children
        Reference< XIndexAccess > xCollection(m_xProps, UNO_QUERY);
        OSL_ENSURE(xCollection.is(), "OFormLayerXMLExport::implExportForm: a form which is not an index access? Suspic�ous!");

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
            static const OUString aStringPropertyNames[] =
            {
                OUString(PROPERTY_NAME), /*OUString(PROPERTY_TARGETURL),*/ OUString(PROPERTY_COMMAND), OUString(PROPERTY_FILTER), OUString(PROPERTY_ORDER)
            };
            static const sal_Int32 nIdCount = sizeof(eStringPropertyIds) / sizeof(eStringPropertyIds[0]);
        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nNameCount = sizeof(aStringPropertyNames) / sizeof(aStringPropertyNames[0]);
            OSL_ENSURE((nIdCount == nNameCount),
                "OFormExport::exportAttributes: somebody tampered with the maps (1)!");
        #endif
            for (i=0; i<nIdCount; ++i)
                exportStringPropertyAttribute(
                    OAttributeMetaData::getFormAttributeNamespace(eStringPropertyIds[i]),
                    OAttributeMetaData::getFormAttributeName(eStringPropertyIds[i]),
                    aStringPropertyNames[i]);

            // #i112082# xlink:type is added as part of exportTargetLocationAttribute

            // now export the data source name or databaselocation or connection resource
            OUString sPropValue;
            m_xProps->getPropertyValue( PROPERTY_DATASOURCENAME ) >>= sPropValue;
            m_bCreateConnectionResourceElement = sPropValue.isEmpty();
            if ( !m_bCreateConnectionResourceElement )
            {
                INetURLObject aURL(sPropValue);
                m_bCreateConnectionResourceElement = ( aURL.GetProtocol() == INET_PROT_FILE );
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
            static const OUString pBooleanPropertyNames[] =
            {
                OUString(PROPERTY_ALLOWDELETES),
                OUString(PROPERTY_ALLOWINSERTS),
                OUString(PROPERTY_ALLOWUPDATES),
                OUString(PROPERTY_APPLYFILTER),
                OUString(PROPERTY_ESCAPEPROCESSING),
                OUString(PROPERTY_IGNORERESULT)
            };
            static const sal_Int8 nBooleanPropertyAttrFlags[] =
            {
                BOOLATTR_DEFAULT_TRUE, BOOLATTR_DEFAULT_TRUE, BOOLATTR_DEFAULT_TRUE, BOOLATTR_DEFAULT_FALSE, BOOLATTR_DEFAULT_TRUE, BOOLATTR_DEFAULT_FALSE
            };
            static const sal_Int32 nIdCount = sizeof(eBooleanPropertyIds) / sizeof(eBooleanPropertyIds[0]);
        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nNameCount = sizeof(pBooleanPropertyNames) / sizeof(pBooleanPropertyNames[0]);
            static const sal_Int32 nFlagsCount = sizeof(nBooleanPropertyAttrFlags) / sizeof(nBooleanPropertyAttrFlags[0]);
            OSL_ENSURE((nIdCount == nNameCount) && (nNameCount == nFlagsCount),
                "OFormExport::exportAttributes: somebody tampered with the maps (2)!");
        #endif
            for (i=0; i<nIdCount; ++i)
                exportBooleanPropertyAttribute(
                    OAttributeMetaData::getFormAttributeNamespace(eBooleanPropertyIds[i]),
                    OAttributeMetaData::getFormAttributeName(eBooleanPropertyIds[i]),
                    pBooleanPropertyNames[i],
                    nBooleanPropertyAttrFlags[i]
                );
        }

        // the enum properties
        {
            static const FormAttributes eEnumPropertyIds[] =
            {
                faEnctype, faMethod, faCommandType, faNavigationMode, faTabbingCycle
            };
            static const OUString pEnumPropertyNames[] =
            {
                OUString(PROPERTY_SUBMIT_ENCODING), OUString(PROPERTY_SUBMIT_METHOD), OUString(PROPERTY_COMMAND_TYPE), OUString(PROPERTY_NAVIGATION), OUString(PROPERTY_CYCLE)
            };
            static const OEnumMapper::EnumProperties eEnumPropertyMaps[] =
            {
                OEnumMapper::epSubmitEncoding, OEnumMapper::epSubmitMethod, OEnumMapper::epCommandType, OEnumMapper::epNavigationType, OEnumMapper::epTabCyle
            };
            static const sal_Int32 nEnumPropertyAttrDefaults[] =
            {
                FormSubmitEncoding_URL, FormSubmitMethod_GET, CommandType::COMMAND, NavigationBarMode_CURRENT, TabulatorCycle_RECORDS
            };
            static const sal_Bool nEnumPropertyAttrDefaultFlags[] =
            {
                sal_False, sal_False, sal_False, sal_False, sal_True
            };
            static const sal_Int32 nIdCount = sizeof(eEnumPropertyIds) / sizeof(eEnumPropertyIds[0]);
        #if OSL_DEBUG_LEVEL > 0
            static const sal_Int32 nNameCount = sizeof(pEnumPropertyNames) / sizeof(pEnumPropertyNames[0]);
            static const sal_Int32 nDefaultCount = sizeof(nEnumPropertyAttrDefaults) / sizeof(nEnumPropertyAttrDefaults[0]);
            static const sal_Int32 nDefaultFlagCount = sizeof(nEnumPropertyAttrDefaultFlags) / sizeof(nEnumPropertyAttrDefaultFlags[0]);
            static const sal_Int32 nMapCount = sizeof(eEnumPropertyMaps) / sizeof(eEnumPropertyMaps[0]);
            OSL_ENSURE((nIdCount == nNameCount) && (nNameCount == nDefaultCount) && (nDefaultCount == nDefaultFlagCount) && (nDefaultFlagCount == nMapCount),
                "OFormExport::exportAttributes: somebody tampered with the maps (3)!");
        #endif
            for (i=0; i<nIdCount; ++i)
                exportEnumPropertyAttribute(
                    OAttributeMetaData::getFormAttributeNamespace(eEnumPropertyIds[i]),
                    OAttributeMetaData::getFormAttributeName(eEnumPropertyIds[i]),
                    pEnumPropertyNames[i],
                    OEnumMapper::getEnumMap(eEnumPropertyMaps[i]),
                    nEnumPropertyAttrDefaults[i],
                    nEnumPropertyAttrDefaultFlags[i]
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
