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

#ifndef SW_UNOMETA_HXX
#define SW_UNOMETA_HXX

#include <deque>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase6.hxx>

#include <sfx2/Metadatable.hxx>

#include <unobaseclass.hxx>


typedef ::std::deque<
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > >
    TextRangeList_t;

class SwPaM;
class SwTxtNode;

namespace sw {
    class Meta;
    class MetaField;
}

typedef ::cppu::ImplInheritanceHelper6
<   ::sfx2::MetadatableMixin
,   ::com::sun::star::lang::XUnoTunnel
,   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::container::XChild
,   ::com::sun::star::container::XEnumerationAccess
,   ::com::sun::star::text::XTextContent
,   ::com::sun::star::text::XText
> SwXMeta_Base;

class SwXMeta
    : public SwXMeta_Base
    , private ::boost::noncopyable
{

public:

    class Impl;

protected:

    ::sw::UnoImplPtr<Impl> m_pImpl;

    virtual void SAL_CALL AttachImpl(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange,
            const sal_uInt16 nWhich)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    virtual ~SwXMeta();

    /// @param pDoc and pMeta != 0, but not & because of ImplInheritanceHelper
    SwXMeta(SwDoc *const pDoc, ::sw::Meta *const pMeta,
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XText> const&
            xParentText,
        TextRangeList_t const*const pPortions);

public:

    SwXMeta(SwDoc *const pDoc);

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::rdf::XMetadatable >
        CreateXMeta(
            ::sw::Meta & rMeta,
            ::com::sun::star::uno::Reference< ::com::sun::star::text::XText>
                const& xParentText = 0,
            ::std::auto_ptr<TextRangeList_t const> pPortions =
                ::std::auto_ptr<TextRangeList_t const>(0));

    /// init params with position of the attribute content (w/out CH_TXTATR)
    bool SetContentRange(
            SwTxtNode *& rpNode, xub_StrLen & rStart, xub_StrLen & rEnd) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >
        GetParentText() const;

    bool CheckForOwnMemberMeta(const SwPaM & rPam, const bool bAbsorb)
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

    // MetadatableMixin
    virtual ::sfx2::Metadatable * GetCoreObject();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        GetModel();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const ::com::sun::star::uno::Sequence< sal_Int8 >& Identifier )
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(
            const ::rtl::OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException);

    // XChild
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface > SAL_CALL
        getParent()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface> const& xParent)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::NoSupportException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw (::com::sun::star::uno::RuntimeException);

    // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XEnumeration >  SAL_CALL
        createEnumeration()
        throw (::com::sun::star::uno::RuntimeException);

    // XTextContent
    virtual void SAL_CALL attach(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange)
        throw (::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL getAnchor()
        throw (::com::sun::star::uno::RuntimeException);

    // XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >
        SAL_CALL getText() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL getStart()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL getEnd()
        throw (::com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getString()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setString(const rtl::OUString& rString)
        throw (::com::sun::star::uno::RuntimeException);

    // XSimpleText
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >  SAL_CALL
        createTextCursor()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >  SAL_CALL
        createTextCursorByRange(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextPosition)
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertString(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xRange,
            const rtl::OUString& aString, sal_Bool bAbsorb)
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertControlCharacter(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xRange,
            sal_Int16 nControlCharacter, sal_Bool bAbsorb)
        throw (::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException);

    // XText
    virtual void SAL_CALL insertTextContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xRange,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent > & xContent,
            sal_Bool bAbsorb)
        throw (::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTextContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent > & xContent)
        throw (::com::sun::star::container::NoSuchElementException,
            ::com::sun::star::uno::RuntimeException);

};


typedef ::cppu::ImplInheritanceHelper2
<   SwXMeta
,   ::com::sun::star::beans::XPropertySet
,   ::com::sun::star::text::XTextField
> SwXMetaField_Base;

class SwXMetaField
    : public SwXMetaField_Base
{

private:

    virtual ~SwXMetaField();

    friend ::com::sun::star::uno::Reference<
            ::com::sun::star::rdf::XMetadatable >
        SwXMeta::CreateXMeta(::sw::Meta &,
            ::com::sun::star::uno::Reference< ::com::sun::star::text::XText>
                const&,
            ::std::auto_ptr<TextRangeList_t const> pPortions);

    SwXMetaField(SwDoc *const pDoc, ::sw::Meta *const pMeta,
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XText> const&
            xParentText,
        TextRangeList_t const*const pPortions);

public:

    SwXMetaField(SwDoc *const pDoc);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(
            const ::rtl::OUString& ServiceName)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames( )
        throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Any& rValue)
        throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::beans::PropertyVetoException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL
        getPropertyValue(const ::rtl::OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XVetoableChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XVetoableChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException);

    // XTextContent
    virtual void SAL_CALL attach(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange)
        throw ( ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XTextRange > SAL_CALL getAnchor()
        throw (::com::sun::star::uno::RuntimeException);

    // XTextField
    virtual rtl::OUString SAL_CALL getPresentation(sal_Bool bShowCommand)
        throw (::com::sun::star::uno::RuntimeException);

};

/// get prefix/suffix from the RDF repository. @throws RuntimeException
void getPrefixAndSuffix(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel>& xModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::rdf::XMetadatable>& xMetaField,
        ::rtl::OUString *const o_pPrefix, ::rtl::OUString *const o_pSuffix);

#endif // SW_UNOMETA_HXX

