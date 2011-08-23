/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SC_XMLSTYLE_HXX
#define _SC_XMLSTYLE_HXX

#include <bf_xmloff/maptype.hxx>
#include <bf_xmloff/xmlaustp.hxx>
#include <bf_xmloff/xmltypes.hxx>
#include <bf_xmloff/xmlprmap.hxx>
#include <bf_xmloff/prhdlfac.hxx>
#include <bf_xmloff/styleexp.hxx>
#include <bf_xmloff/xmlexppr.hxx>
#include <bf_xmloff/contextid.hxx>
namespace binfilter {

extern const XMLPropertyMapEntry aXMLScCellStylesProperties[];
extern const XMLPropertyMapEntry aXMLScColumnStylesProperties[];
extern const XMLPropertyMapEntry aXMLScRowStylesProperties[];
extern const XMLPropertyMapEntry aXMLScTableStylesProperties[];

//CellStyles
#define	XML_SC_TYPE_CELLPROTECTION					(XML_SC_TYPES_START +  1)
#define	XML_SC_TYPE_PRINTCONTENT					(XML_SC_TYPES_START +  2)
#define XML_SC_TYPE_HORIJUSTIFY						(XML_SC_TYPES_START +  3)
#define XML_SC_TYPE_HORIJUSTIFYSOURCE				(XML_SC_TYPES_START +  4)
#define XML_SC_TYPE_ORIENTATION						(XML_SC_TYPES_START +  6)
#define XML_SC_TYPE_ROTATEANGLE						(XML_SC_TYPES_START +  7)
#define XML_SC_TYPE_ROTATEREFERENCE					(XML_SC_TYPES_START +  8)
#define XML_SC_TYPE_BORDERLEFT						(XML_SC_TYPES_START +  9)
#define XML_SC_TYPE_BORDERRIGHT						(XML_SC_TYPES_START + 10)
#define XML_SC_TYPE_BORDERTOP						(XML_SC_TYPES_START + 11)
#define XML_SC_TYPE_BORDERBOTTOM					(XML_SC_TYPES_START + 12)
#define XML_SC_TYPE_VERTJUSTIFY						(XML_SC_TYPES_START + 13)
#define XML_SC_ISTEXTWRAPPED						(XML_SC_TYPES_START + 14)
#define XML_SC_TYPE_EQUAL							(XML_SC_TYPES_START + 15)
#define XML_SC_TYPE_VERTICAL						(XML_SC_TYPES_START + 16)

#define CTF_SC_HORIJUSTIFY							(XML_SC_CTF_START +  1)
#define CTF_SC_HORIJUSTIFY_SOURCE					(XML_SC_CTF_START +  2)
#define CTF_SC_ALLPADDING							(XML_SC_CTF_START +  3)
#define CTF_SC_BOTTOMPADDING						(XML_SC_CTF_START +  4)
#define CTF_SC_LEFTPADDING							(XML_SC_CTF_START +  5)
#define CTF_SC_RIGHTPADDING							(XML_SC_CTF_START +  6)
#define CTF_SC_TOPPADDING							(XML_SC_CTF_START +  7)
#define CTF_SC_ALLBORDER							(XML_SC_CTF_START +  8)
#define CTF_SC_LEFTBORDER							(XML_SC_CTF_START +  9)
#define CTF_SC_RIGHTBORDER							(XML_SC_CTF_START + 10)
#define CTF_SC_TOPBORDER							(XML_SC_CTF_START + 11)
#define CTF_SC_BOTTOMBORDER							(XML_SC_CTF_START + 12)
#define CTF_SC_ALLBORDERWIDTH						(XML_SC_CTF_START + 13)
#define CTF_SC_LEFTBORDERWIDTH						(XML_SC_CTF_START + 14)
#define CTF_SC_RIGHTBORDERWIDTH						(XML_SC_CTF_START + 15)
#define CTF_SC_TOPBORDERWIDTH						(XML_SC_CTF_START + 16)
#define CTF_SC_BOTTOMBORDERWIDTH					(XML_SC_CTF_START + 17)
#define CTF_SC_NUMBERFORMAT							(XML_SC_CTF_START + 18)
#define CTF_SC_MAP									(XML_SC_CTF_START + 19)
#define CTF_SC_PARAINDENT							(XML_SC_CTF_START + 20)
#define CTF_SC_OLDTEXTBACKGROUND					(XML_SC_CTF_START + 21)
#define CTF_SC_IMPORT_MAP							(XML_SC_CTF_START + 22)
#define CTF_SC_CELLSTYLE							(XML_SC_CTF_START + 23)
#define CTF_SC_VALIDATION							(XML_SC_CTF_START + 24)

#define CTF_SC_ROWHEIGHT							(XML_SC_CTF_START + 50)
#define CTF_SC_ROWOPTIMALHEIGHT						(XML_SC_CTF_START + 51)
#define CTF_SC_ROWBREAKBEFORE						(XML_SC_CTF_START + 52)
#define CTF_SC_ISVISIBLE							(XML_SC_CTF_START + 53)

#define CTF_SC_MASTERPAGENAME						(XML_SC_CTF_START + 53)

//ColumnStyles
#define XML_SC_TYPE_BREAKBEFORE						(XML_SC_TYPES_START + 50)

class ScXMLExport;
class ScXMLImport;

class ScXMLCellExportPropertyMapper : public SvXMLExportPropertyMapper
{
protected:
    /** Application-specific filter. By default do nothing. */
    virtual void ContextFilter(
            ::std::vector< XMLPropertyState >& rProperties,
            ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > rPropSet ) const;
public:
    ScXMLCellExportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper );
    virtual ~ScXMLCellExportPropertyMapper();

