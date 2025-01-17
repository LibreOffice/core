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
#ifndef INCLUDED_SW_INC_UNOSETT_HXX
#define INCLUDED_SW_INC_UNOSETT_HXX

#include <sal/config.h>

#include <string_view>

#include "swtypes.hxx"
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <cppuhelper/implbase.hxx>
#include "unobaseclass.hxx"

class SwDoc;
class SwFormatCol;
class SwDocShell;
class SwNumRule;
class SwNumFormat;
class SfxItemPropertySet;
class ProgName;
namespace com::sun::star::beans { struct PropertyValue; }

class SwXFootnoteProperties final : public cppu::WeakImplHelper
<
    css::beans::XPropertySet,
    css::lang::XServiceInfo
>
{
    SwDoc*                      m_pDoc;
    const SfxItemPropertySet*   m_pPropertySet;

    virtual ~SwXFootnoteProperties() override;
public:
    SwXFootnoteProperties(SwDoc* pDoc);

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    void            Invalidate() {m_pDoc = nullptr;}
};

class SwXEndnoteProperties final : public cppu::WeakImplHelper
<
    css::beans::XPropertySet,
    css::lang::XServiceInfo
>
{
    SwDoc*                      m_pDoc;
    const SfxItemPropertySet*   m_pPropertySet;

    virtual ~SwXEndnoteProperties() override;
public:
    SwXEndnoteProperties(SwDoc* pDoc);

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    void            Invalidate() {m_pDoc = nullptr;}
};

class SwXLineNumberingProperties final : public cppu::WeakImplHelper
<
    css::beans::XPropertySet,
    css::lang::XServiceInfo
>
{
    SwDoc*                      m_pDoc;
    const SfxItemPropertySet*   m_pPropertySet;

    virtual ~SwXLineNumberingProperties() override;
public:
    SwXLineNumberingProperties(SwDoc* pDoc);

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    void            Invalidate() {m_pDoc = nullptr;}
};

class SAL_DLLPUBLIC_RTTI SwXNumberingRules : public cppu::WeakImplHelper
<
    css::container::XIndexReplace,
    css::beans::XPropertySet,
    css::container::XNamed,
    css::lang::XServiceInfo
>
{
private:
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    OUString                    m_sNewCharStyleNames[MAXLEVEL];
    OUString                    m_sNewBulletFontNames[MAXLEVEL];
    OUString                    m_sCreatedNumRuleName; //connects to a numbering in SwDoc
    SwDoc*                      m_pDoc; // Only if *not* used as chapter numbering.
    SwDocShell*                 m_pDocShell; // Only if used as chapter numbering.
    SwNumRule*                  m_pNumRule;
    const SfxItemPropertySet*   m_pPropertySet;
    bool                        m_bOwnNumRuleCreated;
protected:
    SwXNumberingRules(SwDocShell& rDocSh);  // chapter numbering
    virtual ~SwXNumberingRules() override;

public:
    SwXNumberingRules(const SwNumRule& rRule, SwDoc* doc = nullptr); // NumRule for paragraphs, numbering styles
    SwXNumberingRules(SwDoc& rDoc); //create a new instance

    //XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const css::uno::Any& Element ) override;

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName(const OUString& Name_) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    css::uno::Sequence< css::beans::PropertyValue>
            GetNumberingRuleByIndex(const SwNumRule& rNumRule, sal_Int32 nIndex)const;
    /// @throws css::uno::RuntimeException
    /// @throws css::lang::IllegalArgumentException
    void    SetNumberingRuleByIndex(SwNumRule& rNumRule,
                const css::uno::Sequence< css::beans::PropertyValue>& rProperties, sal_Int32 nIndex);

    const OUString*         GetNewCharStyleNames() const {return m_sNewCharStyleNames;}
    const OUString*         GetBulletFontNames() const {return m_sNewBulletFontNames;}
    const SwNumRule*        GetNumRule() const {return m_pNumRule;}

    static bool             isInvalidStyle(std::u16string_view rName);
    void    Invalidate()    {m_pDocShell = nullptr;}
    const OUString&   GetCreatedNumRuleName() const {return m_sCreatedNumRuleName;}

    SW_DLLPUBLIC css::uno::Any getPropertyByIndex(sal_Int32 nIndex, const OUString& rPropName);

    static css::uno::Sequence<css::beans::PropertyValue> GetPropertiesForNumFormat(
            const SwNumFormat& rFormat, OUString const& rCharFormatName,
            ProgName const* pHeadingStyleName, OUString const & referer);
    static void SetPropertiesToNumFormat(
            SwNumFormat & aFormat,
            OUString & rCharStyleName,
            OUString *const pBulletFontName,
            OUString *const pHeadingStyleName,
            OUString *const pParagraphStyleName,
            SwDoc *const pDoc,
            SwDocShell *const pDocShell,
            css::uno::Sequence<css::beans::PropertyValue> const& rProperties);

private:
    css::uno::Any GetNumberingRuleByIndex(const SwNumRule& rNumRule, sal_Int32 nIndex, const OUString& rPropName) const;
    static css::uno::Any GetPropertyForNumFormat(
            const SwNumFormat& rFormat, OUString const& rCharFormatName,
            ProgName const* pHeadingStyleName, OUString const & referer, OUString const & rPropName);
};

class SwXChapterNumbering final : public SwXNumberingRules
{

    virtual ~SwXChapterNumbering() override;
public:
    SwXChapterNumbering(SwDocShell& rDocSh);

    using SwXNumberingRules::Invalidate;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
