/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXMLBlockExport.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:52:59 $
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
#ifndef _SW_XMLBLOCKEXPORT_HXX
#define _SW_XMLBLOCKEXPORT_HXX

#ifndef _XMLOFF_XMLEXP_HXX
#include <xmloff/xmlexp.hxx>
#endif

class String;
class SwXMLTextBlocks;

class SwXMLBlockListExport : public SvXMLExport
{
private:
    SwXMLTextBlocks &rBlockList;
public:
    // #110680#
    SwXMLBlockListExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        SwXMLTextBlocks & rBlocks,
        const rtl::OUString &rFileName,
        com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler);

    virtual ~SwXMLBlockListExport ( void ) {}
    sal_uInt32 exportDoc( enum ::xmloff::token::XMLTokenEnum eClass);
    void _ExportAutoStyles() {}
    void _ExportMasterStyles () {}
    void _ExportContent() {}
};

class SwXMLTextBlockExport : public SvXMLExport
{
private:
    SwXMLTextBlocks &rBlockList;
public:
    // #110680#
    SwXMLTextBlockExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        SwXMLTextBlocks & rBlocks,
        const rtl::OUString &rFileName,
        com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler);

    virtual ~SwXMLTextBlockExport ( void ) {}
    sal_uInt32 exportDoc(enum ::xmloff::token::XMLTokenEnum /*eClass*/) {return 0;}
    sal_uInt32 exportDoc(const String & rText);
    void _ExportAutoStyles() {}
    void _ExportMasterStyles () {}
    void _ExportContent() {}
};
#endif
