/*************************************************************************
 *
 *  $RCSfile: convdic.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 15:30:11 $
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

#ifndef _LINGUISTIC_CONVDIC_HXX_
#define _LINGUISTIC_CONVDIC_HXX_

#ifndef _COM_SUN_STAR_LINGUISTIC2_XCONVERSIONDICTIONARY_HPP_
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#include <stl/hash_map>
#include <stl/set>

#ifndef _LINGUISTIC_MISC_HXX_
#include "misc.hxx"
#endif
#ifndef _LINGUISTIC_DEFS_HXX_
#include "defs.hxx"
#endif

// text conversion dictionary extension
#define CONV_DIC_EXT            "tcd"
#define CONV_DIC_DOT_EXT        ".tcd"

#define SN_CONV_DICTIONARY      "com.sun.star.linguistic2.ConversionDictionary"


class SvStream;

///////////////////////////////////////////////////////////////////////////

BOOL    IsConvDic( const String &rFileURL, INT16 &nLang, sal_Int16 &nConvType );

///////////////////////////////////////////////////////////////////////////

struct StrLT
{
    bool operator()( const rtl::OUString &rTxt1, const rtl::OUString &rTxt2 ) const
    {
        return rTxt1 < rTxt2;
    }
};

struct StrEQ
{
    bool operator()( const rtl::OUString &rTxt1, const rtl::OUString &rTxt2 ) const
    {
        return rTxt1 == rTxt2;
    }
};

namespace std
{
template <> struct hash< const rtl::OUString >
{
    inline size_t operator()( const rtl::OUString &rTxt ) const
    {
        return (size_t) rTxt.hashCode();
    }
};
}

typedef std::hash_multimap< const rtl::OUString, const rtl::OUString,
                       std::hash< const rtl::OUString >, StrEQ > ConvMap;

typedef std::set< rtl::OUString, StrLT > ConvMapKeySet;

///////////////////////////////////////////////////////////////////////////

class ConvDic :
    public ::cppu::WeakImplHelper3
    <
        ::com::sun::star::linguistic2::XConversionDictionary,
        ::com::sun::star::util::XFlushable,
        ::com::sun::star::lang::XServiceInfo
    >
{
    friend class ConvDicXMLExport;

protected:

    ::cppu::OInterfaceContainerHelper       aFlushListeners;

    ConvMap         aFromLeft;
    ConvMap         aFromRight;

    String          aMainURL;   // URL to file
    rtl::OUString   aName;
    INT16           nLanguage;
    sal_Int16       nConversionType;
    sal_Int16       nMaxLeftCharCount;
    sal_Int16       nMaxRightCharCount;
    BOOL            bMaxCharCountIsValid;
    BOOL            bNeedEntries;
    BOOL            bIsModified;
    BOOL            bIsActive;
    BOOL            bIsReadonly;

    // disallow copy-constructor and assignment-operator for now
    ConvDic(const ConvDic &);
    ConvDic & operator = (const ConvDic &);

    ConvMap::iterator   GetEntry( ConvMap &rMap, const rtl::OUString &rFirstText, const rtl::OUString &rSecondText );
    void    Load();
    void    Save();

public:
    ConvDic( const String &rName,
             INT16 nLanguage,
             sal_Int16 nConversionType,
             const String &rMainURL );
    virtual ~ConvDic();

    // XConversionDictionary
    virtual ::rtl::OUString SAL_CALL getName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getConversionType(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActive( sal_Bool bActivate ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isActive(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clear(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getConversions( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, ::com::sun::star::linguistic2::ConversionDirection eDirection, sal_Int32 nTextConversionOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getConversionEntries( ::com::sun::star::linguistic2::ConversionDirection eDirection ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEntry( const ::rtl::OUString& aLeftText, const ::rtl::OUString& aRightText ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEntry( const ::rtl::OUString& aLeftText, const ::rtl::OUString& aRightText ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getMaxCharCount( ::com::sun::star::linguistic2::ConversionDirection eDirection ) throw (::com::sun::star::uno::RuntimeException);

    // XFlushable
    virtual void SAL_CALL flush(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);


    static inline ::rtl::OUString
        getImplementationName_Static() throw();
    static com::sun::star::uno::Sequence< ::rtl::OUString >
        getSupportedServiceNames_Static() throw();

    BOOL    HasEntry( const rtl::OUString &rLeftText, const rtl::OUString &rRightText );
    void    AddEntry( const rtl::OUString &rLeftText, const rtl::OUString &rRightText );
    void    RemoveEntry( const rtl::OUString &rLeftText, const rtl::OUString &rRightText );
};

inline ::rtl::OUString ConvDic::getImplementationName_Static() throw()
{
    return A2OU( "com.sun.star.lingu2.ConvDic" );
}

///////////////////////////////////////////////////////////////////////////

#endif

