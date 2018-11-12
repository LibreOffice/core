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

#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <tools/urlobj.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include "eventimp.hxx"
#include <anim.hxx>

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::presentation;
using namespace ::xmloff::token;

SvXMLEnumMapEntry<ClickAction> const aXML_EventActions_EnumMap[] =
{
    { XML_NONE,             ClickAction_NONE    },
    { XML_PREVIOUS_PAGE,    ClickAction_PREVPAGE },
    { XML_NEXT_PAGE,        ClickAction_NEXTPAGE },
    { XML_FIRST_PAGE,       ClickAction_FIRSTPAGE },
    { XML_LAST_PAGE,        ClickAction_LASTPAGE },
    { XML_HIDE,             ClickAction_INVISIBLE },
    { XML_STOP,             ClickAction_STOPPRESENTATION },
    { XML_EXECUTE,          ClickAction_PROGRAM },
    { XML_SHOW,             ClickAction_BOOKMARK },
    { XML_SHOW,             ClickAction_DOCUMENT },
    { XML_EXECUTE_MACRO,    ClickAction_MACRO },
    { XML_VERB,             ClickAction_VERB },
    { XML_FADE_OUT,         ClickAction_VANISH },
    { XML_SOUND,            ClickAction_SOUND },
    { XML_TOKEN_INVALID, ClickAction(0) }
};

class SdXMLEventContext : public SvXMLImportContext
{
private:
    css::uno::Reference< css::drawing::XShape > mxShape;

public:

    SdXMLEventContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, const Reference< XAttributeList>& xAttrList, const Reference< XShape >& rxShape );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,    const Reference< XAttributeList>& xAttrList ) override;
    virtual void EndElement() override;

    bool mbValid;
    bool mbScript;
    ClickAction meClickAction;
    XMLEffect meEffect;
    XMLEffectDirection meDirection;
    sal_Int16 mnStartScale;
    AnimationSpeed meSpeed;
    sal_Int32 mnVerb;
    OUString msSoundURL;
    bool mbPlayFull;
    OUString msMacroName;
    OUString msBookmark;
    OUString msLanguage;
};

class XMLEventSoundContext : public SvXMLImportContext
{
public:

    XMLEventSoundContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, SdXMLEventContext* pParent );
};


XMLEventSoundContext::XMLEventSoundContext( SvXMLImport& rImp, sal_uInt16 nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, SdXMLEventContext* pParent )
: SvXMLImportContext( rImp, nPrfx, rLocalName )
{
    if( pParent && nPrfx == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_SOUND ) )
    {
        const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for(sal_Int16 i=0; i < nAttrCount; i++)
        {
            OUString sAttrName = xAttrList->getNameByIndex( i );
            OUString aAttrLocalName;
            sal_uInt16 nAttrPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aAttrLocalName );
            OUString sValue = xAttrList->getValueByIndex( i );

            switch( nAttrPrefix )
            {
            case XML_NAMESPACE_XLINK:
                if( IsXMLToken( aAttrLocalName, XML_HREF ) )
                {
                    pParent->msSoundURL = rImp.GetAbsoluteReference(sValue);
                }
                break;
            case XML_NAMESPACE_PRESENTATION:
                if( IsXMLToken( aAttrLocalName, XML_PLAY_FULL ) )
                {
                    pParent->mbPlayFull = IsXMLToken( sValue, XML_TRUE );
                }
            }
        }
    }
}

