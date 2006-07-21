/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTableSourceContext.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:50:07 $
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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#ifndef _SC_XMLTABLESOURCECONTEXT_HXX
#include "XMLTableSourceContext.hxx"
#endif
#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_XMLSUBTI_HXX
#include "xmlsubti.hxx"
#endif
#ifndef SC_TABLINK_HXX
#include "tablink.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSHEETLINKABLE_HPP_
#include <com/sun/star/sheet/XSheetLinkable.hpp>
#endif

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLTableSourceContext::ScXMLTableSourceContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sLink(),
    sTableName(),
    sFilterName(),
    sFilterOptions(),
    nRefresh(0),
    nMode(sheet::SheetLinkMode_NORMAL)
{
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));
        if(nPrefix == XML_NAMESPACE_XLINK)
        {
            if (IsXMLToken(aLocalName, XML_HREF))
                sLink = GetScImport().GetAbsoluteReference(sValue);
        }
        else if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_TABLE_NAME))
                sTableName = sValue;
            else if (IsXMLToken(aLocalName, XML_FILTER_NAME))
                sFilterName = sValue;
            else if (IsXMLToken(aLocalName, XML_FILTER_OPTIONS))
                sFilterOptions = sValue;
            else if (IsXMLToken(aLocalName, XML_MODE))
            {
                if (IsXMLToken(sValue, XML_COPY_RESULTS_ONLY))
                    nMode = sheet::SheetLinkMode_VALUE;
            }
            else if (IsXMLToken(aLocalName, XML_REFRESH_DELAY))
            {
                double fTime;
                if( SvXMLUnitConverter::convertTime( fTime, sValue ) )
                    nRefresh = Max( (sal_Int32)(fTime * 86400.0), (sal_Int32)0 );
            }
        }
    }
}

ScXMLTableSourceContext::~ScXMLTableSourceContext()
{
}

SvXMLImportContext *ScXMLTableSourceContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLTableSourceContext::EndElement()
{
    if (sLink.getLength())
    {
        uno::Reference <sheet::XSheetLinkable> xLinkable (GetScImport().GetTables().GetCurrentXSheet(), uno::UNO_QUERY);
        ScDocument* pDoc(GetScImport().GetDocument());
        if (xLinkable.is() && pDoc)
        {
            GetScImport().LockSolarMutex();
            if (pDoc->RenameTab( static_cast<SCTAB>(GetScImport().GetTables().GetCurrentSheet()),
                GetScImport().GetTables().GetCurrentSheetName(), sal_False, sal_True))
            {
                 String aFileString(sLink);
                String aFilterString(sFilterName);
                String aOptString(sFilterOptions);
                String aSheetString(sTableName);

                aFileString = ScGlobal::GetAbsDocName( aFileString, pDoc->GetDocumentShell() );
                if ( !aFilterString.Len() )
                    ScDocumentLoader::GetFilterName( aFileString, aFilterString, aOptString, FALSE );

                BYTE nLinkMode = SC_LINK_NONE;
                if ( nMode == sheet::SheetLinkMode_NORMAL )
                    nLinkMode = SC_LINK_NORMAL;
                else if ( nMode == sheet::SheetLinkMode_VALUE )
                    nLinkMode = SC_LINK_VALUE;

                pDoc->SetLink( static_cast<SCTAB>(GetScImport().GetTables().GetCurrentSheet()),
                    nLinkMode, aFileString, aFilterString, aOptString,
                    aSheetString, nRefresh );
            }
            GetScImport().UnlockSolarMutex();
        }
    }
}

