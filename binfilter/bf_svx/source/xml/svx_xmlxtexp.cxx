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

#include <tools/debug.hxx>

#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>

#include <bf_sfx2/docfile.hxx>


#include <bf_xmloff/xmluconv.hxx>

#include "bf_xmloff/xmlnmspe.hxx"

#include "bf_xmloff/nmspmap.hxx"

#include "bf_xmloff/xmlkywd.hxx"


#include "bf_xmloff/DashStyle.hxx"

#include "bf_xmloff/GradientStyle.hxx"

#include "bf_xmloff/HatchStyle.hxx"

#include "bf_xmloff/ImageStyle.hxx"

#include "bf_xmloff/MarkerStyle.hxx"

#include <com/sun/star/container/XNameContainer.hpp>


#include <unotools/streamwrap.hxx>

#include "xmlgrhlp.hxx"

#include "xmlxtexp.hxx"
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
namespace binfilter {
using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::rtl;
using namespace cppu;

/*N*/ class SvxXMLTableEntryExporter
/*N*/ {
/*N*/ public:
/*N*/ 	SvxXMLTableEntryExporter( SvXMLExport& rExport ) : mrExport( rExport ) {}
/*N*/ 	virtual ~SvxXMLTableEntryExporter();
/*N*/ 
/*N*/ 	virtual void exportEntry( const OUString& rStrName, const Any& rValue ) = 0;
/*N*/ 
/*N*/ protected:
/*N*/ 	SvXMLExport& mrExport;
/*N*/ };
/*N*/ 
/*N*/ ///////////////////////////////////////////////////////////////////////
/*N*/ 
/*N*/ SvxXMLTableEntryExporter::~SvxXMLTableEntryExporter()
/*N*/ {
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
