/*************************************************************************
 *
 *  $RCSfile: SvXMLAutoCorrectImport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mtg $ $Date: 2001-02-16 09:56:46 $
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
#ifndef _SV_XMLAUTOCORRECTIMPORT_HXX
#include <SvXMLAutoCorrectImport.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;

sal_Char __READONLY_DATA sXML_np__block_list[] = "_block-list";

SvXMLAutoCorrectImport::SvXMLAutoCorrectImport ( SvxAutocorrWordList *pNewAutocorr_List, SvxAutoCorrect &rNewAutoCorrect, SvStorageRef &rNewStorage)
: pAutocorr_List (pNewAutocorr_List), rAutoCorrect ( rNewAutoCorrect ), rStorage ( rNewStorage )
{
    GetNamespaceMap().AddAtIndex( XML_NAMESPACE_BLOCKLIST_IDX, sXML_np__block_list,
                                     sXML_n_block_list, XML_NAMESPACE_BLOCKLIST );
}

SvXMLAutoCorrectImport::~SvXMLAutoCorrectImport ( void )
{
}

SvXMLImportContext *SvXMLAutoCorrectImport::CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_BLOCKLIST==nPrefix &&
        rLocalName.compareToAscii( sXML_block_list ) == 0 )
        pContext = new SvXMLWordListContext( *this, nPrefix, rLocalName, xAttrList );
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
    return pContext;
}

SvXMLWordListContext::SvXMLWordListContext(
   SvXMLAutoCorrectImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
   rLocalRef(rImport),
   SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
}

SvXMLImportContext *SvXMLWordListContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_BLOCKLIST &&
        rLocalName.compareToAscii( sXML_block ) == 0)
        pContext = new SvXMLWordContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    else
        pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
    return pContext;
}
SvXMLWordListContext::~SvXMLWordListContext ( void )
{
}

SvXMLWordContext::SvXMLWordContext(
   SvXMLAutoCorrectImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
   rLocalRef(rImport),
   SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
    String sRight, sWrong;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    for (sal_Int16 i=0; i < nAttrCount; i++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
        const OUString& rAttrValue = xAttrList->getValueByIndex( i );
        if (XML_NAMESPACE_BLOCKLIST == nPrefix)
        {
            if (aLocalName.compareToAscii( sXML_abbreviated_name ) == 0)
            {
                sWrong = rAttrValue;
            }
            else if (aLocalName.compareToAscii( sXML_name ) == 0)
            {
                sRight = rAttrValue;
            }
        }
    }
    if (!sWrong.Len() || !sRight.Len() )
        return;
    /*
     * GetLongText always returns false anyway ? *confusion* - mtg*/
    const International& rInter = Application::GetAppInternational();
    BOOL bOnlyTxt = COMPARE_EQUAL != rInter.Compare( sRight, sWrong, INTN_COMPARE_IGNORECASE );
    if( !bOnlyTxt )
    {
        String sLongSave( sRight );
        if( !rLocalRef.rAutoCorrect.GetLongText( *(rLocalRef.rStorage), sWrong, sRight ) &&
            sLongSave.Len() )
        {
            sRight = sLongSave;
            bOnlyTxt = TRUE;
        }
    }
    /*BOOL bOnlyTxt = TRUE;*/
    SvxAutocorrWordPtr pNew = new SvxAutocorrWord( sWrong, sRight, bOnlyTxt );

    if( !rLocalRef.pAutocorr_List->Insert( pNew ) )
        delete pNew;
}

SvXMLWordContext::~SvXMLWordContext ( void )
{
}
SvXMLExceptionListImport::SvXMLExceptionListImport ( SvStringsISortDtor & rNewList )
: rList (rNewList)
{
    GetNamespaceMap().AddAtIndex( XML_NAMESPACE_BLOCKLIST_IDX, sXML_np__block_list,
                                     sXML_n_block_list, XML_NAMESPACE_BLOCKLIST );
}

SvXMLExceptionListImport::~SvXMLExceptionListImport ( void )
{
}

SvXMLImportContext *SvXMLExceptionListImport::CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_BLOCKLIST==nPrefix &&
        rLocalName.compareToAscii( sXML_block_list ) == 0 )
        pContext = new SvXMLExceptionListContext( *this, nPrefix, rLocalName, xAttrList );
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
    return pContext;
}

SvXMLExceptionListContext::SvXMLExceptionListContext(
   SvXMLExceptionListImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
   rLocalRef(rImport),
   SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
}

SvXMLImportContext *SvXMLExceptionListContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_BLOCKLIST &&
        rLocalName.compareToAscii( sXML_block ) == 0)
        pContext = new SvXMLExceptionContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    else
        pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
    return pContext;
}
SvXMLExceptionListContext::~SvXMLExceptionListContext ( void )
{
}

SvXMLExceptionContext::SvXMLExceptionContext(
   SvXMLExceptionListImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
   rLocalRef(rImport),
   SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
    String sWord;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    for (sal_Int16 i=0; i < nAttrCount; i++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
        const OUString& rAttrValue = xAttrList->getValueByIndex( i );
        if (XML_NAMESPACE_BLOCKLIST == nPrefix)
        {
            if (aLocalName.compareToAscii( sXML_abbreviated_name ) == 0)
            {
                sWord = rAttrValue;
            }
        }
    }
    if (!sWord.Len() )
        return;

    String * pNew = new String( sWord );

    if( !rLocalRef.rList.Insert( pNew ) )
        delete pNew;
}

SvXMLExceptionContext::~SvXMLExceptionContext ( void )
{
}
