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

#ifndef INCLUDED_UCBHELPER_SIMPLENAMECLASHRESOLVEREQUEST_HXX
#define INCLUDED_UCBHELPER_SIMPLENAMECLASHRESOLVEREQUEST_HXX

#include <config_options.h>
#include <rtl/ref.hxx>
#include <ucbhelper/interactionrequest.hxx>
#include <ucbhelper/ucbhelperdllapi.h>

namespace ucbhelper
{
class InteractionSupplyName;

/**
  * This class implements a simple name clash resolve interaction request.
  * Instances can be passed directly to XInteractionHandler::handle(...). Each
  * instance contains a NameClashResolveRequest and two interaction
  * continuations: "Abort" and "SupplyName". Another continuation
  * ("ReplaceExistingData") may be supplied optionally.
  *
  * @see css::ucb::NameClashResolveRequest
  * @see InteractionAbort
  * @see InteractioneplaceExistingData
  */
class UNLESS_MERGELIBS(UCBHELPER_DLLPUBLIC) SimpleNameClashResolveRequest final
    : public ucbhelper::InteractionRequest
{
    rtl::Reference<InteractionSupplyName> m_xNameSupplier;

    virtual ~SimpleNameClashResolveRequest() override;

public:
    /**
      * Constructor.
      *
      * @param rTargetFolderURL contains the URL of the folder that contains
      *        the clashing resource.
      * @param rClashingName contains the clashing name.
      */
    SimpleNameClashResolveRequest(const OUString& rTargetFolderURL, const OUString& rClashingName);
    /**
      * This method returns the new name that was supplied by the interaction
      * handler.
      *
      * @return the new name, if supplied.
      */
    OUString const& getNewName() const;
};

} // namespace ucbhelper

#endif /* ! INCLUDED_UCBHELPER_SIMPLENAMECLASHRESOLVEREQUEST_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
