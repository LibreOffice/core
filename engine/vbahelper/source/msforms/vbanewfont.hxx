/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    /// @throws cpo::uno::RuntimeException
    explicit VbaNewFont(const css::uno::Reference<css::beans::XPropertySet>& rxModelProps);

    // XNewFont attributes
    virtual OUString getName() override;
    virtual void setName(const OUString& rName) override;
    virtual double getSize() override;
    virtual void setSize(double fSize) override;
    virtual sal_Int16 getCharset() override;
    virtual void setCharset(sal_Int16 nCharset) override;
    virtual sal_Int16 getWeight() override;
    virtual void setWeight(sal_Int16 nWeight) override;
    virtual bool getBold() override;
    virtual void setBold(bool bBold) override;
    virtual bool getItalic() override;
    virtual void setItalic(bool bItalic) override;
    virtual bool getUnderline() override;
    virtual void setUnderline(bool bUnderline) override;
    virtual bool getStrikethrough() override;
    virtual void setStrikethrough(bool bStrikethrough) override;

private:
    css::uno::Reference<css::beans::XPropertySet> mxProps;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
