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
#include "precompiled_vcl.hxx"

#include "osl/diagnose.h"

#include "vcl/svapp.hxx"

#include "aqua/saldata.hxx"
#include "aqua/salinst.h"

#include "DragSource.hxx"
#include "DropTarget.hxx"
#include "aqua_clipboard.hxx"

using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;


uno::Reference< XInterface > AquaSalInstance::CreateClipboard( const Sequence< Any >& i_rArguments )
{
    if ( Application::IsHeadlessModeEnabled() )
        return SalInstance::CreateClipboard( i_rArguments );

    SalData* pSalData = GetSalData();
    if( ! pSalData->mxClipboard.is() )
        pSalData->mxClipboard = uno::Reference<XInterface>(static_cast< XClipboard* >(new AquaClipboard()), UNO_QUERY);
    return pSalData->mxClipboard;
}

uno::Reference<XInterface> AquaSalInstance::CreateDragSource()
{
    if ( Application::IsHeadlessModeEnabled() )
        return SalInstance::CreateDragSource();

    return uno::Reference<XInterface>(static_cast< XInitialization* >(new DragSource()), UNO_QUERY);
}

uno::Reference<XInterface> AquaSalInstance::CreateDropTarget()
{
    if ( Application::IsHeadlessModeEnabled() )
        return SalInstance::CreateDropTarget();

    return uno::Reference<XInterface>(static_cast< XInitialization* >(new DropTarget()), UNO_QUERY);
}

