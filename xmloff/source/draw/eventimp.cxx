/*************************************************************************
 *
 *  $RCSfile: eventimp.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:08:42 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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

#include <tools/debug.hxx>

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
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

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
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
using namespace ::xmloff::token;

///////////////////////////////////////////////////////////////////////

SvXMLEnumMapEntry __READONLY_DATA aXML_EventActions_EnumMap[] =
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
    { XML_TOKEN_INVALID, 0 }
};

///////////////////////////////////////////////////////////////////////

class SdXMLEventContext : public SvXMLImportContext
{
private:
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mxShape;

public:
    TYPEINFO();

    SdXMLEventContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, const Reference< XAttributeList>& xAttrList, const Reference< XShape >& rxShape );
    virtual ~SdXMLEventContext();

    virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const OUString& rLocalName,    const Reference< XAttributeList>& xAttrList );
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
    OUString msMacroName;
    OUString msBookmark;
    OUString msLanguage;
};

///////////////////////////////////////////////////////////////////////

class XMLEventSoundContext : public SvXMLImportContext
{
    SdXMLEventContext*  mpParent;

public:
    TYPEINFO();

    XMLEventSoundContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, SdXMLEventContext* pParent );
    virtual ~XMLEventSoundContext();
};

TYPEINIT1( XMLEventSoundContext, SvXMLImportContext );

XMLEventSoundContext::XMLEventSoundContext( SvXMLImport& rImp, sal_uInt16 nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, SdXMLEventContext* pParent )
: SvXMLImportContext( rImp, nPrfx, rLocalName ), mpParent( pParent )
{
    if( mpParent && nPrfx == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_SOUND ) )
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
                    mpParent->msSoundURL = rImp.GetAbsoluteReference(sValue);
                }
                break;
            case XML_NAMESPACE_PRESENTATION:
                if( IsXMLToken( aAttrLocalName, XML_PLAY_FULL ) )
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

SdXMLEventContext::SdXMLEventContext( SvXMLImport& rImp,  sal_uInt16 nPrfx, const OUString& rLocalName,  const Reference< XAttributeList >& xAttrList, const Reference< XShape >& rxShape )
:   SvXMLImportContext(rImp, nPrfx, rLocalName),
    mxShape( rxShape ), mbScript( sal_False ), meClickAction( ClickAction_NONE ),
    meEffect( EK_none ), meDirection( ED_none ), mnStartScale( 100 ),
    meSpeed( AnimationSpeed_MEDIUM ), mnVerb(0), mbPlayFull( sal_False )
{
    const OUString msXMLEventName( RTL_CONSTASCII_USTRINGPARAM( "click" ) );

    if( nPrfx == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_EVENT_LISTENER ) )
    {
        mbValid = sal_True;
    }
    else if( nPrfx == XML_NAMESPACE_SCRIPT && IsXMLToken( rLocalName, XML_EVENT_LISTENER ) )
    {
        mbScript = sal_True;
        mbValid = sal_True;
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
                USHORT eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_EventActions_EnumMap ) )
                    meClickAction = (ClickAction)eEnum;
            }
            if( IsXMLToken( aAttrLocalName, XML_EFFECT ) )
            {
                USHORT eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_AnimationEffect_EnumMap ) )
                    meEffect = (XMLEffect)eEnum;
            }
            else if( IsXMLToken( aAttrLocalName, XML_DIRECTION ) )
            {
                USHORT eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_AnimationDirection_EnumMap ) )
                    meDirection = (XMLEffectDirection)eEnum;
            }
            else if( IsXMLToken( aAttrLocalName, XML_START_SCALE ) )
            {
                sal_Int32 nScale;
                if( SvXMLUnitConverter::convertPercent( nScale, sValue ) )
                    mnStartScale = (sal_Int16)nScale;
            }
            else if( IsXMLToken( aAttrLocalName, XML_SPEED ) )
            {
                USHORT eEnum;
                if( SvXMLUnitConverter::convertEnum( eEnum, sValue, aXML_AnimationSpeed_EnumMap ) )
                    meSpeed = (AnimationSpeed)eEnum;
            }
            else if( IsXMLToken( aAttrLocalName, XML_VERB ) )
            {
                SvXMLUnitConverter::convertNumber( mnVerb, sValue );
            }
            break;

        case XML_NAMESPACE_SCRIPT:
            if( IsXMLToken( aAttrLocalName, XML_EVENT_NAME ) )
            {
                sEventName = sValue;
                sal_uInt16 nScriptPrefix =
                    GetImport().GetNamespaceMap().GetKeyByAttrName( sValue, &sEventName );
                mbValid = XML_NAMESPACE_DOM == nScriptPrefix && sEventName == msXMLEventName;
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
//          else if( IsXMLToken( aLocalName, XML_LIBRARY ) )
//          {
//              msLibrary = sValue;
//          }
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
                    const UniString aTmp( rImp.GetAbsoluteReference(sValue) );
                    UniString aTmp2;
                    INetURLObject::translateToInternal( aTmp, aTmp2, INetURLObject::WAS_ENCODED, INetURLObject::DECODE_UNAMBIGUOUS, RTL_TEXTENCODING_UTF8 );
                    msBookmark = aTmp2;
                }
            }
            break;
        }
    }

    if( mbValid )
        mbValid = sEventName.getLength() != 0;
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
            nPropertyCount += 1;
            break;
        case ClickAction_MACRO:
            if ( msLanguage.equalsIgnoreAsciiCaseAscii("starbasic") )
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
            if ( msLanguage.equalsIgnoreAsciiCaseAscii("starbasic") )
            {
                OUString sLibrary;
                const OUString& rApp = GetXMLToken( XML_APPLICATION );
                const OUString& rDoc = GetXMLToken( XML_DOCUMENT );
                if( msMacroName.getLength() > rApp.getLength()+1 &&
                    msMacroName.copy(0,rApp.getLength()).equalsIgnoreAsciiCase( rApp ) &&
                    ':' == msMacroName[rApp.getLength()] )
                {
                    sLibrary = OUString(RTL_CONSTASCII_USTRINGPARAM("StarOffice"));
                    msMacroName = msMacroName.copy( rApp.getLength()+1 );
                }
                else if( msMacroName.getLength() > rDoc.getLength()+1 &&
                    msMacroName.copy(0,rDoc.getLength()).equalsIgnoreAsciiCase( rDoc ) &&
                    ':' == msMacroName[rDoc.getLength()] )
                {
                    sLibrary = rDoc;
                    msMacroName = msMacroName.copy( rDoc.getLength()+1 );
                }

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
                pProperties->Value <<= sLibrary;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
            }
            else
            {
                pProperties->Name =
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "EventType" ) );
                pProperties->Handle = -1;
                pProperties->Value <<= OUString(
                    RTL_CONSTASCII_USTRINGPARAM("Script") );
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
                pProperties++;

                pProperties->Name = OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "Script" ) );
                pProperties->Handle = -1;
                pProperties->Value <<= msMacroName;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
            }
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

SdXMLEventsContext::SdXMLEventsContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName,
        const Reference< XAttributeList>& xAttrList, const Reference< XShape >& rxShape)
: SvXMLImportContext(rImport, nPrfx, rLocalName), mxShape( rxShape )
{
}

SdXMLEventsContext::~SdXMLEventsContext()
{
}

SvXMLImportContext * SdXMLEventsContext::CreateChildContext( USHORT nPrfx, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    return new SdXMLEventContext( GetImport(), nPrfx, rLocalName,  xAttrList, mxShape );
}
