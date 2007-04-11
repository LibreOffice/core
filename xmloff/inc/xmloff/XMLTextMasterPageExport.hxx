/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTextMasterPageExport.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:25:00 $
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

#ifndef _XMLOFF_XMLTEXTMASTERPAGEEXPORT_HXX
#define _XMLOFF_XMLTEXTMASTERPAGEEXPORT_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include <xmloff/attrlist.hxx>
#endif

#ifndef _XMLOFF_XMLPAGEEXPORT_HXX
#include <xmloff/XMLPageExport.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace text { class XText; }
} } }

class XMLOFF_DLLPUBLIC XMLTextMasterPageExport : public XMLPageExport
{
    const ::rtl::OUString sHeaderText;
    const ::rtl::OUString sHeaderOn;
    const ::rtl::OUString sHeaderShareContent;
    const ::rtl::OUString sHeaderTextLeft;

    const ::rtl::OUString sFooterText;
    const ::rtl::OUString sFooterOn;
    const ::rtl::OUString sFooterShareContent;
    const ::rtl::OUString sFooterTextLeft;

protected:

    virtual void exportHeaderFooterContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XText >& rText,
            sal_Bool bAutoStyles, sal_Bool bExportParagraph = sal_True );

    virtual void exportMasterPageContent(
                const ::com::sun::star::uno::Reference <
                    ::com::sun::star::beans::XPropertySet > & rPropSet,
                 sal_Bool bAutoStyles );

public:
    XMLTextMasterPageExport( SvXMLExport& rExp );
    ~XMLTextMasterPageExport();
};

#endif  //  _XMLOFF_XMLTEXTMASTERPAGEEXPORT_HXX

