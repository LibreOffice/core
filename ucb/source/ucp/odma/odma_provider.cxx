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


#ifdef WNT
#include <windows.h>
#endif
#include <ucbhelper/contentidentifier.hxx>
#include "odma_provider.hxx"
#include "odma_content.hxx"
#include "odma_contentprops.hxx"
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <rtl/uri.hxx>
#include <algorithm>
#include <osl/file.hxx>

#include <o3tl/compat_functional.hxx>

using namespace com::sun::star;
using namespace odma;

//=========================================================================
//=========================================================================
//
// ContentProvider Implementation.
//
//=========================================================================
//=========================================================================
ODMHANDLE ContentProvider::m_aOdmHandle = NULL;

ContentProvider::ContentProvider(
                const uno::Reference< lang::XMultiServiceFactory >& rSMgr )
: ::ucbhelper::ContentProviderImplHelper( rSMgr )
{

}

//=========================================================================
// virtual
ContentProvider::~ContentProvider()
{
    ContentsMap::iterator aIter = m_aContents.begin();
    for (;aIter != m_aContents.end() ;++aIter )
    {
        if(aIter->second->m_bIsOpen)
            closeDocument(aIter->first);
    }
    if(m_aOdmHandle)
    {
        NODMUnRegisterApp(m_aOdmHandle);
        m_aOdmHandle = NULL;
    }
}
// -----------------------------------------------------------------------------
inline bool is_current_process_window(HWND hwnd)
{
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    return (pid == GetCurrentProcessId());
}

HWND choose_parent_window()
{
    HWND hwnd_parent = GetForegroundWindow();
    if (!is_current_process_window(hwnd_parent))
       hwnd_parent = GetDesktopWindow();
    return hwnd_parent;
}

ODMHANDLE ContentProvider::getHandle()
{
    if(!m_aOdmHandle)
    {
        ODMSTATUS odm = NODMRegisterApp(&m_aOdmHandle,ODM_API_VERSION,const_cast<char*>(ODMA_ODMA_REGNAME),(DWORD) choose_parent_window( ),NULL);
        switch(odm)
        {
        case ODM_SUCCESS:
            break;
        case ODM_E_NODMS:
            break;
        case ODM_E_CANTINIT:
            break;
        case ODM_E_VERSION:
            break;
        default:
            break;
        }
    }
    return m_aOdmHandle;
}
// -----------------------------------------------------------------------------

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

// @@@ Add own interfaces.
XINTERFACE_IMPL_3( ContentProvider,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   ucb::XContentProvider );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

// @@@ Add own interfaces.
XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      ucb::XContentProvider );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// @@@ Adjust implementation name. Keep the prefix "com.sun.star.comp."!
// @@@ Adjust service name.
XSERVICEINFO_IMPL_1( ContentProvider,
                     rtl::OUString( "com.sun.star.comp.odma.ContentProvider" ),
                     rtl::OUString(ODMA_CONTENT_PROVIDER_SERVICE_NAME ) );

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );

//=========================================================================
//
// XContentProvider methods.
//
//=========================================================================

