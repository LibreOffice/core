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

#include "ItemConverter.hxx"

namespace com::sun::star::lang { class XMultiServiceFactory; }

class SdrModel;

namespace chart::wrapper
{

enum class GraphicObjectType
{
    FilledDataPoint,
    LineDataPoint,
    LineProperties,
    LineAndFillProperties
};

class GraphicPropertyItemConverter : public ItemConverter
{
public:
    GraphicPropertyItemConverter(
        const css::uno::Reference< css::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const css::uno::Reference< css::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
        GraphicObjectType eObjectType );
    virtual ~GraphicPropertyItemConverter() override;

protected:
    virtual const sal_uInt16 * GetWhichPairs() const override;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const override;

    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const override;
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet ) override;

private:
    GraphicObjectType              m_GraphicObjectType;
    SdrModel &                      m_rDrawModel;
    css::uno::Reference< css::lang::XMultiServiceFactory >  m_xNamedPropertyTableFactory;
};

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
