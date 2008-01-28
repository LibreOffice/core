/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: collator_unicode.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 15:33:02 $
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
#ifndef _I18N_COLLATOR_UNICODE_HXX_
#define _I18N_COLLATOR_UNICODE_HXX_

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/XCollator.hpp>
#include <cppuhelper/implbase1.hxx>
#include <osl/module.h>

#include "warnings_guard_unicode_tblcoll.h"

//      ----------------------------------------------------
//      class Collator_Unicode
//      ----------------------------------------------------

namespace com { namespace sun { namespace star { namespace i18n {

class Collator_Unicode : public cppu::WeakImplHelper1 < XCollator >
{
public:
    // Constructors
    Collator_Unicode();
    // Destructor
    ~Collator_Unicode();

    sal_Int32 SAL_CALL compareSubstring( const rtl::OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2) throw(com::sun::star::uno::RuntimeException);

    sal_Int32 SAL_CALL compareString( const rtl::OUString& s1, const rtl::OUString& s2)
        throw(com::sun::star::uno::RuntimeException);

    sal_Int32 SAL_CALL loadCollatorAlgorithm( const rtl::OUString& impl, const lang::Locale& rLocale,
        sal_Int32 collatorOptions) throw(com::sun::star::uno::RuntimeException);


    // following 4 methods are implemented in collatorImpl.
    sal_Int32 SAL_CALL loadDefaultCollator( const lang::Locale&,  sal_Int32)
        throw(com::sun::star::uno::RuntimeException) {throw com::sun::star::uno::RuntimeException();}
    void SAL_CALL loadCollatorAlgorithmWithEndUserOption( const rtl::OUString&, const lang::Locale&,
        const com::sun::star::uno::Sequence< sal_Int32 >&) throw(com::sun::star::uno::RuntimeException) {throw com::sun::star::uno::RuntimeException();}
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL listCollatorAlgorithms( const lang::Locale&)
        throw(com::sun::star::uno::RuntimeException) {throw com::sun::star::uno::RuntimeException();}
    com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL listCollatorOptions( const rtl::OUString& )
        throw(com::sun::star::uno::RuntimeException) {throw com::sun::star::uno::RuntimeException();}

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException );

protected:
    const sal_Char *implementationName;
private:
    RuleBasedCollator *uca_base, *collator;
    oslModule hModule;
};

} } } }

#endif
