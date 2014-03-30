/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_LATENTSTYLEHANDLER_HXX
#define INCLUDED_LATENTSTYLEHANDLER_HXX

#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <com/sun/star/beans/PropertyValue.hpp>

namespace writerfilter
{
namespace dmapper
{

/// Handler for a latent style (w:lsdException element)
class LatentStyleHandler
    : public LoggedProperties
{
    std::vector<beans::PropertyValue> m_aAttributes;

    // Properties
    virtual void lcl_attribute(Id Name, Value& val) SAL_OVERRIDE;
    virtual void lcl_sprm(Sprm& sprm) SAL_OVERRIDE;

public:
    LatentStyleHandler();
    virtual ~LatentStyleHandler();

    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> getAttributes() const;
};

typedef boost::shared_ptr<LatentStyleHandler> LatentStyleHandlerPtr;
} // namespace dmapper
} // namespace writerfilter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
