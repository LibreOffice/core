/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/awt/grid/XGridColumnListener.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/style/HorizontalAlignment.hpp>
#include <com/sun/star/util/XCloneable.hpp>

using namespace cpo::uno;
using namespace css::awt::grid;
using namespace css::lang;
using namespace css::style;

/**
 * a dummy implementation of css.awt.grid.XGridColumn
 */
class DummyColumn : public cppu::WeakImplHelper<XGridColumn>
{
public:
    virtual Any getIdentifier() override { throw RuntimeException(u"Not supported yet."_ustr); }

    virtual void setIdentifier(const Any&) override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }

    virtual sal_Int32 getColumnWidth() override { throw RuntimeException(u"Not supported yet."_ustr); }

    virtual void setColumnWidth(sal_Int32) override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }

    virtual sal_Int32 getMinWidth() override { throw RuntimeException(u"Not supported yet."_ustr); }

    virtual void setMinWidth(sal_Int32) override { throw RuntimeException(u"Not supported yet."_ustr); }

    virtual sal_Int32 getMaxWidth() override { throw RuntimeException(u"Not supported yet."_ustr); }

    virtual void setMaxWidth(sal_Int32) override { throw RuntimeException(u"Not supported yet."_ustr); }

    virtual bool getResizeable() override { throw RuntimeException(u"Not supported yet."_ustr); }

    virtual void setResizeable(bool) override { throw RuntimeException(u"Not supported yet."_ustr); }

    virtual sal_Int32 getFlexibility() override { throw RuntimeException(u"Not supported yet."_ustr); }

    virtual void setFlexibility(sal_Int32) override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }

    virtual HorizontalAlignment getHorizontalAlign() override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }

    virtual void setHorizontalAlign(HorizontalAlignment) override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }

    virtual OUString getTitle() override { throw RuntimeException(u"Not supported yet."_ustr); }

    virtual void setTitle(const OUString&) override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }

    virtual OUString getHelpText() override { throw RuntimeException(u"Not supported yet."_ustr); }

    virtual void setHelpText(const OUString&) override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }

    virtual sal_Int32 getIndex() override { throw RuntimeException(u"Not supported yet."_ustr); }

    virtual sal_Int32 getDataColumnIndex() override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }

    virtual void setDataColumnIndex(sal_Int32) override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }

    virtual void addGridColumnListener(const Reference<XGridColumnListener>&) override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }

    virtual void removeGridColumnListener(const Reference<XGridColumnListener>&) override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }

    virtual void dispose() override { throw RuntimeException(u"Not supported yet."_ustr); }

    virtual void addEventListener(const Reference<XEventListener>&) override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }

    virtual void removeEventListener(const Reference<XEventListener>&) override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }

    virtual Reference<XCloneable> createClone() override
    {
        throw RuntimeException(u"Not supported yet."_ustr);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
