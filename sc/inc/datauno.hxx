/*************************************************************************
 *
 *  $RCSfile: datauno.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2001-03-12 16:43:25 $
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

#ifndef SC_DATAUNO_HXX
#define SC_DATAUNO_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_TABLEFILTERFIELD_HPP_
#include <com/sun/star/sheet/TableFilterField.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_GENERALFUNCTION_HPP_
#include <com/sun/star/sheet/GeneralFunction.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEREFERRER_HPP_
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETFILTERDESCRIPTOR_HPP_
#include <com/sun/star/sheet/XSheetFilterDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCONSOLIDATIONDESCRIPTOR_HPP_
#include <com/sun/star/sheet/XConsolidationDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDATABASERANGES_HPP_
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDATABASERANGE_HPP_
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSUBTOTALDESCRIPTOR_HPP_
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSUBTOTALFIELD_HPP_
#include <com/sun/star/sheet/XSubTotalField.hpp>
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
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif

class ScDBData;
class ScDocShell;

class ScSubTotalFieldObj;
class ScDatabaseRangeObj;
class ScDataPilotDescriptorBase;

struct ScSortParam;


class ScDataUnoConversion
{
public:
    static ScSubTotalFunc   GeneralToSubTotal( com::sun::star::sheet::GeneralFunction eSummary );
    static com::sun::star::sheet::GeneralFunction SubTotalToGeneral( ScSubTotalFunc eSubTotal );
};


//  ImportDescriptor gibt's nicht mehr als Uno-Objekt, nur noch Property-Sequence

class ScImportDescriptor
{
public:
    static void FillImportParam(
                    ScImportParam& rParam,
                    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rSeq );
    static void FillProperties(
                    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rSeq,
                    const ScImportParam& rParam );
    static long GetPropertyCount();
};

//  SortDescriptor gibt's nicht mehr als Uno-Objekt, nur noch Property-Sequence

class ScSortDescriptor
{
public:
    static void FillSortParam(
                    ScSortParam& rParam,
                    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rSeq );
    static void FillProperties(
                    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rSeq,
                    const ScSortParam& rParam );
    static long GetPropertyCount();
};


//  ScSubTotalDescriptorBase - Basisklasse fuer SubTotalDescriptor alleine und im DB-Bereich

//  to uno, both look the same

class ScSubTotalDescriptorBase : public cppu::WeakImplHelper6<
                                        com::sun::star::sheet::XSubTotalDescriptor,
                                        com::sun::star::container::XEnumerationAccess,
                                        com::sun::star::container::XIndexAccess,
                                        com::sun::star::beans::XPropertySet,
                                        com::sun::star::lang::XUnoTunnel,
                                        com::sun::star::lang::XServiceInfo >
{
private:
    SfxItemPropertySet      aPropSet;

    ScSubTotalFieldObj*     GetObjectByIndex_Impl(USHORT nIndex);

public:
                            ScSubTotalDescriptorBase();
    virtual                 ~ScSubTotalDescriptorBase();

                            // in derived classes:
                            // (Fields are within the range)
    virtual void            GetData( ScSubTotalParam& rParam ) const = 0;
    virtual void            PutData( const ScSubTotalParam& rParam ) = 0;

                            // XSubTotalDescriptor
    virtual void SAL_CALL   addNew( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::sheet::SubTotalColumn >& aSubTotalColumns,
                                sal_Int32 nGroupColumn )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   clear() throw(::com::sun::star::uno::RuntimeException);

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

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScSubTotalDescriptorBase* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::sheet::XSubTotalDescriptor> xObj );

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


//  ScSubTotalDescriptor - dummer Container zur Benutzung mit XImportTarget

class ScSubTotalDescriptor : public ScSubTotalDescriptorBase
{
private:
    ScSubTotalParam         aStoredParam;

public:
                            ScSubTotalDescriptor();
    virtual                 ~ScSubTotalDescriptor();

                            // von ScSubTotalDescriptorBase:
    virtual void            GetData( ScSubTotalParam& rParam ) const;
    virtual void            PutData( const ScSubTotalParam& rParam );

                            // Zugriff von aussen:
    void                    SetParam( const ScSubTotalParam& rNew );
//  const ScSubTotalParam&  GetParam() const    { return aStoredParam; }
};


//  ScRangeSubTotalDescriptor - SubTotalDescriptor eines Datenbank-Bereichs

class ScRangeSubTotalDescriptor : public ScSubTotalDescriptorBase
{
private:
    ScDatabaseRangeObj*     pParent;

public:
                            ScRangeSubTotalDescriptor(ScDatabaseRangeObj* pPar);
    virtual                 ~ScRangeSubTotalDescriptor();

                            // von ScSubTotalDescriptorBase:
    virtual void            GetData( ScSubTotalParam& rParam ) const;
    virtual void            PutData( const ScSubTotalParam& rParam );
};


class ScSubTotalFieldObj : public cppu::WeakImplHelper2<
                                com::sun::star::sheet::XSubTotalField,
                                com::sun::star::lang::XServiceInfo >
{
private:
    com::sun::star::uno::Reference<com::sun::star::sheet::XSubTotalDescriptor> xRef;
    ScSubTotalDescriptorBase&   rParent;
    USHORT                      nPos;

public:
                            ScSubTotalFieldObj( ScSubTotalDescriptorBase* pDesc, USHORT nP );
    virtual                 ~ScSubTotalFieldObj();

                            // XSubTotalField
    virtual sal_Int32 SAL_CALL getGroupColumn() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setGroupColumn( sal_Int32 nGroupColumn )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::SubTotalColumn > SAL_CALL
                            getSubTotalColumns() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setSubTotalColumns( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::sheet::SubTotalColumn >& aSubTotalColumns )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScConsolidationDescriptor : public cppu::WeakImplHelper2<
                                        com::sun::star::sheet::XConsolidationDescriptor,
                                        com::sun::star::lang::XServiceInfo >
{
private:
    ScConsolidateParam      aParam;

public:
                            ScConsolidationDescriptor();
    virtual                 ~ScConsolidationDescriptor();

    void                    SetParam( const ScConsolidateParam& rNew );
    const ScConsolidateParam& GetParam() const  { return aParam; }

                            // XConsolidationDescriptor
    virtual ::com::sun::star::sheet::GeneralFunction SAL_CALL getFunction()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFunction( ::com::sun::star::sheet::GeneralFunction nFunction )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress > SAL_CALL
                            getSources(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setSources( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::table::CellRangeAddress >& aSources )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellAddress SAL_CALL getStartOutputPosition()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setStartOutputPosition(
                                const ::com::sun::star::table::CellAddress& aStartOutputPosition )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getUseColumnHeaders() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setUseColumnHeaders( sal_Bool bUseColumnHeaders )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getUseRowHeaders() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setUseRowHeaders( sal_Bool bUseRowHeaders )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getInsertLinks() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setInsertLinks( sal_Bool bInsertLinks )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


//  ScFilterDescriptorBase - Basisklasse fuer FilterDescriptor
//                           alleine, im DB-Bereich und im DataPilot

//  to uno, all three look the same

class ScFilterDescriptorBase : public cppu::WeakImplHelper3<
                                    com::sun::star::sheet::XSheetFilterDescriptor,
                                    com::sun::star::beans::XPropertySet,
                                    com::sun::star::lang::XServiceInfo >
{
private:
    SfxItemPropertySet      aPropSet;

public:
                            ScFilterDescriptorBase();
    virtual                 ~ScFilterDescriptorBase();

                            // in den Ableitungen:
                            // (nField[] hier innerhalb des Bereichs)
    virtual void            GetData( ScQueryParam& rParam ) const = 0;
    virtual void            PutData( const ScQueryParam& rParam ) = 0;

                            // XSheetFilterDescriptor
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::TableFilterField > SAL_CALL
                            getFilterFields() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFilterFields( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::sheet::TableFilterField >& aFilterFields )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


//  ScFilterDescriptor - dummer Container zur Benutzung mit XFilterable

class ScFilterDescriptor : public ScFilterDescriptorBase
{
private:
    ScQueryParam            aStoredParam;       // nField[] hier innerhalb des Bereichs

public:
                            ScFilterDescriptor();
    virtual                 ~ScFilterDescriptor();

                            // von ScFilterDescriptorBase:
    virtual void            GetData( ScQueryParam& rParam ) const;
    virtual void            PutData( const ScQueryParam& rParam );

                            // Zugriff von aussen:
    void                    SetParam( const ScQueryParam& rNew );
    const ScQueryParam&     GetParam() const    { return aStoredParam; }
};


//  ScRangeFilterDescriptor - FilterDescriptor eines Datenbank-Bereichs

class ScRangeFilterDescriptor : public ScFilterDescriptorBase
{
private:
    ScDatabaseRangeObj*     pParent;

public:
                            ScRangeFilterDescriptor(ScDatabaseRangeObj* pPar);
    virtual                 ~ScRangeFilterDescriptor();

                            // von ScFilterDescriptorBase:
    virtual void            GetData( ScQueryParam& rParam ) const;
    virtual void            PutData( const ScQueryParam& rParam );
};


//  ScDataPilotFilterDescriptor - FilterDescriptor eines DataPilotDescriptors

class ScDataPilotFilterDescriptor : public ScFilterDescriptorBase
{
private:
    ScDataPilotDescriptorBase*  pParent;

public:
                            ScDataPilotFilterDescriptor(ScDataPilotDescriptorBase* pPar);
    virtual                 ~ScDataPilotFilterDescriptor();

                            // von ScFilterDescriptorBase:
    virtual void            GetData( ScQueryParam& rParam ) const;
    virtual void            PutData( const ScQueryParam& rParam );
};


class ScDatabaseRangeObj : public cppu::WeakImplHelper5<
                                com::sun::star::sheet::XDatabaseRange,
                                com::sun::star::container::XNamed,
                                com::sun::star::sheet::XCellRangeReferrer,
                                com::sun::star::beans::XPropertySet,
                                com::sun::star::lang::XServiceInfo >,
                           public SfxListener
{
private:
    ScDocShell*             pDocShell;
    String                  aName;
    SfxItemPropertySet      aPropSet;

private:
    ScDBData*               GetDBData_Impl() const;

public:
                            ScDatabaseRangeObj(ScDocShell* pDocSh, const String& rNm);
    virtual                 ~ScDatabaseRangeObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // nField[] hier innerhalb des Bereichs:
    void                    GetQueryParam(ScQueryParam& rQueryParam) const;
    void                    SetQueryParam(const ScQueryParam& rQueryParam);
    void                    GetSortParam(ScSortParam& rSortParam) const;
    void                    SetSortParam(const ScSortParam& rSortParam);
    void                    GetImportParam(ScImportParam& rImportParam) const;
    void                    SetImportParam(const ScImportParam& rImportParam);
    void                    GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const;
    void                    SetSubTotalParam(const ScSubTotalParam& rSubTotalParam);

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XDatabaseRange
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getDataArea()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setDataArea( const ::com::sun::star::table::CellRangeAddress& aDataArea )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            getSortDescriptor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetFilterDescriptor > SAL_CALL
                            getFilterDescriptor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSubTotalDescriptor > SAL_CALL
                            getSubTotalDescriptor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            getImportDescriptor() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   refresh() throw(::com::sun::star::uno::RuntimeException);

                            // XCellRangeReferrer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getReferredCells() throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScDatabaseRangesObj : public cppu::WeakImplHelper4<
                                com::sun::star::sheet::XDatabaseRanges,
                                com::sun::star::container::XEnumerationAccess,
                                com::sun::star::container::XIndexAccess,
                                com::sun::star::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;

    ScDatabaseRangeObj*     GetObjectByIndex_Impl(USHORT nIndex);
    ScDatabaseRangeObj*     GetObjectByName_Impl(const ::rtl::OUString& aName);

public:
                            ScDatabaseRangesObj(ScDocShell* pDocSh);
    virtual                 ~ScDatabaseRangesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XDatabaseRanges
    virtual void SAL_CALL   addNewByName( const ::rtl::OUString& aName,
                                const ::com::sun::star::table::CellRangeAddress& aRange )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};



#endif

