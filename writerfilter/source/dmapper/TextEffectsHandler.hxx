/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_TEXTEFFECTSHANDLER_HXX
#define INCLUDED_TEXTEFFECTSHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <boost/scoped_ptr.hpp>

namespace writerfilter {
namespace dmapper
{

class GrabBagStack;

class TextEffectsHandler : public LoggedProperties
{
private:
    boost::scoped_ptr<GrabBagStack> mpGrabBagStack;

    // LoggedProperties
    virtual void lcl_attribute(Id aName, Value& aValue);
    virtual void lcl_sprm(Sprm& sprm);

public:
    TextEffectsHandler();
    virtual ~TextEffectsHandler();

    css::beans::PropertyValue getInteropGrabBag();
    void enableInteropGrabBag(OUString aName);
    void disableInteropGrabBag();
    bool isInteropGrabBagEnabled();
};

typedef boost::shared_ptr<TextEffectsHandler> TextEffectsHandlerPtr;

}}

#endif // INCLUDED_TEXTEFFECTSHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
