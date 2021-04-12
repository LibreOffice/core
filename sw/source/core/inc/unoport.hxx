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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNOPORT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNOPORT_HXX

#include <memory>
#include <deque>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XTolerantMultiPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <cppuhelper/implbase.hxx>

#include <svl/itemprop.hxx>
#include <svl/listener.hxx>

#include <unocrsr.hxx>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::text { class XTextField; }
namespace com::sun::star::text { class XFootnote; }

class SwFrameFormat;
class SwRangeRedline;
class SwTextRuby;

typedef std::deque<
    css::uno::Reference< css::text::XTextRange > >
    TextRangeList_t;

enum SwTextPortionType
{
    PORTION_TEXT,
    PORTION_FIELD,
    PORTION_FRAME,
    PORTION_FOOTNOTE,
    PORTION_REFMARK_START,
    PORTION_REFMARK_END,
    PORTION_TOXMARK_START,
    PORTION_TOXMARK_END,
    PORTION_BOOKMARK_START,
    PORTION_BOOKMARK_END,
    PORTION_REDLINE_START,
    PORTION_REDLINE_END,
    PORTION_RUBY_START,
    PORTION_RUBY_END,
    PORTION_SOFT_PAGEBREAK,
    PORTION_META,
    PORTION_FIELD_START,
    PORTION_FIELD_SEP,
    PORTION_FIELD_END,
    PORTION_FIELD_START_END,
    PORTION_ANNOTATION,
    PORTION_ANNOTATION_END
};

class SwXTextPortion : public cppu::WeakImplHelper
<
    css::beans::XTolerantMultiPropertySet,
    css::beans::XMultiPropertySet,
    css::beans::XPropertySet,
    css::text::XTextRange,
    css::beans::XPropertyState,
    css::container::XContentEnumerationAccess,
    css::lang::XUnoTunnel,
    css::lang::XServiceInfo
