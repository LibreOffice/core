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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HATCH_HPP_
#include <com/sun/star/drawing/Hatch.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif

#ifndef _SFXDOCFILE_HXX
#include <bf_sfx2/docfile.hxx>
#endif


#ifndef _XMLOFF_XMLUCONV_HXX
#include <bf_xmloff/xmluconv.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "bf_xmloff/xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "bf_xmloff/nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "bf_xmloff/xmlkywd.hxx"
#endif


#ifndef _XMLOFF_DASHSTYLE_HXX
#include "bf_xmloff/DashStyle.hxx"
#endif

#ifndef _XMLOFF_GRADIENTSTYLE_HXX
#include "bf_xmloff/GradientStyle.hxx"
#endif

#ifndef _XMLOFF_HATCHSTYLE_HXX
#include "bf_xmloff/HatchStyle.hxx"
#endif

#ifndef _XMLOFF_IMAGESTYLE_HXX
#include "bf_xmloff/ImageStyle.hxx"
#endif

#ifndef _XMLOFF_MARKERSTYLE_HXX
#include "bf_xmloff/MarkerStyle.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif


#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#ifndef _XMLGRHLP_HXX
#include "xmlgrhlp.hxx"
#endif

#include "xmlxtexp.hxx"
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
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
