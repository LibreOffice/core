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

#include <tools/debug.hxx>

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_CLICKACTION_HPP_
#include <com/sun/star/presentation/ClickAction.hpp>
#endif

#ifndef _URLOBJ_HXX 
#include <tools/urlobj.hxx>
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif


#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_EVENTIMP_HXX
#include "eventimp.hxx"
#endif

#ifndef _XMLOFF_ANIM_HXX
#include "anim.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
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
using namespace ::binfilter::xmloff::token;

///////////////////////////////////////////////////////////////////////

SvXMLEnumMapEntry __READONLY_DATA aXML_EventActions_EnumMap[] =
{
    { XML_NONE,			    ClickAction_NONE	},
    { XML_PREVIOUS_PAGE,	ClickAction_PREVPAGE },
    { XML_NEXT_PAGE,		ClickAction_NEXTPAGE },
    { XML_FIRST_PAGE,		ClickAction_FIRSTPAGE },
    { XML_LAST_PAGE,		ClickAction_LASTPAGE },
    { XML_HIDE,			    ClickAction_INVISIBLE },
    { XML_STOP,			    ClickAction_STOPPRESENTATION },
    { XML_EXECUTE,			ClickAction_PROGRAM },
    { XML_SHOW,			    ClickAction_BOOKMARK },
    { XML_SHOW,			    ClickAction_DOCUMENT },
    { XML_EXECUTE_MACRO,	ClickAction_MACRO },
    { XML_VERB,			    ClickAction_VERB },
    { XML_FADE_OUT,		    ClickAction_VANISH },
    { XML_SOUND,			ClickAction_SOUND },
    { XML_TOKEN_INVALID, 0 }
};

///////////////////////////////////////////////////////////////////////

class SdXMLEventContext : public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxShape;

public:
    TYPEINFO();

    SdXMLEventContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, const Reference< XAttributeList>& xAttrList, const Reference< XShape >& rxShape );
    virtual ~SdXMLEventContext();

    virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const OUString& rLocalName,	const Reference< XAttributeList>& xAttrList );
    virtual void EndElement();

    sal_Bool mbValid;
    sal_Bool mbScript;
    ClickAction meClickAction;
    XMLEffect meEffect;
    XMLEffectDirection meDirection;
    sal_Int16 mnStartScale;
    AnimationSpeed meSpeed;
    sal_Int32 mnVerb;
    OUString msSoundURL;
    sal_Bool mbPlayFull;
    OUString msEventName;
    OUString msLanguage;
    OUString msMacroName;
    OUString msLibrary;
    OUString msBookmark;
};

///////////////////////////////////////////////////////////////////////

class XMLEventSoundContext : public SvXMLImportContext
{
    SdXMLEventContext*	mpParent;

public:
    TYPEINFO();

    XMLEventSoundContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, SdXMLEventContext* pParent );
    virtual ~XMLEventSoundContext();
};

TYPEINIT1( XMLEventSoundContext, SvXMLImportContext );

XMLEventSoundContext::XMLEventSoundContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, SdXMLEventContext* pParent )
: SvXMLImportContext( rImport, nPrfx, rLocalName ), mpParent( pParent )
{
    if( mpParent && nPrfx == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_SOUND ) )
    {
        const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for(sal_Int16 i=0; i < nAttrCount; i++)
        {
            OUString sAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
            OUString sValue = xAttrList->getValueByIndex( i );

            switch( nPrefix )
            {
            case XML_NAMESPACE_XLINK:
                if( IsXMLToken( aLocalName, XML_HREF ) )
                {
                    mpParent->msSoundURL = rImport.GetAbsoluteReference(sValue);
                }
                break;
            case XML_NAMESPACE_PRESENTATION:
                if( IsXMLToken( aLocalName, XML_PLAY_FULL ) )
                {
                    mpParent->mbPlayFull = IsXMLToken( sValue, XML_TRUE );
                }
            }
        }
    }
}

XMLEventSoundContext::~XMLEventSoundContext()
{
}

///////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLEventContext, SvXMLImportContext );

