/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FastPropertyIdRanges.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:40:25 $
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
#ifndef CHART_FASTPROPERTYIDRANGES_HXX
#define CHART_FASTPROPERTYIDRANGES_HXX

namespace chart
{

enum FastPropertyIdRanges
{
    FAST_PROPERTY_ID_START = 10000,
    FAST_PROPERTY_ID_START_DATA_SERIES = FAST_PROPERTY_ID_START + 1000,
    FAST_PROPERTY_ID_START_DATA_POINT  = FAST_PROPERTY_ID_START + 2000,
    FAST_PROPERTY_ID_START_CHAR_PROP   = FAST_PROPERTY_ID_START + 3000,
    FAST_PROPERTY_ID_START_LINE_PROP   = FAST_PROPERTY_ID_START + 4000,
    FAST_PROPERTY_ID_START_FILL_PROP   = FAST_PROPERTY_ID_START + 5000,
    FAST_PROPERTY_ID_START_USERDEF_PROP= FAST_PROPERTY_ID_START + 6000
};

} //  namespace chart

// CHART_FASTPROPERTYIDRANGES_HXX
#endif
