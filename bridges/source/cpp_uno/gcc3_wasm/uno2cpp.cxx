/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/RuntimeException.hpp>

#include <bridge.hxx>
#include <types.hxx>
#include <unointerfaceproxy.hxx>
#include <vtables.hxx>

using namespace ::com::sun::star::uno;

namespace bridges::cpp_uno::shared
{
void unoInterfaceProxyDispatch(uno_Interface* pUnoI, const typelib_TypeDescription* pMemberDescr,
                               void* pReturn, void* pArgs[], uno_Any** ppException)
{
    bridges::cpp_uno::shared::UnoInterfaceProxy* pThis
        = static_cast<bridges::cpp_uno::shared::UnoInterfaceProxy*>(pUnoI);

    switch (pMemberDescr->eTypeClass)
    {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            std::abort();
            break;
        }
        case typelib_TypeClass_INTERFACE_METHOD:
        {
            VtableSlot aVtableSlot(getVtableSlot(
                reinterpret_cast<typelib_InterfaceMethodTypeDescription const*>(pMemberDescr)));

            switch (aVtableSlot.index)
            {
                case 1: // acquire uno interface
                    (*pUnoI->acquire)(pUnoI);
                    *ppException = 0;
                    break;
                case 2: // release uno interface
                    (*pUnoI->release)(pUnoI);
                    *ppException = 0;
                    break;
                case 0: // queryInterface() opt
                {
                    typelib_TypeDescription* pTD = 0;
                    TYPELIB_DANGER_GET(&pTD, reinterpret_cast<Type*>(pArgs[0])->getTypeLibType());
                    if (pTD)
                    {
                        uno_Interface* pInterface = 0;
                        (*pThis->getBridge()->getUnoEnv()->getRegisteredInterface)(
                            pThis->getBridge()->getUnoEnv(), (void**)&pInterface, pThis->oid.pData,
                            (typelib_InterfaceTypeDescription*)pTD);

                        if (pInterface)
                        {
                            ::uno_any_construct(reinterpret_cast<uno_Any*>(pReturn), &pInterface,
                                                pTD, 0);
                            (*pInterface->release)(pInterface);
                            TYPELIB_DANGER_RELEASE(pTD);
                            *ppException = 0;
                            break;
                        }
                        TYPELIB_DANGER_RELEASE(pTD);
                    }
                } // else perform queryInterface()
                    [[fallthrough]];
                default:
                    std::abort();
            }
            break;
        }
        default:
        {
            ::com::sun::star::uno::RuntimeException aExc(
                "illegal member type description!",
                ::com::sun::star::uno::Reference<::com::sun::star::uno::XInterface>());

            Type const& rExcType = cppu::UnoType<decltype(aExc)>::get();
            // binary identical null reference
            ::uno_type_any_construct(*ppException, &aExc, rExcType.getTypeLibType(), 0);
        }
    }
}

} // namespace bridges::cpp_uno::shared

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
