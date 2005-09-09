/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLFootnoteConfigurationImportContext.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:50:45 $
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

#ifndef _XMLOFF_XMLFOOTNOTECONFIGURATIONIMPORTCONTEXT_HXX
#define _XMLOFF_XMLFOOTNOTECONFIGURATIONIMPORTCONTEXT_HXX

#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif

namespace com { namespace sun { namespace star {
    namespace uno { template<class X> class Reference; }
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }
namespace rtl { class OUString; }
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
                ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

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
    // --> OD 2005-01-31 #i40579# - move code from <CreateAndInsertLate(..)>
    // to <Finish(..)>, because at this time all styles it references have been set.
    virtual void XMLFootnoteConfigurationImportContext::Finish(
        sal_Bool bOverwrite );
    // <--

    /// set configuration at document
    void ProcessSettings(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rConfig);

    /// for helper class: set begin notice
    void SetBeginNotice( ::rtl::OUString sText);

    /// for helper class: set end notice
    void SetEndNotice( ::rtl::OUString sText);
};

#endif
