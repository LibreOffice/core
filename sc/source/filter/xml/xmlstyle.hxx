/*************************************************************************
 *
 *  $RCSfile: xmlstyle.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sab $ $Date: 2000-09-29 12:57:20 $
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

#ifndef _XMLSTYLE_HXX
#define _XMLSTYLE_HXX

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include <xmloff/maptype.hxx>
#endif
#ifndef _XMLOFF_XMLASTPLP_HXX
#include <xmloff/xmlaustp.hxx>
#endif
#ifndef _XMLOFF_XMLTYPES_HXX
#include <xmloff/xmltypes.hxx>
#endif
#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include <xmloff/xmlprmap.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include <xmloff/prhdlfac.hxx>
#endif
#ifndef _XMLOFF_STYLEEXP_HXX_
#include <xmloff/styleexp.hxx>
#endif
#ifndef _XMLOFF_XMLEXPPR_HXX
#include <xmloff/xmlexppr.hxx>
#endif

#ifdef _USE_NAMESPACE
using namespace rtl;
#endif

extern const XMLPropertyMapEntry aXMLScCellStylesProperties[];
extern const XMLPropertyMapEntry aXMLScPageStylesProperties[];
extern const XMLPropertyMapEntry aXMLScColumnStylesProperties[];
extern const XMLPropertyMapEntry aXMLScRowStylesProperties[];
extern const XMLPropertyMapEntry aXMLScTableStylesProperties[];

//CellStyles
#define XML_SC_TYPE_CELLBACKCOLOR                   (XML_SC_TYPES_START +  0)
#define XML_SC_TYPE_CELLPROTECTION                  (XML_SC_TYPES_START +  1)
#define XML_SC_TYPE_PRINTCONTENT                    (XML_SC_TYPES_START +  2)
#define XML_SC_TYPE_HORIJUSTIFY                     (XML_SC_TYPES_START +  3)
#define XML_SC_TYPE_HORIJUSTIFYSOURCE               (XML_SC_TYPES_START +  4)
#define XML_SC_TYPE_ISCELLBACKGROUNDTRANSPARENT     (XML_SC_TYPES_START +  5)
#define XML_SC_TYPE_ORIENTATION                     (XML_SC_TYPES_START +  6)
#define XML_SC_TYPE_ROTATEANGLE                     (XML_SC_TYPES_START +  7)
#define XML_SC_TYPE_ROTATEREFERENCE                 (XML_SC_TYPES_START +  8)
#define XML_SC_TYPE_BORDERLEFT                      (XML_SC_TYPES_START +  9)
#define XML_SC_TYPE_BORDERRIGHT                     (XML_SC_TYPES_START + 10)
#define XML_SC_TYPE_BORDERTOP                       (XML_SC_TYPES_START + 11)
#define XML_SC_TYPE_BORDERBOTTOM                    (XML_SC_TYPES_START + 12)
#define XML_SC_TYPE_VERTJUSTIFY                     (XML_SC_TYPES_START + 13)
#define XML_SC_ISTEXTWRAPPED                        (XML_SC_TYPES_START + 14)
//#define XML_SC_TYPE_NUMBERFORMAT                  (XML_SC_TYPES_START + 14)

#define CTF_CELLBACKCOLOR                       1
#define CTF_ISCELLBACKGROUNDTRANSPARENT         2
#define CTF_HORIJUSTIFY                         3
#define CTF_HORIJUSTIFY_SOURCE                  4
#define CTF_ALLPADDING                          5
#define CTF_BOTTOMPADDING                       6
#define CTF_LEFTPADDING                         7
#define CTF_RIGHTPADDING                        8
#define CTF_TOPPADDING                          9
#define CTF_ALLBORDER                           10
#define CTF_LEFTBORDER                          11
#define CTF_RIGHTBORDER                         12
#define CTF_TOPBORDER                           13
#define CTF_BOTTOMBORDER                        14
#define CTF_ALLBORDERWIDTH                      15
#define CTF_LEFTBORDERWIDTH                     16
#define CTF_RIGHTBORDERWIDTH                    17
#define CTF_TOPBORDERWIDTH                      18
#define CTF_BOTTOMBORDERWIDTH                   19
#define CTF_NUMBERFORMAT                        20
#define CTF_MAP                                 21
#define CTF_PARAINDENT                          22

//PageStyles
#define CTF_BACKGROUNDCOLOR                     50

//ColumnStyles
#define XML_SC_TYPE_BREAKBEFORE                     (XML_SC_TYPES_START + 50)

class ScXMLExport;
class ScXMLImport;

class ScXMLExportPropertyMapper : public SvXMLExportPropertyMapper
{
public:
    ScXMLExportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper );
    virtual ~ScXMLExportPropertyMapper();

    /** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
    virtual void handleSpecialItem(
            SvXMLAttributeList& rAttrList,
            const XMLPropertyState& rProperty,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap ) const;
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
            const SvXMLNamespaceMap& rNamespaceMap) const;

    virtual void exportStyleContent(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap) const;

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
/*  const ScXMLExport* pScXMLExport;
    const ScXMLImport* pScXMLImport;
    sal_Bool bIsExport;*/
