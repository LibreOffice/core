/*************************************************************************
 *
 *  $RCSfile: XMLTrackedChangesImportContext.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:39:27 $
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

#ifndef _XMLOFF_XMLTRACKEDCHANGESIMPORTCONTEXT_HXX
#include "XMLTrackedChangesImportContext.hxx"
#endif

#ifndef _XMLOFF_XMLCHANGEDREGIONIMPORTCONTEXT_HXX
#include "XMLChangedRegionImportContext.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif


using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::xml::sax::XAttributeList;
using namespace ::xmloff::token;



TYPEINIT1( XMLTrackedChangesImportContext, SvXMLImportContext );

XMLTrackedChangesImportContext::XMLTrackedChangesImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName) :
        SvXMLImportContext(rImport, nPrefix, rLocalName)
{
}

XMLTrackedChangesImportContext::~XMLTrackedChangesImportContext()
{
}

void XMLTrackedChangesImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList )
{
    sal_Bool bTrackChanges = sal_True;

    // scan for text:track-changes and text:protection-key attributes
    sal_Int16 nLength = xAttrList->getLength();
    for( sal_Int16 i = 0; i < nLength; i++ )
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

        if ( XML_NAMESPACE_TEXT == nPrefix )
        {
            if ( IsXMLToken( sLocalName, XML_TRACK_CHANGES ) )
            {
                sal_Bool bTmp;
                if( SvXMLUnitConverter::convertBool(
                    bTmp, xAttrList->getValueByIndex(i)) )
                {
                    bTrackChanges = bTmp;
                }
            }
        }
    }

    // set tracked changes
    GetImport().GetTextImport()->SetRecordChanges( bTrackChanges );
}


SvXMLImportContext* XMLTrackedChangesImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLImportContext* pContext = NULL;

    if ( (XML_NAMESPACE_TEXT == nPrefix) &&
         IsXMLToken( rLocalName, XML_CHANGED_REGION ) )
    {
        pContext = new XMLChangedRegionImportContext(GetImport(),
                                                     nPrefix, rLocalName);
    }

    if (NULL == pContext)
    {
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);
    }

    return pContext;
}
