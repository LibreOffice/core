/*************************************************************************
 *
 *  $RCSfile: numfmuno.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:03 $
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

#ifndef _NUMFMUNO_HXX
#define _NUMFMUNO_HXX

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATPREVIEWER_HPP_
#include <com/sun/star/util/XNumberFormatPreviewer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATS_HPP_
#include <com/sun/star/util/XNumberFormats.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

class SvNumberformat;
class SvNumberFormatter;
class SvNumberFormatsSupplierObj;


//  SvNumberFormatterServiceObj wird global als Service angemeldet

class SvNumberFormatterServiceObj : public cppu::WeakImplHelper3<
                                        com::sun::star::util::XNumberFormatter,
                                        com::sun::star::util::XNumberFormatPreviewer,
                                        com::sun::star::lang::XServiceInfo>
{
private:
    SvNumberFormatsSupplierObj* pSupplier;

public:
                        SvNumberFormatterServiceObj();
    virtual             ~SvNumberFormatterServiceObj();

                        // XNumberFormatter
    virtual void SAL_CALL attachNumberFormatsSupplier(
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XNumberFormatsSupplier >& xSupplier )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
                            SAL_CALL getNumberFormatsSupplier()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL detectNumberFormat( sal_Int32 nKey, const ::rtl::OUString& aString )
                            throw(::com::sun::star::util::NotNumericException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL convertStringToNumber( sal_Int32 nKey, const ::rtl::OUString& aString )
                            throw(::com::sun::star::util::NotNumericException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL convertNumberToString( sal_Int32 nKey, double fValue )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::util::color SAL_CALL queryColorForNumber( sal_Int32 nKey,
                            double fValue, ::com::sun::star::util::color aDefaultColor )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL formatString( sal_Int32 nKey, const ::rtl::OUString& aString )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::util::color SAL_CALL queryColorForString( sal_Int32 nKey,
                            const ::rtl::OUString& aString,
                                ::com::sun::star::util::color aDefaultColor )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getInputString( sal_Int32 nKey, double fValue )
                            throw(::com::sun::star::uno::RuntimeException);

                        // XNumberFormatPreviewer
    virtual ::rtl::OUString SAL_CALL convertNumberToPreviewString(
                            const ::rtl::OUString& aFormat, double fValue,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bAllowEnglish )
                                throw(::com::sun::star::util::MalformedNumberFormatException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::util::color SAL_CALL queryPreviewColorForNumber(
                            const ::rtl::OUString& aFormat, double fValue,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bAllowEnglish,
                            ::com::sun::star::util::color aDefaultColor )
                                throw(::com::sun::star::util::MalformedNumberFormatException,
                                        ::com::sun::star::uno::RuntimeException);

                        // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                            throw(::com::sun::star::uno::RuntimeException);
};


class SvNumberFormatsObj : public cppu::WeakImplHelper3<
                                        com::sun::star::util::XNumberFormats,
                                        com::sun::star::util::XNumberFormatTypes,
                                        com::sun::star::lang::XServiceInfo>
{
private:
    SvNumberFormatsSupplierObj* pSupplier;

public:
                    SvNumberFormatsObj();
                    SvNumberFormatsObj(SvNumberFormatsSupplierObj* pParent);
    virtual         ~SvNumberFormatsObj();


                        // XNumberFormats
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL
                            getByKey( sal_Int32 nKey ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL queryKeys( sal_Int16 nType,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bCreate )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL queryKey( const ::rtl::OUString& aFormat,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bScan )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL addNew( const ::rtl::OUString& aFormat,
                            const ::com::sun::star::lang::Locale& nLocale )
                                throw(::com::sun::star::util::MalformedNumberFormatException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL addNewConverted( const ::rtl::OUString& aFormat,
                            const ::com::sun::star::lang::Locale& nLocale,
                            const ::com::sun::star::lang::Locale& nNewLocale )
                                throw(::com::sun::star::util::MalformedNumberFormatException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByKey( sal_Int32 nKey ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL generateFormat( sal_Int32 nBaseKey,
                            const ::com::sun::star::lang::Locale& nLocale, sal_Bool bThousands,
                            sal_Bool bRed, sal_Int16 nDecimals, sal_Int16 nLeading )
                                throw(::com::sun::star::uno::RuntimeException);

                        // XNumberFormatTypes
    virtual sal_Int32 SAL_CALL getStandardIndex( const ::com::sun::star::lang::Locale& nLocale )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getStandardFormat( sal_Int16 nType,
                            const ::com::sun::star::lang::Locale& nLocale )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getFormatIndex( sal_Int16 nIndex,
                            const ::com::sun::star::lang::Locale& nLocale )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isTypeCompatible( sal_Int16 nOldType, sal_Int16 nNewType )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getFormatForLocale( sal_Int32 nKey,
                            const ::com::sun::star::lang::Locale& nLocale )
                                throw(::com::sun::star::uno::RuntimeException);

                        // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                            throw(::com::sun::star::uno::RuntimeException);
};


class SvNumberFormatObj : public cppu::WeakImplHelper3<
                                        com::sun::star::beans::XPropertySet,
                                        com::sun::star::beans::XPropertyAccess,
                                        com::sun::star::lang::XServiceInfo>
{
private:
    SvNumberFormatsSupplierObj* pSupplier;
    ULONG                       nKey;

public:
                    SvNumberFormatObj(SvNumberFormatsSupplierObj* pParent, ULONG nK);
    virtual         ~SvNumberFormatObj();

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

                            // XPropertyAccess
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            getPropertyValues() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValues( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& aProps )
                                    throw(::com::sun::star::beans::UnknownPropertyException,
                                            ::com::sun::star::beans::PropertyVetoException,
                                            ::com::sun::star::lang::IllegalArgumentException,
                                            ::com::sun::star::lang::WrappedTargetException,
                                            ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class SvNumberFormatSettingsObj : public cppu::WeakImplHelper2<
                                        com::sun::star::beans::XPropertySet,
                                        com::sun::star::lang::XServiceInfo>
{
private:
    SvNumberFormatsSupplierObj* pSupplier;

public:
                    SvNumberFormatSettingsObj(SvNumberFormatsSupplierObj* pParent);
    virtual         ~SvNumberFormatSettingsObj();


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

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};



#endif