SdXMLEventContext::SdXMLEventContext( SvXMLImport& rImport,  sal_uInt16 nPrfx, const OUString& rLocalName,  const Reference< XAttributeList >& xAttrList, const Reference< XShape >& rxShape )
:	SvXMLImportContext(rImport, nPrfx, rLocalName),
    mxShape( rxShape ), mbScript( sal_False ), meClickAction( ClickAction_NONE ),
    meEffect( EK_none ), meDirection( ED_none ), mnStartScale( 100 ),
    meSpeed( AnimationSpeed_MEDIUM ), mnVerb(0), mbPlayFull( sal_False )
{
    const OUString msXMLEventName( RTL_CONSTASCII_USTRINGPARAM( "on-click" ) );

    if( nPrfx == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_EVENT ) )
    {
        mbValid = sal_True;
    }
    else if( nPrfx == XML_NAMESPACE_SCRIPT && IsXMLToken( rLocalName, XML_EVENT ) )
    {
        mbScript = sal_True;
        mbValid = sal_True;
    }
    else
    {
        return;
    }

    // read attributes
    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; (i < nAttrCount) && mbValid; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );

        switch( nPrefix )
        {
        case XML_NAMESPACE_PRESENTATION:
            if( IsXMLToken( aLocalName, XML_ACTION ) )
            {
                USHORT eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_EventActions_EnumMap ) )
                    meClickAction = (ClickAction)eEnum;
            }
            if( IsXMLToken( aLocalName, XML_EFFECT ) )
            {
                USHORT eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_AnimationEffect_EnumMap ) )
                    meEffect = (XMLEffect)eEnum;
            }
            else if( IsXMLToken( aLocalName, XML_DIRECTION ) )
            {
                USHORT eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_AnimationDirection_EnumMap ) )
                    meDirection = (XMLEffectDirection)eEnum;
            }
            else if( IsXMLToken( aLocalName, XML_START_SCALE ) )
            {
                sal_Int32 nScale;
                if( SvXMLUnitConverter::convertPercent( nScale, sValue ) )
                    mnStartScale = (sal_Int16)nScale;
            }
            else if( IsXMLToken( aLocalName, XML_SPEED ) )
            {
                USHORT eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_AnimationSpeed_EnumMap ) )
                    meSpeed = (AnimationSpeed)eEnum;
            }
            else if( IsXMLToken( aLocalName, XML_VERB ) )
            {
                SvXMLUnitConverter::convertNumber( mnVerb, sValue );
            }
            break;

        case XML_NAMESPACE_SCRIPT:
            if( IsXMLToken( aLocalName, XML_EVENT_NAME ) )
            {
                msEventName = sValue;
                mbValid = msEventName == msXMLEventName;
            }
            else if( IsXMLToken( aLocalName, XML_LANGUAGE ) )
            {
                msLanguage = sValue;
            }
            else if( IsXMLToken( aLocalName, XML_MACRO_NAME ) )
            {
                msMacroName = sValue;
            }
            else if( IsXMLToken( aLocalName, XML_LIBRARY ) )
            {
                msLibrary = sValue;
            }
            break;

        case XML_NAMESPACE_XLINK:
            if( IsXMLToken( aLocalName, XML_HREF ) )
            {
                const rtl::OUString &rTmp( rImport.GetAbsoluteReference(sValue) );
                INetURLObject::translateToInternal( rTmp, msBookmark, INetURLObject::DECODE_UNAMBIGUOUS, RTL_TEXTENCODING_UTF8 );
            }
            break;
        }
    }

    if( mbValid )
        mbValid = msEventName.getLength() != 0;
}

SdXMLEventContext::~SdXMLEventContext()
{
}

SvXMLImportContext * SdXMLEventContext::CreateChildContext( USHORT nPrefix, const OUString& rLocalName, const Reference< XAttributeList>& xAttrList )
{
    return new XMLEventSoundContext( GetImport(), nPrefix, rLocalName, xAttrList, this );
}

