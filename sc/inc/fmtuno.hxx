/*************************************************************************
 *
 *  $RCSfile: fmtuno.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-21 09:34:00 $
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

#ifndef SC_FMTUNO_HXX
#define SC_FMTUNO_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"           // ScRange, ScAddress
#endif

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif

#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSHEETCONDITIONALENTRIES_HPP_
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETCONDITION_HPP_
#include <com/sun/star/sheet/XSheetCondition.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETCONDITIONALENTRY_HPP_
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
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
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif


class ScDocument;
class ScTableConditionalEntry;
class ScConditionalFormat;
class ScValidationData;


//  "erstmal" nur dummer Container

class ScTableConditionalFormat : public cppu::WeakImplHelper5<
                            com::sun::star::sheet::XSheetConditionalEntries,
                            com::sun::star::container::XNameAccess,
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::lang::XUnoTunnel,
                            com::sun::star::lang::XServiceInfo >
{
private:
    List    aEntries;

    ScTableConditionalEntry*    GetObjectByIndex_Impl(USHORT nIndex) const;
    void                        AddEntry_Impl( USHORT nMode,
                                    const String& rExpr1, const String& rExpr2,
                                    const ScAddress& rPos, const String& rStyle );

public:
                            ScTableConditionalFormat();
                            ScTableConditionalFormat(ScDocument* pDoc, ULONG nKey, BOOL bEnglish);
    virtual                 ~ScTableConditionalFormat();

    void                    FillFormat( ScConditionalFormat& rFormat,
                                            ScDocument* pDoc, BOOL bEnglish ) const;
    void                    DataChanged();

                            // XSheetConditionalEntries
    virtual void SAL_CALL   addNew( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::beans::PropertyValue >& aConditionalEntry )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   clear() throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScTableConditionalFormat* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::sheet::XSheetConditionalEntries> xObj );

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};



class ScTableConditionalEntry : public cppu::WeakImplHelper3<
                            com::sun::star::sheet::XSheetCondition,
                            com::sun::star::sheet::XSheetConditionalEntry,
                            com::sun::star::lang::XServiceInfo >
{
private:
    ScTableConditionalFormat*   pParent;
    USHORT                      nMode;      // enum ScConditionMode
    String                      aExpr1;
    String                      aExpr2;
    ScAddress                   aSrcPos;
    String                      aStyle;     // display name as stored in ScStyleSheet

public:
                            ScTableConditionalEntry();
                            ScTableConditionalEntry( ScTableConditionalFormat* pPar,
                                                     USHORT nM, const String& rEx1,
                                                     const String& rEx2, const ScAddress& rPos,
                                                     const String& rSt );
    virtual                 ~ScTableConditionalEntry();

    void                    GetData( USHORT& rM, String& rEx1, String& rEx2,
                                        ScAddress& rPos, String& rSt ) const;

                            // XSheetCondition
    virtual ::com::sun::star::sheet::ConditionOperator SAL_CALL getOperator()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setOperator( ::com::sun::star::sheet::ConditionOperator nOperator )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getFormula1() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFormula1( const ::rtl::OUString& aFormula1 )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getFormula2() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFormula2( const ::rtl::OUString& aFormula2 )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellAddress SAL_CALL getSourcePosition()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSourcePosition( const ::com::sun::star::table::CellAddress& aSourcePosition )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XSheetConditionalEntry
    virtual ::rtl::OUString SAL_CALL getStyleName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setStyleName( const ::rtl::OUString& aStyleName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};



class ScTableValidationObj : public cppu::WeakImplHelper4<
                            com::sun::star::sheet::XSheetCondition,
                            com::sun::star::beans::XPropertySet,
                            com::sun::star::lang::XUnoTunnel,
                            com::sun::star::lang::XServiceInfo >
{
private:
    SfxItemPropertySet  aPropSet;
    USHORT              nMode;          // enum ScConditionMode
    String              aExpr1;
    String              aExpr2;
    ScAddress           aSrcPos;
    USHORT              nValMode;       // enum ScValidationMode
    BOOL                bIgnoreBlank;
    BOOL                bShowInput;
    String              aInputTitle;
    String              aInputMessage;
    BOOL                bShowError;
    USHORT              nErrorStyle;    // enum ScValidErrorStyle
    String              aErrorTitle;
    String              aErrorMessage;

    void                    ClearData_Impl();

public:
                            ScTableValidationObj();
                            ScTableValidationObj(ScDocument* pDoc, ULONG nKey, BOOL bEnglish);
    virtual                 ~ScTableValidationObj();

    ScValidationData*       CreateValidationData( ScDocument* pDoc, BOOL bEnglish ) const;
    void                    DataChanged();

                            // XSheetCondition
    virtual ::com::sun::star::sheet::ConditionOperator SAL_CALL getOperator()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setOperator( ::com::sun::star::sheet::ConditionOperator nOperator )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getFormula1() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFormula1( const ::rtl::OUString& aFormula1 )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getFormula2() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFormula2( const ::rtl::OUString& aFormula2 )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellAddress SAL_CALL getSourcePosition()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSourcePosition( const ::com::sun::star::table::CellAddress& aSourcePosition )
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

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScTableValidationObj* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::beans::XPropertySet> xObj );

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


#endif

