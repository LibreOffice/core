/*************************************************************************
 *
 *  $RCSfile: xmltabe.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:02 $
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

#ifndef _XMLOFF_XMLTABE_HXX
#define _XMLOFF_XMLTABE_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _VCL_MAPUNIT_HXX
#include <vcl/mapunit.hxx>
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif

class SvXMLNamespaceMap;
class SvXMLUnitConverter;

#ifndef _COM_SUN_STAR_STYLE_TABSTOP_HPP_
#include <com/sun/star/style/TabStop.hpp>
#endif

class SvxXMLTabStopExport
{
    const ::rtl::OUString msCDATA;
    const ::rtl::OUString msWS;

    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >            mxHandler;      // the handlers

    SvXMLAttributeList          *mpAttrList;        // a common attribute list
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >          mxAttrList;     // and an interface of it

    const SvXMLNamespaceMap     *mpNamespaceMap;    // the namepspace map
    const SvXMLUnitConverter&   mrUnitConv;

protected:

    // Check if common attribute list is empty.
#ifdef PRODUCT
    void CheckAttrList() {}
#else
    void CheckAttrList();
#endif
    void ClearAttrList();
    void AddAttribute( sal_uInt16 nPrefixKey, const char *pName,
                       const ::rtl::OUString& rValue );
    ::rtl::OUString GetQNameByKey( sal_uInt16 nKey,
                            const ::rtl::OUString& rLocalName ) const;

    void exportTabStop( const ::com::sun::star::style::TabStop* pTabStop );

public:

    SvxXMLTabStopExport(  const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
                          const SvXMLUnitConverter& rUnitConverter );
    virtual ~SvxXMLTabStopExport();

    // core API
    void Export( const ::com::sun::star::uno::Any& rAny, const SvXMLNamespaceMap& rNamespMap );
};


#endif  //  _XMLOFF_XMLTABE_HXX

