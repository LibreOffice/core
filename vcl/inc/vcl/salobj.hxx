/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salobj.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:07:46 $
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

#ifndef _SV_SALOBJ_HXX
#define _SV_SALOBJ_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_SALGTYPE_HXX
#include <vcl/salgtype.hxx>
#endif
#ifndef _SV_SALWTYPE_HXX
#include <vcl/salwtype.hxx>
#endif

struct SystemEnvData;

// -------------------
// - SalObject-Types -
// -------------------

#define SAL_OBJECT_CLIP_INCLUDERECTS            ((USHORT)0x0001)
#define SAL_OBJECT_CLIP_EXCLUDERECTS            ((USHORT)0x0002)
#define SAL_OBJECT_CLIP_ABSOLUTE                ((USHORT)0x0004)

// -------------
// - SalObject -
// -------------

class VCL_DLLPUBLIC SalObject
{
    void*               m_pInst;
    SALOBJECTPROC       m_pCallback;
    BOOL                m_bMouseTransparent:1,
                        m_bEraseBackground:1;
public:
    SalObject() : m_pInst( NULL ), m_pCallback( NULL ), m_bMouseTransparent( FALSE ), m_bEraseBackground( TRUE ) {}
    virtual ~SalObject();

    virtual void                    ResetClipRegion() = 0;
    virtual USHORT                  GetClipRegionType() = 0;
    virtual void                    BeginSetClipRegion( ULONG nRects ) = 0;
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) = 0;
    virtual void                    EndSetClipRegion() = 0;

    virtual void                    SetPosSize( long nX, long nY, long nWidth, long nHeight ) = 0;
    virtual void                    Show( BOOL bVisible ) = 0;
    virtual void                    Enable( BOOL nEnable ) = 0;
    virtual void                    GrabFocus() = 0;

    virtual void                    SetBackground() = 0;
    virtual void                    SetBackground( SalColor nSalColor ) = 0;

    virtual const SystemEnvData*    GetSystemData() const = 0;

    void                    SetCallback( void* pInst, SALOBJECTPROC pProc )
    { m_pInst = pInst; m_pCallback = pProc; }
    long                    CallCallback( USHORT nEvent, const void* pEvent )
    { return m_pCallback ? m_pCallback( m_pInst, this, nEvent, pEvent ) : 0; }
    void                    SetMouseTransparent( BOOL bMouseTransparent )
    { m_bMouseTransparent = bMouseTransparent; }
    BOOL                    IsMouseTransparent()
    { return m_bMouseTransparent; }
    void                    EnableEraseBackground( BOOL bEnable )
    { m_bEraseBackground = bEnable; }
    BOOL                    IsEraseBackgroundEnabled()
    { return m_bEraseBackground; }
};

#endif // _SV_SALOBJ_HXX
