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

#include <svtools/insdlg.hxx>
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>

#include <unotools/configmgr.hxx>
#include <comphelper/classids.hxx>
#include <sot/stg.hxx>
#include <sal/macros.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <com/sun/star/container/XNameAccess.hpp>

#if defined _WIN32
#include <prewin.h>
#include <oleidl.h>
#include <postwin.h>
#else
typedef Size SIZEL;
typedef Point POINTL;
#endif

using namespace ::com::sun::star;

// this struct conforms to the Microsoft
// OBJECTDESCRIPTOR -> see oleidl.h
// (MS platform sdk)

namespace {

struct OleObjectDescriptor
{
    sal_uInt32  cbSize;
    ClsId       clsid;
    sal_uInt32  dwDrawAspect;
    SIZEL       sizel;
    POINTL      pointl;
    sal_uInt32  dwStatus;
    sal_uInt32  dwFullUserTypeName;
    sal_uInt32  dwSrcOfCopy;
};

#if defined _WIN32
static_assert(sizeof(OleObjectDescriptor) == sizeof(OBJECTDESCRIPTOR));
// check the two fields that we use here
static_assert(offsetof(OleObjectDescriptor, dwFullUserTypeName)
              == offsetof(OBJECTDESCRIPTOR, dwFullUserTypeName));
static_assert(offsetof(OleObjectDescriptor, dwSrcOfCopy)
              == offsetof(OBJECTDESCRIPTOR, dwSrcOfCopy));
#endif

}

/********************** SvObjectServerList ********************************
**************************************************************************/

const SvObjectServer * SvObjectServerList::Get( std::u16string_view rHumanName ) const
{
    for(const auto & i : aObjectServerList)
    {
        if( rHumanName == i.GetHumanName() )
            return &i;
    }
    return nullptr;
}

const SvObjectServer * SvObjectServerList::Get( const SvGlobalName & rName ) const
{
    for(const auto & i : aObjectServerList)
    {
        if( rName == i.GetClassName() )
            return &i;
    }
    return nullptr;
}

void SvObjectServerList::Remove( const SvGlobalName & rName )
{
    std::erase_if(aObjectServerList, [rName](const SvObjectServer& rServer) { return rServer.GetClassName() == rName; });
}


void SvObjectServerList::FillInsertObjects()
/* [Description]

    The list is filled with all types which can be selected in the insert-dialog.
*/
{
    try{
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();

        uno::Reference< lang::XMultiServiceFactory > sProviderMSFactory =
            configuration::theDefaultProvider::get(xContext);

        uno::Sequence<uno::Any> aArguments(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", uno::Any(u"/org.openoffice.Office.Embedding/ObjectNames"_ustr)}
        }));
        uno::Reference< container::XNameAccess > xNameAccess(
            sProviderMSFactory->createInstanceWithArguments( u"com.sun.star.configuration.ConfigurationAccess"_ustr, aArguments ),
            uno::UNO_QUERY );

        if( xNameAccess.is())
        {
            const uno::Sequence< OUString > seqNames= xNameAccess->getElementNames();

            for( const auto& rName : seqNames )
            {
                uno::Reference< container::XNameAccess > xEntry ;
                xNameAccess->getByName( rName ) >>= xEntry;
                if ( xEntry.is() )
                {
                    OUString aUIName;
                    OUString aClassID;
                    xEntry->getByName(u"ObjectUIName"_ustr) >>= aUIName;
                    xEntry->getByName(u"ClassID"_ustr) >>= aClassID;

                    if ( !aUIName.isEmpty() )
                    {
                        aUIName = aUIName.replaceAll("%PRODUCTNAME", utl::ConfigManager::getProductName());
                        aUIName = aUIName.replaceAll("%PRODUCTVERSION", utl::ConfigManager::getProductVersion());
                    }

                    SvGlobalName aClassName;
                    if( aClassName.MakeId( aClassID) )
                    {
                        if( !Get( aClassName ) )
                            // not entered yet
                            aObjectServerList.emplace_back( aClassName, aUIName );
                    }
                }
            }
        }


