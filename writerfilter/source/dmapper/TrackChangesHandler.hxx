/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "LoggedResources.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <dmapper/PropertyMap.hxx>

namespace writerfilter::dmapper
{
/** Handler for sprms that contain 'track changes' attributes
    - Author
    - Date
    - ID
    (This class is based on work done in 'MeasureHandler')
 */
class TrackChangesHandler : public LoggedProperties
{
    RedlineParamsPtr m_pRedlineParams;

    // Properties
    virtual void lcl_attribute(Id Name, Value& val) override;
    virtual void lcl_sprm(Sprm& sprm) override;

public:
    explicit TrackChangesHandler(sal_Int32 nToken);
    virtual ~TrackChangesHandler() override;

    /// Compute the UNO properties for the track changes object based on the received tokens.
    css::uno::Sequence<css::beans::PropertyValue> getRedlineProperties() const;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
