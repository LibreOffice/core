/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xmltabi.hxx,v $
 * $Revision: 1.11.134.2 $
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
#ifndef SC_XMLTABI_HXX
#define SC_XMLTABI_HXX

#include "externalrefmgr.hxx"

#include <xmloff/xmlictxt.hxx>
#include <memory>

class ScXMLImport;

struct ScXMLExternalTabData
{
    String maFileUrl;
    ScExternalRefCache::TableTypeRef mpCacheTable;
    sal_Int32 mnRow;
    sal_Int32 mnCol;
    sal_uInt16 mnFileId;

    ScXMLExternalTabData();
};

class ScXMLTableContext : public SvXMLImportContext
{
    rtl::OUString   sPrintRanges;
    ::std::auto_ptr<ScXMLExternalTabData> pExternalRefInfo;
    sal_Int32       nStartOffset;
    sal_Bool        bStartFormPage;
    sal_Bool        bPrintEntireSheet;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLTableContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        const sal_Bool bTempIsSubTable = sal_False,
                        const sal_Int32 nSpannedCols = 0);

    virtual ~ScXMLTableContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

#endif
