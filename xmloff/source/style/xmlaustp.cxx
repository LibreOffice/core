/*************************************************************************
 *
 *  $RCSfile: xmlaustp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2000-09-22 13:41:50 $
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


#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _XMLOFF_XMLASTPL_IMPL_HXX
#include "impastpl.hxx"
#endif
#ifndef _XMLOFF_XMLASTPLP_HXX
#include "xmlaustp.hxx"
#endif

using namespace ::std;
using namespace ::rtl;
using namespace ::com::sun::star;

void SvXMLAutoStylePoolP::exportStyleAttributes(
        SvXMLAttributeList& rAttrList,
        sal_Int32 nFamily,
        const vector< XMLPropertyState >& rProperties,
        const SvXMLExportPropertyMapper& rPropExp,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
}

void SvXMLAutoStylePoolP::exportStyleContent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
        sal_Int32 nFamily,
        const vector< XMLPropertyState >& rProperties,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
}

SvXMLAutoStylePoolP::SvXMLAutoStylePoolP()
{
    pImpl = new SvXMLAutoStylePoolP_Impl;
}

SvXMLAutoStylePoolP::~SvXMLAutoStylePoolP()
{
    delete pImpl;
}

// TODO: romove this
void SvXMLAutoStylePoolP::AddFamily(
        sal_Int32 nFamily,
        const OUString& rStrName,
        XMLPropertySetMapper* pMapper,
        OUString aStrPrefix )
{
    UniReference <XMLPropertySetMapper> xTmp = pMapper;
    AddFamily( nFamily, rStrName, xTmp, aStrPrefix );
}

void SvXMLAutoStylePoolP::AddFamily(
        sal_Int32 nFamily,
        const OUString& rStrName,
        const UniReference < XMLPropertySetMapper > & rMapper,
        const OUString& rStrPrefix )
{
    pImpl->AddFamily( nFamily, rStrName, rMapper, rStrPrefix );
}

void SvXMLAutoStylePoolP::RegisterName( sal_Int32 nFamily,
                                         const OUString& rName )
{
    pImpl->RegisterName( nFamily, rName );
}

OUString SvXMLAutoStylePoolP::Add( sal_Int32 nFamily,
                                  const vector< XMLPropertyState >& rProperties )
{
    OUString sEmpty;
    return pImpl->Add( nFamily, sEmpty, rProperties );
}

OUString SvXMLAutoStylePoolP::Add( sal_Int32 nFamily,
                                  const OUString& rParent,
                                  const vector< XMLPropertyState >& rProperties )
{
    return pImpl->Add( nFamily, rParent, rProperties );
}

OUString SvXMLAutoStylePoolP::Find( sal_Int32 nFamily,
                                   const vector< XMLPropertyState >& rProperties ) const
{
    OUString sEmpty;
    return pImpl->Find( nFamily, sEmpty, rProperties );
}

OUString SvXMLAutoStylePoolP::Find( sal_Int32 nFamily,
                                   const OUString& rParent,
                                   const vector< XMLPropertyState >& rProperties ) const
{
    return pImpl->Find( nFamily, rParent, rProperties );
}

void SvXMLAutoStylePoolP::exportXML( sal_Int32 nFamily,
    const SvXMLExportPropertyMapper& rPropExp,
    const uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
    const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap) const
{
    pImpl->exportXML( nFamily, rPropExp, rHandler, rUnitConverter,
                      rNamespaceMap, this);
}

void SvXMLAutoStylePoolP::ClearEntries()
{
    pImpl->ClearEntries();
}