// virtual
uno::Reference< ucb::XContent > SAL_CALL ContentProvider::queryContent(
        const uno::Reference< ucb::XContentIdentifier >& Identifier )
    throw( ucb::IllegalIdentifierException, uno::RuntimeException )
{
    // Check URL scheme...
    if(!getHandle())
        throw ucb::IllegalIdentifierException();

    rtl::OUString aScheme( rtl::OUString( ODMA_URL_SCHEME ) );
    sal_Int32 nIndex = 0;
    rtl::OUString sOdma = aScheme.getToken(3,'.',nIndex);
    rtl::OUString sCanonicURL = Identifier->getContentIdentifier();
    // check if url starts with odma
    if ( !(Identifier->getContentProviderScheme().equalsIgnoreAsciiCase( aScheme ) ||
           Identifier->getContentProviderScheme().equalsIgnoreAsciiCase( sOdma )) )
        throw ucb::IllegalIdentifierException();

    if(!(   sCanonicURL.matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(ODMA_URL_SCHEME_SHORT ODMA_URL_SHORT)) ||
            sCanonicURL.matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(ODMA_URL_SCHEME ODMA_URL_SHORT))))
        throw ucb::IllegalIdentifierException();


    // @@@ Id normalization may go here...

    uno::Reference< ucb::XContentIdentifier > xCanonicId = Identifier;

    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent
        = queryExistingContent( xCanonicId ).get();
    if ( xContent.is() )
        return xContent;

    // @@@ Decision, which content implementation to instanciate may be
    //     made here ( in case you have different content classes ).

    // Create a new content.

    sCanonicURL = convertURL(sCanonicURL);

    ::rtl::Reference<ContentProperties> aProp;
    // first check if we got an ODMA ID from outside
    if( sCanonicURL.matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(ODMA_URL_ODMAID)))
    {// we get an orignal ODMA id so we have to look for the name
        ::rtl::OString sDocId = ::rtl::OUStringToOString(sCanonicURL,RTL_TEXTENCODING_MS_1252);
        sal_Char* lpszDocName = new sal_Char[ODM_NAME_MAX];

        ODMSTATUS odm = NODMGetDocInfo( getHandle(),
                                        const_cast<sal_Char*>(sDocId.getStr()),
                                        ODM_NAME,
                                        lpszDocName,
                                        ODM_NAME_MAX
                                    );
        if(odm == ODM_SUCCESS)
        {
            aProp = new ContentProperties();
            aProp->m_sDocumentName = ::rtl::OStringToOUString(rtl::OString(lpszDocName),RTL_TEXTENCODING_ASCII_US);
            aProp->m_sDocumentId   = sDocId;
            aProp->m_sContentType  = ::rtl::OUString(ODMA_CONTENT_TYPE);
            append(aProp);
        }
        delete [] lpszDocName;
    }
    else // we got an already fetched name here so look for it
    {
        // we have a valid document name
        aProp = getContentPropertyWithTitle(sCanonicURL);
        if(!aProp.is())
            aProp = getContentPropertyWithSavedAsName(sCanonicURL);
        if(!aProp.is())
        {
            if(sCanonicURL.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("/")))
            { // found only the scheme
                aProp = new ContentProperties();
                aProp->m_sDocumentId = "/";
                aProp->m_sTitle = sCanonicURL;
                aProp->m_bIsFolder = sal_True;
                aProp->m_bIsDocument = !aProp->m_bIsFolder;
                m_aContents.insert(ContentsMap::value_type(aProp->m_sDocumentId,aProp));
            }
            else
                aProp = queryContentProperty(sCanonicURL);
        }
    }
    if(!aProp.is())
        throw ucb::IllegalIdentifierException();

    xContent = new Content( m_xSMgr, this, xCanonicId ,aProp);
    registerNewContent( xContent );

    if ( !xContent->getIdentifier().is() )
        throw ucb::IllegalIdentifierException();

    return xContent;
}
// -----------------------------------------------------------------------------
void ContentProvider::closeDocument(const ::rtl::OString& _sDocumentId)
{
    ContentsMap::iterator aIter = m_aContents.find(_sDocumentId);
    if(aIter != m_aContents.end())
    {
        DWORD dwFlags = ODM_SILENT;
        ODMSTATUS odm = NODMCloseDocEx( ContentProvider::getHandle(),
                                        const_cast<sal_Char*>(_sDocumentId.getStr()),
                                        &dwFlags,
                                        0xFFFFFFFF,
                                        0xFFFFFFFF,
                                        NULL,
                                        0);
        OSL_ENSURE(odm == ODM_SUCCESS,"Error while closing a document!");
        if(odm == ODM_SUCCESS)
            aIter->second->m_bIsOpen = sal_False;
    }
}
// -----------------------------------------------------------------------------
void ContentProvider::saveDocument(const ::rtl::OString& _sDocumentId)
{
    ContentsMap::iterator aIter = m_aContents.find(_sDocumentId);
    if(aIter != m_aContents.end())
    {
        sal_Char* lpszDocId = new sal_Char[ODM_DOCID_MAX];
        DWORD dwFlags = ODM_SILENT;
        ODMSTATUS odm = NODMSaveDocEx(getHandle(),
                                    const_cast<sal_Char*>(_sDocumentId.getStr()),
                                    lpszDocId,
                                    &dwFlags);
        OSL_ENSURE(odm == ODM_SUCCESS,"Could not save document!");
        if(odm != ODM_SUCCESS)
        {
            delete [] lpszDocId;
            throw uno::Exception();
        }
        aIter->second->m_sDocumentId = rtl::OString(lpszDocId);
        delete [] lpszDocId;
    }
}
// -----------------------------------------------------------------------------
util::Date toDate(const ::rtl::OString& _sSQLString)
{
    sal_uInt16  nYear   = 0,
                nMonth  = 0,
                nDay    = 0;
    nYear   = (sal_uInt16)_sSQLString.copy(0,4).toInt32();
    nMonth  = (sal_uInt16)_sSQLString.copy(4,2).toInt32();
    nDay    = (sal_uInt16)_sSQLString.copy(6,2).toInt32();

    return util::Date(nDay,nMonth,nYear);
}
//-----------------------------------------------------------------------------
util::Time toTime(const ::rtl::OString& _sSQLString)
{
    sal_uInt16  nHour   = 0,
                nMinute = 0,
                nSecond = 0;
    nHour   = (sal_uInt16)_sSQLString.copy(8,2).toInt32();
    nMinute = (sal_uInt16)_sSQLString.copy(10,2).toInt32();
    nSecond = (sal_uInt16)_sSQLString.copy(12,2).toInt32();

    return util::Time(0,nHour,nMinute,nSecond);
}
//-----------------------------------------------------------------------------
util::DateTime toDateTime(const ::rtl::OString& _sSQLString)
{
    util::Date aDate = toDate(_sSQLString);
    util::Time aTime = toTime(_sSQLString);

    return util::DateTime(0,aTime.Seconds,aTime.Minutes,aTime.Hours,aDate.Day,aDate.Month,aDate.Year);
}
// -----------------------------------------------------------------------------
void ContentProvider::fillDocumentProperties(const ::rtl::Reference<ContentProperties>& _rProp)
{
    // read some properties from the DMS
    sal_Char* lpszDocInfo = new sal_Char[ODM_DOCID_MAX];
    sal_Char* pDocId = const_cast<sal_Char*>(_rProp->m_sDocumentId.getStr());

    // read the create date of the document
    ODMSTATUS odm = NODMGetDocInfo( getHandle(),
                                    pDocId,
                                    ODM_CREATEDDATE,
                                    lpszDocInfo,
                                    ODM_DOCID_MAX);
    if(odm == ODM_SUCCESS)
        _rProp->m_aDateCreated = toDateTime(::rtl::OString(lpszDocInfo));

    // read the modified date of the document
    odm = NODMGetDocInfo(   getHandle(),
                            pDocId,
                            ODM_MODIFYDATE,
                            lpszDocInfo,
                            ODM_DOCID_MAX);
    if(odm == ODM_SUCCESS)
        _rProp->m_aDateModified = toDateTime(::rtl::OString(lpszDocInfo));

    // read the title of the document
    odm = NODMGetDocInfo(   getHandle(),
                            pDocId,
                            ODM_TITLETEXT,
                            lpszDocInfo,
                            ODM_DOCID_MAX);
    if(odm == ODM_SUCCESS)
        _rProp->m_sTitle = ::rtl::OStringToOUString(rtl::OString(lpszDocInfo),RTL_TEXTENCODING_ASCII_US);

    // read the name of the document
    odm = NODMGetDocInfo(   getHandle(),
                            pDocId,
                            ODM_NAME,
                            lpszDocInfo,
                            ODM_DOCID_MAX);
    if(odm == ODM_SUCCESS)
        _rProp->m_sDocumentName = ::rtl::OStringToOUString(rtl::OString(lpszDocInfo),RTL_TEXTENCODING_ASCII_US);

    // read the author of the document
    odm = NODMGetDocInfo(   getHandle(),
                            pDocId,
                            ODM_AUTHOR,
                            lpszDocInfo,
                            ODM_DOCID_MAX);
    if(odm == ODM_SUCCESS)
        _rProp->m_sAuthor = ::rtl::OStringToOUString(rtl::OString(lpszDocInfo),RTL_TEXTENCODING_ASCII_US);

    // read the subject of the document
    odm = NODMGetDocInfo(   getHandle(),
                            pDocId,
                            ODM_SUBJECT,
                            lpszDocInfo,
                            ODM_DOCID_MAX);
    if(odm == ODM_SUCCESS)
        _rProp->m_sSubject = ::rtl::OStringToOUString(rtl::OString(lpszDocInfo),RTL_TEXTENCODING_ASCII_US);

    // read the keywords of the document
    odm = NODMGetDocInfo(   getHandle(),
                            pDocId,
                            ODM_KEYWORDS,
                            lpszDocInfo,
                            ODM_DOCID_MAX);
    if(odm == ODM_SUCCESS)
        _rProp->m_sKeywords = ::rtl::OStringToOUString(rtl::OString(lpszDocInfo),RTL_TEXTENCODING_ASCII_US);

    delete [] lpszDocInfo;
}
// -----------------------------------------------------------------------------
void ContentProvider::append(const ::rtl::Reference<ContentProperties>& _rProp)
{
    // now fill some more properties
    fillDocumentProperties(_rProp);
    // and append them
    m_aContents.insert(ContentsMap::value_type(_rProp->m_sDocumentId,_rProp));
}
// -----------------------------------------------------------------------------
::rtl::Reference<ContentProperties> ContentProvider::queryContentProperty(const ::rtl::OUString& _sDocumentName)
{
    ::rtl::Reference<ContentProperties> aReturn;
    sal_Char* lpszDMSList   = new sal_Char[ODM_DMSID_MAX];

    ODMSTATUS odm = NODMGetDMS(ODMA_ODMA_REGNAME, lpszDMSList);
    if(odm == ODM_SUCCESS)
    {
        sal_Char* pQueryId = new sal_Char[ODM_QUERYID_MAX];
        lpszDMSList[strlen(lpszDMSList)+1] = '\0';

        ::rtl::OString sTitleText(::rtl::OUStringToOString(_sDocumentName,RTL_TEXTENCODING_ASCII_US));
        ::rtl::OString sQuery("SELECT ODM_DOCID_LATEST, ODM_NAME WHERE ODM_TITLETEXT = '");
        sQuery += sTitleText;
        sQuery += "'";

        DWORD dwFlags = ODM_SPECIFIC;
        odm = NODMQueryExecute(getHandle(), sQuery.getStr(), dwFlags, lpszDMSList, pQueryId );
        if(odm == ODM_SUCCESS)
        {
            sal_uInt16 nCount       = 10;
            sal_uInt16 nMaxCount    = 10;
            sal_Char* lpszDocId     = new sal_Char[ODM_DOCID_MAX * nMaxCount];
            sal_Char* lpszDocName   = new sal_Char[ODM_NAME_MAX * nMaxCount];
            sal_Char* lpszDocInfo   = new sal_Char[ODM_DOCID_MAX];

            ::rtl::OUString sContentType(ODMA_CONTENT_TYPE);
            do
            {
                if(nCount >= nMaxCount)
                {
                    // get the result
                    nCount = nMaxCount;
                    odm = NODMQueryGetResults(getHandle(), pQueryId,lpszDocId, lpszDocName, ODM_NAME_MAX, (WORD*)&nCount);
                }
                if(odm == ODM_SUCCESS)
                    for(sal_uInt16 i = 0; i < nCount; ++i)
                    {
                        odm = NODMGetDocInfo(   getHandle(),
                                                &lpszDocId[ODM_DOCID_MAX*i],
                                                ODM_TITLETEXT,
                                                lpszDocInfo,
                                                ODM_DOCID_MAX);
                        if( odm == ODM_SUCCESS && sTitleText == ::rtl::OString(lpszDocInfo))
                        {
                            aReturn = new ContentProperties();
                            aReturn->m_sDocumentName    = ::rtl::OStringToOUString(rtl::OString(&lpszDocName[ODM_NAME_MAX*i]),RTL_TEXTENCODING_ASCII_US);
                            aReturn->m_sDocumentId  = ::rtl::OString(&lpszDocId[ODM_DOCID_MAX*i]);
                            aReturn->m_sContentType = sContentType;
                            append(aReturn);
                            nCount = 0; // break condition from outer loop
                            break;
                        }
                    }
            }
            while(nCount > nMaxCount);

            delete [] lpszDocInfo;
            delete [] lpszDocId;
            delete [] lpszDocName;
        }

        // now close the query
        odm = NODMQueryClose(ContentProvider::getHandle(), pQueryId);
        delete [] pQueryId;
    }
    delete [] lpszDMSList;


    return aReturn;
}
// -----------------------------------------------------------------------------
::rtl::Reference<ContentProperties> ContentProvider::getContentProperty(const ::rtl::OUString& _sName,
                                                                   const ContentPropertiesMemberFunctor& _aFunctor) const
{
    ::rtl::Reference<ContentProperties> aReturn;
    ContentsMap::const_iterator aFind = ::std::find_if( m_aContents.begin(),
                                                        m_aContents.end(),
                                                        ::o3tl::compose1(
                                                            ::std::bind2nd(_aFunctor,_sName),
                                                            ::o3tl::select2nd<ContentsMap::value_type>()
                                                        )
                                                    );
    if(aFind != m_aContents.end())
        aReturn = aFind->second;
    return aReturn;
}
// -----------------------------------------------------------------------------
::rtl::Reference<ContentProperties> ContentProvider::getContentPropertyWithSavedAsName(const ::rtl::OUString& _sSaveAsName) const
{
    ContentPropertiesMemberFunctor aFunc(::std::mem_fun(&ContentProperties::getSavedAsName));
    return getContentProperty(_sSaveAsName,aFunc);
}
// -----------------------------------------------------------------------------
::rtl::Reference<ContentProperties> ContentProvider::getContentPropertyWithTitle(const ::rtl::OUString& _sTitle) const
{
    ContentPropertiesMemberFunctor aFunc(::std::mem_fun(&ContentProperties::getTitle));
    return getContentProperty(_sTitle,aFunc);
}
// -----------------------------------------------------------------------------
::rtl::Reference<ContentProperties> ContentProvider::getContentPropertyWithDocumentId(const ::rtl::OUString& _sDocumentId) const
{
    ContentPropertiesMemberFunctor aFunc(::std::mem_fun(&ContentProperties::getDocumentId));
    return getContentProperty(_sDocumentId,aFunc);
}
// -----------------------------------------------------------------------------
::rtl::OUString ContentProvider::openDoc(const ::rtl::Reference<ContentProperties>& _rProp)  throw (uno::Exception)
{
    OSL_ENSURE(_rProp.is(),"No valid content properties!");
    if(!_rProp->m_bIsOpen)
    {
        sal_Char *pFileName = new sal_Char[ODM_FILENAME_MAX];

        DWORD dwFlag = ODM_MODIFYMODE | ODM_SILENT;
        ODMSTATUS odm = NODMOpenDoc(getHandle(), dwFlag, const_cast<sal_Char*>(_rProp->m_sDocumentId.getStr()), pFileName);
        switch(odm)
        {
            case ODM_E_INUSE:
                dwFlag = ODM_VIEWMODE;
                if( NODMOpenDoc(getHandle(), dwFlag, const_cast<sal_Char*>(_rProp->m_sDocumentId.getStr()), pFileName) != ODM_SUCCESS)
                    break;
                // else run through
            case ODM_SUCCESS:
                ::osl::FileBase::getFileURLFromSystemPath(::rtl::OStringToOUString(rtl::OString(pFileName),RTL_TEXTENCODING_ASCII_US)
                                                            ,_rProp->m_sFileURL);
                _rProp->m_bIsOpen = sal_True;
                break;
            default:
                delete [] pFileName;
                throw uno::Exception();  // TODO give a more precise error message here
        }

        delete [] pFileName;
    }
    return _rProp->m_sFileURL;
}
// -----------------------------------------------------------------------------
::rtl::OUString ContentProvider::convertURL(const ::rtl::OUString& _sCanonicURL)
{
    sal_Int32 nPos = 0;
    // check if url starts with odma
    if(_sCanonicURL.matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(ODMA_URL_SCHEME_SHORT ODMA_URL_SHORT)))
    { // URL starts with odma:// so we have to remove this
        nPos = ODMA_URL_SHORT_LGTH;
    }
    else if(_sCanonicURL.matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(ODMA_URL_SCHEME ODMA_URL_SHORT)))
    { // URL starts with vnd.sun.star.odma:/// so we have to remove this
        nPos = ODMA_URL_LGTH;
    }

    ::rtl::OUString sCanonicURL = _sCanonicURL;
    // now check what formats we allow
    if(nPos == _sCanonicURL.getLength()) // only ask for root entry
        sCanonicURL = ::rtl::OUString("/");

    if(nPos < sCanonicURL.getLength())
    {
        sCanonicURL = sCanonicURL.copy(nPos);
        sCanonicURL = rtl::Uri::decode(sCanonicURL,rtl_UriDecodeWithCharset,RTL_TEXTENCODING_UTF8);
    }
    if(sCanonicURL.getLength() > 1 && sCanonicURL.getStr()[0] == sal_Unicode('/'))
    {
        sCanonicURL = sCanonicURL.copy(1);
        if(sCanonicURL.getLength() == 1 && sCanonicURL.getStr()[0] == sal_Unicode('.'))
            sCanonicURL = ::rtl::OUString("/");
    }
    return sCanonicURL;
}
// -----------------------------------------------------------------------------
sal_Bool ContentProvider::deleteDocument(const ::rtl::Reference<ContentProperties>& _rProp)
{
    closeDocument(_rProp->m_sDocumentId);
    ODMSTATUS odm = NODMActivate(ContentProvider::getHandle(),
                                 ODM_DELETE,
                                 const_cast< sal_Char*>(_rProp->m_sDocumentId.getStr()));
    if(odm == ODM_SUCCESS)
        m_aContents.erase(_rProp->m_sDocumentId);

    return odm == ODM_SUCCESS;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
