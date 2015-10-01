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

// no include protection. This file is included from elementimport.hxx only.

#ifndef _INCLUDING_FROM_ELEMENTIMPORT_HXX_
#error "do not include this file directly!"
#endif

#include <osl/diagnose.h>

// no namespace. Same as above: this file is included from elementimport.hxx only,
// and this is done inside the namespace

//= OColumnImport
template <class BASE>
OColumnImport< BASE >::OColumnImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
        OControlElement::ElementType _eType)
    :BASE(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType)
    ,m_xColumnFactory(_rxParentContainer, ::com::sun::star::uno::UNO_QUERY)
{
    OSL_ENSURE(m_xColumnFactory.is(), "OColumnImport::OColumnImport: invalid parent container (no factory)!");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
