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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARASPACINGWINDOW_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARASPACINGWINDOW_HXX

#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <vcl/builder.hxx>
#include <vcl/layout.hxx>
#include <svx/relfld.hxx>
#include <svtools/unitconv.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <com/sun/star/ui/XContextChangeEventListener.hpp>
#include <sfx2/sidebar/EnumContext.hxx>

using namespace com::sun::star;

namespace svx {

class MetricListener : public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    MetricListener();
    ~MetricListener();

protected:
    void CreateControllerItem();
    SfxMapUnit GetCoreMetric(sal_uInt16 nSId) const;

    virtual void SetUnit(FieldUnit eUnit) = 0;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) override;
private:
    ::sfx2::sidebar::ControllerItem* m_pMetricControl;
};

class ParaULSpacingWindow : public VclVBox,
                            public VclBuilderContainer,
                            public MetricListener

{
public:
    explicit ParaULSpacingWindow(vcl::Window* pParent, css::uno::Reference<css::frame::XFrame>& xFrame);
    virtual ~ParaULSpacingWindow();
    virtual void dispose() override;

    void SetValue(const SvxULSpaceItem* pItem);

private:
    VclPtr<SvxRelativeField> m_pAboveSpacing;
    VclPtr<SvxRelativeField> m_pBelowSpacing;

    SfxMapUnit m_eUnit;

    void SetUnit(FieldUnit eUnit) override;

    DECL_LINK_TYPED(ModifySpacingHdl, Edit&, void);
};

class ParaLRSpacingWindow : public VclVBox,
                            public VclBuilderContainer,
                            public MetricListener,
                            public css::ui::XContextChangeEventListener
{
public:
    explicit ParaLRSpacingWindow(vcl::Window* pParent, css::uno::Reference<css::frame::XFrame>& xFrame);
    virtual ~ParaLRSpacingWindow();
    virtual void dispose() override;

    // XContextChangeEventListener
    virtual void SAL_CALL notifyContextChangeEvent(const css::ui::ContextChangeEventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual ::css::uno::Any SAL_CALL queryInterface(const ::css::uno::Type&)
        throw (::css::uno::RuntimeException, ::std::exception) override
    {
        return ::css::uno::Any();
    }

    virtual void SAL_CALL acquire() throw () override {}

    virtual void SAL_CALL disposing(const ::css::lang::EventObject&)
        throw (::css::uno::RuntimeException, ::std::exception) override {}

    virtual void SAL_CALL release() throw () override {}

    void Initialize();

protected:
    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) override;

    virtual void SetUnit(FieldUnit eUnit) override;

private:
    VclPtr<SvxRelativeField> m_pBeforeSpacing;
    VclPtr<SvxRelativeField> m_pAfterSpacing;
    VclPtr<SvxRelativeField> m_pFLSpacing;

    SfxMapUnit m_eUnit;

    ::sfx2::sidebar::EnumContext m_aContext;
    ::sfx2::sidebar::ControllerItem* m_pLRSpaceControl;

    void SetValue(SfxItemState eState, const SfxPoolItem* pState);

    DECL_LINK_TYPED(ModifySpacingHdl, Edit&, void);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
