/*************************************************************************
 *
 *  $RCSfile: characterclassificationImpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 10:54:27 $
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
#ifndef _I18N_CHARACTERCLASSIFICATIONIMPL_HXX_
#define _I18N_CHARACTERCLASSIFICATIONIMPL_HXX_

#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <tools/list.hxx>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com { namespace sun { namespace star { namespace i18n {

class CharacterClassificationImpl : public cppu::WeakImplHelper2
<
    XCharacterClassification,
    com::sun::star::lang::XServiceInfo
>
{
public:

    CharacterClassificationImpl( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    virtual ~CharacterClassificationImpl();

    virtual rtl::OUString SAL_CALL toUpper( const rtl::OUString& Text,
        sal_Int32 nPos, sal_Int32 nCount, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL toLower( const rtl::OUString& Text,
        sal_Int32 nPos, sal_Int32 nCount, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL toTitle( const rtl::OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getType( const rtl::OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getCharacterDirection( const rtl::OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getScript( const rtl::OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getCharacterType( const rtl::OUString& text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getStringType( const rtl::OUString& text, sal_Int32 nPos,
        sal_Int32 nCount, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException);
    virtual ParseResult SAL_CALL parseAnyToken( const rtl::OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& rLocale, sal_Int32 nStartCharFlags,
        const rtl::OUString& userDefinedCharactersStart, sal_Int32 nContCharFlags,
        const rtl::OUString& userDefinedCharactersCont )
        throw(com::sun::star::uno::RuntimeException);
    virtual ParseResult SAL_CALL parsePredefinedToken( sal_Int32 nTokenType,
        const rtl::OUString& Text, sal_Int32 nPos, const com::sun::star::lang::Locale& rLocale,
        sal_Int32 nStartCharFlags, const rtl::OUString& userDefinedCharactersStart,
        sal_Int32 nContCharFlags, const rtl::OUString& userDefinedCharactersCont )
        throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
                throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
        throw( com::sun::star::uno::RuntimeException );

private:
    struct lookupTableItem {
        lookupTableItem(const com::sun::star::lang::Locale& rLocale, const rtl::OUString& rName,
        com::sun::star::uno::Reference < XCharacterClassification >& rxCI) :
        aLocale(rLocale), aName(rName), xCI(rxCI) {};
        com::sun::star::lang::Locale aLocale;
        rtl::OUString aName;
        com::sun::star::uno::Reference < XCharacterClassification > xCI;
        sal_Bool SAL_CALL equals(const com::sun::star::lang::Locale& rLocale) {
        return aLocale.Language == rLocale.Language &&
            aLocale.Country == rLocale.Country &&
            aLocale.Variant == rLocale.Variant;
        };
    };
    List lookupTable;
    lookupTableItem *cachedItem;

    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xMSF;
    com::sun::star::uno::Reference < XCharacterClassification > xUCI;

    com::sun::star::uno::Reference < XCharacterClassification > SAL_CALL
    getLocaleSpecificCharacterClassification(const com::sun::star::lang::Locale& rLocale) throw(com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL
    createLocaleSpecificCharacterClassification(const rtl::OUString& serviceName, const com::sun::star::lang::Locale& rLocale);

};

} } } }

#endif
