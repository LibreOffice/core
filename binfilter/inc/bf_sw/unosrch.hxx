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
#ifndef _UNOSRCH_HXX
#define _UNOSRCH_HXX

#ifndef _COM_SUN_STAR_UTIL_XPROPERTYREPLACE_HPP_
#include <com/sun/star/util/XPropertyReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>	// helper for implementations
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
namespace com{namespace sun{namespace star{namespace util{
    struct SearchOptions;
}}}}
namespace binfilter {

struct SfxItemPropertyMap; 
class SfxItemSet; 
/******************************************************************************
 *
 ******************************************************************************/

class SwXTextDocument;
class SwSearchProperties_Impl;


//STRIP008 namespace com{namespace sun{namespace star{namespace util{
//STRIP008 	struct SearchOptions;
//STRIP008 }}}}


/*-----------------19.12.97 12:58-------------------

--------------------------------------------------*/
class SwXTextSearch : public cppu::WeakImplHelper3
<
    ::com::sun::star::util::XPropertyReplace,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel
>
{
    friend class SwXTextDocument;

    String 					sSearchText;
    String 					sReplaceText;

    SwSearchProperties_Impl* 	pSearchProperties;
    SwSearchProperties_Impl* 	pReplaceProperties;


    const SfxItemPropertyMap* 	_pMap;
    sal_Bool 					bAll  : 1;
    sal_Bool					bWord : 1;
    sal_Bool					bBack : 1;
    sal_Bool					bExpr : 1;
    sal_Bool					bCase : 1;
//	sal_Bool					bInSel: 1;  // wie geht Suchen in Selektionen?
    sal_Bool					bStyles:1;
    sal_Bool					bSimilarity : 1;
    sal_Bool					bLevRelax 		:1;
    sal_Int16  					nLevExchange;
    sal_Int16  					nLevAdd;
    sal_Int16 					nLevRemove;

    sal_Bool 					bIsValueSearch :1;
protected:
    virtual ~SwXTextSearch();
public:
    SwXTextSearch();
    


    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XSearchDescriptor
    virtual ::rtl::OUString SAL_CALL getSearchString(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSearchString( const ::rtl::OUString& aString ) throw(::com::sun::star::uno::RuntimeException);

    //XReplaceDescriptor
    virtual ::rtl::OUString SAL_CALL getReplaceString(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setReplaceString(const ::rtl::OUString& aReplaceString) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyReplace
    virtual sal_Bool SAL_CALL getValueSearch(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setValueSearch(sal_Bool ValueSearch_) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getSearchAttributes(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setSearchAttributes(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aSearchAttribs) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getReplaceAttributes(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setReplaceAttributes(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aSearchAttribs) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    void	FillSearchItemSet(SfxItemSet& rSet) const;
    void	FillReplaceItemSet(SfxItemSet& rSet) const;

    sal_Bool	HasSearchAttributes() const;
    sal_Bool	HasReplaceAttributes() const;

    void	FillSearchOptions( ::com::sun::star::util::SearchOptions&
                                        rSearchOpt ) const;
};

} //namespace binfilter
#endif

