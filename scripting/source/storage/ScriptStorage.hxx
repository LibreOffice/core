/*************************************************************************
 *
 *  $RCSfile: ScriptStorage.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: npower $ $Date: 2003-08-19 09:50:31 $
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
#include <cppuhelper/implbase5.hxx> // helper for component factory

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <drafts/com/sun/star/script/framework/storage/XScriptInfoAccess.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptStorageExport.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptStorageRefresh.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptInfo.hpp>

namespace scripting_impl
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

//Typedefs
//=============================================================================
typedef ::std::vector< ScriptData > Datas_vec;
//-----------------------------------------------------------------------------
// function name -> ScriptData
typedef ::std::hash_map < ::rtl::OUString, ScriptData, ::rtl::OUStringHash,
            ::std::equal_to< ::rtl::OUString > > ScriptFunction_hash;
//-----------------------------------------------------------------------------
// language -> hash of function name -> ScriptData
typedef ::std::hash_map < ::rtl::OUString, ScriptFunction_hash,
            ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > >
ScriptData_hash;
//-----------------------------------------------------------------------------
typedef ::std::hash_map < ::rtl::OUString,
css::uno::Reference< css::xml::sax::XExtendedDocumentHandler >,
::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > >
ScriptOutput_hash;
//-----------------------------------------------------------------------------
typedef ::std::hash_map < ::rtl::OUString,
::rtl::OUString, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > >
ScriptLanguages_hash;

//=============================================================================

class ScriptStorage : public
    ::cppu::WeakImplHelper5<
        css::lang::XServiceInfo,
        css::lang::XInitialization,
        dcsssf::storage::XScriptInfoAccess,
        dcsssf::storage::XScriptStorageExport,
    dcsssf::storage::XScriptStorageRefresh >
{
public:
    //Constructors and Destructors
    //=========================================================================
    explicit ScriptStorage(
        const css::uno::Reference< css::uno::XComponentContext > & xContext )
        throw ( css::uno::RuntimeException );
    //-------------------------------------------------------------------------
    virtual ~ScriptStorage() SAL_THROW( () );
    //=========================================================================

    // XServiceInfo impl
    //=========================================================================
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw ( css::uno::RuntimeException );
    //-------------------------------------------------------------------------
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString & ServiceName )
        throw ( css::uno::RuntimeException );
    //-------------------------------------------------------------------------
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw ( css::uno::RuntimeException );
    //-------------------------------------------------------------------------
    static css::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames_Static();
    //=========================================================================

    // XInitialization impl
    //=========================================================================
    virtual void SAL_CALL
        initialize( css::uno::Sequence< css::uno::Any > const & args )
        throw ( css::uno::RuntimeException, css::uno::Exception );
    //=========================================================================

    //XScriptInfoAccess
    //=========================================================================
    /**
     * Get the logical names for this storage
     *
     * @return sequence < ::rtl::OUString >
     *      The logical names
     */
    virtual css::uno::Sequence< ::rtl::OUString >
        SAL_CALL getScriptLogicalNames()
        throw ( css::uno::RuntimeException );

    //=========================================================================
    /**
     * Get the implementations for a given URI
     *
     * @param queryURI
     *      The URI to get the implementations for
     *
     * @return sequence < XScriptInfo >
     *      The URIs of the implementations
     */
    virtual css::uno::Sequence< css::uno::Reference< dcsssf::storage::XScriptInfo > >
        SAL_CALL getImplementations(
            const ::rtl::OUString& queryURI )
        throw ( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    //=========================================================================
    /**
     * Get all script implementations
     *
     *
     * @return sequence < XScriptInfo >
     *      script implementations
     */
    virtual css::uno::Sequence< css::uno::Reference< dcsssf::storage::XScriptInfo > >
        SAL_CALL getAllImplementations()
        throw ( css::uno::RuntimeException );

    //=========================================================================

    /**
     * Save the scripts stored in the ScriptStorage into the corresponding
     * area (document or application)
     */
    void SAL_CALL save()
        throw ( css::uno::RuntimeException );
    //=========================================================================

    /**
     * Refresh the ScriptStorage from the data stored in the corresponding area
     * (document or application).
     */
    void SAL_CALL refresh()
        throw ( css::uno::RuntimeException );
    //=========================================================================

private:

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::ucb::XSimpleFileAccess > m_xSimpleFileAccess;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_xMgr;

    ::std::vector < ::rtl::OUString >  mv_logicalNames;
    static ScriptLanguages_hash* mh_scriptLangs;
    ScriptData_hash mh_implementations;
    ScriptOutput_hash mh_parcels;
    sal_Int32 m_scriptStorageID;
    ::rtl::OUString m_stringUri;

    osl::Mutex m_mutex;
    bool m_bInitialised;

    void updateMaps( const Datas_vec & vScriptDatas );
    void writeMetadataHeader(
        css::uno::Reference < css::xml::sax::XExtendedDocumentHandler > & xExDocHandler );
    void create ()
    throw (css::uno::RuntimeException, css::uno::Exception);
    void createForFilesystem ( const ::rtl::OUString & scriptLanguage )
    throw (css::uno::RuntimeException, css::uno::Exception);
    ::rtl::OUString getFileExtension ( const ::rtl::OUString & stringUri );

}; // class ScriptingStorage

} // namespace scripting_impl

#endif