public:
    /*XMLScPropHdlFactory(const ScXMLExport* pScXMLExport);
    XMLScPropHdlFactory(const ScXMLImport* pScXMLImport);*/
    XMLScPropHdlFactory();
    virtual ~XMLScPropHdlFactory();
    virtual const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nType ) const;
};

class XMLCellStylesPropertySetMapper : public XMLPropertySetMapper
{
protected:
    /** Application-specific filter. By default do nothing. */
    virtual void ContextFilter(
            ::std::vector< XMLPropertyState >& rProperties,
            ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > rPropSet ) const;
public:

    XMLCellStylesPropertySetMapper(const XMLPropertyMapEntry* pEntries,
            const UniReference< XMLPropertyHandlerFactory >& rFactory);
    virtual ~XMLCellStylesPropertySetMapper();
};

class XMLPageStylesPropertySetMapper : public XMLPropertySetMapper
{
protected:
    /** Application-specific filter. By default do nothing. */
    virtual void ContextFilter(
            ::std::vector< XMLPropertyState >& rProperties,
            ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > rPropSet ) const;
public:

    XMLPageStylesPropertySetMapper(const XMLPropertyMapEntry* pEntries,
            const UniReference< XMLPropertyHandlerFactory >& rFactory);
    virtual ~XMLPageStylesPropertySetMapper();
};

class XMLColumnStylesPropertySetMapper : public XMLPropertySetMapper
{
protected:
    /** Application-specific filter. By default do nothing. */
    virtual void ContextFilter(
            ::std::vector< XMLPropertyState >& rProperties,
            ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > rPropSet ) const;
public:

    XMLColumnStylesPropertySetMapper(const XMLPropertyMapEntry* pEntries,
            const UniReference< XMLPropertyHandlerFactory >& rFactory);
    virtual ~XMLColumnStylesPropertySetMapper();
};

class XMLRowStylesPropertySetMapper : public XMLPropertySetMapper
{
protected:
    /** Application-specific filter. By default do nothing. */
    virtual void ContextFilter(
            ::std::vector< XMLPropertyState >& rProperties,
            ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > rPropSet ) const;
public:

    XMLRowStylesPropertySetMapper(const XMLPropertyMapEntry* pEntries,
            const UniReference< XMLPropertyHandlerFactory >& rFactory);
    virtual ~XMLRowStylesPropertySetMapper();
};

class XMLTableStylesPropertySetMapper : public XMLPropertySetMapper
{
protected:
    /** Application-specific filter. By default do nothing. */
    virtual void ContextFilter(
            ::std::vector< XMLPropertyState >& rProperties,
            ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > rPropSet ) const;
public:

    XMLTableStylesPropertySetMapper(const XMLPropertyMapEntry* pEntries,
            const UniReference< XMLPropertyHandlerFactory >& rFactory);
    virtual ~XMLTableStylesPropertySetMapper();
};

class XmlScPropHdl_CellBackColor : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_CellBackColor();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
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

/*class XmlScPropHdl_NumberFormat : public XMLPropertyHandler
{
    const ScXMLExport* pScXMLExport;
    const ScXMLImport* pScXMLImport;
public:
    XmlScPropHdl_NumberFormat(const ScXMLExport* pScXMLExport);
    XmlScPropHdl_NumberFormat(const ScXMLImport* pScXMLImport);
    virtual ~XmlScPropHdl_NumberFormat();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};*/

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

class XmlScPropHdl_IsCellBackgroundTransparent : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_IsCellBackgroundTransparent();
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
#endif

class XmlScPropHdl_IsTextWrapped : public XMLPropertyHandler
{
public:
    virtual ~XmlScPropHdl_IsTextWrapped();
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};
