/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_XMLSTYLE_HXX
#define SC_XMLSTYLE_HXX

#include <xmloff/maptype.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/prhdlfac.hxx>
#include <xmloff/styleexp.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/contextid.hxx>

extern const XMLPropertyMapEntry aXMLScCellStylesProperties[];
extern const XMLPropertyMapEntry aXMLScColumnStylesProperties[];
extern const XMLPropertyMapEntry aXMLScRowStylesProperties[];
extern const XMLPropertyMapEntry aXMLScFromXLSRowStylesProperties[];
extern const XMLPropertyMapEntry aXMLScRowStylesImportProperties[];
extern const XMLPropertyMapEntry aXMLScTableStylesProperties[];
extern const XMLPropertyMapEntry aXMLScTableStylesImportProperties[];

//CellStyles
#define XML_SC_TYPE_CELLPROTECTION                  (XML_SC_TYPES_START +  1)
#define XML_SC_TYPE_PRINTCONTENT                    (XML_SC_TYPES_START +  2)
#define XML_SC_TYPE_HORIJUSTIFY                     (XML_SC_TYPES_START +  3)
#define XML_SC_TYPE_HORIJUSTIFY_METHOD              (XML_SC_TYPES_START +  4)
#define XML_SC_TYPE_HORIJUSTIFYSOURCE               (XML_SC_TYPES_START +  5)
#define XML_SC_TYPE_HORIJUSTIFYREPEAT               (XML_SC_TYPES_START +  6)
#define XML_SC_TYPE_ORIENTATION                     (XML_SC_TYPES_START +  7)
#define XML_SC_TYPE_ROTATEANGLE                     (XML_SC_TYPES_START +  8)
#define XML_SC_TYPE_ROTATEREFERENCE                 (XML_SC_TYPES_START +  9)
#define XML_SC_TYPE_BORDERLEFT                      (XML_SC_TYPES_START + 10)
#define XML_SC_TYPE_BORDERRIGHT                     (XML_SC_TYPES_START + 11)
#define XML_SC_TYPE_BORDERTOP                       (XML_SC_TYPES_START + 12)
#define XML_SC_TYPE_BORDERBOTTOM                    (XML_SC_TYPES_START + 13)
#define XML_SC_TYPE_VERTJUSTIFY                     (XML_SC_TYPES_START + 14)
#define XML_SC_TYPE_VERTJUSTIFY_METHOD              (XML_SC_TYPES_START + 15)
#define XML_SC_ISTEXTWRAPPED                        (XML_SC_TYPES_START + 16)
#define XML_SC_TYPE_EQUAL                           (XML_SC_TYPES_START + 17)
#define XML_SC_TYPE_VERTICAL                        (XML_SC_TYPES_START + 18)

