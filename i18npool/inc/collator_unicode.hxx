/*************************************************************************
 *
 *  $RCSfile: collator_unicode.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 13:36:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
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
#ifndef _I18N_COLLATOR_UNICODE_HXX_
#define _I18N_COLLATOR_UNICODE_HXX_

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/XCollator.hpp>
#include <transliteration_commonclass.hxx>
#include <cppuhelper/implbase2.hxx>

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

    virtual sal_Int32 SAL_CALL compareSubstring( const rtl::OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2) throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL compareString( const rtl::OUString& s1, const rtl::OUString& s2)
        throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL loadCollatorAlgorithm( const rtl::OUString& impl, const lang::Locale& rLocale,
        sal_Int32 collatorOptions) throw(com::sun::star::uno::RuntimeException);


    // following 4 methods are implemented in collatorImpl.
    sal_Int32 SAL_CALL loadDefaultCollator( const lang::Locale& rLocale,  sal_Int32 collatorOptions)
        throw(com::sun::star::uno::RuntimeException) {throw com::sun::star::uno::RuntimeException();}
    void SAL_CALL loadCollatorAlgorithmWithEndUserOption( const rtl::OUString& impl, const lang::Locale& rLocale,
        const com::sun::star::uno::Sequence< sal_Int32 >& collatorOptions) throw(com::sun::star::uno::RuntimeException) {throw com::sun::star::uno::RuntimeException();}
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL listCollatorAlgorithms( const lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException) {throw com::sun::star::uno::RuntimeException();}
    com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL listCollatorOptions( const rtl::OUString& collatorAlgorithmName )
        throw(com::sun::star::uno::RuntimeException) {throw com::sun::star::uno::RuntimeException();}

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException );

protected:
    sal_Char *implementationName;
    transliteration_commonclass* ignore;
    TransliterationModules tranModules;
};

} } } }

#endif
