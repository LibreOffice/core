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
#ifndef _SXCLLITM_HXX
#define _SXCLLITM_HXX

#include <bf_svx/svddef.hxx>

#include <bf_svx/sdynitm.hxx>

#include <bf_svx/sdmetitm.hxx>
namespace binfilter {

//------------------------------------
// class SdrCaptionLineLenItem
// Laenge der ersten Austrittslinie
// Nur bei Type3 und Type4
// Nur relevant, wenn SdrCaptionFitLineLenItem=FALSE
//------------------------------------
class SdrCaptionLineLenItem: public SdrMetricItem {
public:
    SdrCaptionLineLenItem(long nLineLen=0): SdrMetricItem(SDRATTR_CAPTIONLINELEN,nLineLen) {}
    SdrCaptionLineLenItem(SvStream& rIn)  : SdrMetricItem(SDRATTR_CAPTIONLINELEN,rIn)      {}
};

//------------------------------------
// class SdrCaptionFitLineLenItem
// Laenge der ersten Austrittslinie automatisch berechnen oder
// Vorgabe (SdrCaptionLineLenItem) verwenden.
// Nur bei Type3 und Type4
//------------------------------------
class SdrCaptionFitLineLenItem: public SdrYesNoItem {
public:
    SdrCaptionFitLineLenItem(BOOL bBestFit=TRUE): SdrYesNoItem(SDRATTR_CAPTIONFITLINELEN,bBestFit) {}
    SdrCaptionFitLineLenItem(SvStream& rIn)     : SdrYesNoItem(SDRATTR_CAPTIONFITLINELEN,rIn)      {}
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
