/*************************************************************************
 *
 *  $RCSfile: odma_provider.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 13:01:43 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef ODMA_PROVIDER_HXX
#include "odma_provider.hxx"
#endif
#ifndef ODMA_CONTENT_HXX
#include "odma_content.hxx"
#endif
#ifndef ODMA_CONTENTPROPS_HXX
#include "odma_contentprops.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#include <algorithm>
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

using namespace com::sun;
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
: ::ucb::ContentProviderImplHelper( rSMgr )
{

}

//=========================================================================
// virtual
ContentProvider::~ContentProvider()
{
    ContentsMap::iterator aIter = m_aContents.begin();
    for (;aIter != m_aContents.begin() ;++aIter )
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
ODMHANDLE ContentProvider::getHandle()
{
    if(!m_aOdmHandle)
    {
        ODMSTATUS odm = NODMRegisterApp(&m_aOdmHandle,ODM_API_VERSION,ODMA_ODMA_REGNAME,NULL,NULL);
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
                   star::ucb::XContentProvider );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

// @@@ Add own interfaces.
XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      star::ucb::XContentProvider );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// @@@ Adjust implementation name. Keep the prefix "com.sun.star.comp."!
// @@@ Adjust service name.
XSERVICEINFO_IMPL_1( ContentProvider,
                     rtl::OUString::createFromAscii(
                            "com.sun.star.comp.odma.ContentProvider" ),
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ODMA_CONTENT_PROVIDER_SERVICE_NAME) ) );

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
uno::Reference< star::ucb::XContent > SAL_CALL ContentProvider::queryContent(
        const uno::Reference< star::ucb::XContentIdentifier >& Identifier )
    throw( star::ucb::IllegalIdentifierException, uno::RuntimeException )
{
    // Check URL scheme...
    if(!getHandle())
        throw star::ucb::IllegalIdentifierException();

    rtl::OUString aScheme( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(ODMA_URL_SCHEME) ) );
    sal_Int32 nIndex = 0;
    rtl::OUString sOdma = aScheme.getToken(3,'.',nIndex);
    rtl::OUString sCanonicURL = Identifier->getContentIdentifier();
    // check if url starts with odma
    if ( !(Identifier->getContentProviderScheme().equalsIgnoreAsciiCase( aScheme ) ||
           Identifier->getContentProviderScheme().equalsIgnoreAsciiCase( sOdma )) )
        throw star::ucb::IllegalIdentifierException();

    if(!(   sCanonicURL.matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(ODMA_URL_SCHEME_SHORT ODMA_URL_SHORT)) ||
            sCanonicURL.matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(ODMA_URL_SCHEME ODMA_URL_SHORT))))
        throw star::ucb::IllegalIdentifierException();

    // @@@ Further id checks may go here...
#if 0
    if ( id-check-failes )
        throw star::ucb::IllegalIdentifierException();
#endif

    // @@@ Id normalization may go here...
#if 0
    // Normalize URL and create new Id.
    rtl::OUString aCanonicURL = ( Identifier->getContentIdentifier() );
    uno::Reference< star::ucb::XContentIdentifier > xCanonicId
        = new ::ucb::ContentIdentifier( m_xSMgr, aCanonicURL );
#else
    uno::Reference< star::ucb::XContentIdentifier > xCanonicId = Identifier;
#endif

    vos::OGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< star::ucb::XContent > xContent
        = queryExistingContent( xCanonicId ).getBodyPtr();
    if ( xContent.is() )
        return xContent;

    // @@@ Decision, which content implementation to instanciate may be
    //     made here ( in case you have different content classes ).

    // Create a new content. Note that the content will insert itself
    // into providers content list by calling addContent(...) from it's ctor.

    sCanonicURL = convertURL(sCanonicURL);

    ::vos::ORef<ContentProperties> aProp;
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
            aProp->m_sContentType  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ODMA_CONTENT_TYPE));
            append(aProp);
        }
        delete lpszDocName;
    }
    else // we got an already fetched name here so look for it
    {
        // we have a valid document name
        aProp = getContentPropertyWithTitle(sCanonicURL);
        if(!aProp.isValid())
            aProp = getContentPropertyWithSavedAsName(sCanonicURL);
        if(!aProp.isValid())
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
    if(!aProp.isValid())
        throw star::ucb::IllegalIdentifierException();

    xContent = new Content( m_xSMgr, this, xCanonicId ,aProp);

    if ( !xContent->getIdentifier().is() )
        throw star::ucb::IllegalIdentifierException();

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
            delete lpszDocId;
            throw uno::Exception();
        }
        aIter->second->m_sDocumentId = rtl::OString(lpszDocId);
        delete lpszDocId;
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
void ContentProvider::fillDocumentProperties(const ::vos::ORef<ContentProperties>& _rProp)
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

/*
    odm = NODMGetDocInfo(   getHandle(),
                                    const_cast<sal_Char*>(_rProp->m_sDocumentId.getStr()),
                                    ODM_URL,
                                    lpszDocInfo,
                                    ODM_DOCID_MAX);
*/
    delete lpszDocInfo;
}
// -----------------------------------------------------------------------------
void ContentProvider::append(const ::vos::ORef<ContentProperties>& _rProp)
{
    // now fill some more properties
    fillDocumentProperties(_rProp);
    // and append them
    m_aContents.insert(ContentsMap::value_type(_rProp->m_sDocumentId,_rProp));
}
// -----------------------------------------------------------------------------
::vos::ORef<ContentProperties> ContentProvider::queryContentProperty(const ::rtl::OUString& _sDocumentName)
{
    ::vos::ORef<ContentProperties> aReturn;
    sal_Char* lpszDMSList   = new sal_Char[ODM_DMSID_MAX];

    ODMSTATUS odm = NODMGetDMS(ODMA_ODMA_REGNAME, lpszDMSList);
    if(odm == ODM_SUCCESS)
    {
        sal_Char* pQueryId = new sal_Char[ODM_QUERYID_MAX];
        lpszDMSList[strlen(lpszDMSList)+1] = '\0';

        ::rtl::OString sTitleText(::rtl::OUStringToOString(_sDocumentName,RTL_TEXTENCODING_ASCII_US));
        ::rtl::OString sQuery("SELECT ODM_DOCID, ODM_NAME WHERE ODM_TITLETEXT = '");
        sQuery += sTitleText;
        sQuery += "'";

        DWORD dwFlags = ODM_SPECIFIC;
        odm = NODMQueryExecute(getHandle(), sQuery,dwFlags, lpszDMSList, pQueryId );
        if(odm == ODM_SUCCESS)
        {
            sal_uInt16 nCount       = 10;
            sal_uInt16 nMaxCount    = 10;
            sal_Char* lpszDocId     = new sal_Char[ODM_DOCID_MAX * nMaxCount];
            sal_Char* lpszDocName   = new sal_Char[ODM_NAME_MAX * nMaxCount];
            sal_Char* lpszDocInfo   = new sal_Char[ODM_DOCID_MAX];

            ::rtl::OUString sContentType(RTL_CONSTASCII_USTRINGPARAM(ODMA_CONTENT_TYPE));
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

            delete lpszDocInfo;
            delete lpszDocId;
            delete lpszDocName;
        }

        // now close the query
        odm = NODMQueryClose(ContentProvider::getHandle(), pQueryId);
        delete pQueryId;
    }
    delete lpszDMSList;


    return aReturn;
}
// -----------------------------------------------------------------------------
::vos::ORef<ContentProperties> ContentProvider::getContentProperty(const ::rtl::OUString& _sName,
                                                                   const ContentPropertiesMemberFunctor& _aFunctor) const
{
    ::vos::ORef<ContentProperties> aReturn;
    ContentsMap::const_iterator aFind = ::std::find_if( m_aContents.begin(),
                                                        m_aContents.end(),
                                                        ::std::compose1(
                                                            ::std::bind2nd(_aFunctor,_sName),
                                                            ::std::select2nd<ContentsMap::value_type>()
                                                        )
                                                    );
    if(aFind != m_aContents.end())
        aReturn = aFind->second;
    return aReturn;
}
// -----------------------------------------------------------------------------
::vos::ORef<ContentProperties> ContentProvider::getContentPropertyWithSavedAsName(const ::rtl::OUString& _sSaveAsName) const
{
    ContentPropertiesMemberFunctor aFunc(::std::mem_fun(&ContentProperties::getSavedAsName));
    return getContentProperty(_sSaveAsName,aFunc);
}
// -----------------------------------------------------------------------------
::vos::ORef<ContentProperties> ContentProvider::getContentPropertyWithTitle(const ::rtl::OUString& _sTitle) const
{
    ContentPropertiesMemberFunctor aFunc(::std::mem_fun(&ContentProperties::getTitle));
    return getContentProperty(_sTitle,aFunc);
}
// -----------------------------------------------------------------------------
::rtl::OUString ContentProvider::openDoc(const ::vos::ORef<ContentProperties>& _rProp)  throw (::com::sun::star::uno::Exception)
{
    OSL_ENSURE(_rProp.isValid(),"No valid content properties!");
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
                delete pFileName;
                throw uno::Exception();  // TODO give a more precise error message here
        }

        delete pFileName;
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
        sCanonicURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));

    if(nPos < sCanonicURL.getLength())
    {
        sCanonicURL = sCanonicURL.copy(nPos);
        sCanonicURL = rtl::Uri::decode(sCanonicURL,rtl_UriDecodeWithCharset,RTL_TEXTENCODING_UTF8);
    }
    if(sCanonicURL.getLength() > 1 && sCanonicURL.getStr()[0] == sal_Unicode('/'))
    {
        sCanonicURL = sCanonicURL.copy(1);
        if(sCanonicURL.getLength() == 1 && sCanonicURL.getStr()[0] == sal_Unicode('.'))
            sCanonicURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    }
    return sCanonicURL;
}
// -----------------------------------------------------------------------------
sal_Bool ContentProvider::deleteDocument(const ::vos::ORef<ContentProperties>& _rProp)
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
