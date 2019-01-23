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

#ifndef INCLUDED_VCL_ACCEL_HXX
#define INCLUDED_VCL_ACCEL_HXX

#include <tools/link.hxx>
#include <vcl/keycod.hxx>
#include <vcl/dllapi.h>
#include <memory>

class ImplAccelData;
class ImplAccelEntry;
class CommandEvent;

class VCL_DLLPUBLIC Accelerator
{
    friend class ImplAccelManager;

private:
    std::unique_ptr<ImplAccelData> mpData;
    Link<Accelerator&,void> maActivateHdl;
    Link<Accelerator&,void> maSelectHdl;

    // Will be set by AcceleratorManager
    vcl::KeyCode            maCurKeyCode;
    sal_uInt16              mnCurId;
    bool*                   mpDel;

    SAL_DLLPRIVATE  void    ImplInit();
    SAL_DLLPRIVATE  void    ImplCopyData( ImplAccelData& rAccelData );
    SAL_DLLPRIVATE  void    ImplDeleteData();
    SAL_DLLPRIVATE  void    ImplInsertAccel(
                                sal_uInt16 nItemId,
                                const vcl::KeyCode& rKeyCode,
                                bool bEnable,
                                Accelerator* pAutoAccel );

    SAL_DLLPRIVATE  ImplAccelEntry*
                            ImplGetAccelData( const vcl::KeyCode& rKeyCode ) const;

public:
                            Accelerator();
                            Accelerator( const Accelerator& rAccel );
                            ~Accelerator();

    void                    Activate();
    void                    Select();

    void                    InsertItem( sal_uInt16 nItemId, const vcl::KeyCode& rKeyCode );

    sal_uInt16              GetCurItemId() const { return mnCurId; }
    const vcl::KeyCode&     GetCurKeyCode() const { return maCurKeyCode; }

    sal_uInt16              GetItemCount() const;
    sal_uInt16              GetItemId( sal_uInt16 nPos ) const;

    Accelerator*            GetAccel( sal_uInt16 nItemId ) const;

    void                    SetActivateHdl( const Link<Accelerator&,void>& rLink ) { maActivateHdl = rLink; }
    void                    SetSelectHdl( const Link<Accelerator&,void>& rLink ) { maSelectHdl = rLink; }

    Accelerator&            operator=( const Accelerator& rAccel );

    static bool             ToggleMnemonicsOnHierarchy(const CommandEvent& rCEvent, const vcl::Window *pWindow);
    static void             GenerateAutoMnemonicsOnHierarchy(const vcl::Window* pWindow);
};

#endif // INCLUDED_VCL_ACCEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
