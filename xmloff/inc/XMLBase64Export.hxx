/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLBase64Export.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:44:58 $
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
#ifndef _XMLOFF_XMLBASE64EXPORT_HXX
#define _XMLOFF_XMLBASE64EXPORT_HXX

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream; } } } }
class SvXMLExport;

class XMLBase64Export
{
    SvXMLExport&        rExport;

protected:

    SvXMLExport& GetExport() { return rExport; }

public:

    XMLBase64Export( SvXMLExport& rExport );

    sal_Bool exportXML( const ::com::sun::star::uno::Reference <
            ::com::sun::star::io::XInputStream > & rIn );
    sal_Bool exportElement( const ::com::sun::star::uno::Reference <
            ::com::sun::star::io::XInputStream > & rIn,
            sal_uInt16 nNamespace,
            enum ::xmloff::token::XMLTokenEnum eName );
    sal_Bool exportOfficeBinaryDataElement(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::io::XInputStream > & rIn );
};


#endif