//      CTF_SC_HORIJUSTIFY                          (XML_SC_CTF_START +  1)
//      CTF_SC_HORIJUSTIFY_SOURCE                   (XML_SC_CTF_START +  2)
#define CTF_SC_ALLPADDING                           (XML_SC_CTF_START +  3)
#define CTF_SC_BOTTOMPADDING                        (XML_SC_CTF_START +  4)
#define CTF_SC_LEFTPADDING                          (XML_SC_CTF_START +  5)
#define CTF_SC_RIGHTPADDING                         (XML_SC_CTF_START +  6)
#define CTF_SC_TOPPADDING                           (XML_SC_CTF_START +  7)
#define CTF_SC_ALLBORDER                            (XML_SC_CTF_START +  8)
#define CTF_SC_LEFTBORDER                           (XML_SC_CTF_START +  9)
#define CTF_SC_RIGHTBORDER                          (XML_SC_CTF_START + 10)
#define CTF_SC_TOPBORDER                            (XML_SC_CTF_START + 11)
#define CTF_SC_BOTTOMBORDER                         (XML_SC_CTF_START + 12)
#define CTF_SC_ALLBORDERWIDTH                       (XML_SC_CTF_START + 13)
#define CTF_SC_LEFTBORDERWIDTH                      (XML_SC_CTF_START + 14)
#define CTF_SC_RIGHTBORDERWIDTH                     (XML_SC_CTF_START + 15)
#define CTF_SC_TOPBORDERWIDTH                       (XML_SC_CTF_START + 16)
#define CTF_SC_BOTTOMBORDERWIDTH                    (XML_SC_CTF_START + 17)
#define CTF_SC_NUMBERFORMAT                         (XML_SC_CTF_START + 18)
#define CTF_SC_MAP                                  (XML_SC_CTF_START + 19)
//      CTF_SC_PARAINDENT                           (XML_SC_CTF_START + 20)
//      CTF_SC_OLDTEXTBACKGROUND                    (XML_SC_CTF_START + 21)
#define CTF_SC_IMPORT_MAP                           (XML_SC_CTF_START + 22)
#define CTF_SC_CELLSTYLE                            (XML_SC_CTF_START + 23)
#define CTF_SC_VALIDATION                           (XML_SC_CTF_START + 24)
#define CTF_SC_DIAGONALTLBR                         (XML_SC_CTF_START + 25)
#define CTF_SC_DIAGONALTLBRWIDTH                    (XML_SC_CTF_START + 26)
#define CTF_SC_DIAGONALBLTR                         (XML_SC_CTF_START + 27)
#define CTF_SC_DIAGONALBLTRWIDTH                    (XML_SC_CTF_START + 28)
#define CTF_SC_DIAGONALTLBRWIDTHS                   (XML_SC_CTF_START + 29)
#define CTF_SC_DIAGONALBLTRWIDTHS                   (XML_SC_CTF_START + 30)

#define CTF_SC_ROWHEIGHT                            (XML_SC_CTF_START + 50)
#define CTF_SC_ROWOPTIMALHEIGHT                     (XML_SC_CTF_START + 51)
#define CTF_SC_ROWBREAKBEFORE                       (XML_SC_CTF_START + 52)
#define CTF_SC_ISVISIBLE                            (XML_SC_CTF_START + 53)

#define CTF_SC_MASTERPAGENAME                       (XML_SC_CTF_START + 53)
#define CTF_SC_HYPERLINK                            (XML_SC_CTF_START + 54)

//ColumnStyles
#define XML_SC_TYPE_BREAKBEFORE                     (XML_SC_TYPES_START + 50)

class ScXMLExport;

class ScXMLCellExportPropertyMapper : public SvXMLExportPropertyMapper
{
protected:
    /** Application-specific filter. By default do nothing. */
    virtual void ContextFilter(
            bool bEnableFoFontFamily,
            ::std::vector< XMLPropertyState >& rProperties,
            ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > rPropSet ) const;
public:
    ScXMLCellExportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper );
    virtual ~ScXMLCellExportPropertyMapper();
    virtual void handleElementItem(
            SvXMLExport& rExport,
            const XMLPropertyState& rProperty,
            sal_uInt16 nFlags,
            const ::std::vector< XMLPropertyState > *pProperties = 0,
            sal_uInt32 nIdx = 0 ) const;

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
            bool bEnableFoFontFamily,
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
        const OUString& rPoolStyleName,
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
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_PrintContent : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_PrintContent();
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_JustifyMethod : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_JustifyMethod();
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_HoriJustify : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_HoriJustify();
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_HoriJustifySource : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_HoriJustifySource();
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_HoriJustifyRepeat : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_HoriJustifyRepeat();
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_Orientation : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_Orientation();
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_RotateAngle : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_RotateAngle();
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_RotateReference : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_RotateReference();
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_VertJustify : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_VertJustify();
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_BreakBefore : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_BreakBefore();
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_IsTextWrapped : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_IsTextWrapped();
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_IsEqual : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_IsEqual() {}
    virtual bool equals( const ::com::sun::star::uno::Any& /* r1 */, const ::com::sun::star::uno::Any& /* r2 */ ) const { return true; }
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XmlScPropHdl_Vertical : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_Vertical();
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
