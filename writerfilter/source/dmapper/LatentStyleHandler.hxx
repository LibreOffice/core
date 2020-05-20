/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_LATENTSTYLEHANDLER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_LATENTSTYLEHANDLER_HXX

#include "LoggedResources.hxx"
#include <vector>
#include <com/sun/star/beans/PropertyValue.hpp>

namespace writerfilter
{
namespace dmapper
{
/// Handler for a latent style (w:lsdException element)
class LatentStyleHandler : public LoggedProperties
{
    std::vector<css::beans::PropertyValue> m_aAttributes;

    // Properties
    void lcl_attribute(Id nId, Value& rVal) override;
    void lcl_sprm(Sprm& sprm) override;

public:
    LatentStyleHandler();
    ~LatentStyleHandler() override;

    const std::vector<css::beans::PropertyValue>& getAttributes() const;
};

} // namespace dmapper
} // namespace writerfilter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
