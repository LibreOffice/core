/*************************************************************************
 *
 *  $RCSfile: PageMasterPropHdlFactory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-26 08:35:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_PAGEMASTERPROPHDLFACTORY_HXX
#include "PageMasterPropHdlFactory.hxx"
#endif

#ifndef _XMLOFF_XMLTYPES_HXX
#include "xmltypes.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX
#include "xmlbahdl.hxx"
#endif
#ifndef _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX
#include "NamedBoolPropertyHdl.hxx"
#endif
#ifndef _XMLOFF_XMLTEXTCOLUMNSPPROPERTYHANDLER_HXX
#include "XMLTextColumnsPropertyHandler.hxx"
#endif

#ifndef _XMLOFF_PAGEMASTERPROPHDL_HXX_
#include "PageMasterPropHdl.hxx"
#endif
#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include "PageMasterStyleMap.hxx"
#endif


using namespace ::rtl;

//______________________________________________________________________________

XMLPageMasterPropHdlFactory::XMLPageMasterPropHdlFactory() :
    XMLPropertyHandlerFactory()
{
}

XMLPageMasterPropHdlFactory::~XMLPageMasterPropHdlFactory()
{
}

const XMLPropertyHandler* XMLPageMasterPropHdlFactory::GetPropertyHandler( sal_Int32 nType ) const
{
    nType &= MID_FLAG_MASK;

    XMLPropertyHandler* pHdl = (XMLPropertyHandler*) XMLPropertyHandlerFactory::GetPropertyHandler( nType );
    if( !pHdl )
    {
        switch( nType )
        {
            case XML_PM_TYPE_PAGESTYLELAYOUT:
                pHdl = new XMLPMPropHdl_PageStyleLayout();
            break;
            case XML_PM_TYPE_NUMFORMAT:
                pHdl = new XMLPMPropHdl_NumFormat();
            break;
            case XML_PM_TYPE_NUMLETTERSYNC:
                pHdl = new XMLPMPropHdl_NumLetterSync();
            break;
            case XML_PM_TYPE_PAPERTRAYNUMBER:
                pHdl = new XMLPMPropHdl_PaperTrayNumber();
            break;
            case XML_PM_TYPE_PRINTORIENTATION:
                pHdl = new XMLNamedBoolPropertyHdl(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_landscape ) ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_portrait ) ) );
            break;
            case XML_PM_TYPE_PRINTANNOTATIONS:
                pHdl = new XMLPMPropHdl_Print( sXML_annotations );
            break;
            case XML_PM_TYPE_PRINTCHARTS:
                pHdl = new XMLPMPropHdl_Print( sXML_charts );
            break;
            case XML_PM_TYPE_PRINTDRAWING:
                pHdl = new XMLPMPropHdl_Print( sXML_drawings );
            break;
            case XML_PM_TYPE_PRINTFORMULAS:
                pHdl = new XMLPMPropHdl_Print( sXML_formulas );
            break;
            case XML_PM_TYPE_PRINTGRID:
                pHdl = new XMLPMPropHdl_Print( sXML_grid );
            break;
            case XML_PM_TYPE_PRINTHEADERS:
                pHdl = new XMLPMPropHdl_Print( sXML_headers );
            break;
            case XML_PM_TYPE_PRINTOBJECTS:
                pHdl = new XMLPMPropHdl_Print( sXML_objects );
            break;
            case XML_PM_TYPE_PRINTZEROVALUES:
                pHdl = new XMLPMPropHdl_Print( sXML_zero_values );
            break;
            case XML_PM_TYPE_PRINTPAGEORDER:
                pHdl = new XMLNamedBoolPropertyHdl(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_ttb ) ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_ltr ) ) );
            break;
            case XML_PM_TYPE_FIRSTPAGENUMBER:
                pHdl = new XMLNumberNonePropHdl( sXML_continue, 2 );
            break;
            case XML_TYPE_TEXT_COLUMNS:
                pHdl = new XMLTextColumnsPropertyHandler;
            break;
        }

        if( pHdl )
            PutHdlCache( nType, pHdl );
    }
    return pHdl;
}


