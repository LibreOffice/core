/*************************************************************************
 *
 *  $RCSfile: hierarchycontent.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:54:18 $
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

#ifndef _HIERARCHYCONTENT_HXX
#define _HIERARCHYCONTENT_HXX

#ifndef __LIST__
#include <stl/list>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef _UCBHELPER_CONTENTHELPER_HXX
#include <ucbhelper/contenthelper.hxx>
#endif

#ifndef _HIERARCHYDATA_HXX
#include "hierarchydata.hxx"
#endif

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    struct PropertyValue;
} } } }

namespace com { namespace sun { namespace star { namespace sdbc {
    class XRow;
} } } }

namespace com { namespace sun { namespace star { namespace ucb {
    struct TransferInfo;
} } } }

namespace hierarchy_ucp
{

//=========================================================================

#define HIERARCHY_ROOT_FOLDER_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.HierarchyRootFolderContent"
#define HIERARCHY_FOLDER_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.HierarchyFolderContent"
#define HIERARCHY_LINK_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.HierarchyLinkContent"

//=========================================================================

struct HierarchyContentProperties : HierarchyEntryData
{
    ::rtl::OUString aContentType;   // ContentType
    sal_Bool        bIsDocument;    // IsDocument
    sal_Bool        bIsFolder;      // IsFolder

    HierarchyContentProperties()
    : bIsDocument( sal_False ), bIsFolder( sal_True ) {}
};

//=========================================================================

class HierarchyContent : public ::ucb::ContentImplHelper,
                         public com::sun::star::ucb::XContentCreator
{
    enum ContentKind  { LINK, FOLDER, ROOT };
    enum ContentState { TRANSIENT,  // created via CreateNewContent,
                                       // but did not process "insert" yet
                        PERSISTENT, // processed "insert"
                        DEAD        // processed "delete"
                      };

    HierarchyContentProperties m_aProps;
    ContentKind                m_eKind;
    ContentState               m_eState;

private:
    HierarchyContent(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            ::ucb::ContentProviderImplHelper* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const HierarchyContentProperties& rProps );
    HierarchyContent(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            ::ucb::ContentProviderImplHelper* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const com::sun::star::ucb::ContentInfo& Info );

    virtual const ::ucb::PropertyInfoTableEntry& getPropertyInfoTable();
    virtual const ::ucb::CommandInfoTableEntry&  getCommandInfoTable();
    virtual ::rtl::OUString getParentURL();

    static sal_Bool hasData(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier );
    sal_Bool hasData(
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier )
    { return hasData( m_xSMgr, Identifier ); }
    static sal_Bool loadData(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            HierarchyContentProperties& rProps );
    sal_Bool storeData();
    sal_Bool renameData( const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XContentIdentifier >& xOldId,
                         const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XContentIdentifier >& xNewId );
    sal_Bool removeData();

    void setKind( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& Identifier );

    sal_Bool isFolder() const { return ( m_eKind > LINK ); }

    ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentIdentifier >
    getIdentifierFromTitle();

    typedef vos::ORef< HierarchyContent > HierarchyContentRef;
    typedef std::list< HierarchyContentRef > HierarchyContentRefList;
    void queryChildren( HierarchyContentRefList& rChildren );

    sal_Bool exchangeIdentity(
                const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XContentIdentifier >& xNewId );

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Sequence<
                             ::com::sun::star::beans::Property >& rProperties );
    void setPropertyValues(
            const ::com::sun::star::uno::Sequence<
                     ::com::sun::star::beans::PropertyValue >& rValues );

    void insert( sal_Int32 nNameClashResolve )
        throw( ::com::sun::star::ucb::CommandAbortedException );

    void destroy( sal_Bool bDeletePhysical )
        throw( ::com::sun::star::ucb::CommandAbortedException );

    void HierarchyContent::transfer(
                        const ::com::sun::star::ucb::TransferInfo& rInfo )
        throw( ::com::sun::star::ucb::CommandAbortedException );

public:
    // Create existing content. Fail, if not already exists.
    static HierarchyContent* create(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            ::ucb::ContentProviderImplHelper* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier );

    // Create new content. Fail, if already exists.
    static HierarchyContent* create(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            ::ucb::ContentProviderImplHelper* pProvider,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >& Identifier,
            const com::sun::star::ucb::ContentInfo& Info );

    virtual ~HierarchyContent();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL
    getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException );

    // XContent
    virtual rtl::OUString SAL_CALL
    getContentType()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier > SAL_CALL
    getIdentifier()
        throw( com::sun::star::uno::RuntimeException );

    // XCommandProcessor
    virtual com::sun::star::uno::Any SAL_CALL
    execute( const com::sun::star::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( com::sun::star::uno::Exception,
               com::sun::star::ucb::CommandAbortedException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    abort( sal_Int32 CommandId )
        throw( com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Additional interfaces
    //////////////////////////////////////////////////////////////////////

    // XContentCreator
    virtual com::sun::star::uno::Sequence<
                com::sun::star::ucb::ContentInfo > SAL_CALL
    queryCreatableContentsInfo()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContent > SAL_CALL
    createNewContent( const com::sun::star::ucb::ContentInfo& Info )
        throw( com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValues( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                       const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::Property >& rProperties,
                       const HierarchyContentProperties& rData,
                       const ::vos::ORef< ::ucb::ContentProviderImplHelper >&
                               rProvider,
                       const ::rtl::OUString& rContentId );
};

} // namespace hierarchy_ucp

#endif /* !_HIERARCHYCONTENT_HXX */
