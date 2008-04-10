/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: inputsequencechecker.hxx,v $
 * $Revision: 1.7 $
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
#ifndef _I18N_INPUTCHECKER_HXX_
#define _I18N_INPUTCHECKER_HXX_

#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/i18n/XExtendedInputSequenceChecker.hpp>

#include <vector>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class InputSequenceCheckerImpl
//  ----------------------------------------------------
class InputSequenceCheckerImpl : public cppu::WeakImplHelper2
<
    com::sun::star::i18n::XExtendedInputSequenceChecker,
    com::sun::star::lang::XServiceInfo
>
{
public:
    InputSequenceCheckerImpl( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    InputSequenceCheckerImpl();
    ~InputSequenceCheckerImpl();

    virtual sal_Bool SAL_CALL checkInputSequence(const rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL correctInputSequence(rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException );

protected:
    sal_Int16 checkMode;
    const sal_Char *serviceName;

private :
    struct lookupTableItem {
        lookupTableItem(const sal_Char* rLanguage, const com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedInputSequenceChecker >& rxISC) :
            aLanguage(rLanguage), xISC(rxISC) {}
        const sal_Char* aLanguage;
        com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedInputSequenceChecker > xISC;
    };
    std::vector<lookupTableItem*> lookupTable;
    lookupTableItem *cachedItem;

    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xMSF;

    com::sun::star::uno::Reference< com::sun::star::i18n::XExtendedInputSequenceChecker >& SAL_CALL getInputSequenceChecker(sal_Char* rLanguage)
        throw (com::sun::star::uno::RuntimeException);
    sal_Char* SAL_CALL getLanguageByScripType(sal_Unicode cChar, sal_Unicode nChar);
};

} } } }

#endif // _I18N_BREAKITERATOR_HXX_
