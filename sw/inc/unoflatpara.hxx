/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoflatpara.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 09:44:10 $
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

#ifndef _UNOFLATPARA_HXX
#define _UNOFLATPARA_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_XFLATPARAGRAPH_HPP_
#include <com/sun/star/text/XFlatParagraph.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XFLATPARAGRAPHITERATOR_HPP_
#include <com/sun/star/text/XFlatParagraphIterator.hpp>
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
#ifndef _MODELTOVIEWHELPER_HXX
#include <modeltoviewhelper.hxx>
#endif

#ifndef _UNOTEXTMARKUP_HXX
#include <unotextmarkup.hxx>
#endif

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
    public ::cppu::WeakImplHelper1<
        css::text::XFlatParagraph >,
    public SwXTextMarkup
{
public:
    SwXFlatParagraph( SwTxtNode& rTxtNode, rtl::OUString aExpandText, const ModelToViewHelper::ConversionMap* pConversionMap );
    virtual ~SwXFlatParagraph();

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

private:
    SwXFlatParagraph( const SwXFlatParagraph & ); // not defined
    SwXFlatParagraph & operator = ( const SwXFlatParagraph & ); // not defined

    rtl::OUString maExpandText;
};

/******************************************************************************
 * SwXFlatParagraphIterator
 ******************************************************************************/

class SwXFlatParagraphIterator:
    public ::cppu::WeakImplHelper1<
        css::text::XFlatParagraphIterator>
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

private:
    SwXFlatParagraphIterator( const SwXFlatParagraphIterator & ); // not defined
    SwXFlatParagraphIterator & operator =(const SwXFlatParagraphIterator & ); // not defined

    // container to hold the 'hard' references as long as necessary and valid
    std::set< css::uno::Reference< css::text::XFlatParagraph > >    m_aFlatParaList;

    SwDoc* mpDoc;
    const sal_Int32 mnType;
    const sal_Bool mbAutomatic;

    ULONG mnCurrentNode;    // used for non-automatic mode
    ULONG mnStartNode;      // used for non-automatic mode
    ULONG mnEndNode;        // used for non-automatic mode
    sal_Bool mbWrapped;     // used for non-automatic mode
};

#endif
