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
#ifndef _SFX_APPDATA_HXX
#define _SFX_APPDATA_HXX

#include <tools/link.hxx>
#include <tools/list.hxx>
#include <bf_svtools/lstner.hxx>
#include <vcl/timer.hxx>
#include <tools/string.hxx>
#include "rtl/ref.hxx"

#include "bitset.hxx"
class SvStrings;
class Config;
class SvVerbList;
class SvtSaveOptions;
class SvtUndoOptions;
class SvtHelpOptions;

namespace binfilter {

class SvUShorts;
class SfxItemPool;
class SfxCancelManager;
class SfxApplication;

class SfxBmkMenu;
class SfxProgress;
class SfxChildWinFactArr_Impl;
class SfxToolBoxConfig;
class SfxDdeDocTopics_Impl;
class SfxEventConfiguration;
class SfxMacroConfig;
class SfxInitLinkList;
class SfxFilterMatcher;
struct SfxFrameObjectFactoryPtr;
struct SfxPluginObjectFactoryPtr;
class ISfxTemplateCommon;
class SfxFilterMatcher;
class SfxStatusDispatcher;
class SfxMiscCfg;
class SfxDocumentTemplates;
class SfxFrameArr_Impl;
class SfxObjectFactory;
class SfxObjectShell;
namespace sfx2 { namespace appl { class ImeStatusWindow; } }

//=========================================================================
// SfxAppData_Impl
//=========================================================================

class SfxAppData_Impl 
{
public:
    IndexBitSet                         aIndexBitSet;           // for counting noname documents

    // single instance classes
    SfxFrameObjectFactoryPtr*           pSfxFrameObjectFactoryPtr;

    // special members
    SfxInitLinkList*                    pInitLinkList;

    // application members
    SfxFilterMatcher*                   pMatcher;
    SfxCancelManager*                   pCancelMgr;

    // global pointers
    SfxItemPool*                        pPool;
    SfxEventConfiguration*              pEventConfig;
    SfxMiscCfg*                         pMiscConfig;

    // "current" functionality
    SfxObjectShell*						pThisDocument;
    SfxProgress*                        pProgress;

    USHORT                              nBasicCallLevel;
    USHORT                              nRescheduleLocks;
    USHORT                              nInReschedule;

    ::rtl::Reference< sfx2::appl::ImeStatusWindow > m_xImeStatusWindow;

    BOOL                                bInQuit : 1;
    BOOL                                bInException : 1;
    BOOL                                bOLEResize : 1;

                                        SfxAppData_Impl( SfxApplication* );
                                        ~SfxAppData_Impl();
};

}//end of namespace binfilter
#endif // #ifndef _SFX_APPDATA_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
