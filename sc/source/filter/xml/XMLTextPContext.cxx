/*************************************************************************
 *
 *  $RCSfile: XMLTextPContext.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: sab $ $Date: 2001-10-18 08:52:34 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifndef _SC_XMLTEXTPCONTEXT_HXX
#include "XMLTextPContext.hxx"
#endif
#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif
#ifndef SC_XMLCELLI_HXX
#include "xmlcelli.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif

using namespace com::sun::star;
using namespace xmloff::token;

class ScXMLTextTContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLTextTContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLTextPContext* pTextPContext);

    virtual ~ScXMLTextTContext();
};


ScXMLTextTContext::ScXMLTextTContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLTextPContext* pTextPContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    if (pTextPContext)
    {
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        rtl::OUString aLocalName;
        rtl::OUString sValue;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                xAttrList->getNameByIndex( i ), &aLocalName );
            sValue = xAttrList->getValueByIndex( i );

            if ((nPrefix == XML_NAMESPACE_TEXT) && IsXMLToken(aLocalName, XML_C))
                pTextPContext->AddSpaces(sValue.toInt32());
        }
    }
}

ScXMLTextTContext::~ScXMLTextTContext()
{
}

//------------------------------------------------------------------

ScXMLTextPContext::ScXMLTextPContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xTempAttrList,
                                      ScXMLTableRowCellContext* pTempCellContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pTextPContext(NULL),
    pCellContext(pTempCellContext),
    sOUText(),
    sLName(rLName),
    xAttrList(xTempAttrList),
    nPrefix(nPrfx),
    bIsOwn(sal_True)
{
    // here are no attributes
}

ScXMLTextPContext::~ScXMLTextPContext()
{
    if (pTextPContext)
        delete pTextPContext;
}

void ScXMLTextPContext::AddSpaces(sal_Int32 nSpaceCount)
{
    sal_Char* pChars = new sal_Char[nSpaceCount];
    memset(pChars, ' ', nSpaceCount);
    sOUText.appendAscii(pChars, nSpaceCount);
}

SvXMLImportContext *ScXMLTextPContext::CreateChildContext( USHORT nTempPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xTempAttrList )
{
    SvXMLImportContext *pContext = NULL;
    if (!pTextPContext &&
        (nTempPrefix == XML_NAMESPACE_TEXT) &&
        IsXMLToken(rLName, XML_S))
        pContext = new ScXMLTextTContext( GetScImport(), nTempPrefix, rLName, xTempAttrList, this);
    else
    {
        if (!pTextPContext)
        {
            pCellContext->SetCursorOnTextImport(sOUText.makeStringAndClear());
            pTextPContext = GetScImport().GetTextImport()->CreateTextChildContext(
                                    GetScImport(), nPrefix, sLName, xAttrList);
        }
        if (pTextPContext)
            pContext = pTextPContext->CreateChildContext(nTempPrefix, rLName, xTempAttrList);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetScImport(), nTempPrefix, rLName );

    return pContext;
}

void ScXMLTextPContext::Characters( const ::rtl::OUString& rChars )
{
    if (!pTextPContext)
        sOUText.append(rChars);
    else
        pTextPContext->Characters(rChars);
}

void ScXMLTextPContext::EndElement()
{
    if (!pTextPContext)
        pCellContext->SetString(sOUText.makeStringAndClear());
//      GetScImport().GetTextImport()->GetCursor()->setString(sOUText.makeStringAndClear());
    else
    {
        pTextPContext->EndElement();
        GetScImport().SetRemoveLastChar(sal_True);
    }
}

