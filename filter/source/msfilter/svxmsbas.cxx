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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
#include <tools/debug.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <svx/svxerr.hxx>
#include <filter/msfilter/svxmsbas.hxx>
#include <msvbasic.hxx>
#include <filter/msfilter/msocximex.hxx>
#include <sot/storinfo.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
using namespace com::sun::star::beans;
using namespace com::sun::star::io;
using namespace com::sun::star::awt;
#include <comphelper/storagehelper.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>

using namespace com::sun::star::container;
using namespace com::sun::star::script;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star;

using rtl::OUString;

static ::rtl::OUString sVBAOption( RTL_CONSTASCII_USTRINGPARAM( "Option VBASupport 1\n" ) ); 

int SvxImportMSVBasic::Import( const String& rStorageName,
                                const String &rSubStorageName,
                                BOOL bAsComment, BOOL bStripped )
{
    std::vector< String > codeNames;
    return Import(  rStorageName, rSubStorageName, codeNames, bAsComment, bStripped );
}

int SvxImportMSVBasic::Import( const String& rStorageName,
                                const String &rSubStorageName,
                                const std::vector< String >& codeNames,
                                BOOL bAsComment, BOOL bStripped )
{
    int nRet = 0;
    if( bImport && ImportCode_Impl( rStorageName, rSubStorageName, codeNames,
                                    bAsComment, bStripped ))
        nRet |= 1;

    if (bImport)
        ImportForms_Impl(rStorageName, rSubStorageName);

    if( bCopy && CopyStorage_Impl( rStorageName, rSubStorageName ))
        nRet |= 2;

    return nRet;
}

bool SvxImportMSVBasic::ImportForms_Impl(const String& rStorageName,
    const String& rSubStorageName)
{
    SvStorageRef xVBAStg(xRoot->OpenSotStorage(rStorageName,
        STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYALL));
    if (!xVBAStg.Is() || xVBAStg->GetError())
        return false;

    std::vector<String> aUserForms;
    SvStorageInfoList aContents;
    xVBAStg->FillInfoList(&aContents);
    for (USHORT nI = 0; nI < aContents.Count(); ++nI)
    {
          SvStorageInfo& rInfo = aContents.GetObject(nI);
          if (!rInfo.IsStream() && rInfo.GetName() != rSubStorageName)
              aUserForms.push_back(rInfo.GetName());
    }

    if (aUserForms.empty())
        return false;

    bool bRet = true;
    SFX_APP()->EnterBasicCall();
    try
    {
        Reference<XMultiServiceFactory> xSF(comphelper::getProcessServiceFactory());

        Reference<XComponentContext> xContext;
        Reference<XPropertySet> xProps(xSF, UNO_QUERY);
        xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext;


        Reference<XLibraryContainer> xLibContainer = rDocSh.GetDialogContainer();
        DBG_ASSERT( xLibContainer.is(), "No BasicContainer!" );

        String aLibName( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );
        Reference<XNameContainer> xLib;
        if (xLibContainer.is())
        {
            if( !xLibContainer->hasByName(aLibName))
                xLibContainer->createLibrary(aLibName);

            Any aLibAny = xLibContainer->getByName( aLibName );
            aLibAny >>= xLib;
        }

        if(xLib.is())
        {
            typedef std::vector<String>::iterator myIter;
            myIter aEnd = aUserForms.end();
            for (myIter aIter = aUserForms.begin(); aIter != aEnd; ++aIter)
            {
                SvStorageRef xForm (xVBAStg->OpenSotStorage(*aIter,
                    STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYALL));

                if (!xForm.Is() || xForm->GetError())
                    continue;

                SvStorageStreamRef xFrame = xForm->OpenSotStream(
                    String( RTL_CONSTASCII_USTRINGPARAM( "\3VBFrame" ) ),
                    STREAM_STD_READ | STREAM_NOCREATE);

                if (!xFrame.Is() || xFrame->GetError())
                    continue;

                SvStorageStreamRef xTypes = xForm->OpenSotStream(
                    String( 'f' ), STREAM_STD_READ | STREAM_NOCREATE);

                if (!xTypes.Is() || xTypes->GetError())
                    continue;

                //<UserForm Name=""><VBFrame></VBFrame>"
                String sData;
                String sLine;
                while(xFrame->ReadByteStringLine(sLine, RTL_TEXTENCODING_MS_1252))
                {
                    sData += sLine;
                    sData += '\n';
                }
                sData.ConvertLineEnd();

                Reference<container::XNameContainer> xDialog(
                    xSF->createInstance(
                       OUString(RTL_CONSTASCII_USTRINGPARAM(
                           "com.sun.star.awt.UnoControlDialogModel"))), uno::UNO_QUERY);

                OCX_UserForm aForm(xVBAStg, *aIter, *aIter, xDialog, xSF );
                aForm.pDocSh = &rDocSh;
                sal_Bool bOk = aForm.Read(xTypes);
                DBG_ASSERT(bOk, "Had unexpected content, not risking this module");
                if (bOk)
                    aForm.Import(xLib);
            }
        }
    }
    catch(...)
    {
        DBG_ERRORFILE( "SvxImportMSVBasic::ImportForms_Impl - any exception caught" );
        //bRet = false;
    }
    SFX_APP()->LeaveBasicCall();
    return bRet;
}


