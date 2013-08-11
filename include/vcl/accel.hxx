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

#ifndef _SV_ACCEL_HXX
#define _SV_ACCEL_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <tools/link.hxx>
#include <tools/resid.hxx>
#include <tools/rc.hxx>
#include <vcl/keycod.hxx>

class ImplAccelData;
class ImplAccelEntry;

// ---------------
// - Accelerator -
// ---------------

class VCL_DLLPUBLIC Accelerator : public Resource
{
    friend class ImplAccelManager;

private:
    ImplAccelData*  mpData;
    OUString        maHelpStr;
    Link            maActivateHdl;
    Link            maDeactivateHdl;
    Link            maSelectHdl;

    // Will be set by AcceleratorManager
    KeyCode         maCurKeyCode;
    sal_uInt16      mnCurId;
    sal_uInt16      mnCurRepeat;
    sal_Bool        mbIsCancel;
    sal_Bool*       mpDel;

    SAL_DLLPRIVATE  void        ImplInit();
    SAL_DLLPRIVATE  void        ImplCopyData( ImplAccelData& rAccelData );
    SAL_DLLPRIVATE  void        ImplDeleteData();
    SAL_DLLPRIVATE  void        ImplInsertAccel( sal_uInt16 nItemId, const KeyCode& rKeyCode,
                                     sal_Bool bEnable, Accelerator* pAutoAccel );

    SAL_DLLPRIVATE  ImplAccelEntry* ImplGetAccelData( const KeyCode& rKeyCode ) const;

protected:
    SAL_DLLPRIVATE  void        ImplLoadRes( const ResId& rResId );

public:
                    Accelerator();
                    Accelerator( const Accelerator& rAccel );
                    Accelerator( const ResId& rResId );
    virtual         ~Accelerator();

    virtual void    Activate();
    virtual void    Deactivate();
    virtual void    Select();

    void            InsertItem( sal_uInt16 nItemId, const KeyCode& rKeyCode );
    void            InsertItem( const ResId& rResId );

    sal_uInt16          GetCurItemId() const { return mnCurId; }
    const KeyCode&  GetCurKeyCode() const { return maCurKeyCode; }
    sal_uInt16          GetCurRepeat() const { return mnCurRepeat; }
    sal_Bool            IsCancel() const { return mbIsCancel; }

    sal_uInt16          GetItemCount() const;
    sal_uInt16          GetItemId( sal_uInt16 nPos ) const;
    KeyCode         GetKeyCode( sal_uInt16 nItemId ) const;

    Accelerator*    GetAccel( sal_uInt16 nItemId ) const;

    void            SetHelpText( const OUString& rHelpText ) { maHelpStr = rHelpText; }
    const OUString& GetHelpText() const { return maHelpStr; }

    void            SetActivateHdl( const Link& rLink ) { maActivateHdl = rLink; }
    const Link&     GetActivateHdl() const { return maActivateHdl; }
    void            SetDeactivateHdl( const Link& rLink ) { maDeactivateHdl = rLink; }
    const Link&     GetDeactivateHdl() const { return maDeactivateHdl; }
    void            SetSelectHdl( const Link& rLink ) { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const { return maSelectHdl; }

    Accelerator&    operator=( const Accelerator& rAccel );
};

#endif  // _SV_ACCEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
