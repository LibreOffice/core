/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlPropertyHandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:18 $
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
#include "precompiled_reportdesign.hxx"

#include "xmlPropertyHandler.hxx"
#ifndef RPT_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_TEXTIMP_HXX_
#include <xmloff/txtimp.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

//--------------------------------------------------------------------------
namespace rptxml
{
//--------------------------------------------------------------------------
    using namespace ::com::sun::star;
    using namespace xmloff;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
DBG_NAME( rpt_OXMLRptPropHdlFactory )

OXMLRptPropHdlFactory::OXMLRptPropHdlFactory()
{
    DBG_CTOR( rpt_OXMLRptPropHdlFactory,NULL);

}
// -----------------------------------------------------------------------------
OXMLRptPropHdlFactory::~OXMLRptPropHdlFactory()
{

    DBG_DTOR( rpt_OXMLRptPropHdlFactory,NULL);
}
// -----------------------------------------------------------------------------
}// rptxml
// -----------------------------------------------------------------------------

