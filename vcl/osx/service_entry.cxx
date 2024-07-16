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


#include <vcl/svapp.hxx>
#include <dndhelper.hxx>
#include <vcl/sysdata.hxx>

#include <osx/saldata.hxx>
#include <osx/salinst.h>

#include "DragSource.hxx"
#include "DropTarget.hxx"
#include "clipboard.hxx"

using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;

uno::Reference< XInterface > AquaSalInstance::CreateClipboard( const Sequence< Any >& i_rArguments )
{
    if ( Application::IsHeadlessModeEnabled() || IsRunningUnitTest() || IsRunningUITest() )
        return SalInstance::CreateClipboard( i_rArguments );

    SalData* pSalData = GetSalData();
    if( ! pSalData->mxClipboard.is() )
        pSalData->mxClipboard.set(static_cast< XClipboard* >(new AquaClipboard(nullptr, true)), UNO_QUERY);
    return pSalData->mxClipboard;
}

uno::Reference<XInterface> AquaSalInstance::ImplCreateDragSource(const SystemEnvData* pSysEnv)
{
    return vcl::OleDnDHelper(new DragSource(), reinterpret_cast<sal_IntPtr>(pSysEnv->mpNSView),
                             vcl::DragOrDrop::Drag);
}

uno::Reference<XInterface> AquaSalInstance::ImplCreateDropTarget(const SystemEnvData* pSysEnv)
{
    return vcl::OleDnDHelper(new DropTarget(), reinterpret_cast<sal_IntPtr>(pSysEnv->mpNSView),
                             vcl::DragOrDrop::Drop);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