SdXMLEventContext::SdXMLEventContext( SvXMLImport& rImp,  sal_uInt16 nPrfx, const OUString& rLocalName,  const Reference< XAttributeList >& xAttrList, const Reference< XShape >& rxShape )
    : SvXMLImportContext(rImp, nPrfx, rLocalName)
    , mxShape(rxShape), mbValid(false), mbScript(false)
    , meClickAction(ClickAction_NONE), meEffect(EK_none)
    , meDirection(ED_none), mnStartScale(100), meSpeed(AnimationSpeed_MEDIUM)
    , mnVerb(0), mbPlayFull(false)
{
    if( nPrfx == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_EVENT_LISTENER ) )
    {
        mbValid = true;
    }
    else if( nPrfx == XML_NAMESPACE_SCRIPT && IsXMLToken( rLocalName, XML_EVENT_LISTENER ) )
    {
        mbScript = true;
        mbValid = true;
    }
    else
    {
        return;
    }

    // read attributes
    OUString sEventName;
    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; (i < nAttrCount) && mbValid; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aAttrLocalName;
        sal_uInt16 nAttrPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aAttrLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );

        switch( nAttrPrefix )
        {
        case XML_NAMESPACE_PRESENTATION:
            if( IsXMLToken( aAttrLocalName, XML_ACTION ) )
            {
                SvXMLUnitConverter::convertEnum( meClickAction, sValue, aXML_EventActions_EnumMap );
            }
            if( IsXMLToken( aAttrLocalName, XML_EFFECT ) )
            {
                SvXMLUnitConverter::convertEnum( meEffect, sValue, aXML_AnimationEffect_EnumMap );
            }
            else if( IsXMLToken( aAttrLocalName, XML_DIRECTION ) )
            {
                SvXMLUnitConverter::convertEnum( meDirection, sValue, aXML_AnimationDirection_EnumMap );
            }
            else if( IsXMLToken( aAttrLocalName, XML_START_SCALE ) )
            {
                sal_Int32 nScale;
                if (::sax::Converter::convertPercent( nScale, sValue ))
                    mnStartScale = static_cast<sal_Int16>(nScale);
            }
            else if( IsXMLToken( aAttrLocalName, XML_SPEED ) )
            {
                SvXMLUnitConverter::convertEnum( meSpeed, sValue, aXML_AnimationSpeed_EnumMap );
            }
            else if( IsXMLToken( aAttrLocalName, XML_VERB ) )
            {
                ::sax::Converter::convertNumber( mnVerb, sValue );
            }
            break;

        case XML_NAMESPACE_SCRIPT:
            if( IsXMLToken( aAttrLocalName, XML_EVENT_NAME ) )
            {
                sEventName = sValue;
                sal_uInt16 nScriptPrefix =
                    GetImport().GetNamespaceMap().GetKeyByAttrName( sValue, &sEventName );
                mbValid = XML_NAMESPACE_DOM == nScriptPrefix && sEventName == "click";
            }
            else if( IsXMLToken( aAttrLocalName, XML_LANGUAGE ) )
            {
                // language is not evaluated!
                OUString aScriptLanguage;
                msLanguage = sValue;
                sal_uInt16 nScriptPrefix = rImp.GetNamespaceMap().
                    GetKeyByAttrName( msLanguage, &aScriptLanguage );
                if( XML_NAMESPACE_OOO == nScriptPrefix )
                    msLanguage = aScriptLanguage;
            }
            else if( IsXMLToken( aAttrLocalName, XML_MACRO_NAME ) )
            {
                msMacroName = sValue;
            }
            break;

        case XML_NAMESPACE_XLINK:
            if( IsXMLToken( aAttrLocalName, XML_HREF ) )
            {
                if ( mbScript )
                {
                    msMacroName = sValue;
                }
                else
                {
                    const OUString &rTmp =
                        rImp.GetAbsoluteReference(sValue);
                    INetURLObject::translateToInternal( rTmp, msBookmark,
                        INetURLObject::DecodeMechanism::Unambiguous );
                }
            }
            break;
        }
    }

    if( mbValid )
        mbValid = !sEventName.isEmpty();
}

SvXMLImportContextRef SdXMLEventContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList>& xAttrList )
{
    return new XMLEventSoundContext( GetImport(), nPrefix, rLocalName, xAttrList, this );
}

