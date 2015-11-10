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
,   css::lang::XUnoTunnel
,   css::lang::XServiceInfo
,   css::beans::XPropertySet
,   css::container::XNamed
,   css::text::XTextContent
> SwXBookmark_Base;

class SwXBookmark
    : public SwXBookmark_Base
{

private:

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

protected:

    void attachToRangeEx(
            const css::uno::Reference< css::text::XTextRange > & xTextRange,
            IDocumentMarkAccess::MarkType eType)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException );
    virtual void attachToRange(
            const css::uno::Reference< css::text::XTextRange > & xTextRange)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException);

    const ::sw::mark::IMark* GetBookmark() const;

    void registerInMark( SwXBookmark& rXMark, ::sw::mark::IMark* const pMarkBase );

    virtual ~SwXBookmark();

    /// @param pDoc and pMark != 0, but not & because of ImplInheritanceHelper
    SwXBookmark(::sw::mark::IMark *const pMark, SwDoc *const pDoc);

    /// descriptor
    SwXBookmark();

public:

    static css::uno::Reference< css::text::XTextContent>
        CreateXBookmark(SwDoc & rDoc, ::sw::mark::IMark * pBookmark);

    /// @return IMark for this, but only if it lives in pDoc
    static ::sw::mark::IMark const* GetBookmarkInDoc(SwDoc const*const pDoc,
            const css::uno::Reference<css::lang::XUnoTunnel> & xUT);

    // MetadatableMixin
    virtual ::sfx2::Metadatable* GetCoreObject() override;
    virtual css::uno::Reference< css::frame::XModel > GetModel() override;

    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const css::uno::Sequence< sal_Int8 >& rIdentifier)
        throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener(
            const css::uno::Reference< css::lang::XEventListener > & xListener)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener(
            const css::uno::Reference< css::lang::XEventListener > & xListener)
        throw (css::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const css::uno::Any& rValue)
        throw (css::beans::UnknownPropertyException,
                css::beans::PropertyVetoException,
                css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<
                css::beans::XVetoableChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    // XNamed
    virtual OUString SAL_CALL getName()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setName(const OUString& rName)
        throw (css::uno::RuntimeException, std::exception) override;

    // XTextContent
    virtual void SAL_CALL attach(
            const css::uno::Reference< css::text::XTextRange > & xTextRange)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getAnchor()
        throw (css::uno::RuntimeException, std::exception) override;

};

class SwXFieldmarkParameters
    : public ::cppu::WeakImplHelper< css::container::XNameContainer>
    , private SwClient
{
    public:
        SwXFieldmarkParameters(::sw::mark::IFieldmark* const pFieldmark)
        {
            pFieldmark->Add(this);
        }

        // XNameContainer
        virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) throw (css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeByName( const OUString& Name ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        // XNameReplace
        virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        // XNameAccess
        virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (css::uno::RuntimeException, std::exception) override;
        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException, std::exception) override;
    protected:
        //SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;
    private:
        ::sw::mark::IFieldmark::parameter_map_t* getCoreParameters() throw (css::uno::RuntimeException);
};

typedef cppu::ImplInheritanceHelper< SwXBookmark,
    css::text::XFormField > SwXFieldmark_Base;

class SwXFieldmark
    : public SwXFieldmark_Base
{

private:
    ::sw::mark::ICheckboxFieldmark* getCheckboxFieldmark();
    bool isReplacementObject;

protected:

    SwXFieldmark(bool isReplacementObject,
            ::sw::mark::IMark* pBkm = nullptr, SwDoc* pDoc = nullptr);

public:

    static css::uno::Reference<css::text::XTextContent>
        CreateXFieldmark(SwDoc & rDoc, ::sw::mark::IMark * pMark,
                bool isReplacementObject = false);

    virtual void attachToRange(
            const css::uno::Reference<css::text::XTextRange > & xTextRange)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException) override;
    virtual OUString SAL_CALL getFieldType()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setFieldType(const OUString& description )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XNameContainer > SAL_CALL getParameters(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const css::uno::Any& rValue)
        throw (css::beans::UnknownPropertyException,
                css::beans::PropertyVetoException,
                css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
};

class SwXODFCheckboxField : public SwXFieldmark
{
public:
    SwXODFCheckboxField( ::sw::mark::IMark* pBkm = nullptr, SwDoc* pDoc = nullptr) : SwXFieldmark(true,
            pBkm, pDoc) {}
};
#endif // INCLUDED_SW_SOURCE_CORE_INC_UNOBOOKMARK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