BOOL SvxImportMSVBasic::CopyStorage_Impl( const String& rStorageName,
                                         const String& rSubStorageName)
{
    BOOL bValidStg = FALSE;
    {
        SvStorageRef xVBAStg( xRoot->OpenSotStorage( rStorageName,
                                    STREAM_READWRITE | STREAM_NOCREATE |
                                    STREAM_SHARE_DENYALL ));
        if( xVBAStg.Is() && !xVBAStg->GetError() )
        {
            SvStorageRef xVBASubStg( xVBAStg->OpenSotStorage( rSubStorageName,
                                     STREAM_READWRITE | STREAM_NOCREATE |
                                    STREAM_SHARE_DENYALL ));
            if( xVBASubStg.Is() && !xVBASubStg->GetError() )
            {
                // then we will copy these storages into the (temporary) storage of the document
                bValidStg = TRUE;
            }
        }
    }

    if( bValidStg )
    {
        String aDstStgName( GetMSBasicStorageName() );
        SotStorageRef xDst = SotStorage::OpenOLEStorage( rDocSh.GetStorage(), aDstStgName, STREAM_READWRITE | STREAM_TRUNC );
        SotStorageRef xSrc = xRoot->OpenSotStorage( rStorageName, STREAM_STD_READ );

        // TODO/LATER: should we commit the storage?
        xSrc->CopyTo( xDst );
        xDst->Commit();
        ErrCode nError = xDst->GetError();
        if ( nError == ERRCODE_NONE )
            nError = xSrc->GetError();
        if ( nError != ERRCODE_NONE )
            xRoot->SetError( nError );
        else
            bValidStg = TRUE;
    }

    return bValidStg;
}

