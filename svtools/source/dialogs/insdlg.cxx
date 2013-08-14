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

#include <insdlg.hxx>
#include <sores.hxx>
#include <svtresid.hxx>

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

//---------------------------------------------
// this struct conforms to the Microsoft
// OBJECTDESCRIPTOR -> see oleidl.h
// (MS platform sdk)
//---------------------------------------------

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
    for( size_t i = 0; i < aObjectServerList.size(); i++ )
    {
        if( rHumanName == aObjectServerList[ i ].GetHumanName() )
            return &aObjectServerList[ i ];
    }
    return NULL;
}

const SvObjectServer * SvObjectServerList::Get( const SvGlobalName & rName ) const
{
    for( size_t i = 0; i < aObjectServerList.size(); i++ )
    {
        if( rName == aObjectServerList[ i ].GetClassName() )
            return &aObjectServerList[ i ];
    }
    return NULL;
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

//---------------------------------------------------------------------
void SvObjectServerList::FillInsertObjects()
/* [Beschreibung]

    Die Liste wird mit allen Typen gef"ullt, die im Insert-Dialog
    ausgew"ahlt werden k"onnen.
*/
{
    try{
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();

        uno::Reference< lang::XMultiServiceFactory > sProviderMSFactory =
            configuration::theDefaultProvider::get(xContext);

        OUString sReaderService( "com.sun.star.configuration.ConfigurationAccess" );
        uno::Sequence< uno::Any > aArguments( 1 );
        beans::PropertyValue aPathProp;
        aPathProp.Name = OUString( "nodepath" );
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
                    if( aClassName.MakeId( String( aClassID )))
                    {
                        if( !Get( aClassName ) )
                            // noch nicht eingetragen
                            aObjectServerList.push_back( SvObjectServer( aClassName, aUIName ) );
                    }
                }
            }
        }


#ifdef WNT
        SvGlobalName aOleFact( SO3_OUT_CLASSID );
        String aOleObj( SVT_RESSTR( STR_FURTHER_OBJECT ) );
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

