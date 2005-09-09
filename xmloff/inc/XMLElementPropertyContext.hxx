/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLElementPropertyContext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:47:45 $
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

#ifndef _XMLOFF_XMLELEMENTPROPERTYCONTEXT_HXX
#define _XMLOFF_XMLELEMENTPROPERTYCONTEXT_HXX

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif
#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

class SvXMLImport;

namespace rtl
{
    class OUString;
}

class XMLElementPropertyContext : public SvXMLImportContext
{
    sal_Bool        bInsert;

protected:

    ::std::vector< XMLPropertyState > &rProperties;
    XMLPropertyState aProp;

    sal_Bool IsInsert() const { return bInsert; }
    void SetInsert( sal_Bool bIns ) { bInsert = bIns; }

public:
    TYPEINFO();

    XMLElementPropertyContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                               const ::rtl::OUString& rLName,
                               const XMLPropertyState& rProp,
                                ::std::vector< XMLPropertyState > &rProps );

    virtual ~XMLElementPropertyContext();

    virtual void EndElement();
};


#endif  //  _XMLOFF_ELEMENTPROPERTYCONTEXT_HXX

