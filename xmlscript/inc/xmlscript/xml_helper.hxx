/*************************************************************************
 *
 *  $RCSfile: xml_helper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2001-02-16 14:14:47 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _XMLSCRIPT_XML_HELPER_HXX_
#define _XMLSCRIPT_XML_HELPER_HXX_

#include <rtl/byteseq.hxx>

#include <com/sun/star/xml/XImporter.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>


namespace xmlscript
{

struct NameSpaceUid
{
    ::rtl::OUString     sURI;
    sal_Int32           nUid;

    NameSpaceUid( ::rtl::OUString const & sURI_, sal_Int32 nUid_ )
        : sURI( sURI_ )
        , nUid( nUid_ )
        {}
};

//==================================================================================================
::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL createDocumentHandler(
    NameSpaceUid const * pNamespaceUids, sal_Int32 nNameSpaceUids,
    sal_Int32 nUnknownNamespaceUid,
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::XImporter > const & xImporter,
    bool bSingleThreadedUse = true )
    throw ();

//==================================================================================================
::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
SAL_CALL createInputStream(
    ::rtl::ByteSequence const & rInData )
    throw ();
//==================================================================================================
::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
SAL_CALL createOutputStream(
    ::rtl::ByteSequence * pOutData )
    throw ();

};

#endif
