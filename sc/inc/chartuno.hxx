/*************************************************************************
 *
 *  $RCSfile: chartuno.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:48 $
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

#ifndef SC_CHARTUNO_HXX
#define SC_CHARTUNO_HXX

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _COM_SUN_STAR_TABLE_XTABLECHART_HPP_
#include <com/sun/star/table/XTableChart.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_XTABLECHARTS_HPP_
#include <com/sun/star/table/XTableCharts.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEMBEDDEDOBJECTSUPPLIER_HPP_
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif


class ScDocShell;
class ScRangeListRef;
class ScChartObj;


class ScChartsObj : public cppu::WeakImplHelper4<
                            com::sun::star::table::XTableCharts,
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::container::XIndexAccess,
                            com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    USHORT                  nTab;           // Charts sind pro Sheet

    ScChartObj*             GetObjectByIndex_Impl(long nIndex) const;
    ScChartObj*             GetObjectByName_Impl(const ::rtl::OUString& aName) const;

public:
                            ScChartsObj(ScDocShell* pDocSh, USHORT nT);
    virtual                 ~ScChartsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XTableCharts
    virtual void SAL_CALL   addNewByName( const ::rtl::OUString& aName,
                                    const ::com::sun::star::awt::Rectangle& aRect,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::table::CellRangeAddress >& aRanges,
                                    sal_Bool bColumnHeaders, sal_Bool bRowHeaders )
                                        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByName( const ::rtl::OUString& aName )
                                        throw(::com::sun::star::uno::RuntimeException);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScChartObj : public cppu::WeakImplHelper3<
                            com::sun::star::table::XTableChart,
                            com::sun::star::document::XEmbeddedObjectSupplier,
                            com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    USHORT                  nTab;           // Charts sind pro Sheet
    String                  aChartName;

    void    Update_Impl( const ScRangeListRef& rRanges, BOOL bColHeaders, BOOL bRowHeaders );
    void    GetData_Impl( ScRangeListRef& rRanges, BOOL& rColHeaders, BOOL& rRowHeaders ) const;

public:
                            ScChartObj(ScDocShell* pDocSh, USHORT nT, const String& rN);
    virtual                 ~ScChartObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XTableChart
    virtual sal_Bool SAL_CALL getHasColumnHeaders() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setHasColumnHeaders( sal_Bool bHasColumnHeaders )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getHasRowHeaders() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setHasRowHeaders( sal_Bool bHasRowHeaders )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress > SAL_CALL
                            getRanges(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setRanges( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::table::CellRangeAddress >& aRanges )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XEmbeddedObjectSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > SAL_CALL
                            getEmbeddedObject() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};

#endif

