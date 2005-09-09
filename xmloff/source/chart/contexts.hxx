/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contexts.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:30:43 $
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
#ifndef _SCH_XML_CONTEXTS_HXX_
#define _SCH_XML_CONTEXTS_HXX_

#include "SchXMLImport.hxx"
#include "SchXMLTableContext.hxx"

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif
#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

namespace com { namespace sun { namespace star { namespace xml { namespace sax {
        class XAttributeList;
}}}}}

/* ========================================

   These contexts are only nedded by
   SchXMLImport not by the SchXMLImportHelper
   that is also used by other applications

   ======================================== */

class SchXMLDocContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;

public:
    SchXMLDocContext(
        SchXMLImportHelper& rImpHelper,
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const rtl::OUString& rLName );
    virtual ~SchXMLDocContext();

    TYPEINFO();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ========================================

class SchXMLBodyContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;

public:
    SchXMLBodyContext(
        SchXMLImportHelper& rImpHelper,
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const rtl::OUString& rLName );
    virtual ~SchXMLBodyContext();

    virtual void EndElement();
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ========================================

#endif  // _SCH_XML_CONTEXTS_HXX_
