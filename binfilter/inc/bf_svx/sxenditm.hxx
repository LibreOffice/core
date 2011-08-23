/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _SXENDITM_HXX
#define _SXENDITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif

#ifndef _SDMETITM_HXX
#include <bf_svx/sdmetitm.hxx>
#endif
namespace binfilter {

class SdrEdgeNode1HorzDistItem: public SdrMetricItem {
public:
    SdrEdgeNode1HorzDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE1HORZDIST,nVal)  {}
    SdrEdgeNode1HorzDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE1HORZDIST,rIn) {}
};

class SdrEdgeNode1VertDistItem: public SdrMetricItem {
public:
    SdrEdgeNode1VertDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE1VERTDIST,nVal)  {}
    SdrEdgeNode1VertDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE1VERTDIST,rIn) {}
};

class SdrEdgeNode2HorzDistItem: public SdrMetricItem {
public:
    SdrEdgeNode2HorzDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE2HORZDIST,nVal)  {}
    SdrEdgeNode2HorzDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE2HORZDIST,rIn) {}
};

class SdrEdgeNode2VertDistItem: public SdrMetricItem {
public:
    SdrEdgeNode2VertDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE2VERTDIST,nVal)  {}
    SdrEdgeNode2VertDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE2VERTDIST,rIn) {}
};

class SdrEdgeNode1GlueDistItem: public SdrMetricItem {
public:
    SdrEdgeNode1GlueDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE1GLUEDIST,nVal)  {}
    SdrEdgeNode1GlueDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE1GLUEDIST,rIn) {}
};

class SdrEdgeNode2GlueDistItem: public SdrMetricItem {
public:
    SdrEdgeNode2GlueDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE2GLUEDIST,nVal)  {}
    SdrEdgeNode2GlueDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE2GLUEDIST,rIn) {}
};

}//end of namespace binfilter
#endif
