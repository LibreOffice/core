/*************************************************************************
 *
 *  $RCSfile: hierarchydata.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 14:23:11 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

#ifndef _HIERARCHYDATA_HXX
#define _HIERARCHYDATA_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

namespace com { namespace sun { namespace star {
    namespace container {
        class XHierarchicalNameAccess;
    }
    namespace util {
        class XOfficeInstallationDirectories;
    }
} } }

namespace hierarchy_ucp
{

//=========================================================================

class HierarchyEntryData
{
public:
    enum Type { NONE, LINK, FOLDER };

    HierarchyEntryData() : m_aType( NONE ) {}
    HierarchyEntryData( const Type & rType ) : m_aType( rType ) {}

    const rtl::OUString & getName() const { return m_aName; }
    void setName( const rtl::OUString & rName ) { m_aName = rName; }

    const rtl::OUString & getTitle() const { return m_aTitle; }
    void setTitle( const rtl::OUString & rTitle ) { m_aTitle = rTitle; }

    const rtl::OUString & getTargetURL() const { return m_aTargetURL; }
    void setTargetURL( const rtl::OUString & rURL ) { m_aTargetURL = rURL; }

    Type getType() const
    { return ( m_aType != NONE ) ? m_aType
                                 : m_aTargetURL.getLength()
                                    ? LINK
                                    : FOLDER; }
    void setType( const Type & rType ) { m_aType = rType; }

private:
    rtl::OUString m_aName;      // Name (language independent)
    rtl::OUString m_aTitle;     // Title (language dependent)
    rtl::OUString m_aTargetURL; // Target URL ( links only )
    Type          m_aType;      // Type
};

//=========================================================================

class HierarchyContentProvider;
class HierarchyUri;

class HierarchyEntry
{
    ::rtl::OUString m_aServiceSpecifier;
    ::rtl::OUString m_aName;
    ::rtl::OUString m_aPath;
    ::osl::Mutex    m_aMutex;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > m_xConfigProvider;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XHierarchicalNameAccess >
                                                           m_xRootReadAccess;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::util::XOfficeInstallationDirectories >
                                                           m_xOfficeInstDirs;
    sal_Bool m_bTriedToGetRootReadAccess;  // #82494#

private:
    ::rtl::OUString createPathFromHierarchyURL( const HierarchyUri & rURI );
    ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XHierarchicalNameAccess >
    getRootReadAccess();

public:
    HierarchyEntry( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                    HierarchyContentProvider* pProvider,
                    const ::rtl::OUString& rURL );

    sal_Bool hasData();

    sal_Bool getData( HierarchyEntryData& rData );

    sal_Bool setData( const HierarchyEntryData& rData, sal_Bool bCreate );

    sal_Bool move( const ::rtl::OUString& rNewURL,
                   const HierarchyEntryData& rData );

    sal_Bool remove();

    // Iteration.

    struct iterator_Impl;

    class iterator
    {
    friend class HierarchyEntry;

        iterator_Impl*  m_pImpl;

    public:
        iterator();
        ~iterator();

        const HierarchyEntryData& operator*() const;
    };

    sal_Bool first( iterator& it );
    sal_Bool next ( iterator& it );
};

} // namespace hierarchy_ucp

#endif /* !_HIERARCHYDATA_HXX */
