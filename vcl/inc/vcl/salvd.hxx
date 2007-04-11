/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salvd.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:09:27 $
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

#ifndef _SV_SALVD_HXX
#define _SV_SALVD_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

class SalGraphics;

// --------------------
// - SalVirtualDevice -
// --------------------

class VCL_DLLPUBLIC SalVirtualDevice
{
public:                     // public for Sal Implementation
    SalVirtualDevice() {}
    virtual ~SalVirtualDevice();

    // SalGraphics or NULL, but two Graphics for all SalVirtualDevices
    // must be returned
    virtual SalGraphics*            GetGraphics() = 0;
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics ) = 0;

                            // Set new size, without saving the old contents
    virtual BOOL                    SetSize( long nNewDX, long nNewDY ) = 0;

    /// Get actual VDev size in pixel
    virtual void                    GetSize( long& rWidth, long& rHeight ) = 0;
};

#endif // _SV_SALVD_HXX
