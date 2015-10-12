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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_GRAPHICPROPERTYITEMCONVERTER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_GRAPHICPROPERTYITEMCONVERTER_HXX

#include "ItemConverter.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

class SdrModel;

namespace chart
{
namespace wrapper
{

class GraphicPropertyItemConverter : public ItemConverter
{
public:
    enum eGraphicObjectType
    {
        FILLED_DATA_POINT,
        LINE_DATA_POINT,
        LINE_PROPERTIES,
        FILL_PROPERTIES,
        LINE_AND_FILL_PROPERTIES
    };

    GraphicPropertyItemConverter(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
        eGraphicObjectType eObjectType = FILLED_DATA_POINT );
    virtual ~GraphicPropertyItemConverter();

protected:
    virtual const sal_uInt16 * GetWhichPairs() const override;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const override;

    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
        throw (css::uno::Exception, std::exception) override;
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
        throw( ::com::sun::star::uno::Exception ) override;

private:
    eGraphicObjectType              m_eGraphicObjectType;
    SdrModel &                      m_rDrawModel;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory >  m_xNamedPropertyTableFactory;
};

} //  namespace wrapper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_INC_GRAPHICPROPERTYITEMCONVERTER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
