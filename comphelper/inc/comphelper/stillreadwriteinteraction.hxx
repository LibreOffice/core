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

#ifndef _COMPHELPER_STILLREADWRITEINTERACTION_HXX_
#define _COMPHELPER_STRILLREADWRITEINTERACTION_HXX_

//_______________________________________________
// includes
#include <ucbhelper/interceptedinteraction.hxx>

#ifndef __COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP__
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#include <ucbhelper/interceptedinteraction.hxx>
#include "comphelper/comphelperdllapi.h"

//_______________________________________________
// namespace

namespace comphelper{
class COMPHELPER_DLLPUBLIC StillReadWriteInteraction : public ::ucbhelper::InterceptedInteraction
{
private:
    static const sal_Int32 HANDLE_INTERACTIVEIOEXCEPTION       = 0;
    static const sal_Int32 HANDLE_UNSUPPORTEDDATASINKEXCEPTION = 1;

    sal_Bool m_bUsed;
    sal_Bool m_bHandledByMySelf;
    sal_Bool m_bHandledByInternalHandler;

public:
    StillReadWriteInteraction(const com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >& xHandler);

    void resetInterceptions();
    void resetErrorStates();
    sal_Bool wasWriteError();

private:
    virtual ucbhelper::InterceptedInteraction::EInterceptionState intercepted(const ::ucbhelper::InterceptedInteraction::InterceptedRequest&                         aRequest,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& xRequest);

};
}
#endif
