/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accel.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_ACCEL_HXX
#define _SV_ACCEL_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
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
    XubString       maHelpStr;
    Link            maActivateHdl;
    Link            maDeactivateHdl;
    Link            maSelectHdl;

    // Werden vom AcceleratorManager gesetzt
    KeyCode         maCurKeyCode;
    USHORT          mnCurId;
    USHORT          mnCurRepeat;
    BOOL            mbIsCancel;
    BOOL*           mpDel;

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE  void        ImplInit();
    SAL_DLLPRIVATE  void        ImplCopyData( ImplAccelData& rAccelData );
    SAL_DLLPRIVATE  void        ImplDeleteData();
    SAL_DLLPRIVATE  void        ImplInsertAccel( USHORT nItemId, const KeyCode& rKeyCode,
                                     BOOL bEnable, Accelerator* pAutoAccel );

    SAL_DLLPRIVATE  ImplAccelEntry* ImplGetAccelData( const KeyCode& rKeyCode ) const;
//#endif

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

    void            InsertItem( USHORT nItemId, const KeyCode& rKeyCode );
    void            InsertItem( const ResId& rResId );
    void            RemoveItem( USHORT nItemId );
    void            RemoveItem( const KeyCode rKeyCode );
    void            Clear();

    USHORT          GetCurItemId() const { return mnCurId; }
    const KeyCode&  GetCurKeyCode() const { return maCurKeyCode; }
    USHORT          GetCurRepeat() const { return mnCurRepeat; }
    BOOL            IsCancel() const { return mbIsCancel; }

    USHORT          GetItemCount() const;
    USHORT          GetItemId( USHORT nPos ) const;
    KeyCode         GetItemKeyCode( USHORT nPos ) const;
    USHORT          GetItemId( const KeyCode& rKeyCode ) const;
    KeyCode         GetKeyCode( USHORT nItemId ) const;
    BOOL            IsIdValid( USHORT nItemId ) const;
    BOOL            IsKeyCodeValid( const KeyCode rKeyCode ) const;
    BOOL            Call( const KeyCode& rKeyCode, USHORT nRepeat = 0 );

    void            SetAccel( USHORT nItemId, Accelerator* pAccel );
    Accelerator*    GetAccel( USHORT nItemId ) const;
    void            SetAccel( const KeyCode rKeyCode, Accelerator* pAccel );
    Accelerator*    GetAccel( const KeyCode rKeyCode ) const;

    void            EnableItem( USHORT nItemId, BOOL bEnable = TRUE );
    BOOL            IsItemEnabled( USHORT nItemId ) const;
    void            EnableItem( const KeyCode rKeyCode, BOOL bEnable = TRUE );
    BOOL            IsItemEnabled( const KeyCode rKeyCode ) const;

    void            SetHelpText( const XubString& rHelpText ) { maHelpStr = rHelpText; }
    const XubString& GetHelpText() const { return maHelpStr; }

    void            SetActivateHdl( const Link& rLink ) { maActivateHdl = rLink; }
    const Link&     GetActivateHdl() const { return maActivateHdl; }
    void            SetDeactivateHdl( const Link& rLink ) { maDeactivateHdl = rLink; }
    const Link&     GetDeactivateHdl() const { return maDeactivateHdl; }
    void            SetSelectHdl( const Link& rLink ) { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const { return maSelectHdl; }

    Accelerator&    operator=( const Accelerator& rAccel );
};

#endif  // _SV_ACCEL_HXX
