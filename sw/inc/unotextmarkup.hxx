/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unotextmarkup.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 09:44:28 $
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

#ifndef _UNOTEXTMARKUP_HXX
#define _UNOTEXTMARKUP_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTMARKUP_HPP_
#include <com/sun/star/text/XTextMarkup.hpp>
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
#ifndef _MODELTOVIEWHELPER_HXX
#include <modeltoviewhelper.hxx>
#endif

#include <map>

namespace com { namespace sun { namespace star { namespace container {
    class XStringKeyMap;
} } } }

class SwTxtNode;
class SwWrongList;
class SfxPoolItem;

/** Implementation of the css::text::XTextMarkup interface
 */
class SwXTextMarkup:
    public ::cppu::WeakImplHelper1<
        ::com::sun::star::text::XTextMarkup >,
    public SwClient
{
public:
    SwXTextMarkup( SwTxtNode& rTxtNode, const ModelToViewHelper::ConversionMap* pConversionMap );
    virtual ~SwXTextMarkup();

    // ::com::sun::star::smarttags::XTextMarkup:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XStringKeyMap > SAL_CALL getMarkupInfoContainer() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL commitTextMarkup(::sal_Int32 nType, const ::rtl::OUString & aIdentifier, ::sal_Int32 nStart, ::sal_Int32 nLength, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XStringKeyMap > & xMarkupInfoContainer) throw (::com::sun::star::uno::RuntimeException);

    //SwClient
    virtual void        Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

private:
    SwXTextMarkup( const SwXTextMarkup & ); // not defined
    SwXTextMarkup & operator =( const SwXTextMarkup & ); // not defined

protected:
    SwTxtNode* mpTxtNode;
    const ModelToViewHelper::ConversionMap* mpConversionMap;

};


/** Implementation of the ::com::sun::star::container::XStringKeyMap interface
 */
class SwXStringKeyMap:
    public ::cppu::WeakImplHelper1<
        ::com::sun::star::container::XStringKeyMap>
{
public:
    SwXStringKeyMap();

    // ::com::sun::star::container::XStringKeyMap:
    virtual ::com::sun::star::uno::Any SAL_CALL getValue(const ::rtl::OUString & aKey) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::container::NoSuchElementException);
    virtual ::sal_Bool SAL_CALL hasValue(const ::rtl::OUString & aKey) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertValue(const ::rtl::OUString & aKey, const ::com::sun::star::uno::Any & aValue) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException);
    virtual ::sal_Int32 SAL_CALL getCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getKeyByIndex(::sal_Int32 nIndex) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException);
    virtual ::com::sun::star::uno::Any SAL_CALL getValueByIndex(::sal_Int32 nIndex) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException);

private:
    SwXStringKeyMap(SwXStringKeyMap &); // not defined
    void operator =(SwXStringKeyMap &); // not defined

    virtual ~SwXStringKeyMap() {}

    std::map< rtl::OUString, ::com::sun::star::uno::Any > maMap;
};

#endif


