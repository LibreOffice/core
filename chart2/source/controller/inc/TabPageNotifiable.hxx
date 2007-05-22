/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TabPageNotifiable.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:56:47 $
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
#ifndef CHART2_TABPAGENOTIFIABLE_HXX
#define CHART2_TABPAGENOTIFIABLE_HXX

#include <vcl/tabpage.hxx>
#include <tools/color.hxx>

// color to use as foreground for an invalid range
#define RANGE_SELECTION_INVALID_RANGE_FOREGROUND_COLOR COL_WHITE
// color to use as background for an invalid range
#define RANGE_SELECTION_INVALID_RANGE_BACKGROUND_COLOR 0xff6563

namespace chart
{

class TabPageNotifiable
{
public:
    virtual void setInvalidPage( TabPage * pTabPage ) = 0;
    virtual void setValidPage( TabPage * pTabPage ) = 0;
};

} //  namespace chart

// CHART2_TABPAGENOTIFIABLE_HXX
#endif
