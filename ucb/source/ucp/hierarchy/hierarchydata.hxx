/*************************************************************************
 *
 *  $RCSfile: hierarchydata.hxx,v $
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

namespace com { namespace sun { namespace star { namespace util {
    class XChangesBatch;
    class XStringEscape;
} } } }

namespace hierarchy_ucp
{

//=========================================================================

struct HierarchyEntryData
{
    ::rtl::OUString aTitle;     // Title
    ::rtl::OUString aTargetURL; // Target URL ( links only )

    HierarchyEntryData() {}
    HierarchyEntryData( const ::rtl::OUString& rTitle,
                        const ::rtl::OUString& rTargetURL )
    : aTitle( rTitle ), aTargetURL( rTargetURL ) {}
};

//=========================================================================

class HierarchyEntry
{
    ::rtl::OUString m_aPath;
    ::osl::Mutex    m_aMutex;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > m_xConfigProvider;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::util::XStringEscape >        m_xEscaper;

private:
    sal_Bool getData( ::com::sun::star::uno::Any& rTitle,
                      ::com::sun::star::uno::Any& rTargetURL,
                      sal_Bool bChildren,
                      ::com::sun::star::uno::Any& rChildren );
    sal_Bool setData( const ::rtl::OUString& rOldPath,
                      const ::rtl::OUString& rPath,
                      const ::com::sun::star::uno::Any& rTitle,
                      const ::com::sun::star::uno::Any& rTargetURL,
                      sal_Bool bCreate,
                      sal_Bool bFailIfExists,
                      sal_Bool bChildren,
                      const ::com::sun::star::uno::Any& rChildren,
                      const ::com::sun::star::uno::Reference<
                          ::com::sun::star::util::XChangesBatch >& rxBatch,
                      const ::rtl::OUString& rBatchPath );
    ::rtl::OUString createPathFromHierarchyURL( const ::rtl::OUString& rURL );

public:
    HierarchyEntry( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                    const ::rtl::OUString& rURL );

    sal_Bool hasData();

    sal_Bool getData( HierarchyEntryData& rData );

    sal_Bool setData( const HierarchyEntryData& rData, sal_Bool bCreate );

    sal_Bool move( const ::rtl::OUString& rNewURL );

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
