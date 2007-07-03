/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTextPContext.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 15:53:40 $
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

#ifndef _SC_XMLTEXTPCONTEXT_HXX
#define _SC_XMLTEXTPCONTEXT_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

class ScXMLImport;
class ScXMLTableRowCellContext;

class ScXMLTextPContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList> xAttrList;
    SvXMLImportContext*         pTextPContext;
    ScXMLTableRowCellContext*   pCellContext;
    rtl::OUString               sLName;
    rtl::OUString               sSimpleContent;     // copy of the first Character call's argument
    rtl::OUStringBuffer*        pContentBuffer;     // used if there's more than one string
    USHORT                      nPrefix;
    sal_Bool                    bIsOwn;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLTextPContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLTableRowCellContext* pCellContext);

    virtual ~ScXMLTextPContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void EndElement();

    void AddSpaces(sal_Int32 nSpaceCount);
};

#endif
