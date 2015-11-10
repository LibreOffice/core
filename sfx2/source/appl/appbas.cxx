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

#include <config_features.h>
#include <config_options.h>

#include <sal/config.h>

#include <cassert>

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/uno/Reference.h>
#include <basic/basrdll.hxx>
#include <officecfg/Office/Common.hxx>
#include <svl/macitem.hxx>
#include <basic/sbxfac.hxx>
#include <basic/sbx.hxx>
#include <vcl/gradient.hxx>
#include <svl/rectitem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <basic/sbmod.hxx>
#include <svl/whiter.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbstar.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <basic/sbuno.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>

#include <sfx2/module.hxx>
#include "arrdecl.hxx"
#include <sfx2/app.hxx>
#include "sfxtypes.hxx"
#include <sfx2/sfxresid.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/minfitem.hxx>
#include "app.hrc"
#include <sfx2/evntconf.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dinfdlg.hxx>
#include "appdata.hxx"
#include <sfx2/sfxhelp.hxx>
#include <basic/basmgr.hxx>
#include <svtools/svtools.hrc>
#include <unotools/configmgr.hxx>
#include "sorgitm.hxx"
#include "appbaslib.hxx"
#include <basic/basicmanagerrepository.hxx>

#include <svl/srchitem.hxx>
#include <osl/socket.hxx>

#define SFX_TYPEMAP
#include "sfxslots.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::script;

using ::basic::BasicManagerRepository;


sal_uInt16 SfxApplication::SaveBasicAndDialogContainer() const
{
    if ( pAppData_Impl->pBasicManager->isValid() )
        pAppData_Impl->pBasicManager->storeAllLibraries();
    return 0;
}

BasicManager* SfxApplication::GetBasicManager()
{
#if !HAVE_FEATURE_SCRIPTING
    return 0;
#else
    if (utl::ConfigManager::IsAvoidConfig())
        return nullptr;
    return BasicManagerRepository::getApplicationBasicManager( true );
#endif
}

XLibraryContainer * SfxApplication::GetDialogContainer()
{
#if !HAVE_FEATURE_SCRIPTING
    return NULL;
#else
    if (utl::ConfigManager::IsAvoidConfig())
        return nullptr;
    if ( !pAppData_Impl->pBasicManager->isValid() )
        GetBasicManager();
    return pAppData_Impl->pBasicManager->getLibraryContainer( SfxBasicManagerHolder::DIALOGS );
#endif
}



XLibraryContainer * SfxApplication::GetBasicContainer()
{
#if !HAVE_FEATURE_SCRIPTING
    return NULL;
#else
    if (utl::ConfigManager::IsAvoidConfig())
        return nullptr;
    if ( !pAppData_Impl->pBasicManager->isValid() )
        GetBasicManager();
    return pAppData_Impl->pBasicManager->getLibraryContainer( SfxBasicManagerHolder::SCRIPTS );
#endif
}

StarBASIC* SfxApplication::GetBasic()
{
#if !HAVE_FEATURE_SCRIPTING
    return 0;
#else
    if (utl::ConfigManager::IsAvoidConfig())
        return nullptr;
    return GetBasicManager()->GetLib(0);
#endif
}

void SfxApplication::PropExec_Impl( SfxRequest &rReq )
{
    sal_uInt16 nSID = rReq.GetSlot();
    switch ( nSID )
    {
        case SID_ATTR_UNDO_COUNT:
        {
            if (const SfxUInt16Item* pCountItem = rReq.GetArg<SfxUInt16Item>(nSID))
            {
                std::shared_ptr< comphelper::ConfigurationChanges > batch(
                    comphelper::ConfigurationChanges::create());
                officecfg::Office::Common::Undo::Steps::set(
                    pCountItem->GetValue(), batch);
                batch->commit();
            }
            break;
        }

        default:
            assert(false);
    }
}

void SfxApplication::PropState_Impl( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    for ( sal_uInt16 nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        switch ( nSID )
        {
            case SID_ATTR_UNDO_COUNT:
                rSet.Put(
                    SfxUInt16Item(
                        SID_ATTR_UNDO_COUNT,
                        officecfg::Office::Common::Undo::Steps::get()));
                break;

            default:
                assert(false);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
