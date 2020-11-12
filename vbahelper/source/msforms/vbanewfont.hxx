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

#ifndef INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBANEWFONT_HXX
#define INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBANEWFONT_HXX

#include <ooo/vba/msforms/XNewFont.hpp>
#include <vbahelper/vbahelper.hxx>
#include <cppuhelper/implbase.hxx>

class VbaNewFont : public cppu::WeakImplHelper<ov::msforms::XNewFont>
{
public:
    /// @throws css::uno::RuntimeException
    explicit VbaNewFont(const css::uno::Reference<css::beans::XPropertySet>& rxModelProps);

    // XNewFont attributes
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName(const OUString& rName) override;
    virtual double SAL_CALL getSize() override;
    virtual void SAL_CALL setSize(double fSize) override;
    virtual sal_Int16 SAL_CALL getCharset() override;
    virtual void SAL_CALL setCharset(sal_Int16 nCharset) override;
    virtual sal_Int16 SAL_CALL getWeight() override;
    virtual void SAL_CALL setWeight(sal_Int16 nWeight) override;
    virtual sal_Bool SAL_CALL getBold() override;
    virtual void SAL_CALL setBold(sal_Bool bBold) override;
    virtual sal_Bool SAL_CALL getItalic() override;
    virtual void SAL_CALL setItalic(sal_Bool bItalic) override;
    virtual sal_Bool SAL_CALL getUnderline() override;
    virtual void SAL_CALL setUnderline(sal_Bool bUnderline) override;
    virtual sal_Bool SAL_CALL getStrikethrough() override;
    virtual void SAL_CALL setStrikethrough(sal_Bool bStrikethrough) override;

private:
    css::uno::Reference<css::beans::XPropertySet> mxProps;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
