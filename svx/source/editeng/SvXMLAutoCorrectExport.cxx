/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SvXMLAutoCorrectExport.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:33:34 $
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
#include "precompiled_svx.hxx"
#ifndef _SV_XMLAUTOCORRECTEXPORT_HXX
#include <SvXMLAutoCorrectExport.hxx>
#endif
#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::rtl;

// #110680#
SvXMLAutoCorrectExport::SvXMLAutoCorrectExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const SvxAutocorrWordList *  pNewAutocorr_List,
    const rtl::OUString &rFileName,
    com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler)
:   SvXMLExport( xServiceFactory, rFileName, rHandler ),
    pAutocorr_List( pNewAutocorr_List )
{
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_BLOCK_LIST),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
}

sal_uInt32 SvXMLAutoCorrectExport::exportDoc(enum XMLTokenEnum /*eClass*/)
{
    GetDocHandler()->startDocument();

    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    {
        SvXMLElementExport aRoot (*this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK_LIST, sal_True, sal_True);
        sal_uInt16 nBlocks= pAutocorr_List->Count();
        for ( sal_uInt16 i = 0; i < nBlocks; i++)
        {
            SvxAutocorrWord* p = pAutocorr_List->GetObject(i);

            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_ABBREVIATED_NAME,
                          OUString(p->GetShort()));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_NAME,
                          OUString(p->IsTextOnly() ? p->GetLong() : p->GetShort()));

            SvXMLElementExport aBlock( *this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK, sal_True, sal_True);
        }
    }
    GetDocHandler()->endDocument();
    return 0;
}

// #110680#
SvXMLExceptionListExport::SvXMLExceptionListExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const SvStringsISortDtor &rNewList,
    const rtl::OUString &rFileName,
    com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler)
:   SvXMLExport( xServiceFactory, rFileName, rHandler ),
    rList( rNewList )
{
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_BLOCK_LIST ),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
}

sal_uInt32 SvXMLExceptionListExport::exportDoc(enum XMLTokenEnum /*eClass*/)
{
    GetDocHandler()->startDocument();

    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    {
        SvXMLElementExport aRoot (*this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK_LIST, sal_True, sal_True);
        sal_uInt16 nBlocks= rList.Count();
        for ( sal_uInt16 i = 0; i < nBlocks; i++)
        {
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_ABBREVIATED_NAME,
                          OUString( *rList[i] ) );
            SvXMLElementExport aBlock( *this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK, sal_True, sal_True);
        }
    }
    GetDocHandler()->endDocument();
    return 0;
}
