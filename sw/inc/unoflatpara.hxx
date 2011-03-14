/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _UNOFLATPARA_HXX
#define _UNOFLATPARA_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/text/XFlatParagraph.hpp>
#include <com/sun/star/text/XFlatParagraphIterator.hpp>
#include <calbck.hxx>
#include <modeltoviewhelper.hxx>
#include <unotextmarkup.hxx>

#include <set>

namespace css = com::sun::star;

namespace com { namespace sun { namespace star { namespace container {
    class XStringKeyMap;
} } } }

class SwTxtNode;
class SwDoc;

/******************************************************************************
 * SwXFlatParagraph
 ******************************************************************************/

class SwXFlatParagraph:
    public ::cppu::WeakImplHelper2
    <
        css::text::XFlatParagraph,
        css::lang::XUnoTunnel
    >,
    public SwXTextMarkup
{
public:
    SwXFlatParagraph( SwTxtNode& rTxtNode, rtl::OUString aExpandText, const ModelToViewHelper::ConversionMap* pConversionMap );
    virtual ~SwXFlatParagraph();

    virtual     ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // text::XTextMarkup:
    virtual css::uno::Reference< css::container::XStringKeyMap > SAL_CALL getMarkupInfoContainer() throw (css::uno::RuntimeException);
    virtual void SAL_CALL commitTextMarkup(::sal_Int32 nType, const ::rtl::OUString & aIdentifier, ::sal_Int32 nStart, ::sal_Int32 nLength, const css::uno::Reference< css::container::XStringKeyMap > & xMarkupInfoContainer) throw (css::uno::RuntimeException);

    // text::XFlatParagraph:
    virtual ::rtl::OUString SAL_CALL getText() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isModified() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setChecked(::sal_Int32 nType, ::sal_Bool bVal) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isChecked(::sal_Int32 nType) throw (css::uno::RuntimeException);
    virtual css::lang::Locale SAL_CALL getLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException);
    virtual css::lang::Locale SAL_CALL getPrimaryLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException);
    virtual void SAL_CALL changeText(::sal_Int32 nPos, ::sal_Int32 nLen, const ::rtl::OUString & aNewText, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException);
    virtual void SAL_CALL changeAttributes(::sal_Int32 nPos, ::sal_Int32 nLen, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException);
    virtual css::uno::Sequence< ::sal_Int32 > SAL_CALL getLanguagePortions() throw (css::uno::RuntimeException);

    const SwTxtNode* getTxtNode() const;

    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(const css::uno::Sequence< sal_Int8 >& rId)
        throw (css::uno::RuntimeException);

private:
    SwXFlatParagraph( const SwXFlatParagraph & ); // not defined
    SwXFlatParagraph & operator = ( const SwXFlatParagraph & ); // not defined

    rtl::OUString maExpandText;
};

/******************************************************************************
 * SwXFlatParagraphIterator
 ******************************************************************************/

class SwXFlatParagraphIterator:
    public ::cppu::WeakImplHelper1
    <
        css::text::XFlatParagraphIterator
    >,
    public SwClient     // to get notified when doc is closed...
{
public:
    SwXFlatParagraphIterator( SwDoc& rDoc, sal_Int32 nType, sal_Bool bAutomatic );
    virtual ~SwXFlatParagraphIterator();

    // text::XFlatParagraphIterator:
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getFirstPara() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getNextPara() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getLastPara() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getParaBefore(const css::uno::Reference< css::text::XFlatParagraph > & xPara) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException);
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getParaAfter(const css::uno::Reference< css::text::XFlatParagraph > & xPara) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException);

    // SwClient
    virtual void    Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );

private:
    SwXFlatParagraphIterator( const SwXFlatParagraphIterator & ); // not defined
    SwXFlatParagraphIterator & operator =(const SwXFlatParagraphIterator & ); // not defined

    // container to hold the 'hard' references as long as necessary and valid
    std::set< css::uno::Reference< css::text::XFlatParagraph > >    m_aFlatParaList;

    SwDoc* mpDoc;
    const sal_Int32 mnType;
    const sal_Bool mbAutomatic;

    sal_uLong mnCurrentNode;    // used for non-automatic mode
    sal_uLong mnStartNode;      // used for non-automatic mode
    sal_uLong mnEndNode;        // used for non-automatic mode
    sal_Bool mbWrapped;     // used for non-automatic mode
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
