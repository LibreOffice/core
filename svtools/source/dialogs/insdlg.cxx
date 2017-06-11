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
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/container/XNameAccess.hpp>

using namespace ::com::sun::star;


// this struct conforms to the Microsoft
// OBJECTDESCRIPTOR -> see oleidl.h
// (MS platform sdk)


struct OleObjectDescriptor
{
    sal_uInt32  cbSize;
    ClsId       clsid;
    sal_uInt32  dwDrawAspect;
    Size        sizel;
    Point       pointl;
    sal_uInt32  dwStatus;
    sal_uInt32  dwFullUserTypeName;
    sal_uInt32  dwSrcOfCopy;
};

/********************** SvObjectServerList ********************************
**************************************************************************/

const SvObjectServer * SvObjectServerList::Get( const OUString & rHumanName ) const
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
    for( size_t i = 0; i < aObjectServerList.size(); )
    {
        if( aObjectServerList[ i ].GetClassName() == rName )
        {
            SvObjectServerList_impl::iterator it = aObjectServerList.begin() + i;
            aObjectServerList.erase( it );
        }
        else
        {
            ++i;
        }
    }
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

        OUString sReaderService( "com.sun.star.configuration.ConfigurationAccess" );
        uno::Sequence< uno::Any > aArguments( 1 );
        beans::PropertyValue aPathProp;
        aPathProp.Name = "nodepath";
        aPathProp.Value <<= OUString( "/org.openoffice.Office.Embedding/ObjectNames" );
        aArguments[0] <<= aPathProp;

        uno::Reference< container::XNameAccess > xNameAccess(
            sProviderMSFactory->createInstanceWithArguments( sReaderService,aArguments ),
            uno::UNO_QUERY );

        if( xNameAccess.is())
        {
            uno::Sequence< OUString > seqNames= xNameAccess->getElementNames();
            sal_Int32 nInd;

            OUString aStringProductName( "%PRODUCTNAME" );
            sal_Int32 nStringProductNameLength = aStringProductName.getLength();

            OUString aStringProductVersion( "%PRODUCTVERSION" );
            sal_Int32 nStringProductVersionLength = aStringProductVersion.getLength();

            for( nInd = 0; nInd < seqNames.getLength(); nInd++ )
            {
                uno::Reference< container::XNameAccess > xEntry ;
                xNameAccess->getByName( seqNames[nInd] ) >>= xEntry;
                if ( xEntry.is() )
                {
                    OUString aUIName;
                    OUString aClassID;
                    xEntry->getByName("ObjectUIName") >>= aUIName;
                    xEntry->getByName("ClassID") >>= aClassID;

                    if ( !aUIName.isEmpty() )
                    {
                        // replace %PRODUCTNAME
                        sal_Int32 nIndex = aUIName.indexOf( aStringProductName );
                        while( nIndex != -1 )
                        {
                            aUIName = aUIName.replaceAt(
                                nIndex, nStringProductNameLength,
                                utl::ConfigManager::getProductName() );
                            nIndex = aUIName.indexOf( aStringProductName );
                        }

                        // replace %PRODUCTVERSION
                        nIndex = aUIName.indexOf( aStringProductVersion );
                        while( nIndex != -1 )
                        {
                            aUIName = aUIName.replaceAt(
                                nIndex, nStringProductVersionLength,
                                utl::ConfigManager::getProductVersion() );
                            nIndex = aUIName.indexOf( aStringProductVersion );
                        }
                    }

                    SvGlobalName aClassName;
                    if( aClassName.MakeId( aClassID) )
                    {
                        if( !Get( aClassName ) )
                            // not entered yet
                            aObjectServerList.push_back( SvObjectServer( aClassName, aUIName ) );
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
        const char* mpResId;
    };

    static const SotResourcePair aSotResourcePairs[] =
    {
        { SotClipboardFormatId::STRING,                    STR_FORMAT_STRING },
        { SotClipboardFormatId::BITMAP,                    STR_FORMAT_BITMAP },
        { SotClipboardFormatId::GDIMETAFILE,               STR_FORMAT_GDIMETAFILE },
        { SotClipboardFormatId::RTF,                       STR_FORMAT_RTF },
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
    };

    OUString aUIName;
    const char* pResId = nullptr;

    for( sal_uInt32 i = 0, nCount = SAL_N_ELEMENTS( aSotResourcePairs ); ( i < nCount ) && !pResId; i++ )
    {
        if( aSotResourcePairs[ i ].mnSotId == nId )
            pResId = aSotResourcePairs[ i ].mpResId;
    }

    if (pResId)
        aUIName = SvtResId(pResId);
    else
        aUIName = SotExchange::GetFormatName( nId );

    return aUIName;
}

bool SvPasteObjectHelper::GetEmbeddedName(const TransferableDataHelper& rData, OUString& _rName, OUString& _rSource, SotClipboardFormatId& _nFormat)
{
    bool bRet = false;
    if( _nFormat == SotClipboardFormatId::EMBED_SOURCE_OLE || _nFormat == SotClipboardFormatId::EMBEDDED_OBJ_OLE )
    {
        datatransfer::DataFlavor aFlavor;
        SotExchange::GetFormatDataFlavor( SotClipboardFormatId::OBJECTDESCRIPTOR_OLE, aFlavor );

        uno::Any aAny;
        if( rData.HasFormat( aFlavor ) &&
            ( aAny = rData.GetAny(aFlavor, OUString()) ).hasValue() )
        {
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
                        reinterpret_cast< sal_Char* >( pOleObjDescr ) +
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
                        reinterpret_cast< sal_Char* >( pOleObjDescr ) +
                            pOleObjDescr->dwSrcOfCopy );

                _rSource += pSrcOfCopy;
            }
            else
                _rSource = SvtResId(STR_UNKNOWN_SOURCE);
        }
        bRet = true;
    }
    return bRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
