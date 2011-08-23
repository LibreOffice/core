/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLFOOTNOTECONFIGURATIONIMPORTCONTEXT_HXX
#define _XMLOFF_XMLFOOTNOTECONFIGURATIONIMPORTCONTEXT_HXX

#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif

namespace com { namespace sun { namespace star {
    namespace uno {	template<class X> class Reference; }
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }
namespace rtl {	class OUString; }
namespace binfilter {
class SvXMLImport;

/// import footnote and endnote configuration elements
class XMLFootnoteConfigurationImportContext : public SvXMLStyleContext
{
    const ::rtl::OUString sPropertyAnchorCharStyleName;
    const ::rtl::OUString sPropertyCharStyleName;
    const ::rtl::OUString sPropertyNumberingType;
    const ::rtl::OUString sPropertyPageStyleName;
    const ::rtl::OUString sPropertyParagraphStyleName;
    const ::rtl::OUString sPropertyPrefix;
    const ::rtl::OUString sPropertyStartAt;
    const ::rtl::OUString sPropertySuffix;
    const ::rtl::OUString sPropertyPositionEndOfDoc;
    const ::rtl::OUString sPropertyFootnoteCounting;
    const ::rtl::OUString sPropertyEndNotice;
    const ::rtl::OUString sPropertyBeginNotice;

    ::rtl::OUString sCitationStyle;
    ::rtl::OUString sAnchorStyle;
    ::rtl::OUString sDefaultStyle;
    ::rtl::OUString sPageStyle;
    ::rtl::OUString sPrefix;
    ::rtl::OUString sSuffix;
    ::rtl::OUString sNumFormat;
    ::rtl::OUString sNumSync;
    ::rtl::OUString sBeginNotice;
    ::rtl::OUString sEndNotice;

    SvXMLTokenMap* pAttrTokenMap;

    sal_Int16 nOffset;
    sal_Int16 nNumbering;
    sal_Bool bPosition;
    sal_Bool bIsEndnote;

public:

    TYPEINFO();

    XMLFootnoteConfigurationImportContext( 
        SvXMLImport& rImport, 
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList> & xAttrList,
        sal_Bool bEndnote);

    virtual ~XMLFootnoteConfigurationImportContext();

    /// parse attributes
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );

    /// for footnotes, also parse begin and end notices
    virtual SvXMLImportContext *CreateChildContext( 
        USHORT nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );

    /// get token map for attributes
    const SvXMLTokenMap& GetFtnConfigAttrTokenMap();

    /// set configuration at document; calls ProcessSettings
    /// Uses CreateAndInsertLate() to ensure that all styles it references 
    /// have been set.
    virtual void CreateAndInsertLate( sal_Bool bOverwrite );

    /// set configuration at document
    void ProcessSettings(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rConfig);

    /// for helper class: set begin notice
    void SetBeginNotice( ::rtl::OUString sText);

    /// for helper class: set end notice
    void SetEndNotice( ::rtl::OUString sText);
};

}//end of namespace binfilter
#endif
