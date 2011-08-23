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
#ifndef _XMLOFF_XMLTEXTCOLUMNSEXPORT_HXX
#define _XMLOFF_XMLTEXTCOLUMNSEXPORT_HXX

namespace com { namespace sun { namespace star { namespace uno {
    class Any; } } } } 
namespace binfilter {
class SvXMLExport;

class XMLTextColumnsExport
{
    SvXMLExport&		rExport;

    const ::rtl::OUString sSeparatorLineIsOn;
    const ::rtl::OUString sSeparatorLineWidth;
    const ::rtl::OUString sSeparatorLineColor;
    const ::rtl::OUString sSeparatorLineRelativeHeight;
    const ::rtl::OUString sSeparatorLineVerticalAlignment;
    const ::rtl::OUString sIsAutomatic;
    const ::rtl::OUString sAutomaticDistance;

protected:
    
    SvXMLExport& GetExport() { return rExport; }

public:

    XMLTextColumnsExport( SvXMLExport& rExport );
    
    void exportXML( const ::com::sun::star::uno::Any& rAny );
};


}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
