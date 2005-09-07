/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: collatorImpl.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:51:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _I18N_COLLATORIMPL_HXX_
#define _I18N_COLLATORIMPL_HXX_

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/XLocaleData.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

#ifndef INCLUDED_SAL_ALLOCA_H
#include <sal/alloca.h>
#endif
#include <vector>

namespace com { namespace sun { namespace star { namespace i18n {
//      ----------------------------------------------------
//      class CollatorImpl
//      ----------------------------------------------------
class CollatorImpl : public cppu::WeakImplHelper2
<
    XCollator,
    com::sun::star::lang::XServiceInfo
>
{
public:

    // Constructors
    CollatorImpl( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    // Destructor
    ~CollatorImpl();

    virtual sal_Int32 SAL_CALL compareSubstring(const rtl::OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2) throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL compareString( const rtl::OUString& s1,
        const rtl::OUString& s2) throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL loadDefaultCollator( const lang::Locale& rLocale,  sal_Int32 collatorOptions)
        throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL loadCollatorAlgorithm(  const rtl::OUString& impl, const lang::Locale& rLocale,
        sal_Int32 collatorOptions) throw(com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL loadCollatorAlgorithmWithEndUserOption( const rtl::OUString& impl, const lang::Locale& rLocale,
        const com::sun::star::uno::Sequence< sal_Int32 >& collatorOptions) throw(com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL listCollatorAlgorithms( const lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL listCollatorOptions( const rtl::OUString& collatorAlgorithmName )
        throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException );

protected:
    lang::Locale nLocale;
private :
    struct lookupTableItem {
        lang::Locale aLocale;
        rtl::OUString algorithm;
        rtl::OUString service;
        com::sun::star::uno::Reference < XCollator > xC;
        lookupTableItem(const lang::Locale& rLocale, const rtl::OUString& _algorithm, const rtl::OUString& _service,
        com::sun::star::uno::Reference < XCollator >& _xC) : aLocale(rLocale), algorithm(_algorithm), service(_service), xC(_xC) {}
        sal_Bool SAL_CALL equals(const lang::Locale& rLocale, const rtl::OUString& _algorithm) {
        return aLocale.Language == rLocale.Language &&
            aLocale.Country == rLocale.Country &&
            aLocale.Variant == rLocale.Variant &&
            algorithm == _algorithm;
        }
    };
    std::vector<lookupTableItem*> lookupTable;
    lookupTableItem *cachedItem;

    // Service Factory
    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xMSF;
    // lang::Locale Data
    com::sun::star::uno::Reference < XLocaleData > localedata;

    sal_Bool SAL_CALL createCollator(const lang::Locale& rLocale, const rtl::OUString& serviceName,
        const rtl::OUString& rSortAlgorithm) throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL loadCachedCollator(const lang::Locale& rLocale, const rtl::OUString& rSortAlgorithm)
        throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif
