/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_SMARTTAGHANDLER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_SMARTTAGHANDLER_HXX

#include "LoggedResources.hxx"

namespace writerfilter
{
namespace dmapper
{

/// Handler for smart tags, i.e. <w:smartTag> and below.
class SmartTagHandler
    : public LoggedProperties
{
    OUString m_aURI;
    OUString m_aElement;

public:
    SmartTagHandler();
    virtual ~SmartTagHandler();

    virtual void lcl_attribute(Id Name, Value& val) override;
    virtual void lcl_sprm(Sprm& sprm) override;

    void setURI(const OUString& rURI);
    void setElement(const OUString& rElement);
};

} // namespace dmapper
} // namespace writerfilter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