OUString SvPasteObjectHelper::GetSotFormatUIName( SotFormatStringId nId )
{
    struct SotResourcePair
    {
        SotFormatStringId   mnSotId;
        sal_uInt16              mnResId;
    };

    static const SotResourcePair aSotResourcePairs[] =
    {
        { SOT_FORMAT_STRING,                    STR_FORMAT_STRING },
        { SOT_FORMAT_BITMAP,                    STR_FORMAT_BITMAP },
        { SOT_FORMAT_GDIMETAFILE,               STR_FORMAT_GDIMETAFILE },
        { SOT_FORMAT_RTF,                       STR_FORMAT_RTF },
        { SOT_FORMATSTR_ID_DRAWING,             STR_FORMAT_ID_DRAWING },
        { SOT_FORMATSTR_ID_SVXB,                STR_FORMAT_ID_SVXB },
        { SOT_FORMATSTR_ID_INTERNALLINK_STATE,  STR_FORMAT_ID_INTERNALLINK_STATE },
        { SOT_FORMATSTR_ID_SOLK,                STR_FORMAT_ID_SOLK },
        { SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK,   STR_FORMAT_ID_NETSCAPE_BOOKMARK },
        { SOT_FORMATSTR_ID_STARSERVER,          STR_FORMAT_ID_STARSERVER },
        { SOT_FORMATSTR_ID_STAROBJECT,          STR_FORMAT_ID_STAROBJECT },
        { SOT_FORMATSTR_ID_APPLETOBJECT,        STR_FORMAT_ID_APPLETOBJECT },
        { SOT_FORMATSTR_ID_PLUGIN_OBJECT,       STR_FORMAT_ID_PLUGIN_OBJECT },
        { SOT_FORMATSTR_ID_STARWRITER_30,       STR_FORMAT_ID_STARWRITER_30 },
        { SOT_FORMATSTR_ID_STARWRITER_40,       STR_FORMAT_ID_STARWRITER_40 },
        { SOT_FORMATSTR_ID_STARWRITER_50,       STR_FORMAT_ID_STARWRITER_50 },
        { SOT_FORMATSTR_ID_STARWRITERWEB_40,    STR_FORMAT_ID_STARWRITERWEB_40 },
        { SOT_FORMATSTR_ID_STARWRITERWEB_50,    STR_FORMAT_ID_STARWRITERWEB_50 },
        { SOT_FORMATSTR_ID_STARWRITERGLOB_40,   STR_FORMAT_ID_STARWRITERGLOB_40 },
        { SOT_FORMATSTR_ID_STARWRITERGLOB_50,   STR_FORMAT_ID_STARWRITERGLOB_50 },
        { SOT_FORMATSTR_ID_STARDRAW,            STR_FORMAT_ID_STARDRAW },
        { SOT_FORMATSTR_ID_STARDRAW_40,         STR_FORMAT_ID_STARDRAW_40 },
        { SOT_FORMATSTR_ID_STARIMPRESS_50,      STR_FORMAT_ID_STARIMPRESS_50 },
        { SOT_FORMATSTR_ID_STARDRAW_50,         STR_FORMAT_ID_STARDRAW_50 },
        { SOT_FORMATSTR_ID_STARCALC,            STR_FORMAT_ID_STARCALC },
        { SOT_FORMATSTR_ID_STARCALC_40,         STR_FORMAT_ID_STARCALC_40 },
        { SOT_FORMATSTR_ID_STARCALC_50,         STR_FORMAT_ID_STARCALC_50 },
        { SOT_FORMATSTR_ID_STARCHART,           STR_FORMAT_ID_STARCHART },
        { SOT_FORMATSTR_ID_STARCHART_40,        STR_FORMAT_ID_STARCHART_40 },
        { SOT_FORMATSTR_ID_STARCHART_50,        STR_FORMAT_ID_STARCHART_50 },
        { SOT_FORMATSTR_ID_STARIMAGE,           STR_FORMAT_ID_STARIMAGE },
        { SOT_FORMATSTR_ID_STARIMAGE_40,        STR_FORMAT_ID_STARIMAGE_40 },
        { SOT_FORMATSTR_ID_STARIMAGE_50,        STR_FORMAT_ID_STARIMAGE_50 },
        { SOT_FORMATSTR_ID_STARMATH,            STR_FORMAT_ID_STARMATH },
        { SOT_FORMATSTR_ID_STARMATH_40,         STR_FORMAT_ID_STARMATH_40 },
        { SOT_FORMATSTR_ID_STARMATH_50,         STR_FORMAT_ID_STARMATH_50 },
        { SOT_FORMATSTR_ID_STAROBJECT_PAINTDOC, STR_FORMAT_ID_STAROBJECT_PAINTDOC },
        { SOT_FORMATSTR_ID_HTML,                STR_FORMAT_ID_HTML },
        { SOT_FORMATSTR_ID_HTML_SIMPLE,         STR_FORMAT_ID_HTML_SIMPLE },
        { SOT_FORMATSTR_ID_BIFF_5,              STR_FORMAT_ID_BIFF_5 },
        { SOT_FORMATSTR_ID_BIFF_8,              STR_FORMAT_ID_BIFF_8 },
        { SOT_FORMATSTR_ID_SYLK,                STR_FORMAT_ID_SYLK },
        { SOT_FORMATSTR_ID_LINK,                STR_FORMAT_ID_LINK },
        { SOT_FORMATSTR_ID_DIF,                 STR_FORMAT_ID_DIF },
        { SOT_FORMATSTR_ID_MSWORD_DOC,          STR_FORMAT_ID_MSWORD_DOC },
        { SOT_FORMATSTR_ID_STAR_FRAMESET_DOC,   STR_FORMAT_ID_STAR_FRAMESET_DOC },
        { SOT_FORMATSTR_ID_OFFICE_DOC,          STR_FORMAT_ID_OFFICE_DOC },
        { SOT_FORMATSTR_ID_NOTES_DOCINFO,       STR_FORMAT_ID_NOTES_DOCINFO },
        { SOT_FORMATSTR_ID_SFX_DOC,             STR_FORMAT_ID_SFX_DOC },
        { SOT_FORMATSTR_ID_STARCHARTDOCUMENT_50,STR_FORMAT_ID_STARCHARTDOCUMENT_50 },
        { SOT_FORMATSTR_ID_GRAPHOBJ,            STR_FORMAT_ID_GRAPHOBJ },
        { SOT_FORMATSTR_ID_STARWRITER_60,       STR_FORMAT_ID_STARWRITER_60 },
        { SOT_FORMATSTR_ID_STARWRITERWEB_60,    STR_FORMAT_ID_STARWRITERWEB_60 },
        { SOT_FORMATSTR_ID_STARWRITERGLOB_60,   STR_FORMAT_ID_STARWRITERGLOB_60 },
        { SOT_FORMATSTR_ID_STARDRAW_60,         STR_FORMAT_ID_STARDRAW_60 },
        { SOT_FORMATSTR_ID_STARIMPRESS_60,      STR_FORMAT_ID_STARIMPRESS_60 },
        { SOT_FORMATSTR_ID_STARCALC_60,         STR_FORMAT_ID_STARCALC_60 },
        { SOT_FORMATSTR_ID_STARCHART_60,        STR_FORMAT_ID_STARCHART_60 },
        { SOT_FORMATSTR_ID_STARMATH_60,         STR_FORMAT_ID_STARMATH_60 },
        { SOT_FORMATSTR_ID_WMF,                 STR_FORMAT_ID_WMF },
        { SOT_FORMATSTR_ID_DBACCESS_QUERY,      STR_FORMAT_ID_DBACCESS_QUERY },
        { SOT_FORMATSTR_ID_DBACCESS_TABLE,      STR_FORMAT_ID_DBACCESS_TABLE },
        { SOT_FORMATSTR_ID_DBACCESS_COMMAND,    STR_FORMAT_ID_DBACCESS_COMMAND },
        { SOT_FORMATSTR_ID_DIALOG_60,           STR_FORMAT_ID_DIALOG_60 },
        { SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR,   STR_FORMAT_ID_FILEGRPDESCRIPTOR },
        { SOT_FORMATSTR_ID_HTML_NO_COMMENT,     STR_FORMAT_ID_HTML_NO_COMMENT }
    };

    String aUIName;
    sal_uInt16 nResId = 0;

    for( sal_uInt32 i = 0, nCount = SAL_N_ELEMENTS( aSotResourcePairs ); ( i < nCount ) && !nResId; i++ )
    {
        if( aSotResourcePairs[ i ].mnSotId == nId )
            nResId = aSotResourcePairs[ i ].mnResId;
    }

    if( nResId )
        aUIName = SVT_RESSTR( nResId );
    else
        aUIName = SotExchange::GetFormatName( nId );

    return aUIName;
}
// -----------------------------------------------------------------------------
sal_Bool SvPasteObjectHelper::GetEmbeddedName(const TransferableDataHelper& rData, OUString& _rName, OUString& _rSource, SotFormatStringId& _nFormat)
{
    sal_Bool bRet = sal_False;
    if( _nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE_OLE || _nFormat == SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE )
    {
        datatransfer::DataFlavor aFlavor;
        SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE, aFlavor );

        uno::Any aAny;
        if( rData.HasFormat( aFlavor ) &&
            ( aAny = rData.GetAny( aFlavor ) ).hasValue() )
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
                // _nFormat = SOT_FORMATSTR_ID_EMBED_SOURCE_OLE;
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
                _rSource = SVT_RESSTR(STR_UNKNOWN_SOURCE);
        }
        bRet = sal_True;
    }
    return bRet;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
