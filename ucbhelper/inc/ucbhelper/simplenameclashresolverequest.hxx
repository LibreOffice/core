/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _UCBHELPER_SIMPLENAMECLASHRESOLVEREQUEST_HXX
#define _UCBHELPER_SIMPLENAMECLASHRESOLVEREQUEST_HXX

#include <rtl/ref.hxx>
#include <ucbhelper/interactionrequest.hxx>
#include "ucbhelper/ucbhelperdllapi.h"

namespace ucbhelper {

/**
  * This class implements a simple name clash resolve interaction request.
  * Instances can be passed directly to XInteractionHandler::handle(...). Each
  * instance contains a NameClashResolveRequest and two interaction
  * continuations: "Abort" and "SupplyName". Another continuation
  * ("ReplaceExistingData") may be supplied optionally.
  *
  * @see com::sun::star::ucb::NameClashResolveRequest
  * @see InteractionAbort
  * @see InteractioneplaceExistingData
  * @see InteractionSupplyName
  */
class UCBHELPER_DLLPUBLIC SimpleNameClashResolveRequest : public ucbhelper::InteractionRequest
{
    rtl::Reference< InteractionSupplyName > m_xNameSupplier;

public:
    /**
      * Constructor.
      *
      * @param rTargetFolderURL contains the URL of the folder that contains
      *        the clashing resource.
      * @param rClashingName contains the clashing name.
      * @param rProposedNewName contains a proposal for the new name or is
      *        empty.
      * @param bSupportsOverwriteData indicates whether an
      *        InteractionReplaceExistingData continuation shall be supplied
      *        with the interaction request.
      */
    SimpleNameClashResolveRequest( const rtl::OUString & rTargetFolderURL,
                                   const rtl::OUString & rClashingName,
                                   const rtl::OUString & rProposedNewName,
                                   sal_Bool bSupportsOverwriteData = sal_True );
    /**
      * This method returns the new name that was supplied by the interaction
      * handler.
      *
      * @return the new name, if supplied.
      */
    const rtl::OUString getNewName() const
    { return m_xNameSupplier->getName(); }

};

} // namespace ucbhelper

#endif /* !_UCBHELPER_SIMPLENAMECLASHRESOLVEREQUEST_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
