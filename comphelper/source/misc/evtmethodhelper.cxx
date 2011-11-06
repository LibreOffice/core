/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include "comphelper/evtmethodhelper.hxx"
#include "cppuhelper/typeprovider.hxx"

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Type;

namespace comphelper
{

    Sequence< ::rtl::OUString> getEventMethodsForType(const Type& type)
    {
        typelib_InterfaceTypeDescription *pType=0;
        type.getDescription( (typelib_TypeDescription**)&pType);

        if(!pType)
            return Sequence< ::rtl::OUString>();

        Sequence< ::rtl::OUString> aNames(pType->nMembers);
        ::rtl::OUString* pNames = aNames.getArray();
        for(sal_Int32 i=0;i<pType->nMembers;i++,++pNames)
        {
            // the decription reference
            typelib_TypeDescriptionReference* pMemberDescriptionReference = pType->ppMembers[i];
            // the description for the reference
            typelib_TypeDescription* pMemberDescription = NULL;
            typelib_typedescriptionreference_getDescription(&pMemberDescription, pMemberDescriptionReference);
            if (pMemberDescription)
            {
                typelib_InterfaceMemberTypeDescription* pRealMemberDescription =
                reinterpret_cast<typelib_InterfaceMemberTypeDescription*>(pMemberDescription);
                *pNames = pRealMemberDescription->pMemberName;
            }
        }
        typelib_typedescription_release( (typelib_TypeDescription *)pType );
        return aNames;
    }

}







