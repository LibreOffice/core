/*************************************************************************
 *
 *  $RCSfile: dapiuno.hxx,v $
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

#ifndef SC_DAPIUNO_HXX
#define SC_DAPIUNO_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_RANGEUTL_HXX
#include "rangeutl.hxx"     // ScArea
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XDATAPILOTTABLE_HPP_
#include <com/sun/star/sheet/XDataPilotTable.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDATAPILOTTABLES_HPP_
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDATAPILOTDESCRIPTOR_HPP_
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_GENERALFUNCTION_HPP_
#include <com/sun/star/sheet/GeneralFunction.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDORIENTATION_HPP_
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif


class ScDocShell;
class ScPivot;

class ScDataPilotTableObj;
class ScDataPilotFieldObj;


#define SC_FIELDORIENT_ALL  USHRT_MAX


class ScDataPilotConversion
{
public:
    static com::sun::star::sheet::GeneralFunction   FirstFunc( USHORT nBits );
    static USHORT           FunctionBit( com::sun::star::sheet::GeneralFunction eFunc );
};



//  DataPilotTables Collection ist pro Tabelle

class ScDataPilotTablesObj : public cppu::WeakImplHelper4<
                                        com::sun::star::sheet::XDataPilotTables,
                                        com::sun::star::container::XEnumerationAccess,
                                        com::sun::star::container::XIndexAccess,
                                        com::sun::star::lang::XServiceInfo>,
                                    public SfxListener
{
private:
    ScDocShell*             pDocShell;
    USHORT                  nTab;

    ScDataPilotTableObj*    GetObjectByIndex_Impl(USHORT nIndex);
    ScDataPilotTableObj*    GetObjectByName_Impl(const ::rtl::OUString& aName);

public:
                            ScDataPilotTablesObj(ScDocShell* pDocSh, USHORT nT);
    virtual                 ~ScDataPilotTablesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XDataPilotTables
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotDescriptor > SAL_CALL
                            createDataPilotDescriptor() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   insertNewByName( const ::rtl::OUString& aName,
                                    const ::com::sun::star::table::CellAddress& aOutputAddress,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::sheet::XDataPilotDescriptor >& xDescriptor )
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


//  ScDataPilotDescriptorBase is never instantiated directly
class ScDataPilotDescriptorBase : public com::sun::star::sheet::XDataPilotDescriptor,
                                  public com::sun::star::lang::XServiceInfo,
                                  public com::sun::star::lang::XUnoTunnel,
                                  public com::sun::star::lang::XTypeProvider,
                                  public cppu::OWeakObject,
                                  public SfxListener
{
private:
    ScDocShell*     pDocShell;

public:
                            ScDataPilotDescriptorBase(ScDocShell* pDocSh);
    virtual                 ~ScDataPilotDescriptorBase();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   release() throw(::com::sun::star::uno::RuntimeException);

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ScDocShell*             GetDocShell() const     { return pDocShell; }

                            // in den Ableitungen:
    virtual void            GetParam( ScPivotParam& rParam, ScQueryParam& rQuery,
                                        ScArea& rSrcArea ) const = 0;
    virtual void            SetParam( const ScPivotParam& rParam, const ScQueryParam& rQuery,
                                        const ScArea& rSrcArea ) = 0;

                            // XDataPilotDescriptor
                            //  getName, setName, getTag, setTag in derived classes

    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getSourceRange()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSourceRange( const ::com::sun::star::table::CellRangeAddress& aSourceRange )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetFilterDescriptor > SAL_CALL
                            getFilterDescriptor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
                            getDataPilotFields() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
                            getColumnFields() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
                            getRowFields() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
                            getPageFields() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
                            getDataFields() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
                            getHiddenFields() throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScDataPilotDescriptorBase* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::sheet::XDataPilotDescriptor> xObj );

                            // XTypeProvider (overloaded in ScDataPilotTableObj)
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo is in derived classes
};


class ScDataPilotDescriptor : public ScDataPilotDescriptorBase
{
private:
    ScPivotParam    aParam;
    ScQueryParam    aQuery;
    ScArea          aSrcArea;
    String          aNameStr;
    String          aTagStr;

public:
                            ScDataPilotDescriptor(ScDocShell* pDocSh);
    virtual                 ~ScDataPilotDescriptor();

    virtual void    GetParam( ScPivotParam& rParam, ScQueryParam& rQuery, ScArea& rSrcArea ) const;
    virtual void    SetParam( const ScPivotParam& rParam,
                                const ScQueryParam& rQuery, const ScArea& rSrcArea );

                            // rest of XDataPilotDescriptor (incl. XNamed)
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTag() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setTag( const ::rtl::OUString& aTag )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScDataPilotTableObj : public ScDataPilotDescriptorBase,
                            public com::sun::star::sheet::XDataPilotTable
{
private:
    USHORT                  nTab;
    String                  aName;

public:
                            ScDataPilotTableObj(ScDocShell* pDocSh, USHORT nT, const String& rN);
    virtual                 ~ScDataPilotTableObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   release() throw(::com::sun::star::uno::RuntimeException);

    virtual void    GetParam( ScPivotParam& rParam, ScQueryParam& rQuery, ScArea& rSrcArea ) const;
    virtual void    SetParam( const ScPivotParam& rParam,
                                const ScQueryParam& rQuery, const ScArea& rSrcArea );

                            // rest of XDataPilotDescriptor (incl. XNamed)
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTag() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setTag( const ::rtl::OUString& aTag )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XDataPilotTable
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getOutputRange()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   refresh() throw(::com::sun::star::uno::RuntimeException);

                            // XTypeProvider (overloaded)
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScDataPilotFieldsObj : public cppu::WeakImplHelper4<
                                        com::sun::star::container::XEnumerationAccess,
                                        com::sun::star::container::XIndexAccess,
                                        com::sun::star::container::XNameAccess,
                                        com::sun::star::lang::XServiceInfo>
{
private:
    ScDataPilotDescriptorBase*  pParent;
    USHORT                      nType;  // DataPilotFieldOrientation oder SC_FIELDORIENT_ALL

    ScDataPilotFieldObj*    GetObjectByIndex_Impl(USHORT nIndex) const;
    ScDataPilotFieldObj*    GetObjectByName_Impl(const ::rtl::OUString& aName) const;

public:
                            ScDataPilotFieldsObj(ScDataPilotDescriptorBase* pPar, USHORT nTy);
    virtual                 ~ScDataPilotFieldsObj();

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


class ScDataPilotFieldObj : public cppu::WeakImplHelper3<
                                        com::sun::star::container::XNamed,
                                        com::sun::star::beans::XPropertySet,
                                        com::sun::star::lang::XServiceInfo>
{
private:
    SfxItemPropertySet          aPropSet;
    ScDataPilotDescriptorBase*  pParent;
    USHORT                      nField;         // Spalte im Datenbereich
    USHORT                      nSourceType;
    USHORT                      nSourcePos;

public:
                            ScDataPilotFieldObj(ScDataPilotDescriptorBase* pPar,
                                                    USHORT nF, USHORT nST, USHORT nSP);
    virtual                 ~ScDataPilotFieldObj();

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
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

                            // nur noch aus Property-Funktionen gerufen:
    com::sun::star::sheet::DataPilotFieldOrientation getOrientation(void) const;
    void setOrientation(com::sun::star::sheet::DataPilotFieldOrientation Orientation);
    com::sun::star::sheet::GeneralFunction getFunction(void) const;
    void setFunction(com::sun::star::sheet::GeneralFunction Function);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


#endif

