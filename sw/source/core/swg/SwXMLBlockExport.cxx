/*************************************************************************
 *
 *  $RCSfile: SwXMLBlockExport.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mtg $ $Date: 2001-07-05 14:16:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/
#ifndef _SW_XMLBLOCKEXPORT_HXX
#include <SwXMLBlockExport.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;
using ::xmloff::token::XMLTokenEnum;

sal_Char __READONLY_DATA sXML_np__block_list[] = "_block-list";

SwXMLBlockListExport::SwXMLBlockListExport( SwXMLTextBlocks & rBlocks, const rtl::OUString &rFileName,
          com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler)
: rBlockList(rBlocks),
  SvXMLExport(rFileName, rHandler)
{
    _GetNamespaceMap().AddAtIndex( XML_NAMESPACE_BLOCKLIST_IDX, sXML_np_block_list,
                                  sXML_n_block_list, XML_NAMESPACE_BLOCKLIST );
}

sal_uInt32 SwXMLBlockListExport::exportDoc(enum XMLTokenEnum eClass)
{
    GetDocHandler()->startDocument();

    sal_uInt16 nPos = _GetNamespaceMap().GetIndexByKey( XML_NAMESPACE_BLOCKLIST );
    AddAttribute ( XML_NAMESPACE_NONE, _GetNamespaceMap().GetAttrNameByIndex ( nPos ),
                              _GetNamespaceMap().GetNameByIndex ( nPos ) );
    AddAttribute( XML_NAMESPACE_BLOCKLIST, sXML_list_name,
                       OUString(rBlockList.GetName()));
    {
        SvXMLElementExport pRoot (*this, XML_NAMESPACE_BLOCKLIST, sXML_block_list, sal_True, sal_True);
        sal_uInt16 nBlocks= rBlockList.GetCount();
        for ( sal_uInt16 i = 0; i < nBlocks; i++)
        {
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          sXML_abbreviated_name,
                          OUString(rBlockList.GetShortName(i)));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          sXML_package_name,
                          OUString(rBlockList.GetPackageName(i)));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          sXML_name,
                          OUString(rBlockList.GetLongName(i)));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          sXML_unformatted_text,
                          rBlockList.IsOnlyTextBlock(i) ?
                              OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_true ) ) :
                            OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_false ) ) );

            SvXMLElementExport aBlock( *this, XML_NAMESPACE_BLOCKLIST, sXML_block, sal_True, sal_True);
        }
    }
    GetDocHandler()->endDocument();
    return 0;
}

SwXMLTextBlockExport::SwXMLTextBlockExport( SwXMLTextBlocks & rBlocks, const rtl::OUString &rFileName,
          com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler)
: rBlockList(rBlocks),
  SvXMLExport(rFileName, rHandler)
{
    _GetNamespaceMap().AddAtIndex( XML_NAMESPACE_BLOCKLIST_IDX, sXML_np_block_list,
                                  sXML_n_block_list, XML_NAMESPACE_BLOCKLIST );
}

sal_uInt32 SwXMLTextBlockExport::exportDoc(const String &rText)
{
    GetDocHandler()->startDocument();

    sal_uInt16 nPos = _GetNamespaceMap().GetIndexByKey( XML_NAMESPACE_OFFICE );
    AddAttribute ( XML_NAMESPACE_NONE, _GetNamespaceMap().GetAttrNameByIndex ( nPos ),
                              _GetNamespaceMap().GetNameByIndex ( nPos ) );
    nPos = _GetNamespaceMap().GetIndexByKey( XML_NAMESPACE_TEXT );
    AddAttribute ( XML_NAMESPACE_NONE, _GetNamespaceMap().GetAttrNameByIndex ( nPos ),
                              _GetNamespaceMap().GetNameByIndex ( nPos ) );
    {
        SvXMLElementExport aDocument (*this, XML_NAMESPACE_OFFICE, sXML_document, sal_True, sal_True);
        {
            SvXMLElementExport aBody (*this, XML_NAMESPACE_OFFICE, sXML_body, sal_True, sal_True);
            {
                xub_StrLen nPos = 0;
                do
                {
                    String sTemp ( rText.GetToken( 0, '\015', nPos ) );
                     SvXMLElementExport aPara (*this, XML_NAMESPACE_TEXT, sXML_p, sal_True, sal_False);
                    GetDocHandler()->characters(sTemp);
                } while (STRING_NOTFOUND != nPos );
            }

        }
    }
    GetDocHandler()->endDocument();
    return 0;
}
