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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNOBOOKMARK_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNOBOOKMARK_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XFormField.hpp>

#include <cppuhelper/implbase.hxx>

#include <sfx2/Metadatable.hxx>

#include <unobaseclass.hxx>
#include <IDocumentMarkAccess.hxx>

class SwDoc;

typedef ::cppu::ImplInheritanceHelper
<   ::sfx2::MetadatableMixin
,   ::com::sun::star::lang::XUnoTunnel
,   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::beans::XPropertySet
,   ::com::sun::star::container::XNamed
,   ::com::sun::star::text::XTextContent
> SwXBookmark_Base;

class SwXBookmark
    : public SwXBookmark_Base
{

private:

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

protected:

    void attachToRangeEx(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange,
            IDocumentMarkAccess::MarkType eType)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException );
    virtual void attachToRange(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    const ::sw::mark::IMark* GetBookmark() const;

    void registerInMark( SwXBookmark& rXMark, ::sw::mark::IMark* const pMarkBase );

    virtual ~SwXBookmark();

    /// @param pDoc and pMark != 0, but not & because of ImplInheritanceHelper
    SwXBookmark(::sw::mark::IMark *const pMark, SwDoc *const pDoc);

    /// descriptor
    SwXBookmark();

public:

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XTextContent>
        CreateXBookmark(SwDoc & rDoc, ::sw::mark::IMark * pBookmark);

    /// @return IMark for this, but only if it lives in pDoc
    static ::sw::mark::IMark const* GetBookmarkInDoc(SwDoc const*const pDoc,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XUnoTunnel> & xUT);

    // MetadatableMixin
    virtual ::sfx2::Metadatable* GetCoreObject() override;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel > GetModel() override;

    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Any& rValue)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XVetoableChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XVetoableChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XNamed
    virtual OUString SAL_CALL getName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setName(const OUString& rName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTextContent
    virtual void SAL_CALL attach(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL getAnchor()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

};

class SwXFieldmarkParameters
    : public ::cppu::WeakImplHelper< ::com::sun::star::container::XNameContainer>
    , private SwClient
{
    public:
        SwXFieldmarkParameters(::sw::mark::IFieldmark* const pFieldmark)
        {
            pFieldmark->Add(this);
        }

        // XNameContainer
        virtual void SAL_CALL insertByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeByName( const OUString& Name ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        // XNameReplace
        virtual void SAL_CALL replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        // XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    protected:
        //SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;
    private:
        ::sw::mark::IFieldmark::parameter_map_t* getCoreParameters() throw (::com::sun::star::uno::RuntimeException);
};

typedef cppu::ImplInheritanceHelper< SwXBookmark,
    ::com::sun::star::text::XFormField > SwXFieldmark_Base;

class SwXFieldmark
    : public SwXFieldmark_Base
{

private:
    ::sw::mark::ICheckboxFieldmark* getCheckboxFieldmark();
    bool isReplacementObject;

protected:

    SwXFieldmark(bool isReplacementObject,
            ::sw::mark::IMark* pBkm = 0, SwDoc* pDoc = 0);

public:

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XTextContent>
        CreateXFieldmark(SwDoc & rDoc, ::sw::mark::IMark * pMark,
                bool isReplacementObject = false);

    virtual void attachToRange(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException) override;
    virtual OUString SAL_CALL getFieldType()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setFieldType(const OUString& description )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL getParameters(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Any& rValue)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
};

class SwXODFCheckboxField : public SwXFieldmark
{
public:
    SwXODFCheckboxField( ::sw::mark::IMark* pBkm = 0, SwDoc* pDoc = 0) : SwXFieldmark(true,
            pBkm, pDoc) {}
};
#endif // INCLUDED_SW_SOURCE_CORE_INC_UNOBOOKMARK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
