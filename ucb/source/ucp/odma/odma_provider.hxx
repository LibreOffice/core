/*************************************************************************
 *
 *  $RCSfile: odma_provider.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 13:01:52 $
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

// @@@ Adjust multi-include-protection-ifdef.
#ifndef ODMA_PROVIDER_HXX
#define ODMA_PROVIDER_HXX

#ifndef _UCBHELPER_PROVIDERHELPER_HXX
#include <ucbhelper/providerhelper.hxx>
#endif
#ifndef ODMA_LIB_HXX
#include "odma_lib.hxx"
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#include <map>
#ifndef ODMA_CONTENTPROPS_HXX
#include "odma_contentprops.hxx"
#endif
// @@@ Adjust namespace name.
namespace odma {

//=========================================================================

// @@@ Adjust defines.

// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.
#define ODMA_CONTENT_PROVIDER_SERVICE_NAME \
                "com.sun.star.ucb.OdmaContentProvider"
//  #define ODMA_CONTENT_PROVIDER_SERVICE_NAME_LENGTH   34

// URL scheme. This is the scheme the provider will be able to create
// contents for. The UCB will select the provider ( i.e. in order to create
// contents ) according to this scheme.
#define ODMA_URL_ODMAID         "::ODMA"
#define ODMA_URL_SCHEME         "vnd.sun.star.odma"
#define ODMA_URL_SCHEME_SHORT   "odma"
#define ODMA_URL_SHORT          ":"
#define ODMA_URL_SHORT_LGTH     5
#define ODMA_URL_LGTH           18
#define ODMA_URL_ODMAID_LGTH    6

// UCB Content Type.
#define ODMA_CONTENT_TYPE       "application/" ODMA_URL_SCHEME "-content"
#define ODMA_ODMA_REGNAME       "sodma"
#define ODM_NAME_MAX            64      // Max length of a name document including
                                        // the terminating NULL character.

//=========================================================================
class ContentProperties;
class ContentProvider : public ::ucb::ContentProviderImplHelper
{
    typedef ::std::map< ::rtl::OString, ::vos::ORef<ContentProperties> > ContentsMap;
    ContentsMap      m_aContents;  // contains all ContentProperties
    static ODMHANDLE m_aOdmHandle; // the one and only ODMA handle to our DMS

    /** fillDocumentProperties fills the given _rProp with ODMA properties
        @param  _rProp  the ContentProperties
    */
    void fillDocumentProperties(const ::vos::ORef<ContentProperties>& _rProp);

    /**
    */
    ::vos::ORef<ContentProperties> getContentProperty(const ::rtl::OUString& _sName,
                                                       const ContentPropertiesMemberFunctor& _aFunctor) const;
public:
    ContentProvider( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr );
    virtual ~ContentProvider();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XContentProvider
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( ::com::sun::star::ucb::IllegalIdentifierException,
               ::com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Additional interfaces
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////
    static ODMHANDLE getHandle();

    /** append add an entry to the internal map
        @param  _rProp  the content properties
    */
    void append(const ::vos::ORef<ContentProperties>& _rProp);

    /** closeDocument closes the document
        @param  _sDocumentId    the id of the document
    */
    void closeDocument(const ::rtl::OString& _sDocumentId);

    /** saveDocument saves the document in DMS
        @param  _sDocumentId    the id of the document
    */
    void saveDocument(const ::rtl::OString& _sDocumentId);

    /** queryContentProperty query in the DMS for a content which document name is equal to _sDocumentName
        @param  _sDocumentName  the document to query for

        @return the content properties for this content or an empty refernce
    */
    ::vos::ORef<ContentProperties> queryContentProperty(const ::rtl::OUString& _sDocumentName);

    /** getContentProperty returns the ContentProperties for the first content with that title
        @param  _sTitle the title of the document

        @return the content properties
    */
    ::vos::ORef<ContentProperties> getContentPropertyWithTitle(const ::rtl::OUString& _sTitle) const;

    /** getContentProperty returns the ContentProperties for the first content with that SavedAsName
        @param  _sSaveAsName    the SavedAsName of the document

        @return the content properties
    */
    ::vos::ORef<ContentProperties> getContentPropertyWithSavedAsName(const ::rtl::OUString& _sSaveAsName) const;

    /** openDoc returns the URL for the temporary file for the specific Content and opens it
        @param  _rProp  used for check if already open, the member m_sFileURL will be set if is wan't opened yet

        @return the URL of the temporary file
    */
    static ::rtl::OUString openDoc(const ::vos::ORef<ContentProperties>& _rProp) throw (::com::sun::star::uno::Exception);

    /** convertURL converts a normal URL into an ODMA understandable name
        @param  _sCanonicURL    the URL from ContentIndentifier

        @return the ODMA name
    */
    static ::rtl::OUString convertURL(const ::rtl::OUString& _sCanonicURL);

    /** deleteDocument deletes the document inside the DMS and remove the content properties from inside list
        @param  _rProp  the ContentProperties

        @return true when successful
    */
    sal_Bool deleteDocument(const ::vos::ORef<ContentProperties>& _rProp);
};

}

#endif