#ifdef _WIN32
        SvGlobalName aOleFact( SO3_OUT_CLASSID );
        OUString aOleObj( SvtResId( STR_FURTHER_OBJECT ) );
        aObjectServerList.push_back( SvObjectServer( aOleFact, aOleObj ) );
#endif

    }catch(const container::NoSuchElementException&)
    {
    }catch(const uno::Exception&)
    {
    }
    catch(...)
    {
    }
}

OUString SvPasteObjectHelper::GetSotFormatUIName( SotClipboardFormatId nId )
{
    struct SotResourcePair
    {
        SotClipboardFormatId   mnSotId;
        TranslateId            mpResId;
    };

    static const SotResourcePair aSotResourcePairs[] =
    {
        { SotClipboardFormatId::STRING,              STR_FORMAT_STRING },
        { SotClipboardFormatId::BITMAP,              STR_FORMAT_BITMAP },
        { SotClipboardFormatId::GDIMETAFILE,         STR_FORMAT_GDIMETAFILE },
        { SotClipboardFormatId::RTF,                 STR_FORMAT_RTF },
        { SotClipboardFormatId::DRAWING,             STR_FORMAT_ID_DRAWING },
        { SotClipboardFormatId::SVXB,                STR_FORMAT_ID_SVXB },
        { SotClipboardFormatId::INTERNALLINK_STATE,  STR_FORMAT_ID_INTERNALLINK_STATE },
        { SotClipboardFormatId::SOLK,                STR_FORMAT_ID_SOLK },
        { SotClipboardFormatId::NETSCAPE_BOOKMARK,   STR_FORMAT_ID_NETSCAPE_BOOKMARK },
        { SotClipboardFormatId::STARSERVER,          STR_FORMAT_ID_STARSERVER },
        { SotClipboardFormatId::STAROBJECT,          STR_FORMAT_ID_STAROBJECT },
        { SotClipboardFormatId::APPLETOBJECT,        STR_FORMAT_ID_APPLETOBJECT },
        { SotClipboardFormatId::PLUGIN_OBJECT,       STR_FORMAT_ID_PLUGIN_OBJECT },
        { SotClipboardFormatId::STARWRITER_30,       STR_FORMAT_ID_STARWRITER_30 },
        { SotClipboardFormatId::STARWRITER_40,       STR_FORMAT_ID_STARWRITER_40 },
        { SotClipboardFormatId::STARWRITER_50,       STR_FORMAT_ID_STARWRITER_50 },
        { SotClipboardFormatId::STARWRITERWEB_40,    STR_FORMAT_ID_STARWRITERWEB_40 },
        { SotClipboardFormatId::STARWRITERWEB_50,    STR_FORMAT_ID_STARWRITERWEB_50 },
        { SotClipboardFormatId::STARWRITERGLOB_40,   STR_FORMAT_ID_STARWRITERGLOB_40 },
        { SotClipboardFormatId::STARWRITERGLOB_50,   STR_FORMAT_ID_STARWRITERGLOB_50 },
        { SotClipboardFormatId::STARDRAW,            STR_FORMAT_ID_STARDRAW },
        { SotClipboardFormatId::STARDRAW_40,         STR_FORMAT_ID_STARDRAW_40 },
        { SotClipboardFormatId::STARIMPRESS_50,      STR_FORMAT_ID_STARIMPRESS_50 },
        { SotClipboardFormatId::STARDRAW_50,         STR_FORMAT_ID_STARDRAW_50 },
        { SotClipboardFormatId::STARCALC,            STR_FORMAT_ID_STARCALC },
        { SotClipboardFormatId::STARCALC_40,         STR_FORMAT_ID_STARCALC_40 },
        { SotClipboardFormatId::STARCALC_50,         STR_FORMAT_ID_STARCALC_50 },
        { SotClipboardFormatId::STARCHART,           STR_FORMAT_ID_STARCHART },
        { SotClipboardFormatId::STARCHART_40,        STR_FORMAT_ID_STARCHART_40 },
        { SotClipboardFormatId::STARCHART_50,        STR_FORMAT_ID_STARCHART_50 },
        { SotClipboardFormatId::STARIMAGE,           STR_FORMAT_ID_STARIMAGE },
        { SotClipboardFormatId::STARIMAGE_40,        STR_FORMAT_ID_STARIMAGE_40 },
        { SotClipboardFormatId::STARIMAGE_50,        STR_FORMAT_ID_STARIMAGE_50 },
        { SotClipboardFormatId::STARMATH,            STR_FORMAT_ID_STARMATH },
        { SotClipboardFormatId::STARMATH_40,         STR_FORMAT_ID_STARMATH_40 },
        { SotClipboardFormatId::STARMATH_50,         STR_FORMAT_ID_STARMATH_50 },
        { SotClipboardFormatId::STAROBJECT_PAINTDOC, STR_FORMAT_ID_STAROBJECT_PAINTDOC },
        { SotClipboardFormatId::HTML,                STR_FORMAT_ID_HTML },
        { SotClipboardFormatId::HTML_SIMPLE,         STR_FORMAT_ID_HTML_SIMPLE },
        { SotClipboardFormatId::BIFF_5,              STR_FORMAT_ID_BIFF_5 },
        { SotClipboardFormatId::BIFF_8,              STR_FORMAT_ID_BIFF_8 },
        { SotClipboardFormatId::SYLK,                STR_FORMAT_ID_SYLK },
        { SotClipboardFormatId::LINK,                STR_FORMAT_ID_LINK },
        { SotClipboardFormatId::DIF,                 STR_FORMAT_ID_DIF },
        { SotClipboardFormatId::MSWORD_DOC,          STR_FORMAT_ID_MSWORD_DOC },
        { SotClipboardFormatId::STAR_FRAMESET_DOC,   STR_FORMAT_ID_STAR_FRAMESET_DOC },
        { SotClipboardFormatId::OFFICE_DOC,          STR_FORMAT_ID_OFFICE_DOC },
        { SotClipboardFormatId::NOTES_DOCINFO,       STR_FORMAT_ID_NOTES_DOCINFO },
        { SotClipboardFormatId::SFX_DOC,             STR_FORMAT_ID_SFX_DOC },
        { SotClipboardFormatId::STARCHARTDOCUMENT_50,STR_FORMAT_ID_STARCHARTDOCUMENT_50 },
        { SotClipboardFormatId::GRAPHOBJ,            STR_FORMAT_ID_GRAPHOBJ },
        { SotClipboardFormatId::STARWRITER_60,       STR_FORMAT_ID_STARWRITER_60 },
        { SotClipboardFormatId::STARWRITERWEB_60,    STR_FORMAT_ID_STARWRITERWEB_60 },
        { SotClipboardFormatId::STARWRITERGLOB_60,   STR_FORMAT_ID_STARWRITERGLOB_60 },
        { SotClipboardFormatId::STARDRAW_60,         STR_FORMAT_ID_STARDRAW_60 },
        { SotClipboardFormatId::STARIMPRESS_60,      STR_FORMAT_ID_STARIMPRESS_60 },
        { SotClipboardFormatId::STARCALC_60,         STR_FORMAT_ID_STARCALC_60 },
        { SotClipboardFormatId::STARCHART_60,        STR_FORMAT_ID_STARCHART_60 },
        { SotClipboardFormatId::STARMATH_60,         STR_FORMAT_ID_STARMATH_60 },
        { SotClipboardFormatId::WMF,                 STR_FORMAT_ID_WMF },
        { SotClipboardFormatId::DBACCESS_QUERY,      STR_FORMAT_ID_DBACCESS_QUERY },
        { SotClipboardFormatId::DBACCESS_TABLE,      STR_FORMAT_ID_DBACCESS_TABLE },
        { SotClipboardFormatId::DBACCESS_COMMAND,    STR_FORMAT_ID_DBACCESS_COMMAND },
        { SotClipboardFormatId::DIALOG_60,           STR_FORMAT_ID_DIALOG_60 },
        { SotClipboardFormatId::FILEGRPDESCRIPTOR,   STR_FORMAT_ID_FILEGRPDESCRIPTOR },
        { SotClipboardFormatId::HTML_NO_COMMENT,     STR_FORMAT_ID_HTML_NO_COMMENT },
        { SotClipboardFormatId::RICHTEXT,            STR_FORMAT_ID_RICHTEXT },
        { SotClipboardFormatId::STRING_TSVC,         STR_FORMAT_ID_STRING_TSVC },
        { SotClipboardFormatId::PNG,                 STR_FORMAT_ID_PNG_BITMAP },
    };

    TranslateId pResId;

    sal_uInt32 const nCount = SAL_N_ELEMENTS( aSotResourcePairs );
    for (sal_uInt32 i = 0; ( i < nCount ) && !pResId; ++i)
    {
        if( aSotResourcePairs[ i ].mnSotId == nId )
            pResId = aSotResourcePairs[ i ].mpResId;
    }

    OUString aUIName;
    if (pResId)
        aUIName = SvtResId(pResId);
    else
        aUIName = SotExchange::GetFormatName( nId );

    return aUIName;
}

