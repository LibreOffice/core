/*************************************************************************
 *
 *  $RCSfile: ScriptStorage.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: lkovacs $ $Date: 2002-09-23 14:17:49 $
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
#ifndef __SCRIPTING_STORAGE_SCRIPTSTORAGE_HXX_
#define __SCRIPTING_STORAGE_SCRIPTSTORAGE_HXX_

#include <vector>
#include <hash_map>

#include <osl/mutex.hxx>
#include <cppuhelper/implbase6.hxx> // helper for component factory

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptAccessManager.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptImplAccess.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptStorageExport.hpp>
#include <drafts/com/sun/star/script/framework/storage/ScriptImplInfo.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptInfo.hpp>
#include <drafts/com/sun/star/script/framework/storage/NoSuchView.hpp>
#include <drafts/com/sun/star/script/framework/scripturi/XScriptURI.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#ifndef _DRAFTS_COM_SUN_STAR_SCRIPT_FRAMEWORK_STORAGE_XPARCELINVOCATIONPREP_HPP_
#include <drafts/com/sun/star/script/framework/storage/XParcelInvocationPrep.hpp>
#endif

namespace scripting_impl
{

//Typedefs
//=============================================================================
typedef ::std::vector< ::drafts::com::sun::star::script::framework::storage::ScriptImplInfo >
Impls_vec;
//-----------------------------------------------------------------------------
typedef ::std::hash_map < ::rtl::OUString, Impls_vec,
::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > >
ScriptInfo_hash;
//-----------------------------------------------------------------------------
typedef ::std::hash_map < ::rtl::OUString,
::com::sun::star::uno::Reference<
::com::sun::star::xml::sax::XExtendedDocumentHandler >,
::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > >
ScriptOutput_hash;
//=============================================================================

class ScriptStorage
            : public ::cppu::WeakImplHelper6<
            ::com::sun::star::lang::XServiceInfo,
            ::com::sun::star::lang::XInitialization,
            ::drafts::com::sun::star::script::framework::storage::XScriptImplAccess,
            ::drafts::com::sun::star::script::framework::storage::XScriptStorageExport,
            ::drafts::com::sun::star::script::framework::storage::XScriptAccessManager,
            ::drafts::com::sun::star::script::framework::storage::XParcelInvocationPrep >
{
    // private member
private:

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > m_xSimpleFileAccess;

    ::std::vector < ::rtl::OUString >  mv_logicalNames;
    ScriptInfo_hash mh_implementations;
    ScriptOutput_hash mh_parcels;
    sal_uInt16 m_scriptStorageID;

    osl::Mutex     m_mutex;

    void updateMaps(Impls_vec vScriptII);
    void writeMetadataHeader(::com::sun::star::uno::Reference < ::com::sun::star::xml::sax::XExtendedDocumentHandler > & );
    /**
       This function copies the contents of the source folder into the
       destination folder. If the destination folder does not exist, it
       is created. If the destination folder exists, it is deleted and then
       created. All URIs supported by the relevant XSimpleFileAccess
       implementation are supported.

        @params src
            the source folder (file URI)

        @params dest
            the destination folder (file URI)
    */
    void copyFolder(const ::rtl::OUString & src, const ::rtl::OUString & dest) throw (::com::sun::star::uno::RuntimeException);

public:
    //Constructors and Destructors
    //=========================================================================
    explicit ScriptStorage( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::uno::XComponentContext >& );
    //-------------------------------------------------------------------------
    virtual ~ScriptStorage() SAL_THROW( () );
    //=========================================================================

    // XServiceInfo impl
    //=========================================================================
    virtual ::rtl::OUString SAL_CALL getImplementationName()
    throw (::com::sun::star::uno::RuntimeException);
    //-------------------------------------------------------------------------
    virtual sal_Bool SAL_CALL supportsService(
        const ::rtl::OUString & ServiceName )
    throw (::com::sun::star::uno::RuntimeException);
    //-------------------------------------------------------------------------
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
    SAL_CALL getSupportedServiceNames()
    throw (::com::sun::star::uno::RuntimeException);
    //-------------------------------------------------------------------------
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >
    SAL_CALL getSupportedServiceNames_Static();
    //=========================================================================

    // XInitialization impl
    //=========================================================================
    virtual void SAL_CALL initialize( ::com::sun::star::uno::Sequence
                                      < ::com::sun::star::uno::Any > const & args )
    throw (::com::sun::star::uno::Exception);
    //=========================================================================

    // XScriptAccessManager impl
    //=========================================================================
    /**
     * Gets the sequence of XScriptInfo interfaces for the corresponding
     * logical name that is passed in
     *
     * @param name
     *      The logical name of the script
     *
     * @return Sequence< XScriptInfo >
     *      A sequence of XScriptInfos which represent the implementations
     *      of the passed in logical name
     */
    virtual ::com::sun::star::uno::Sequence<
    ::com::sun::star::uno::Reference<
    ::drafts::com::sun::star::script::framework::storage::XScriptInfo > >
    SAL_CALL getScriptInfoService( const ::rtl::OUString & name )
    throw (::com::sun::star::uno::RuntimeException);
    //=========================================================================

    //XScriptImplAccess
    //=========================================================================
    /**
     * Get the implementations for a given URI
     *
     * @param queryURI
     *      The URI to get the implementations for
     *
     * @return XScriptURI
     *      The URIs of the implementations
     */
    virtual ::com::sun::star::uno::Sequence<
    ::com::sun::star::uno::Reference<
    ::drafts::com::sun::star::script::framework::scripturi::XScriptURI > >
    SAL_CALL getImplementations( const ::com::sun::star::uno::Reference<
                                 ::drafts::com::sun::star::script::framework::scripturi::XScriptURI >& queryURI )
    throw (::com::sun::star::lang::IllegalArgumentException,
           ::com::sun::star::uno::RuntimeException);
    //=========================================================================


    //XNamingAccess
    //=========================================================================
    /**
     * Get a certain type of view of the naming heirarchy
     *
     * @param viewName
     *      The view name
     *
     * @return XInterface
     *      The view of the hierarchy
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    ScriptStorage::getView( const ::rtl::OUString& viewName )
    throw (::drafts::com::sun::star::script::framework::storage::NoSuchView,
           ::com::sun::star::uno::RuntimeException);
    //=========================================================================

    // XScriptStorageExport
    void SAL_CALL save()
    throw (::com::sun::star::uno::RuntimeException);
    //=========================================================================

    //XParcelInvocationPrep
    //=========================================================================
    /**
        copies a parcel to a temporary location

        @params parcelURI
            the location of the parcel (file URI) to be copied

        @return
            <type>::rtl::OUString</type> the new location of the parcel (file URI)
    */
    ::rtl::OUString SAL_CALL prepareForInvocation( const ::rtl::OUString& parcelURI )
    throw (::com::sun::star::uno::RuntimeException);
    //=========================================================================
}
; // class ScriptingStorage

}

#endif
