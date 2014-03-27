/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNOFLATPARA_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNOFLATPARA_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/text/XFlatParagraph.hpp>
#include <com/sun/star/text/XFlatParagraphIterator.hpp>
#include <calbck.hxx>
#include <modeltoviewhelper.hxx>
#include <unotextmarkup.hxx>

#include <set>

namespace com { namespace sun { namespace star { namespace container {
    class XStringKeyMap;
} } } }

namespace com { namespace sun { namespace star { namespace text {
    class XTextRange;
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
    SwXFlatParagraph( SwTxtNode& rTxtNode, const OUString& aExpandText, const ModelToViewHelper& rConversionMap );
    virtual ~SwXFlatParagraph();

    virtual     ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire(  ) throw() SAL_OVERRIDE;
    virtual void SAL_CALL release(  ) throw() SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // text::XTextMarkup:
    virtual css::uno::Reference< css::container::XStringKeyMap > SAL_CALL getMarkupInfoContainer() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL commitStringMarkup(::sal_Int32 nType, const OUString & aIdentifier, ::sal_Int32 nStart, ::sal_Int32 nLength,
                                   const css::uno::Reference< css::container::XStringKeyMap > & xMarkupInfoContainer) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL commitTextRangeMarkup(::sal_Int32 nType, const OUString & aIdentifier, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange> & xRange,
                                                const css::uno::Reference< css::container::XStringKeyMap > & xMarkupInfoContainer) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // text::XFlatParagraph:
    virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Bool SAL_CALL isModified() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setChecked(::sal_Int32 nType, ::sal_Bool bVal) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Bool SAL_CALL isChecked(::sal_Int32 nType) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::lang::Locale SAL_CALL getLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;
    virtual css::lang::Locale SAL_CALL getPrimaryLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL changeText(::sal_Int32 nPos, ::sal_Int32 nLen, const OUString & aNewText, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL changeAttributes(::sal_Int32 nPos, ::sal_Int32 nLen, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< ::sal_Int32 > SAL_CALL getLanguagePortions() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    const SwTxtNode* getTxtNode() const;

    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(const css::uno::Sequence< sal_Int8 >& rId)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    SwXFlatParagraph( const SwXFlatParagraph & ); // not defined
    SwXFlatParagraph & operator = ( const SwXFlatParagraph & ); // not defined

    OUString maExpandText;
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
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getFirstPara() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getNextPara() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getLastPara() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getParaBefore(const css::uno::Reference< css::text::XFlatParagraph > & xPara) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getParaAfter(const css::uno::Reference< css::text::XFlatParagraph > & xPara) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;

protected:
    // SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) SAL_OVERRIDE;

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
