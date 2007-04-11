/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:46:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_ACCEL_HXX
#define _SV_ACCEL_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _TOOLS_RC_HXX
#include <tools/rc.hxx>
#endif
#ifndef _SV_KEYCOD_HXX
#include <vcl/keycod.hxx>
#endif

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