void SdXMLEventContext::EndElement()
{
    const OUString msAPIEventName( RTL_CONSTASCII_USTRINGPARAM( "OnClick" ) );

    if( !mbValid )
        return;

    do
    {
        Reference< XEventsSupplier > xEventsSupplier( mxShape, UNO_QUERY );
        if( !xEventsSupplier.is() )
            break;

        Reference< XNameReplace > xEvents( xEventsSupplier->getEvents() );
        DBG_ASSERT( xEvents.is(), "XEventsSupplier::getEvents() returned NULL" );
        if( !xEvents.is() )
            break;

        if( !xEvents->hasByName( msAPIEventName ) )
            break;

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
        case ClickAction_MACRO:
            nPropertyCount += 1;
            break;

        case ClickAction_SOUND:
            nPropertyCount += 2;
            break;

        case ClickAction_VANISH:
            nPropertyCount += 4;
            break;
        }

        uno::Sequence< beans::PropertyValue > aProperties( nPropertyCount );
        beans::PropertyValue* pProperties = aProperties.getArray();

        if( ClickAction_MACRO == meClickAction )
        {
            pProperties->Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "EventType" ) );
            pProperties->Handle = -1;
            pProperties->Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM("StarBasic") );
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            pProperties->Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "MacroName" ) );
            pProperties->Handle = -1;
            pProperties->Value <<= msMacroName;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            pProperties->Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Library" ) );
            pProperties->Handle = -1;
            pProperties->Value <<= msLibrary;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
        }
        else
        {
            pProperties->Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "EventType" ) );
            pProperties->Handle = -1;
            pProperties->Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM("Presentation") );
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            // ClickAction_BOOKMARK and ClickAction_DOCUMENT share the same xml event
            // so check here if its realy a bookmark or maybe a document
            if( meClickAction == ClickAction_BOOKMARK )
            {
                if( msBookmark.compareToAscii( "#", 1 ) != 0 )
                    meClickAction = ClickAction_DOCUMENT;
            }

            pProperties->Name = OUString( RTL_CONSTASCII_USTRINGPARAM("ClickAction") );
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

                // Note: no break here!!!

            case ClickAction_DOCUMENT:
            case ClickAction_PROGRAM:
                pProperties->Name = OUString( RTL_CONSTASCII_USTRINGPARAM("Bookmark") );
                pProperties->Handle = -1;
                pProperties->Value <<= msBookmark;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
                break;

            case ClickAction_VANISH:
                pProperties->Name = OUString( RTL_CONSTASCII_USTRINGPARAM("Effect") );
                pProperties->Handle = -1;
                pProperties->Value <<= ImplSdXMLgetEffect( meEffect, meDirection, mnStartScale, sal_True );
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
                pProperties++;

                pProperties->Name = OUString( RTL_CONSTASCII_USTRINGPARAM("Speed") );
                pProperties->Handle = -1;
                pProperties->Value <<= meSpeed;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
                pProperties++;

                // NOTE: no break here!!!

            case ClickAction_SOUND:
                pProperties->Name = OUString( RTL_CONSTASCII_USTRINGPARAM("SoundURL") );
                pProperties->Handle = -1;
                pProperties->Value <<= msSoundURL;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
                pProperties++;

                pProperties->Name = OUString( RTL_CONSTASCII_USTRINGPARAM("PlayFull") );
                pProperties->Handle = -1;
                pProperties->Value = ::cppu::bool2any(mbPlayFull);
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
                break;

            case ClickAction_VERB:
                pProperties->Name = OUString( RTL_CONSTASCII_USTRINGPARAM("Verb") );
                pProperties->Handle = -1;
                pProperties->Value <<= mnVerb;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
                break;
            }
        }

        uno::Any aAny;
        aAny <<= aProperties;
        xEvents->replaceByName( msAPIEventName, aAny );

    } while(0);
}

///////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLEventsContext, SvXMLImportContext );

SdXMLEventsContext::SdXMLEventsContext( SvXMLImport& rImport, sal_uInt16 nPrfx,	const OUString& rLocalName,
        const Reference< XAttributeList>& xAttrList, const Reference< XShape >& rxShape)
: SvXMLImportContext(rImport, nPrfx, rLocalName), mxShape( rxShape )
{
}

SdXMLEventsContext::~SdXMLEventsContext()
{
}

SvXMLImportContext * SdXMLEventsContext::CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    return new SdXMLEventContext( GetImport(), nPrefix, rLocalName,  xAttrList, mxShape );
}
}//end of namespace binfilter
