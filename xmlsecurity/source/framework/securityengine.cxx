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


#include "securityengine.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;

SecurityEngine::SecurityEngine()
    :m_nIdOfTemplateEC(-1),
     m_nNumOfResolvedReferences(0),
     m_nIdOfKeyEC(-1),
     m_bMissionDone(false),
     m_nSecurityId(-1),
     m_nStatus(::com::sun::star::xml::crypto::SecurityOperationStatus_UNKNOWN)
{
}

/* XReferenceResolvedListener */
void SAL_CALL SecurityEngine::referenceResolved( sal_Int32 /*referenceId*/)
    throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception)
{
    m_nNumOfResolvedReferences++;
    tryToPerform();
}

/* XKeyCollector */
void SAL_CALL SecurityEngine::setKeyId( sal_Int32 id )
    throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception)
{
    m_nIdOfKeyEC = id;
    tryToPerform();
}

/* XMissionTaker */
sal_Bool SAL_CALL SecurityEngine::endMission(  )
    throw (com::sun::star::uno::RuntimeException, std::exception)
{
    bool rc = m_bMissionDone;

    if (!rc)
    {
        clearUp( );

        notifyResultListener();
        m_bMissionDone = true;
    }

    m_xResultListener = nullptr;
    m_xSAXEventKeeper = nullptr;

    return rc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
