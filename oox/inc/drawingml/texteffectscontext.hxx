/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_OOX_DRAWINGML_TEXTEFFECTSCONTEXT_HXX
#define INCLUDED_OOX_DRAWINGML_TEXTEFFECTSCONTEXT_HXX

#include <com/sun/star/beans/PropertyValue.hpp>

#include <oox/helper/grabbagstack.hxx>
#include <oox/core/contexthandler2.hxx>
#include <memory>
#include <vector>

namespace oox { namespace drawingml {

class TextEffectsContext : public oox::core::ContextHandler2
{
public:
    TextEffectsContext(oox::core::ContextHandler2Helper& rParent,
                       sal_Int32 aElementToken,
                       std::vector<css::beans::PropertyValue>& rTextEffectsProperties);
    virtual ~TextEffectsContext();

    virtual void onStartElement(const oox::AttributeList& rAttribs) override;
    virtual void onEndElement() override;

    virtual oox::core::ContextHandlerRef onCreateContext(sal_Int32 Element, const oox::AttributeList& rAttribs) override;

protected:
    std::vector<css::beans::PropertyValue>& mrTextEffectsProperties;
    std::unique_ptr<oox::GrabBagStack> mpGrabBagStack;
    sal_Int32 mnCurrentElement;

private:
    void processAttributes(const AttributeList& rAttribs);
    void pushAttributeToGrabBag (const sal_Int32& aAttributeId, const OUString& rElementName, const AttributeList& rAttribs);
};

} }

#endif // INCLUDED_OOX_DRAWINGML_TEXTEFFECTSCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
