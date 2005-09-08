/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTableMasterPageExport.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:58:53 $
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

#ifndef _SC_XMLTABLEMASTERPAGEEXPORT_HXX
#define _SC_XMLTABLEMASTERPAGEEXPORT_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTMASTERPAGEEXPORT_HXX
#include <xmloff/XMLTextMasterPageExport.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XHEADERFOOTERCONTENT_HPP_
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#endif

#include "xmlexprt.hxx"

namespace com { namespace sun { namespace star {
    namespace text { class XText; }
} } }

class XMLTableMasterPageExport : public XMLTextMasterPageExport
{
    void exportHeaderFooter(const com::sun::star::uno::Reference < com::sun::star::sheet::XHeaderFooterContent >& xHeaderFooter,
                            const xmloff::token::XMLTokenEnum aName,
                            const sal_Bool bDisplay);

protected:
    virtual void exportHeaderFooterContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XText >& rText,
            sal_Bool bAutoStyles, sal_Bool bProgress );

    virtual void exportMasterPageContent(
                const ::com::sun::star::uno::Reference <
                    ::com::sun::star::beans::XPropertySet > & rPropSet,
                 sal_Bool bAutoStyles );

public:
    XMLTableMasterPageExport( ScXMLExport& rExp );
    ~XMLTableMasterPageExport();
};


#endif  //  _XMLOFF_XMLTABLEMASTERPAGEEXPORT_HXX