BOOL SvxImportMSVBasic::ImportCode_Impl( const String& rStorageName,
                                        const String &rSubStorageName,
                                        const std::vector< String >& codeNames,
                                        BOOL bAsComment, BOOL bStripped )
{
    BOOL bRet = FALSE;
    VBA_Impl aVBA( *xRoot, bAsComment );
    if( aVBA.Open(rStorageName,rSubStorageName) )
    {
        SFX_APP()->EnterBasicCall();
        Reference<XLibraryContainer> xLibContainer = rDocSh.GetBasicContainer();
        DBG_ASSERT( xLibContainer.is(), "No BasicContainer!" );
        
        if( !bAsComment ) try
        {
            Reference< vba::XVBACompatibility > xVBACompat( xLibContainer, UNO_QUERY_THROW );
            xVBACompat->setVBACompatibilityMode( sal_True );
            /*  Force creation of the VBAGlobals object, each application will
                create the right one and store it at the Basic manager. */
            Reference< XMultiServiceFactory > xFactory( rDocSh.GetModel(), UNO_QUERY_THROW );
            xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.VBAGlobals" ) ) );
        }
        catch( Exception& )
        {
        }

        UINT16 nStreamCount = aVBA.GetNoStreams();
        Reference<XNameContainer> xLib;
        String aLibName( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );
        if( xLibContainer.is() && nStreamCount )
        {
            if( !xLibContainer->hasByName( aLibName ) )
                xLibContainer->createLibrary( aLibName );

            Any aLibAny = xLibContainer->getByName( aLibName );
            aLibAny >>= xLib;
        }
        if( xLib.is() )
        {
            Reference< script::vba::XVBAModuleInfo > xVBAModuleInfo( xLib, UNO_QUERY );
            Reference< container::XNameAccess > xVBACodeNamedObjectAccess;
            if ( !bAsComment )
            {
                Reference< XMultiServiceFactory> xSF(rDocSh.GetModel(), UNO_QUERY);
                if ( xSF.is() )
                {
                    try
                    {
                        xVBACodeNamedObjectAccess.set( xSF->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.VBAObjectModuleObjectProvider"))), UNO_QUERY );
                    }
                    catch( Exception& ) { }
                }
            }
            typedef  std::hash_map< rtl::OUString, uno::Any, ::rtl::OUStringHash,
::std::equal_to< ::rtl::OUString > > NameModuleDataHash;
            typedef  std::hash_map< rtl::OUString, script::ModuleInfo, ::rtl::OUStringHash,
::std::equal_to< ::rtl::OUString > > NameModuleInfoHash;

            NameModuleDataHash moduleData;
            NameModuleInfoHash moduleInfos;

            for( UINT16 i=0; i<nStreamCount;i++)
            {
                StringArray aDecompressed = aVBA.Decompress(i);
#if 0
/*  DR 2005-08-11 #124850# Do not filter special characters from module name.
    Just put the original module name and let the Basic interpreter deal with
    it. Needed for roundtrip...
 */
                ByteString sByteBasic(aVBA.GetStreamName(i),
                    RTL_TEXTENCODING_ASCII_US,
                        (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_UNDERLINE|
                        RTL_UNICODETOTEXT_FLAGS_INVALID_UNDERLINE |
                        RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0 |
                        RTL_UNICODETOTEXT_FLAGS_NOCOMPOSITE)
                );

                const String sBasicModule(sByteBasic,
                    RTL_TEXTENCODING_ASCII_US);
#else
                const String &sBasicModule = aVBA.GetStreamName( i);
#endif
                /* #117718# expose information regarding type of Module
                * Class, Form or plain 'ould VBA module with a REM statment
                * at the top of the module. Mapping of Module Name
                * to type is performed in  VBA_Impl::Open() method,
                * ( msvbasic.cxx ) by examining the PROJECT stream.
                */

                // using name from aVBA.GetStreamName
                // because the encoding of the same returned
                // is the same as the encoding for the names
                // that are keys in the map used by GetModuleType method
                const String &sOrigVBAModName = aVBA.GetStreamName( i );
                ModType mType = aVBA.GetModuleType( sOrigVBAModName );

                rtl::OUString sClassRem( RTL_CONSTASCII_USTRINGPARAM( "Rem Attribute VBA_ModuleType=" ) );

                rtl::OUString modeTypeComment;

                switch( mType )
                {
                    case ModuleType::CLASS:
                        modeTypeComment = sClassRem +
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "VBAClassModule\n" ) );
                        break;
                    case ModuleType::FORM:
                        modeTypeComment = sClassRem +
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "VBAFormModule\n" ) );
                        break;
                    case ModuleType::DOCUMENT:
                        modeTypeComment = sClassRem +
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "VBADocumentModule\n" ) );
                        break;
                    case ModuleType::NORMAL:
                        modeTypeComment = sClassRem +
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "VBAModule\n" ) );
                        break;
                    case ModuleType::UNKNOWN:
                        modeTypeComment = sClassRem +
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "VBAUnknown\n" ) );
                        break;
                    default:
                        DBG_ERRORFILE( "SvxImportMSVBasic::ImportCode_Impl - unknown module type" );
                        break;
                }
                static ::rtl::OUString sClassOption( RTL_CONSTASCII_USTRINGPARAM( "Option ClassModule\n" ) ); 
                if ( !bAsComment )
                {
                    modeTypeComment += sVBAOption;
                    if ( mType == ModuleType::CLASS )
                        modeTypeComment += sClassOption;
                }

                String sModule(sBasicModule); //#i52606# no need to split Macros in 64KB blocks any more!
                String sTemp;
                if (bAsComment)
                {
                    sTemp+=String(RTL_CONSTASCII_USTRINGPARAM( "Sub " ));
                    String sMunge(sModule);
                    //Streams can have spaces in them, but modulenames
                    //cannot !
                    sMunge.SearchAndReplaceAll(' ','_');

                    sTemp += sMunge;
                    sTemp.AppendAscii("\n");
                };
                ::rtl::OUString aSource(sTemp);

                for(ULONG j=0;j<aDecompressed.GetSize();j++)
                {
                    if (bStripped)
                    {
                        String *pStr = aDecompressed.Get(j);
                        bool bMac = true;
                        xub_StrLen nBegin = pStr->Search('\x0D');
                        if ((STRING_NOTFOUND != nBegin) && (pStr->Len() > 1) && (pStr->GetChar(nBegin+1) == '\x0A'))
                            bMac = false;

                        const char cLineEnd = bMac ? '\x0D' : '\x0A';
                        const String sAttribute(String::CreateFromAscii(
                            bAsComment ? "Rem Attribute" : "Attribute"));
                        nBegin = 0;
                        while (STRING_NOTFOUND != (nBegin =	pStr->Search(sAttribute, nBegin)))
                        {
                            if ((nBegin) && pStr->GetChar(nBegin-1) != cLineEnd)
                            {
                                // npower #i63766# Need to skip instances of Attribute
                                // that are NOT Attribute statements
                                nBegin = nBegin + sAttribute.Len();
                                continue;
                            }
                            xub_StrLen nEnd = pStr->Search(cLineEnd ,nBegin);
                            // DR #i26521# catch STRING_NOTFOUND, will loop endless otherwise
                            if( nEnd == STRING_NOTFOUND )
                                pStr->Erase();
                            else
                                pStr->Erase(nBegin, (nEnd-nBegin)+1);
                        }
                    }
                    if( aDecompressed.Get(j)->Len() )
                    {
                        aSource+=::rtl::OUString( *aDecompressed.Get(j) );
                    }

                }
                if (bAsComment)
                {
                        aSource += rtl::OUString::createFromAscii("\nEnd Sub");
                }
                ::rtl::OUString aModName( sModule );
                aSource = modeTypeComment + aSource;

                Any aSourceAny;
                OSL_TRACE("erm %d", mType );
                    aSourceAny <<= aSource;
                if (  !bAsComment )
                {
                    OSL_TRACE("vba processing  %d", mType );
                    script::ModuleInfo sModuleInfo;
                    sModuleInfo.ModuleType = mType;
                    moduleInfos[ aModName ] = sModuleInfo;
                }
                 moduleData[ aModName ] = aSourceAny;
            }
            // Hack for missing codenames ( only know to happen in excel but... )
            // only makes sense to do this if we are importing non-commented basic
            if ( !bAsComment )
            {
                for ( std::vector< String >::const_iterator it =  codeNames.begin(); it != codeNames.end(); ++it )
                {
                    script::ModuleInfo sModuleInfo;
                    sModuleInfo.ModuleType = ModuleType::DOCUMENT;
                    moduleInfos[ *it ] = sModuleInfo;
                    moduleData[ *it ] = uno::makeAny( sVBAOption );
                }
            }
            NameModuleDataHash::iterator it_end = moduleData.end();
            for ( NameModuleDataHash::iterator it = moduleData.begin(); it != it_end; ++it )
            {
                NameModuleInfoHash::iterator it_info = moduleInfos.find( it->first );
                if ( it_info != moduleInfos.end() )
                {
                    ModuleInfo& sModuleInfo = it_info->second;
                    if ( sModuleInfo.ModuleType == ModuleType::FORM )
                        // hack, the module ( imo document basic should...
                        // know the XModel... ) but it doesn't
                        sModuleInfo.ModuleObject.set( rDocSh.GetModel(), UNO_QUERY );
                    //  document modules, we should be able to access
                    //  the api objects at this time
                    else if ( sModuleInfo.ModuleType == ModuleType::DOCUMENT )
                    {
                        if ( xVBACodeNamedObjectAccess.is() )
                        {
                            try
                            {
                                sModuleInfo.ModuleObject.set( xVBACodeNamedObjectAccess->getByName( it->first ), uno::UNO_QUERY );    
                                OSL_TRACE("** Straight up creation of Module");
                            }
                            catch(uno::Exception& e)
                            {
                                OSL_TRACE("Failed to get documument object for %s", rtl::OUStringToOString( it->first, RTL_TEXTENCODING_UTF8 ).getStr() );
                            }
                        }
                    }
                    xVBAModuleInfo->insertModuleInfo( it->first, sModuleInfo );
                }
                     
                if( xLib->hasByName( it->first ) )
                    xLib->replaceByName( it->first, it->second );
                else
                    xLib->insertByName( it->first, it->second );
            }
            bRet = true;
        }
        SFX_APP()->LeaveBasicCall();
    }
    return bRet;
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
