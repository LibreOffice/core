/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _HIERARCHYDATA_HXX
#define _HIERARCHYDATA_HXX

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
