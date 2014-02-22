/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <tools/wintypes.hxx>
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
        
        examine();

        
        m_rContext.getGlobalContext().ClearAttrList();

        
        exportAttributes();

        
        implStartElement(getXMLElementName());

        
        exportSubTags();

        implEndElement();
    }

    void OElementExport::examine()
    {
        
    }

    void OElementExport::exportAttributes()
    {
        
    }

    void OElementExport::exportSubTags()
    {
        
        exportRemainingProperties();

        
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
        
        
        
        

        OUString sToWriteServiceName = sServiceName;
#define CHECK_N_TRANSLATE( name )   \
        else if (sServiceName.equals(SERVICE_PERSISTENT_COMPONENT_##name)) \
            sToWriteServiceName = SERVICE_##name

        if (false)
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

        
        AddAttribute(
            OAttributeMetaData::getCommonControlAttributeNamespace(CCA_SERVICE_NAME),
            OAttributeMetaData::getCommonControlAttributeName(CCA_SERVICE_NAME),
            sToWriteServiceName);
    }

    void OElementExport::exportEvents()
    {
        if (!m_aEvents.getLength())
            
            return;

        Reference< XNameReplace > xWrapper = new OEventDescriptorMapper(m_aEvents);
        m_rContext.getGlobalContext().GetEventExport().Export(xWrapper);
    }

    
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
        
        if (CCA_NAME & m_nIncludeCommon)
        {
            exportStringPropertyAttribute(
                OAttributeMetaData::getCommonControlAttributeNamespace(CCA_NAME),
                OAttributeMetaData::getCommonControlAttributeName(CCA_NAME),
                PROPERTY_NAME
                );
        #if OSL_DEBUG_LEVEL > 0
            
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_NAME;
        #endif
        }

        
        if (m_nIncludeCommon & CCA_SERVICE_NAME)
        {
            exportServiceNameAttribute();
        #if OSL_DEBUG_LEVEL > 0
            
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_SERVICE_NAME;
        #endif
        }
    }

    void OControlExport::exportInnerAttributes()
    {
        
        if (CCA_CONTROL_ID & m_nIncludeCommon)
        {
            OSL_ENSURE(!m_sControlId.isEmpty(), "OControlExport::exportInnerAttributes: have no control id for the control!");
            m_rContext.getGlobalContext().AddAttributeIdLegacy(
                XML_NAMESPACE_FORM, m_sControlId);
        #if OSL_DEBUG_LEVEL > 0
            
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_CONTROL_ID;
        #endif
        }

        
        exportGenericHandlerAttributes();

        
        exportCommonControlAttributes();

        
        exportDatabaseAttributes();

        
        exportBindingAtributes();

        
        exportSpecialAttributes();

        
        flagStyleProperties();
    }

    void OControlExport::exportAttributes()
    {
        exportOuterAttributes();
    }

    void OControlExport::exportSubTags() throw (Exception)
    {
        
        
        
        
        exportedProperty(PROPERTY_CONTROLLABEL);

        
        
        
        
        
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

            
            
            exportedProperty( PROPERTY_RICH_TEXT );

            
            
            
            
            
            
            exportedProperty( OUString( "CharCrossedOut" ) );
        }

        if ( m_eType == LISTBOX )
        {
            
            if ( controlHasUserSuppliedListEntries() )
                exportedProperty( PROPERTY_DEFAULT_SELECT_SEQ );

            
            
            exportedProperty( PROPERTY_STRING_ITEM_LIST );
            exportedProperty( PROPERTY_VALUE_SEQ );
            exportedProperty( PROPERTY_SELECT_SEQ );
            exportedProperty( PROPERTY_LISTSOURCE );
        }
        if ( m_eType == COMBOBOX )
            exportedProperty( PROPERTY_STRING_ITEM_LIST );

        
        OElementExport::exportSubTags();

        
        switch (m_eType)
        {
            case LISTBOX:
                
                
                
                if ( controlHasUserSuppliedListEntries() )
                    exportListSourceAsElements();
                break;
            case GRID:
            {   
                Reference< XIndexAccess > xColumnContainer(m_xProps, UNO_QUERY);
                OSL_ENSURE(xColumnContainer.is(), "OControlExport::exportSubTags: a grid control which is no IndexAccess?!!");
                if (xColumnContainer.is())
                    m_rContext.exportCollectionElements(xColumnContainer);
            }
            break;
            case COMBOBOX:
            {   
                DBG_CHECK_PROPERTY( PROPERTY_STRING_ITEM_LIST, Sequence< OUString > );

                
                
                
                if ( controlHasUserSuppliedListEntries() )
                {
                    
                    Sequence< OUString > aListItems;
                    m_xProps->getPropertyValue(PROPERTY_STRING_ITEM_LIST) >>= aListItems;
                    
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
                
                
                const PropertyDescription* propDescription = metadata::getPropertyDescription( prop->Name );
                if ( propDescription == NULL )
                    continue;

                
                
                PPropertyHandler handler = (*propDescription->factory)( propDescription->propertyId );
                if ( !handler.get() )
                {
                    SAL_WARN( "xmloff.forms", "OControlExport::exportGenericHandlerAttributes: invalid property handler provided by the factory!" );
                    continue;
                }

                OUString attributeValue;
                if ( propDescription->propertyGroup == NO_GROUP )
                {
                    
                    if ( !shouldExportProperty( prop->Name ) )
                        
                        
                    {
                        exportedProperty( prop->Name );
                        continue;
                    }

                    const Any propValue = m_xProps->getPropertyValue( prop->Name );
                    attributeValue = handler->getAttributeValue( propValue );
                }
                else
                {
                    
                    

                    
                    PropertyDescriptionList descriptions;
                    metadata::getPropertyGroup( propDescription->propertyGroup, descriptions );

                    
                    PropertyValues aValues;
                    for (   PropertyDescriptionList::iterator desc = descriptions.begin();
                            desc != descriptions.end();
                            ++desc
                        )
                    {
                        
                        const Any propValue = m_xProps->getPropertyValue( (*desc)->propertyName );
                        aValues[ (*desc)->propertyId ] = propValue;
                    }

                    
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

        
        
        
        

        
        

        
        {
            
            static const sal_Int32 nStringPropertyAttributeIds[] =
            {
                CCA_LABEL, CCA_TITLE
            };
            
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
                    
                    m_nIncludeCommon = m_nIncludeCommon & ~nStringPropertyAttributeIds[i];
                #endif
                }
        }

        
        {
            static const sal_Int32 nBooleanPropertyAttributeIds[] =
            {   
                CCA_CURRENT_SELECTED, CCA_DISABLED, CCA_DROPDOWN, CCA_PRINTABLE, CCA_READONLY, CCA_SELECTED, CCA_TAB_STOP, CCA_ENABLEVISIBLE
            };
            static const OUString pBooleanPropertyNames[] =
            {   
                OUString(PROPERTY_STATE), OUString(PROPERTY_ENABLED),
                OUString(PROPERTY_DROPDOWN), OUString(PROPERTY_PRINTABLE),
                OUString(PROPERTY_READONLY), OUString(PROPERTY_DEFAULT_STATE),
                OUString(PROPERTY_TABSTOP), OUString(PROPERTY_ENABLEVISIBLE)
            };
            static const sal_Bool nBooleanPropertyAttrFlags[] =
            {   
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
                    
                    m_nIncludeCommon = m_nIncludeCommon & ~nBooleanPropertyAttributeIds[i];
        #endif
                }
        }

        
        {
            
            static sal_Int32 nIntegerPropertyAttributeIds[] =
            {   
                CCA_SIZE, CCA_TAB_INDEX
            };
            static const OUString pIntegerPropertyNames[] =
            {   
                OUString(PROPERTY_LINECOUNT), OUString(PROPERTY_TABINDEX)
            };
            static const sal_Int16 nIntegerPropertyAttrDefaults[] =
            {   
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
                    
                    m_nIncludeCommon = m_nIncludeCommon & ~nIntegerPropertyAttributeIds[i];
        #endif
                }

        }

        
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
                
                m_nIncludeCommon = m_nIncludeCommon & ~CCA_VISUAL_EFFECT;
            #endif
            }
        }

        

        
        if (m_nIncludeCommon & CCA_TARGET_FRAME)
        {
            exportTargetFrameAttribute();
        #if OSL_DEBUG_LEVEL > 0
            
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_TARGET_FRAME;
        #endif
        }

        
        if ( m_nIncludeCommon & CCA_MAX_LENGTH )
        {
            
            

            
            OUString sTextLenPropertyName( PROPERTY_MAXTEXTLENGTH );
            if ( m_xPropertyInfo->hasPropertyByName( PROPERTY_PERSISTENCE_MAXTEXTLENGTH ) )
                sTextLenPropertyName = PROPERTY_PERSISTENCE_MAXTEXTLENGTH;

            
            exportInt16PropertyAttribute(
                OAttributeMetaData::getCommonControlAttributeNamespace( CCA_MAX_LENGTH ),
                OAttributeMetaData::getCommonControlAttributeName( CCA_MAX_LENGTH ),
                sTextLenPropertyName,
                0
            );

            
            exportedProperty( PROPERTY_MAXTEXTLENGTH );
            exportedProperty( PROPERTY_PERSISTENCE_MAXTEXTLENGTH );

        #if OSL_DEBUG_LEVEL > 0
            
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_MAX_LENGTH;
        #endif
        }

        if (m_nIncludeCommon & CCA_TARGET_LOCATION)
        {
            exportTargetLocationAttribute(false);
        #if OSL_DEBUG_LEVEL > 0
            
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_TARGET_LOCATION;
        #endif
        }

        
        if (m_nIncludeCommon & CCA_IMAGE_DATA)
        {
            exportImageDataAttribute();
        #if OSL_DEBUG_LEVEL > 0
            
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_IMAGE_DATA;
        #endif
        }

        
        
        if (m_nIncludeCommon & CCA_FOR)
        {
            if (!m_sReferringControls.isEmpty())
            {   
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCA_FOR),
                    OAttributeMetaData::getCommonControlAttributeName(CCA_FOR),
                    m_sReferringControls);
            }
        #if OSL_DEBUG_LEVEL > 0
            
            m_nIncludeCommon = m_nIncludeCommon & ~CCA_FOR;
        #endif
        }

        if ((CCA_CURRENT_VALUE | CCA_VALUE) & m_nIncludeCommon)
        {
            const sal_Char* pCurrentValuePropertyName = NULL;
            const sal_Char* pValuePropertyName = NULL;

            
            getValuePropertyNames(m_eType, m_nClassId, pCurrentValuePropertyName, pValuePropertyName);

            static const sal_Char* pCurrentValueAttributeName = OAttributeMetaData::getCommonControlAttributeName(CCA_CURRENT_VALUE);
            static const sal_Char* pValueAttributeName = OAttributeMetaData::getCommonControlAttributeName(CCA_VALUE);
            static const sal_uInt16 nCurrentValueAttributeNamespaceKey = OAttributeMetaData::getCommonControlAttributeNamespace(CCA_CURRENT_VALUE);
            static const sal_uInt16 nValueAttributeNamespaceKey = OAttributeMetaData::getCommonControlAttributeNamespace(CCA_VALUE);

            
            if (pCurrentValuePropertyName && (CCA_CURRENT_VALUE & m_nIncludeCommon))
            {
                
                
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
            
            m_nIncludeCommon = m_nIncludeCommon & ~(CCA_CURRENT_VALUE | CCA_VALUE);
        #endif
        }

        OSL_ENSURE(0 == m_nIncludeCommon,
            "OControlExport::exportCommonControlAttributes: forgot some flags!");
            
            
    }

    void OControlExport::exportDatabaseAttributes()
    {
#if OSL_DEBUG_LEVEL > 0
        sal_Int32 nIncludeDatabase = m_nIncludeDatabase;
#endif
        
        if (DA_DATA_FIELD & m_nIncludeDatabase)
        {
            exportStringPropertyAttribute(
                OAttributeMetaData::getDatabaseAttributeNamespace(DA_DATA_FIELD),
                OAttributeMetaData::getDatabaseAttributeName(DA_DATA_FIELD),
                PROPERTY_DATAFIELD);
            RESET_BIT( nIncludeDatabase, DA_DATA_FIELD );
        }

        
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
            
            nIncludeBinding = nIncludeBinding & ~( BA_LINKED_CELL | BA_LIST_LINKING_TYPE );
        #endif
        }

        if ( m_nIncludeBindings & BA_LIST_CELL_RANGE )
        {
            exportCellListSourceRange();
        #if OSL_DEBUG_LEVEL > 0
            
            nIncludeBinding = nIncludeBinding & ~BA_LIST_CELL_RANGE;
        #endif
        }

        if ( m_nIncludeBindings & BA_XFORMS_BIND )
        {
            exportXFormsBindAttributes();
        #if OSL_DEBUG_LEVEL > 0
            
            nIncludeBinding = nIncludeBinding & ~BA_XFORMS_BIND;
        #endif
        }

        if ( m_nIncludeBindings & BA_XFORMS_LISTBIND )
        {
            exportXFormsListAttributes();
        #if OSL_DEBUG_LEVEL > 0
            
            nIncludeBinding = nIncludeBinding & ~BA_XFORMS_LISTBIND;
        #endif
        }

        if ( m_nIncludeBindings & BA_XFORMS_SUBMISSION )
        {
            exportXFormsSubmissionAttributes();
        #if OSL_DEBUG_LEVEL > 0
            
            nIncludeBinding = nIncludeBinding & ~BA_XFORMS_SUBMISSION;
        #endif
        }

        OSL_ENSURE( 0 == nIncludeBinding,
            "OControlExport::exportBindingAtributes: forgot some flags!");
            
            
    }

    void OControlExport::exportSpecialAttributes()
    {
        sal_Int32 i=0;

        
        {
            static const sal_Int32 nBooleanPropertyAttributeIds[] =
            {   
                SCA_VALIDATION, SCA_MULTI_LINE, SCA_AUTOMATIC_COMPLETION, SCA_MULTIPLE, SCA_DEFAULT_BUTTON, SCA_IS_TRISTATE,
                SCA_TOGGLE, SCA_FOCUS_ON_CLICK
            };
            static const OUString pBooleanPropertyNames[] =
            {   
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
                
                m_nIncludeSpecial = m_nIncludeSpecial & ~*pAttributeId;
            #endif
                }
            }
        }

        
        {
            static sal_Int32 nIntegerPropertyAttributeIds[] =
            {   
                SCA_PAGE_STEP_SIZE
            };
            static const OUString pIntegerPropertyNames[] =
            {   
                OUString(PROPERTY_BLOCK_INCREMENT)
            };
            static const sal_Int32 nIntegerPropertyAttrDefaults[] =
            {   
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
                
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCA_STEP_SIZE;
            #endif
            }

        }

        
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
                
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCA_CURRENT_STATE;
            #endif
            }
        }

        
        
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
                
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCA_REPEAT_DELAY;
            #endif
            }
        }

        
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
                
                m_nIncludeSpecial = m_nIncludeSpecial & ~SCA_ECHO_CHAR;
            #endif
            }
        }

        
        {
            static const sal_Int32 nStringPropertyAttributeIds[] =
            {   
                SCA_GROUP_NAME
            };
            static const OUString pStringPropertyNames[] =
            {   
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
                
                m_nIncludeSpecial = m_nIncludeSpecial & ~nStringPropertyAttributeIds[i];
            #endif
                }
        }

        if ((SCA_MIN_VALUE | SCA_MAX_VALUE) & m_nIncludeSpecial)
        {
            
            
            
            const sal_Char* pMinValuePropertyName = NULL;
            const sal_Char* pMaxValuePropertyName = NULL;
            getValueLimitPropertyNames(m_nClassId, pMinValuePropertyName, pMaxValuePropertyName);

            OSL_ENSURE((NULL == pMinValuePropertyName) == (0 == (SCA_MIN_VALUE & m_nIncludeSpecial)),
                "OControlExport::exportCommonControlAttributes: no property found for the min value attribute!");
            OSL_ENSURE((NULL == pMaxValuePropertyName) == (0 == (SCA_MAX_VALUE & m_nIncludeSpecial)),
                "OControlExport::exportCommonControlAttributes: no property found for the max value attribute!");

            
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
        
        DBG_CHECK_PROPERTY_NO_TYPE( PROPERTY_LISTSOURCE );

        OUString sListSource = getScalarListSourceValue();
        if ( !sListSource.isEmpty() )
        {   
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
        
        Sequence< OUString > aItems, aValues;
        DBG_CHECK_PROPERTY( PROPERTY_STRING_ITEM_LIST, Sequence< OUString > );
        m_xProps->getPropertyValue(PROPERTY_STRING_ITEM_LIST) >>= aItems;

        DBG_CHECK_PROPERTY( PROPERTY_LISTSOURCE, Sequence< OUString > );
        if ( 0 == ( m_nIncludeDatabase & DA_LIST_SOURCE ) )
            m_xProps->getPropertyValue(PROPERTY_LISTSOURCE) >>= aValues;
        

        
        Int16Set aSelection, aDefaultSelection;
        getSequenceInt16PropertyAsSet(PROPERTY_SELECT_SEQ, aSelection);
        getSequenceInt16PropertyAsSet(PROPERTY_DEFAULT_SELECT_SEQ, aDefaultSelection);

        
        OUString sTrue;
        OUStringBuffer sBuffer;
        ::sax::Converter::convertBool(sBuffer, true);
        sTrue = sBuffer.makeStringAndClear();

        
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
                
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCA_LABEL),
                    OAttributeMetaData::getCommonControlAttributeName(CCA_LABEL),
                    *pItems);
                ++pItems;
            }
            if (i < nValues)
            {
                
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCA_VALUE),
                    OAttributeMetaData::getCommonControlAttributeName(CCA_VALUE),
                    *pValues);
                ++pValues;
            }

            Int16Set::iterator aSelectedPos = aSelection.find(i);
            if (aSelection.end() != aSelectedPos)
            {   
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCA_CURRENT_SELECTED),
                    OAttributeMetaData::getCommonControlAttributeName(CCA_CURRENT_SELECTED),
                    sTrue
                    );
                aSelection.erase(aSelectedPos);
            }

            Int16Set::iterator aDefaultSelectedPos = aDefaultSelection.find(i);
            if (aDefaultSelection.end() != aDefaultSelectedPos)
            {   
                AddAttribute(
                    OAttributeMetaData::getCommonControlAttributeNamespace(CCA_SELECTED),
                    OAttributeMetaData::getCommonControlAttributeName(CCA_SELECTED),
                    sTrue
                    );
                aDefaultSelection.erase(aDefaultSelectedPos);
            }
            SvXMLElementExport aFormElement(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, "option", sal_True, sal_True);
        }

        
        
        
        

        if ( !aSelection.empty() || !aDefaultSelection.empty() )
        {
            sal_Int16 nLastSelected = -1;
            if ( !aSelection.empty() )
                nLastSelected = *(--aSelection.end());

            sal_Int16 nLastDefaultSelected = -1;
            if ( !aDefaultSelection.empty() )
                nLastDefaultSelected = *(--aDefaultSelection.end());

            
            sal_Int16 nLastReferredEntry = std::max(nLastSelected, nLastDefaultSelected);
            OSL_ENSURE(nLastReferredEntry >= nMaxLen, "OControlExport::exportListSourceAsElements: inconsistence!");
                
                

            for (sal_Int16 i=nMaxLen; i<=nLastReferredEntry; ++i)
            {
                if (aSelection.end() != aSelection.find(i))
                {   
                    AddAttribute(
                        OAttributeMetaData::getCommonControlAttributeNamespace(CCA_CURRENT_SELECTED),
                        OAttributeMetaData::getCommonControlAttributeName(CCA_CURRENT_SELECTED),
                        sTrue
                        );
                }

                if (aDefaultSelection.end() != aDefaultSelection.find(i))
                {   
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
        
        const sal_Char *pOuterElementName = getOuterXMLElementName();
        m_pOuterElement = pOuterElementName
                               ? new SvXMLElementExport(
                                        m_rContext.getGlobalContext(),
                                        XML_NAMESPACE_FORM,
                                        pOuterElementName, sal_True,
                                        sal_True)
                            : 0;

        
        exportInnerAttributes();

        
        OElementExport::implStartElement(_pName);
    }

    void OControlExport::implEndElement()
    {
        
        OElementExport::implEndElement();

        
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

        
        m_nClassId = FormComponentType::CONTROL;
        DBG_CHECK_PROPERTY( PROPERTY_CLASSID, sal_Int16 );
        m_xProps->getPropertyValue(PROPERTY_CLASSID) >>= m_nClassId;
        bool knownType = false;
        switch (m_nClassId)
        {
            case FormComponentType::DATEFIELD:
                m_eType = DATE;
                knownType = true;
                
            case FormComponentType::TIMEFIELD:
                if ( !knownType )
                {
                    m_eType = TIME;
                    knownType = true;
                }
                m_nIncludeSpecial |= SCA_VALIDATION;
                
            case FormComponentType::NUMERICFIELD:
            case FormComponentType::CURRENCYFIELD:
            case FormComponentType::PATTERNFIELD:
                if ( !knownType )
                {
                    m_eType = FORMATTED_TEXT;
                    knownType = true;
                }
                
            case FormComponentType::TEXTFIELD:
            {   

                if ( !knownType )
                {
                    
                    if (m_xPropertyInfo->hasPropertyByName(PROPERTY_FORMATKEY))
                    {
                        m_eType = FORMATTED_TEXT;
                    }
                    else
                    {
                        
                        

                        
                        sal_Int16 nEchoChar = 0;
                        if (m_xPropertyInfo->hasPropertyByName(PROPERTY_ECHOCHAR))
                            
                            m_xProps->getPropertyValue(PROPERTY_ECHOCHAR) >>= nEchoChar;
                        if (nEchoChar)
                        {
                            m_eType = PASSWORD;
                            m_nIncludeSpecial |= SCA_ECHO_CHAR;
                        }
                        else
                        {
                            
                            sal_Bool bMultiLine = sal_False;
                            if (m_xPropertyInfo->hasPropertyByName(PROPERTY_MULTILINE))
                                
                                bMultiLine = ::cppu::any2bool(m_xProps->getPropertyValue(PROPERTY_MULTILINE));

                            if ( bMultiLine )
                                m_eType = TEXT_AREA;
                            else
                                
                                m_eType = TEXT;
                        }
                    }
                    knownType = true;
                }

                
                
                m_nIncludeCommon =
                    CCA_NAME | CCA_SERVICE_NAME | CCA_DISABLED |
                    CCA_PRINTABLE | CCA_TAB_INDEX | CCA_TAB_STOP | CCA_TITLE;

                if  (   ( m_nClassId != FormComponentType::DATEFIELD )
                    &&  ( m_nClassId != FormComponentType::TIMEFIELD )
                    )
                    
                    m_nIncludeCommon |= CCA_VALUE;

                
                m_nIncludeDatabase = DA_DATA_FIELD | DA_INPUT_REQUIRED;

                
                m_nIncludeEvents = EA_CONTROL_EVENTS | EA_ON_CHANGE | EA_ON_SELECT;

                
                if  (   ( m_nClassId == FormComponentType::TEXTFIELD )
                    ||  ( m_nClassId == FormComponentType::PATTERNFIELD )
                    )
                    m_nIncludeDatabase |= DA_CONVERT_EMPTY;

                
                if ( m_nClassId != FormComponentType::FILECONTROL )
                    m_nIncludeCommon |= CCA_READONLY;

                
                if ( m_nClassId == FormComponentType::TEXTFIELD )
                    m_nIncludeCommon |= CCA_MAX_LENGTH;

                
                if (FORMATTED_TEXT == m_eType)
                {   
                    if  ( FormComponentType::PATTERNFIELD != m_nClassId )   
                        m_nIncludeSpecial |= SCA_MAX_VALUE | SCA_MIN_VALUE;

                    if (FormComponentType::TEXTFIELD != m_nClassId)
                        
                        m_nIncludeSpecial |= SCA_VALIDATION;
                }

                
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
                
                {
                    
                    
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
                
            case FormComponentType::IMAGEBUTTON:
                if (BUTTON != m_eType)
                {
                    
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
                
            case FormComponentType::RADIOBUTTON:
                m_nIncludeCommon =
                    CCA_NAME | CCA_SERVICE_NAME | CCA_DISABLED | CCA_LABEL | CCA_PRINTABLE |
                    CCA_TAB_INDEX | CCA_TAB_STOP | CCA_TITLE | CCA_VALUE | CCA_VISUAL_EFFECT;
                if (CHECKBOX != m_eType)
                {   
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
                

            case FormComponentType::NAVIGATIONBAR:
                
                

            case FormComponentType::CONTROL:
                m_eType = GENERIC_CONTROL;
                
                m_nIncludeCommon = CCA_NAME | CCA_SERVICE_NAME;
                    
                    
                    
                m_nIncludeEvents = EA_CONTROL_EVENTS;
                    
                break;
        }

        
        m_nIncludeCommon |= CCA_CONTROL_ID;

        
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

            
            {
                if ( aHelper.isCellRangeListSource( aHelper.getCurrentListSource( ) ) )
                    m_nIncludeBindings |= BA_LIST_CELL_RANGE;
            }
        }

        
        if( !getXFormsBindName( m_xProps ).isEmpty() )
        {
            m_nIncludeBindings |= BA_XFORMS_BIND;
        }

        
        if( !getXFormsListBindName( m_xProps ).isEmpty() )
        {
            m_nIncludeBindings |= BA_XFORMS_LISTBIND;
        }

        
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
            
            OUString sBoundFieldPropertyName( "BoundField" );
            if ( m_xPropertyInfo.is() && m_xPropertyInfo->hasPropertyByName( sBoundFieldPropertyName ) )
            {
                Reference< XPropertySet > xBoundField;
                m_xProps->getPropertyValue( sBoundFieldPropertyName ) >>= xBoundField;
                if ( xBoundField.is() )
                    return true;
            }

            
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
            
            Reference< XListEntrySink > xEntrySink( m_xProps, UNO_QUERY );
            if ( xEntrySink.is() && xEntrySink->getListEntrySource().is() )
                return false;

            if ( m_xPropertyInfo.is() && m_xPropertyInfo->hasPropertyByName( PROPERTY_LISTSOURCETYPE ) )
            {
                ListSourceType eListSourceType = ListSourceType_VALUELIST;
                OSL_VERIFY( m_xProps->getPropertyValue( PROPERTY_LISTSOURCETYPE ) >>= eListSourceType );
                if ( eListSourceType == ListSourceType_VALUELIST )
                    
                    return true;

                
                
                return getScalarListSourceValue().isEmpty();
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OControlExport::controlHasUserSuppliedListEntries: caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
        }

        OSL_FAIL( "OControlExport::controlHasUserSuppliedListEntries: unreachable code!" );
            
        return true;
    }

    
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
        
        DBG_CHECK_PROPERTY( PROPERTY_COLUMNSERVICENAME, OUString );
        OUString sColumnServiceName;
        m_xProps->getPropertyValue(PROPERTY_COLUMNSERVICENAME) >>= sColumnServiceName;
        
        
        
        sal_Int32 nLastSep = sColumnServiceName.lastIndexOf('.');
        OSL_ENSURE(-1 != nLastSep, "OColumnExport::startExportElement: invalid service name!");
        sColumnServiceName = sColumnServiceName.copy(nLastSep + 1);
        sColumnServiceName =
            m_rContext.getGlobalContext().GetNamespaceMap().GetQNameByKey(
                XML_NAMESPACE_OOO, sColumnServiceName );
        
        AddAttribute( OAttributeMetaData::getCommonControlAttributeNamespace(CCA_SERVICE_NAME)
                    , OAttributeMetaData::getCommonControlAttributeName(CCA_SERVICE_NAME)
                    , sColumnServiceName);
        
        exportedProperty(PROPERTY_COLUMNSERVICENAME);

    }

    const sal_Char* OColumnExport::getOuterXMLElementName() const
    {
        return "column";
    }

    void OColumnExport::exportAttributes()
    {
        OControlExport::exportAttributes();

        
        exportStringPropertyAttribute(
            OAttributeMetaData::getCommonControlAttributeNamespace(CCA_LABEL),
            OAttributeMetaData::getCommonControlAttributeName(CCA_LABEL),
            PROPERTY_LABEL);

        
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

        
        m_nIncludeCommon &= ~(CCA_FOR | CCA_PRINTABLE | CCA_TAB_INDEX | CCA_TAB_STOP | CCA_LABEL);
        m_nIncludeSpecial &= ~(SCA_ECHO_CHAR | SCA_AUTOMATIC_COMPLETION | SCA_MULTIPLE | SCA_MULTI_LINE);

        if (FormComponentType::DATEFIELD != m_nClassId)
            
            m_nIncludeCommon &= ~CCA_DROPDOWN;
    }

    
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
            m_xProps->getPropertyValue( PROPERTY_DATASOURCENAME ) >>= sPropValue; 
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

        
        OElementExport::exportSubTags();
        
        Reference< XIndexAccess > xCollection(m_xProps, UNO_QUERY);
        OSL_ENSURE(xCollection.is(), "OFormLayerXMLExport::implExportForm: a form which is not an index access? Suspic�ous!");

        if (xCollection.is())
            m_rContext.exportCollectionElements(xCollection);
    }

    void OFormExport::exportAttributes()
    {
        sal_Int32 i=0;

        
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

        
        exportServiceNameAttribute();
        
        exportTargetFrameAttribute();
        
        exportTargetLocationAttribute(true);    

        
        exportStringSequenceAttribute(
            OAttributeMetaData::getFormAttributeNamespace(faMasterFields),
            OAttributeMetaData::getFormAttributeName(faMasterFields),
            PROPERTY_MASTERFIELDS);
        
        exportStringSequenceAttribute(
            OAttributeMetaData::getFormAttributeNamespace(faDetailFiels),
            OAttributeMetaData::getFormAttributeName(faDetailFiels),
            PROPERTY_DETAILFIELDS);
    }
}   

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
