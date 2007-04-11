/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxtraitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:41:19 $
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
#ifndef _SXTRAITM_HXX
#define _SXTRAITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef _SDMETITM_HXX
#include <svx/sdmetitm.hxx>
#endif

//------------------------------
// class SdrTransformRef1XItem
//------------------------------
class SdrTransformRef1XItem: public SdrMetricItem {
public:
    SdrTransformRef1XItem(long nHgt=0): SdrMetricItem(SDRATTR_TRANSFORMREF1X,nHgt)  {}
    SdrTransformRef1XItem(SvStream& rIn): SdrMetricItem(SDRATTR_TRANSFORMREF1X,rIn) {}
};

//------------------------------
// class SdrTransformRef1YItem
//------------------------------
class SdrTransformRef1YItem: public SdrMetricItem {
public:
    SdrTransformRef1YItem(long nHgt=0): SdrMetricItem(SDRATTR_TRANSFORMREF1Y,nHgt)  {}
    SdrTransformRef1YItem(SvStream& rIn): SdrMetricItem(SDRATTR_TRANSFORMREF1Y,rIn) {}
};

//------------------------------
// class SdrTransformRef2XItem
//------------------------------
class SdrTransformRef2XItem: public SdrMetricItem {
public:
    SdrTransformRef2XItem(long nHgt=0): SdrMetricItem(SDRATTR_TRANSFORMREF2X,nHgt)  {}
    SdrTransformRef2XItem(SvStream& rIn): SdrMetricItem(SDRATTR_TRANSFORMREF2X,rIn) {}
};

//------------------------------
// class SdrTransformRef2YItem
//------------------------------
class SdrTransformRef2YItem: public SdrMetricItem {
public:
    SdrTransformRef2YItem(long nHgt=0): SdrMetricItem(SDRATTR_TRANSFORMREF2Y,nHgt)  {}
    SdrTransformRef2YItem(SvStream& rIn): SdrMetricItem(SDRATTR_TRANSFORMREF2Y,rIn) {}
};

#endif
