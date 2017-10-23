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

#include "swtypes.hxx"
#include "calbck.hxx"
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase5.hxx>
#include <svl/itemprop.hxx>
#include "unobaseclass.hxx"

class SwDoc;
class SwFormatCol;
class SwDocShell;
class SwNumRule;
class SwNumFormat;

class SwXFootnoteProperties : public cppu::WeakAggImplHelper2
<
    css::beans::XPropertySet,
    css::lang::XServiceInfo
>
{
    SwDoc*                      pDoc;
    const SfxItemPropertySet*   m_pPropertySet;
protected:
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

    void            Invalidate() {pDoc = nullptr;}
};

class SwXEndnoteProperties : public cppu::WeakAggImplHelper2
<
    css::beans::XPropertySet,
    css::lang::XServiceInfo
>
{
    SwDoc*                      pDoc;
    const SfxItemPropertySet*   m_pPropertySet;
protected:
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

    void            Invalidate() {pDoc = nullptr;}
};

class SwXLineNumberingProperties : public cppu::WeakAggImplHelper2
<
    css::beans::XPropertySet,
    css::lang::XServiceInfo
>
{
    SwDoc*                      pDoc;
    const SfxItemPropertySet*   m_pPropertySet;
protected:
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

    void            Invalidate() {pDoc = nullptr;}
};

class SwXNumberingRules : public cppu::WeakAggImplHelper5
<
    css::container::XIndexReplace,
    css::lang::XUnoTunnel,
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
    SwDoc*                      pDoc;
    SwDocShell*                 pDocShell; // Only if used as chapter numbering.
    SwNumRule*                  pNumRule;
    const SfxItemPropertySet*   m_pPropertySet;
    bool                        bOwnNumRuleCreated;
protected:
    virtual ~SwXNumberingRules() override;

public:
    SwXNumberingRules(SwDocShell& rDocSh);  // chapter numbering
    SwXNumberingRules(const SwNumRule& rRule, SwDoc* doc = nullptr); // NumRule for paragraphs, numbering styles
    SwXNumberingRules(SwDoc& rDoc); //create a new instance

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

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
    const SwNumRule*        GetNumRule() {return pNumRule;}

    static bool             isInvalidStyle(const OUString &rName);
    void    Invalidate()    {pDocShell = nullptr;}
    const OUString&   GetCreatedNumRuleName() const {return m_sCreatedNumRuleName;}

    static css::uno::Sequence<css::beans::PropertyValue> GetPropertiesForNumFormat(
            const SwNumFormat& rFormat, OUString const& rCharFormatName,
            OUString const* pHeadingStyleName);
    static void SetPropertiesToNumFormat(
            SwNumFormat & aFormat,
            OUString & rCharStyleName,
            OUString *const pBulletFontName,
            OUString *const pHeadingStyleName,
            OUString *const pParagraphStyleName,
            SwDoc *const pDoc, SwDocShell *const pDocShell,
            css::uno::Sequence<css::beans::PropertyValue> const& rProperties);

};

class SwXChapterNumbering : public SwXNumberingRules
{
protected:
    virtual ~SwXChapterNumbering() override;
public:
    SwXChapterNumbering(SwDocShell& rDocSh);

    using SwXNumberingRules::Invalidate;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};

class SwXTextColumns : public cppu::WeakAggImplHelper4
<

    css::lang::XUnoTunnel,
    css::beans::XPropertySet,
    css::text::XTextColumns,
    css::lang::XServiceInfo
>
{
    sal_Int32                   nReference;
    css::uno::Sequence< css::text::TextColumn>    aTextColumns;
    bool                        bIsAutomaticWidth;
    sal_Int32                   nAutoDistance;

    const SfxItemPropertySet*   m_pPropSet;

    //separator line
    sal_Int32                   nSepLineWidth;
    sal_Int32                   nSepLineColor;
    sal_Int8                    nSepLineHeightRelative;
    css::style::VerticalAlignment nSepLineVertAlign;
    bool                        bSepLineIsOn;
    sal_Int8                    nSepLineStyle;

protected:
    virtual ~SwXTextColumns() override;
public:
    SwXTextColumns();
    SwXTextColumns(const SwFormatCol& rFormatCol);

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    //XTextColumns
    virtual sal_Int32 SAL_CALL getReferenceValue(  ) override;
    virtual sal_Int16 SAL_CALL getColumnCount(  ) override;
    virtual void SAL_CALL setColumnCount( sal_Int16 nColumns ) override;
    virtual css::uno::Sequence< css::text::TextColumn > SAL_CALL getColumns(  ) override;
    virtual void SAL_CALL setColumns( const css::uno::Sequence< css::text::TextColumn >& Columns ) override;

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

    sal_Int32   GetSepLineWidth() const {return nSepLineWidth;}
    sal_Int32   GetSepLineColor() const {return     nSepLineColor;}
    sal_Int8    GetSepLineHeightRelative() const {return    nSepLineHeightRelative;}
    css::style::VerticalAlignment GetSepLineVertAlign() const {return nSepLineVertAlign;}
    bool        GetSepLineIsOn() const {return  bSepLineIsOn;}
    sal_Int8    GetSepLineStyle() const {return nSepLineStyle;}

    bool        IsAutomaticWidth() const {return bIsAutomaticWidth;}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
