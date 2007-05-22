/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_BarGeometry.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:39:13 $
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
#ifndef _CHART2_RES_BARGEOMETRY_HXX
#define _CHART2_RES_BARGEOMETRY_HXX

// header for class FixedText
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for class ListBox
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

class BarGeometryResources
{
public:
    BarGeometryResources( Window* pParent );
    virtual ~BarGeometryResources();

    void  SetPosPixel( const Point& rPosition );
    Point GetPosPixel() const;
    Size  GetSizePixel() const;

    void Show( bool bShow );
    void Enable( bool bEnable );

    USHORT GetSelectEntryCount() const;
    USHORT GetSelectEntryPos() const;
    void SelectEntryPos( USHORT nPos );

    void SetSelectHdl( const Link& rLink );

private:
    FixedText   m_aFT_Geometry;
    ListBox     m_aLB_Geometry;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
