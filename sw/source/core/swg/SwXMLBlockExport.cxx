/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXMLBlockExport.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:22:50 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifndef _SW_XMLBLOCKEXPORT_HXX
#include <SwXMLBlockExport.hxx>
#endif
#ifndef _SW_XMLTEXTBLOCKS_HXX
#include <SwXMLTextBlocks.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using ::rtl::OUString;

// #110680#
SwXMLBlockListExport::SwXMLBlockListExport(
    const uno::Reference< lang::XMultiServiceFactory > xServiceFactory,
    SwXMLTextBlocks & rBlocks,
    const rtl::OUString &rFileName,
    uno::Reference< xml::sax::XDocumentHandler> &rHandler)
:   SvXMLExport( xServiceFactory, rFileName, rHandler ),
    rBlockList(rBlocks)
{
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_BLOCK_LIST ),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
}

sal_uInt32 SwXMLBlockListExport::exportDoc(enum XMLTokenEnum )
{
    GetDocHandler()->startDocument();

    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    AddAttribute( XML_NAMESPACE_BLOCKLIST,
                  XML_LIST_NAME,
                  OUString (rBlockList.GetName()));
    {
        SvXMLElementExport pRoot (*this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK_LIST, sal_True, sal_True);
        sal_uInt16 nBlocks= rBlockList.GetCount();
        for ( sal_uInt16 i = 0; i < nBlocks; i++)
        {
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_ABBREVIATED_NAME,
                          OUString(rBlockList.GetShortName(i)));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_PACKAGE_NAME,
                          OUString(rBlockList.GetPackageName(i)));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_NAME,
                          OUString(rBlockList.GetLongName(i)));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_UNFORMATTED_TEXT,
                          rBlockList.IsOnlyTextBlock(i) ? XML_TRUE : XML_FALSE );

            SvXMLElementExport aBlock( *this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK, sal_True, sal_True);
        }
    }
    GetDocHandler()->endDocument();
    return 0;
}

// #110680#
SwXMLTextBlockExport::SwXMLTextBlockExport(
    const uno::Reference< lang::XMultiServiceFactory > xServiceFactory,
    SwXMLTextBlocks & rBlocks,
    const rtl::OUString &rFileName,
    uno::Reference< xml::sax::XDocumentHandler> &rHandler)
:   SvXMLExport( xServiceFactory, rFileName, rHandler ),
    rBlockList(rBlocks)
{
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_BLOCK_LIST ),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_OFFICE ),
                            GetXMLToken(XML_N_OFFICE_OOO),
                            XML_NAMESPACE_OFFICE );
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_TEXT ),
                            GetXMLToken(XML_N_TEXT_OOO),
                            XML_NAMESPACE_TEXT );
}

sal_uInt32 SwXMLTextBlockExport::exportDoc(const String &rText)
{
    GetDocHandler()->startDocument();

    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_TEXT ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_TEXT ) );
    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_OFFICE ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_OFFICE ) );
    AddAttribute( XML_NAMESPACE_BLOCKLIST,
                  XML_LIST_NAME,
                  OUString (rBlockList.GetName()));
    {
        SvXMLElementExport aDocument (*this, XML_NAMESPACE_OFFICE, XML_DOCUMENT, sal_True, sal_True);
        {
            SvXMLElementExport aBody (*this, XML_NAMESPACE_OFFICE, XML_BODY, sal_True, sal_True);
            {
                xub_StrLen nPos = 0;
                do
                {
                    String sTemp ( rText.GetToken( 0, '\015', nPos ) );
                     SvXMLElementExport aPara (*this, XML_NAMESPACE_TEXT, XML_P, sal_True, sal_False);
                    GetDocHandler()->characters(sTemp);
                } while (STRING_NOTFOUND != nPos );
            }

        }
    }
    GetDocHandler()->endDocument();
    return 0;
}
