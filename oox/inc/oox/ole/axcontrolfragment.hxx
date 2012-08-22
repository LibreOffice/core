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

#ifndef OOX_OLE_AXCONTROLFRAGMENT_HXX
#define OOX_OLE_AXCONTROLFRAGMENT_HXX

#include "oox/core/fragmenthandler2.hxx"

namespace oox {
namespace ole {

class ControlModelBase;
class EmbeddedControl;

// ============================================================================

/** Context handler for ActiveX form control model properties. */
class AxControlPropertyContext : public ::oox::core::ContextHandler2
{
public:
    explicit            AxControlPropertyContext(
                            ::oox::core::FragmentHandler2& rFragment,
                            ControlModelBase& rModel );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    ControlModelBase&   mrModel;
    sal_Int32           mnPropId;           ///< Identifier of currently processed property.
};

// ============================================================================

/** Fragment handler for an embedded ActiveX form control fragment. */
class AxControlFragment : public ::oox::core::FragmentHandler2
{
public:
    explicit            AxControlFragment(
                            ::oox::core::XmlFilterBase& rFilter,
                            const ::rtl::OUString& rFragmentPath,
                            EmbeddedControl& rControl );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    EmbeddedControl&    mrControl;
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
