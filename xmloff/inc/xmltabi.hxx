/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmltabi.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:20:25 $
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

#ifndef _XMLOFF_XMLTABI_HXX
#define _XMLOFF_XMLTABI_HXX

#ifndef _XMLOFF_XMLELEMENTPROPERTYCONTEXT_HXX
#include "XMLElementPropertyContext.hxx"
#endif

class SvxXMLTabStopArray_Impl;
class SvXMLImport;

namespace rtl
{
    class OUString;
}

class SvxXMLTabStopImportContext : public XMLElementPropertyContext
{
private:
    SvxXMLTabStopArray_Impl*    mpTabStops;

public:
    TYPEINFO();

    SvxXMLTabStopImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                const ::rtl::OUString& rLName,
                                const XMLPropertyState& rProp,
                                 ::std::vector< XMLPropertyState > &rProps );

    virtual ~SvxXMLTabStopImportContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();
};


#endif  //  _XMLOFF_XMLTABI_HXX

