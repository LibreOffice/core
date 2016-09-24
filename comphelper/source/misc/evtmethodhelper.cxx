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

#include <comphelper/evtmethodhelper.hxx>
#include <cppuhelper/typeprovider.hxx>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Type;

namespace comphelper
{

    Sequence< OUString> getEventMethodsForType(const Type& type)
    {
        typelib_InterfaceTypeDescription *pType=nullptr;
        type.getDescription(reinterpret_cast<typelib_TypeDescription**>(&pType));

        if(!pType)
            return Sequence< OUString>();

        Sequence< OUString> aNames(pType->nMembers);
        OUString* pNames = aNames.getArray();
        for(sal_Int32 i=0;i<pType->nMembers;i++,++pNames)
        {
            // the description reference
            typelib_TypeDescriptionReference* pMemberDescriptionReference = pType->ppMembers[i];
            // the description for the reference
            typelib_TypeDescription* pMemberDescription = nullptr;
            typelib_typedescriptionreference_getDescription(&pMemberDescription, pMemberDescriptionReference);
            if (pMemberDescription)
            {
                typelib_InterfaceMemberTypeDescription* pRealMemberDescription =
                reinterpret_cast<typelib_InterfaceMemberTypeDescription*>(pMemberDescription);
                *pNames = pRealMemberDescription->pMemberName;
            }
        }
        typelib_typedescription_release( &pType->aBase );
        return aNames;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
