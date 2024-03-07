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

#pragma once

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

#include <cppuhelper/implbase.hxx>

#include <svl/itemprop.hxx>
#include <svl/listener.hxx>

#include <unocrsr.hxx>
#include "unorefmark.hxx"
#include "unoidx.hxx"
#include "unofootnote.hxx"
#include "unometa.hxx"
#include "unocontentcontrol.hxx"
#include "unolinebreak.hxx"
#include "unobookmark.hxx"

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::text { class XTextField; }
namespace com::sun::star::text { class XFootnote; }

class SwFrameFormat;
class SwRangeRedline;
class SwTextRuby;
class SwXText;
class SwXTextPortion;

typedef std::deque<
    rtl::Reference<SwXTextPortion> >
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
    PORTION_ANNOTATION_END,
    PORTION_LINEBREAK,
    PORTION_CONTENT_CONTROL,
    PORTION_LIST_AUTOFMT
};

class SwXTextPortion : public cppu::WeakImplHelper
<
    css::beans::XTolerantMultiPropertySet,
    css::beans::XMultiPropertySet,
    css::beans::XPropertySet,
    css::text::XTextRange,
    css::beans::XPropertyState,
    css::container::XContentEnumerationAccess,
    css::lang::XServiceInfo
>,
    public SvtListener
{
private:

    const SfxItemPropertySet *  m_pPropSet;
    const css::uno::Reference< SwXText > m_xParentText;
    rtl::Reference< SwXReferenceMark > m_xRefMark;
    rtl::Reference< SwXDocumentIndexMark > m_xTOXMark;
    rtl::Reference< SwXBookmark > m_xBookmark;
    rtl::Reference< SwXFootnote > m_xFootnote;
    css::uno::Reference< css::text::XTextField >
        m_xTextField;
    rtl::Reference< SwXMeta > m_xMeta;
    rtl::Reference<SwXLineBreak> m_xLineBreak;
    rtl::Reference<SwXContentControl> m_xContentControl;
    std::optional< css::uno::Any > m_oRubyText;
    std::optional< css::uno::Any > m_oRubyStyle;
    std::optional< css::uno::Any > m_oRubyAdjust;
    std::optional< css::uno::Any > m_oRubyIsAbove;
    std::optional< css::uno::Any > m_oRubyPosition;
    sw::UnoCursorPointer m_pUnoCursor;

    SwFrameFormat*                  m_pFrameFormat;
    const SwTextPortionType     m_ePortionType;

    bool                        m_bIsCollapsed;

    /// Expose the paragraph's RES_PARATR_LIST_AUTOFMT, not the char props of the underlying (empty)
    /// text.
    bool m_bIsListAutoFormat;

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
    SwXTextPortion(const SwUnoCursor* pPortionCursor, css::uno::Reference< SwXText > xParent, SwTextPortionType   eType   );
    SwXTextPortion(const SwUnoCursor* pPortionCursor, css::uno::Reference< SwXText > xParent, SwFrameFormat& rFormat );

    // for Ruby
    SwXTextPortion(const SwUnoCursor* pPortionCursor,
        SwTextRuby const& rAttr,
        css::uno::Reference< SwXText > xParent,
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

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    //XContentEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL createContentEnumeration(const OUString& aServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() override;

    void SetRefMark( rtl::Reference< SwXReferenceMark > const & xMark)
    { m_xRefMark = xMark; }

    void SetTOXMark( rtl::Reference< SwXDocumentIndexMark > const & xMark)
    { m_xTOXMark = xMark; }

    void SetBookmark( rtl::Reference< SwXBookmark > const & xMark)
    { m_xBookmark = xMark; }

    void SetFootnote( rtl::Reference< SwXFootnote > const & xNote)
    { m_xFootnote = xNote; }

    void SetTextField( css::uno::Reference< css::text::XTextField> const & xField)
    { m_xTextField = xField; }

    void SetMeta( rtl::Reference< SwXMeta > const & xMeta)
    { m_xMeta = xMeta; }

    void SetLineBreak(rtl::Reference<SwXLineBreak> const& xLineBreak)
    {
        m_xLineBreak = xLineBreak;
    }

    void SetContentControl(const rtl::Reference<SwXContentControl>& xContentControl)
    {
        m_xContentControl = xContentControl;
    }

    void SetCollapsed(bool bSet)        { m_bIsCollapsed = bSet;}

    SwTextPortionType GetTextPortionType() const { return m_ePortionType; }

    SwUnoCursor& GetCursor() const
        { return *m_pUnoCursor; }
};

class SwXTextPortionEnumeration final
    : public ::cppu::WeakImplHelper
        < css::container::XEnumeration
        , css::lang::XServiceInfo
        >
{
    TextRangeList_t m_Portions; // contains all portions, filled by ctor
    sw::UnoCursorPointer m_pUnoCursor;

    virtual ~SwXTextPortionEnumeration() override;

public:
    /// @param bOnlyTextFields tries to return less data, but may return more than just text fields
    SwXTextPortionEnumeration(SwPaM& rParaCursor,
            css::uno::Reference< SwXText > const & xParent,
            const sal_Int32 nStart, const sal_Int32 nEnd, bool bOnlyTextFields = false );

    SwXTextPortionEnumeration(SwPaM& rParaCursor,
        TextRangeList_t && rPortions );

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
        css::uno::Reference< SwXText > const& xParent,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
