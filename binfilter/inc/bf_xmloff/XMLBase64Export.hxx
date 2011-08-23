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
#ifndef _XMLOFF_XMLBASE64EXPORT_HXX
#define _XMLOFF_XMLBASE64EXPORT_HXX

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream; } } } } 
namespace binfilter {
class SvXMLExport;

class XMLBase64Export
{
    SvXMLExport&		rExport;

protected:

    SvXMLExport& GetExport() { return rExport; }

public:

    XMLBase64Export( SvXMLExport& rExport );

    sal_Bool exportXML( const ::com::sun::star::uno::Reference <
            ::com::sun::star::io::XInputStream > & rIn );
    sal_Bool exportElement( const ::com::sun::star::uno::Reference <
            ::com::sun::star::io::XInputStream > & rIn,
            sal_uInt16 nNamespace,
            enum ::binfilter::xmloff::token::XMLTokenEnum eName );
    sal_Bool exportOfficeBinaryDataElement(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::io::XInputStream > & rIn );
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
