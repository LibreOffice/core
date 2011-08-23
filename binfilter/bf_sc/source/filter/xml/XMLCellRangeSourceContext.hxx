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

#ifndef _SC_XMLCELLRANGESOURCECONTEXT_HXX
#define _SC_XMLCELLRANGESOURCECONTEXT_HXX

#ifndef _XMLOFF_XMLIMP_HXX
#include <bf_xmloff/xmlimp.hxx>
#endif
namespace binfilter {

class ScXMLImport;


//___________________________________________________________________

struct ScMyImpCellRangeSource
{
    ::rtl::OUString				sSourceStr;
    ::rtl::OUString				sFilterName;
    ::rtl::OUString				sFilterOptions;
    ::rtl::OUString				sURL;
    sal_Int32					nColumns;
    sal_Int32					nRows;
    sal_Int32					nRefresh;

                                ScMyImpCellRangeSource();
};


//___________________________________________________________________

class ScXMLCellRangeSourceContext : public SvXMLImportContext
{
private:
    const ScXMLImport&			GetScImport() const	{ return (const ScXMLImport&)GetImport(); }
    ScXMLImport&				GetScImport()		{ return (ScXMLImport&)GetImport(); }

public:
                                ScXMLCellRangeSourceContext(
                                    ScXMLImport& rImport,
                                    USHORT nPrfx,
                                    const ::rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
                                    ScMyImpCellRangeSource* pCellRangeSource
                                    );
    virtual						~ScXMLCellRangeSourceContext();

    virtual SvXMLImportContext*	CreateChildContext(
                                    USHORT nPrefix,
                                    const ::rtl::OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual void				EndElement();
};


} //namespace binfilter
#endif