void SdXMLEventContext::EndElement()
{
    if( !mbValid )
        return;

    do
    {
        Reference< XEventsSupplier > xEventsSupplier( mxShape, UNO_QUERY );
        if( !xEventsSupplier.is() )
            break;

        Reference< XNameReplace > xEvents( xEventsSupplier->getEvents() );
        SAL_WARN_IF( !xEvents.is(), "xmloff", "XEventsSupplier::getEvents() returned NULL" );
        if( !xEvents.is() )
            break;

        OUString sAPIEventName;
        uno::Sequence< beans::PropertyValue > aProperties;

        sAPIEventName = "OnClick";

        if( mbScript )
            meClickAction = ClickAction_MACRO;

        sal_Int32 nPropertyCount = 2;
        switch( meClickAction )
        {
            case ClickAction_NONE:
            case ClickAction_PREVPAGE:
            case ClickAction_NEXTPAGE:
            case ClickAction_FIRSTPAGE:
            case ClickAction_LASTPAGE:
            case ClickAction_INVISIBLE:
            case ClickAction_STOPPRESENTATION:
                break;
            case ClickAction_PROGRAM:
            case ClickAction_VERB:
            case ClickAction_BOOKMARK:
            case ClickAction_DOCUMENT:
                nPropertyCount += 1;
                break;
            case ClickAction_MACRO:
                if ( msLanguage.equalsIgnoreAsciiCase("starbasic") )
                    nPropertyCount += 1;
                break;

            case ClickAction_SOUND:
                nPropertyCount += 2;
                break;

            case ClickAction_VANISH:
                nPropertyCount += 4;
                break;
            default:
                break;
        }

        aProperties.realloc( nPropertyCount );
        beans::PropertyValue* pProperties = aProperties.getArray();

        if( ClickAction_MACRO == meClickAction )
        {
            if ( msLanguage.equalsIgnoreAsciiCase("starbasic") )
            {
                OUString sLibrary;
                const OUString& rApp = GetXMLToken( XML_APPLICATION );
                const OUString& rDoc = GetXMLToken( XML_DOCUMENT );
                if( msMacroName.getLength() > rApp.getLength()+1 &&
                    msMacroName.copy(0,rApp.getLength()).equalsIgnoreAsciiCase( rApp ) &&
                    ':' == msMacroName[rApp.getLength()] )
                {
                    sLibrary = "StarOffice";
                    msMacroName = msMacroName.copy( rApp.getLength()+1 );
                }
                else if( msMacroName.getLength() > rDoc.getLength()+1 &&
                    msMacroName.copy(0,rDoc.getLength()).equalsIgnoreAsciiCase( rDoc ) &&
                    ':' == msMacroName[rDoc.getLength()] )
                {
                    sLibrary = rDoc;
                    msMacroName = msMacroName.copy( rDoc.getLength()+1 );
                }

                pProperties->Name = "EventType";
                pProperties->Handle = -1;
                pProperties->Value <<= OUString( "StarBasic" );
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
                pProperties++;

                pProperties->Name = "MacroName";
                pProperties->Handle = -1;
                pProperties->Value <<= msMacroName;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
                pProperties++;

                pProperties->Name = "Library";
                pProperties->Handle = -1;
                pProperties->Value <<= sLibrary;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
            }
            else
            {
                pProperties->Name = "EventType";
                pProperties->Handle = -1;
                pProperties->Value <<= OUString( "Script" );
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
                pProperties++;

                pProperties->Name = "Script";
                pProperties->Handle = -1;
                pProperties->Value <<= msMacroName;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
            }
        }
        else
        {
            pProperties->Name = "EventType";
            pProperties->Handle = -1;
            pProperties->Value <<= OUString( "Presentation" );
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            // ClickAction_BOOKMARK and ClickAction_DOCUMENT share the same xml event
            // so check here if its really a bookmark or maybe a document
            if( meClickAction == ClickAction_BOOKMARK )
            {
                if( !msBookmark.startsWith( "#" ) )
                    meClickAction = ClickAction_DOCUMENT;
            }

            pProperties->Name = "ClickAction";
            pProperties->Handle = -1;
            pProperties->Value <<= meClickAction;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            switch( meClickAction )
            {
                case ClickAction_NONE:
                case ClickAction_PREVPAGE:
                case ClickAction_NEXTPAGE:
                case ClickAction_FIRSTPAGE:
                case ClickAction_LASTPAGE:
                case ClickAction_INVISIBLE:
                case ClickAction_STOPPRESENTATION:
                    break;

                case ClickAction_BOOKMARK:
                    msBookmark = msBookmark.copy(1);

                    [[fallthrough]];

                case ClickAction_DOCUMENT:
                case ClickAction_PROGRAM:
                    pProperties->Name = "Bookmark";
                    pProperties->Handle = -1;
                    pProperties->Value <<= msBookmark;
                    pProperties->State = beans::PropertyState_DIRECT_VALUE;
                    break;

                case ClickAction_VANISH:
                    pProperties->Name = "Effect";
                    pProperties->Handle = -1;
                    pProperties->Value <<= ImplSdXMLgetEffect( meEffect, meDirection, mnStartScale, true );
                    pProperties->State = beans::PropertyState_DIRECT_VALUE;
                    pProperties++;

                    pProperties->Name = "Speed";
                    pProperties->Handle = -1;
                    pProperties->Value <<= meSpeed;
                    pProperties->State = beans::PropertyState_DIRECT_VALUE;
                    pProperties++;

                    [[fallthrough]];

                case ClickAction_SOUND:
                    pProperties->Name = "SoundURL";
                    pProperties->Handle = -1;
                    pProperties->Value <<= msSoundURL;
                    pProperties->State = beans::PropertyState_DIRECT_VALUE;
                    pProperties++;

                    pProperties->Name = "PlayFull";
                    pProperties->Handle = -1;
                    pProperties->Value <<= mbPlayFull;
                    pProperties->State = beans::PropertyState_DIRECT_VALUE;
                    break;

                case ClickAction_VERB:
                    pProperties->Name = "Verb";
                    pProperties->Handle = -1;
                    pProperties->Value <<= mnVerb;
                    pProperties->State = beans::PropertyState_DIRECT_VALUE;
                    break;
                case ClickAction_MACRO:
                    OSL_FAIL("xmloff::SdXMLEventContext::EndElement(), ClickAction_MACRO must be handled in different if case");
                    break;
                default:
                    break;
            }
        }
        xEvents->replaceByName( sAPIEventName, uno::Any( aProperties ) );

    } while(false);
}


SdXMLEventsContext::SdXMLEventsContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName,
        const Reference< XAttributeList>&, const Reference< XShape >& rxShape)
: SvXMLImportContext(rImport, nPrfx, rLocalName), mxShape( rxShape )
{
}

SdXMLEventsContext::~SdXMLEventsContext()
{
}

SvXMLImportContextRef SdXMLEventsContext::CreateChildContext( sal_uInt16 nPrfx, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList )
{
    return new SdXMLEventContext( GetImport(), nPrfx, rLocalName,  xAttrList, mxShape );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
