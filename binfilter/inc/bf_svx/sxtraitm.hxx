/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SXTRAITM_HXX
#define _SXTRAITM_HXX

#include <bf_svx/svddef.hxx>

#include <bf_svx/sdmetitm.hxx>
namespace binfilter {

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

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