bool SvPasteObjectHelper::GetEmbeddedName(const TransferableDataHelper& rData, OUString& _rName, OUString& _rSource, SotClipboardFormatId const & _nFormat)
{
    if( _nFormat != SotClipboardFormatId::EMBED_SOURCE_OLE && _nFormat != SotClipboardFormatId::EMBEDDED_OBJ_OLE )
        return false;

    datatransfer::DataFlavor aFlavor;
    SotExchange::GetFormatDataFlavor( SotClipboardFormatId::OBJECTDESCRIPTOR_OLE, aFlavor );

    if( !rData.HasFormat( aFlavor ) )
        return false;

    uno::Any aAny = rData.GetAny(aFlavor, OUString());
    if (!aAny.hasValue())
        return false;

    uno::Sequence< sal_Int8 > anySequence;
    aAny >>= anySequence;

    OleObjectDescriptor* pOleObjDescr =
            reinterpret_cast< OleObjectDescriptor* >( anySequence.getArray( ) );

    // determine the user friendly description of the embedded object
    if ( pOleObjDescr->dwFullUserTypeName )
    {
        // we set the pointer to the start of user friendly description
        // string. it starts  at &OleObjectDescriptor + dwFullUserTypeName.
        // dwFullUserTypeName is the offset in bytes.
        // the user friendly description string is '\0' terminated.
        const sal_Unicode* pUserTypeName =
            reinterpret_cast< sal_Unicode* >(
                reinterpret_cast< char* >( pOleObjDescr ) +
                    pOleObjDescr->dwFullUserTypeName );

        _rName += pUserTypeName;
        // the following statement was here for historical reasons, it is commented out since it causes bug i49460
        // _nFormat = SotClipboardFormatId::EMBED_SOURCE_OLE;
    }

    // determine the source of the embedded object
    if ( pOleObjDescr->dwSrcOfCopy )
    {
        // we set the pointer to the start of source string
        // it starts  at &OleObjectDescriptor + dwSrcOfCopy.
        // dwSrcOfCopy is the offset in bytes.
        // the source string is '\0' terminated.
        const sal_Unicode* pSrcOfCopy =
            reinterpret_cast< sal_Unicode* >(
                reinterpret_cast< char* >( pOleObjDescr ) +
                    pOleObjDescr->dwSrcOfCopy );

        _rSource += pSrcOfCopy;
    }
    else
        _rSource = SvtResId(STR_UNKNOWN_SOURCE);

    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