    /** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
    virtual void handleSpecialItem(
            SvXMLAttributeList& rAttrList,
            const XMLPropertyState& rProperty,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const ::std::vector< XMLPropertyState > *pProperties = 0,
            sal_uInt32 nIdx = 0 ) const;
};

class ScXMLRowExportPropertyMapper : public SvXMLExportPropertyMapper
{
protected:
    /** Application-specific filter. By default do nothing. */
    virtual void ContextFilter(
            ::std::vector< XMLPropertyState >& rProperties,
            ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > rPropSet ) const;
public:
    ScXMLRowExportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper );
    virtual ~ScXMLRowExportPropertyMapper();
};

class ScXMLColumnExportPropertyMapper : public SvXMLExportPropertyMapper
{
public:
    ScXMLColumnExportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper );
    virtual ~ScXMLColumnExportPropertyMapper();

    /** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
    virtual void handleSpecialItem(
            SvXMLAttributeList& rAttrList,
            const XMLPropertyState& rProperty,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const ::std::vector< XMLPropertyState > *pProperties = 0,
            sal_uInt32 nIdx = 0 ) const;
};

class ScXMLTableExportPropertyMapper : public SvXMLExportPropertyMapper
{
protected:
public:
    ScXMLTableExportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper );
    virtual ~ScXMLTableExportPropertyMapper();

    /** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
    virtual void handleSpecialItem(
            SvXMLAttributeList& rAttrList,
            const XMLPropertyState& rProperty,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const ::std::vector< XMLPropertyState > *pProperties = 0,
            sal_uInt32 nIdx = 0 ) const;
};

class ScXMLAutoStylePoolP : public SvXMLAutoStylePoolP
{
    ScXMLExport& rScXMLExport;

    virtual void exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const;

    virtual void exportStyleContent(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp
            , const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const;

public:
            ScXMLAutoStylePoolP(ScXMLExport& rScXMLExport);
    virtual ~ScXMLAutoStylePoolP();
};

class ScXMLStyleExport : public XMLStyleExport
{
    virtual void exportStyleAttributes(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle );
    virtual void exportStyleContent(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle );
public:
    ScXMLStyleExport(
        SvXMLExport& rExp,
        const ::rtl::OUString& rPoolStyleName,
        SvXMLAutoStylePoolP *pAutoStyleP=0 );
    virtual ~ScXMLStyleExport();
};

class XMLScPropHdlFactory : public XMLPropertyHandlerFactory
{
public:
    XMLScPropHdlFactory();
    virtual ~XMLScPropHdlFactory();
    virtual const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nType ) const;
};

class XmlScPropHdl_CellProtection : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_CellProtection();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_PrintContent : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_PrintContent();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_HoriJustify : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_HoriJustify();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_HoriJustifySource : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_HoriJustifySource();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_Orientation : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_Orientation();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_RotateAngle : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_RotateAngle();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_RotateReference : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_RotateReference();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_VertJustify : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_VertJustify();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_BreakBefore : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_BreakBefore();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_IsTextWrapped : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_IsTextWrapped();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_IsEqual : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_IsEqual() {}
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const { return sal_True; }
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_Vertical : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_Vertical();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