>,
    public SvtListener
{
private:

    const SfxItemPropertySet *  m_pPropSet;
    const css::uno::Reference< css::text::XText >
        m_xParentText;
    css::uno::Reference< css::text::XTextContent >
        m_xRefMark;
    css::uno::Reference< css::text::XTextContent >
        m_xTOXMark;
    css::uno::Reference< css::text::XTextContent >
        m_xBookmark;
    css::uno::Reference< css::text::XFootnote >
        m_xFootnote;
    css::uno::Reference< css::text::XTextField >
        m_xTextField;
    css::uno::Reference< css::text::XTextContent >
        m_xMeta;
    std::unique_ptr< css::uno::Any > m_pRubyText;
    std::unique_ptr< css::uno::Any > m_pRubyStyle;
    std::unique_ptr< css::uno::Any > m_pRubyAdjust;
    std::unique_ptr< css::uno::Any > m_pRubyIsAbove;
    std::unique_ptr< css::uno::Any > m_pRubyPosition;
    sw::UnoCursorPointer m_pUnoCursor;

    SwFrameFormat*                  m_pFrameFormat;
    const SwTextPortionType     m_ePortionType;

    bool                        m_bIsCollapsed;

    void init(const SwUnoCursor* pPortionCursor);

protected:
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::beans::PropertyVetoException
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    void SetPropertyValues_Impl(
        const css::uno::Sequence< OUString >& aPropertyNames,
        const css::uno::Sequence< css::uno::Any >& aValues );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::uno::Any > GetPropertyValues_Impl(
        const css::uno::Sequence< OUString >& aPropertyNames );

    void GetPropertyValue( css::uno::Any &rVal,
                const SfxItemPropertyMapEntry& rEntry, SwUnoCursor *pUnoCursor, std::unique_ptr<SfxItemSet> &pSet );

    /// @throws css::uno::RuntimeException
    css::uno::Sequence<css::beans::GetDirectPropertyTolerantResult> GetPropertyValuesTolerant_Impl(
        const css::uno::Sequence< OUString >& rPropertyNames, bool bDirectValuesOnly );

    virtual ~SwXTextPortion() override;

    virtual void Notify(const SfxHint& rHint) override;

public:
    SwXTextPortion(const SwUnoCursor* pPortionCursor, css::uno::Reference< css::text::XText > const& rParent, SwTextPortionType   eType   );
    SwXTextPortion(const SwUnoCursor* pPortionCursor, css::uno::Reference< css::text::XText > const& rParent, SwFrameFormat& rFormat );

    // for Ruby
    SwXTextPortion(const SwUnoCursor* pPortionCursor,
        SwTextRuby const& rAttr,
        css::uno::Reference< css::text::XText > const& xParent,
        bool bIsEnd );

    //XTextRange
    virtual css::uno::Reference< css::text::XText >  SAL_CALL getText() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL  getStart() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL   getEnd() override;
    virtual OUString SAL_CALL  getString() override;
    virtual void SAL_CALL  setString(const OUString& aString) override;

    //XTolerantMultiPropertySet
    virtual css::uno::Sequence< css::beans::SetPropertyTolerantFailed > SAL_CALL setPropertyValuesTolerant( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::beans::GetPropertyTolerantResult > SAL_CALL getPropertyValuesTolerant( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual css::uno::Sequence< css::beans::GetDirectPropertyTolerantResult > SAL_CALL getDirectPropertyValuesTolerant( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    //XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    //XUnoTunnel
    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    //XContentEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL createContentEnumeration(const OUString& aServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() override;

    void SetRefMark( css::uno::Reference< css::text::XTextContent > const & xMark)
    { m_xRefMark = xMark; }

    void SetTOXMark( css::uno::Reference< css::text::XTextContent > const & xMark)
    { m_xTOXMark = xMark; }

    void SetBookmark( css::uno::Reference< css::text::XTextContent > const & xMark)
    { m_xBookmark = xMark; }

    void SetFootnote( css::uno::Reference< css::text::XFootnote > const & xNote)
    { m_xFootnote = xNote; }

    void SetTextField( css::uno::Reference< css::text::XTextField> const & xField)
    { m_xTextField = xField; }

    void SetMeta( css::uno::Reference< css::text::XTextContent > const & xMeta)
    { m_xMeta = xMeta; }

    void SetCollapsed(bool bSet)        { m_bIsCollapsed = bSet;}

    SwTextPortionType GetTextPortionType() const { return m_ePortionType; }

    SwUnoCursor& GetCursor() const
        { return *m_pUnoCursor; }
};

class SwXTextPortionEnumeration final
    : public ::cppu::WeakImplHelper
        < css::container::XEnumeration
        , css::lang::XServiceInfo
        , css::lang::XUnoTunnel
        >
{
    TextRangeList_t m_Portions; // contains all portions, filled by ctor
    sw::UnoCursorPointer m_pUnoCursor;

    virtual ~SwXTextPortionEnumeration() override;

public:
    SwXTextPortionEnumeration(SwPaM& rParaCursor,
            css::uno::Reference< css::text::XText > const & xParent,
            const sal_Int32 nStart, const sal_Int32 nEnd );

    SwXTextPortionEnumeration(SwPaM& rParaCursor,
        TextRangeList_t const & rPortions );

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    //XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual css::uno::Any SAL_CALL nextElement() override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;
};

class SwXRedlinePortion final : public SwXTextPortion
{
private:
    SwRangeRedline const& m_rRedline;

    bool Validate();

    using SwXTextPortion::GetPropertyValue;

    virtual ~SwXRedlinePortion() override;

public:
    SwXRedlinePortion(
        SwRangeRedline const& rRedline,
        SwUnoCursor const* pPortionCursor,
        css::uno::Reference< css::text::XText > const& xParent,
        bool const bIsStart);

    /// @throws std::exception
    static css::uno::Any  GetPropertyValue(
            std::u16string_view PropertyName, SwRangeRedline const& rRedline);
    /// @throws std::exception
    static css::uno::Sequence< css::beans::PropertyValue > CreateRedlineProperties(
                SwRangeRedline const& rRedline, bool const bIsStart);

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId() override;

    // XPropertySet
    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
