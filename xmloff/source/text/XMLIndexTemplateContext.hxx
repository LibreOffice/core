/*************************************************************************
 *
 *  $RCSfile: XMLIndexTemplateContext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dvo $ $Date: 2000-11-14 14:42:50 $
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

#ifndef _XMLOFF_XMLINDEXTEMPLATECONTEXT_HXX_
#define _XMLOFF_XMLINDEXTEMPLATECONTEXT_HXX_

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef __SGI_STL_VECTOR
#include <stl/vector>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }
namespace rtl { class OUString; }
struct SvXMLEnumMapEntry;


// constants for the XMLIndexTemplateContext constructor

// TOC and user defined index:
extern const SvXMLEnumMapEntry aLevelNameTOCMap[];
extern const sal_Char* aLevelStylePropNameTOCMap[];
extern const sal_Bool aAllowedTokenTypesTOC[];
extern const sal_Bool aAllowedTokenTypesUser[];

// alphabetical index:
extern const SvXMLEnumMapEntry aLevelNameAlphaMap[];
extern const sal_Char* aLevelStylePropNameAlphaMap[];
extern const sal_Bool aAllowedTokenTypesAlpha[];

// bibliography:
extern const SvXMLEnumMapEntry aLevelNameBibliographyMap[];
extern const sal_Char* aLevelStylePropNameBibliographyMap[];
extern const sal_Bool aAllowedTokenTypesBibliography[];

// table, illustration and object tables:
extern const SvXMLEnumMapEntry* aLevelNameTableMap; // NULL: no outline-level
extern const sal_Char* aLevelStylePropNameTableMap[];
extern const sal_Bool aAllowedTokenTypesTable[];


/**
 * Import index entry templates
 */
class XMLIndexTemplateContext : public SvXMLImportContext
{
    // pick up PropertyValues to be turned into a sequence.
    ::std::vector< ::com::sun::star::beans::PropertyValues > aValueVector;

    ::rtl::OUString sStyleName;

    const SvXMLEnumMapEntry* pOutlineLevelNameMap;
    const sal_Char* pOutlineLevelAttrName;
    const sal_Char** pOutlineLevelStylePropMap;
    const sal_Bool* pAllowedTokenTypesMap;

    sal_Int32 nOutlineLevel;
    sal_Bool bStyleNameOK;
    sal_Bool bOutlineLevelOK;

    // PropertySet of current index
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & rPropertySet;

public:

    // constants made available to other contexts (template entry
    // contexts, in particular)
    const ::rtl::OUString sTokenEntryNumber;
    const ::rtl::OUString sTokenEntryText;
    const ::rtl::OUString sTokenTabStop;
    const ::rtl::OUString sTokenText;
    const ::rtl::OUString sTokenPageNumber;
    const ::rtl::OUString sTokenChapterInfo;
    const ::rtl::OUString sTokenHyperlinkStart;
    const ::rtl::OUString sTokenHyperlinkEnd;
    const ::rtl::OUString sTokenBibliographyDataField;

    const ::rtl::OUString sCharacterStyleName;
    const ::rtl::OUString sTokenType;
    const ::rtl::OUString sText;
    const ::rtl::OUString sTabStopRightAligned;
    const ::rtl::OUString sTabStopPosition;
    const ::rtl::OUString sTabStopFillCharacter;
    const ::rtl::OUString sBibliographyDataField;
    const ::rtl::OUString sChapterFormat;

    const ::rtl::OUString sLevelFormat;
    const ::rtl::OUString sParaStyleLevel;


    TYPEINFO();

    XMLIndexTemplateContext(
        SvXMLImport& rImport,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName,
        const SvXMLEnumMapEntry* aLevelNameMap,
        const sal_Char* pLevelAttrName,
        const sal_Char** aLevelStylePropNameMap,
        const sal_Bool* aAllowedTokenTypes);

    ~XMLIndexTemplateContext();

    /** add template; to be called by child template entry contexts */
    void addTemplateEntry(
        const ::com::sun::star::beans::PropertyValues& aValues);

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual void EndElement();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

#endif
