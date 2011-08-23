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

#ifndef _UITOOL_HXX
#define _UITOOL_HXX

#include <vcl/wintypes.hxx>
#include <vcl/field.hxx>

/*N*/ #include <tools/debug.hxx> //for stripping
class Printer; 
class ListBox; 
class DateTime; 
namespace binfilter {
class SfxItemSet; 
class MetricFormatter;
class SfxMedium;
class SwPageDesc;

class SvxTabStopItem;
class SwWrtShell;

class SwDocShell;
class SwFrmFmt;
class SwTabCols;


// Umschalten einer Metric

// BoxInfoAttribut fuellen

// SfxItemSets <-> PageDesc
void ItemSetToPageDesc( const SfxItemSet& rSet, SwPageDesc& rPageDesc );
void PageDescToItemSet( const SwPageDesc& rPageDesc, SfxItemSet& rSet);

} //namespace binfilter
#endif // _UITOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
