/*************************************************************************
 *
 *  $RCSfile: characterclassificationImpl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2002-03-26 17:57:44 $
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

/**************************************************************************

    Source Code Control System - Updates

    $Log: not supported by cvs2svn $
    Revision 1.1  2002/03/26 13:36:40  bustamam
    #97583# add Include files

    Revision 1.4  2001/10/19 21:16:45  bustamam.harun
    #84725# Add XServiceInfo implementation

    Revision 1.3  2001/05/18 17:57:33  er
    #79771# optimize: disentangled: cclass_unicode not derived from CharacterClassificationImpl; reuse instance if locale didn't change; rtl::OUString instead of String

    Revision 1.2  2001/04/12 23:53:54  bustamam.harun
    Fix compile problem on Solaris: change String to rtl::OUString

    Revision 1.1  2001/03/27 21:18:55  bustamam.harun
    Add locale sensitive character classification

    Revision 1.6  2001/01/29 17:05:55  er
    CharacterClassification with service manager

    Revision 1.5  2000/10/29 17:01:45  er
    i18n API moved from com.sun.star.lang to com.sun.star.i18n

    Revision 1.4  2000/08/11 14:51:29  er
    removed queryInterface/aquire/release, using WeakImplHelper instead

    Revision 1.3  2000/07/06 15:49:21  gmu
    changed parsing functions

    Revision 1.2  2000/07/06 15:21:04  er
    define USE_I18N_DEFAULT_IMPLEMENTATION and Locale dependent source

    Revision 1.1  2000/07/06 08:51:54  er
    new: CharacterClassification with parser


**************************************************************************/

#endif
