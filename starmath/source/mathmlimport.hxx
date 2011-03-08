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

#ifndef _MATHMLIMPORT_HXX_
#define _MATHMLIMPORT_HXX_

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/DocumentSettingsContext.hxx>
#include <xmloff/xmltoken.hxx>

#include <node.hxx>

class SfxMedium;
namespace com { namespace sun { namespace star {
    namespace io {
        class XInputStream;
        class XOutputStream; }
    namespace beans {
        class XPropertySet; }
} } }

////////////////////////////////////////////////////////////

class SmXMLImportWrapper
{
    com::sun::star::uno::Reference<com::sun::star::frame::XModel> xModel;

public:
    SmXMLImportWrapper(com::sun::star::uno::Reference<com::sun::star::frame::XModel> &rRef)
        : xModel(rRef) {}

    ULONG Import(SfxMedium &rMedium);

    ULONG ReadThroughComponent(
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInputStream,
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xModelComponent,
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rFactory,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const sal_Char* pFilterName,
        sal_Bool bEncrypted );

    ULONG ReadThroughComponent(
         const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xModelComponent,
        const sal_Char* pStreamName,
        const sal_Char* pCompatibilityStreamName,
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rFactory,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const sal_Char* pFilterName );
};

////////////////////////////////////////////////////////////

class SmXMLImport : public SvXMLImport
{
        SvXMLTokenMap *pPresLayoutElemTokenMap;
        SvXMLTokenMap *pPresLayoutAttrTokenMap;
        SvXMLTokenMap *pFencedAttrTokenMap;
        SvXMLTokenMap *pOperatorAttrTokenMap;
        SvXMLTokenMap *pAnnotationAttrTokenMap;
        SvXMLTokenMap *pPresElemTokenMap;
        SvXMLTokenMap *pPresScriptEmptyElemTokenMap;
        SvXMLTokenMap *pPresTableElemTokenMap;
        SvXMLTokenMap *pColorTokenMap;

        SmNodeStack aNodeStack;
        sal_Bool bSuccess;
        String aText;

public:
    SmXMLImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        sal_uInt16 nImportFlags=IMPORT_ALL);
    virtual ~SmXMLImport() throw ();

    // XServiceInfo (override parent method)
    ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

    // XUnoTunnel
    sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw(::com::sun::star::uno::RuntimeException);
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();

    void SAL_CALL endDocument(void)
        throw( ::com::sun::star::xml::sax::SAXException,
        ::com::sun::star::uno::RuntimeException );

    SvXMLImportContext *CreateContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateMathContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateRowContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateFracContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateNumberContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateTextContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateAnnotationContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateStringContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateIdentifierContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateOperatorContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateSpaceContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateSqrtContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateRootContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateStyleContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreatePaddedContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreatePhantomContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateFencedContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateErrorContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateSubContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateSupContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateSubSupContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateUnderContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateOverContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateUnderOverContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateMultiScriptsContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateNoneContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreatePrescriptsContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateTableContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateTableRowContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateTableCellContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateAlignGroupContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);
    SvXMLImportContext *CreateActionContext(sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference <
        com::sun::star::xml::sax::XAttributeList> &xAttrList);

    const SvXMLTokenMap &GetPresLayoutElemTokenMap();
    const SvXMLTokenMap &GetPresLayoutAttrTokenMap();
    const SvXMLTokenMap &GetFencedAttrTokenMap();
    const SvXMLTokenMap &GetOperatorAttrTokenMap();
    const SvXMLTokenMap &GetAnnotationAttrTokenMap();
    const SvXMLTokenMap &GetPresElemTokenMap();
    const SvXMLTokenMap &GetPresScriptEmptyElemTokenMap();
    const SvXMLTokenMap &GetPresTableElemTokenMap();
    const SvXMLTokenMap &GetColorTokenMap();

    SmNodeStack & GetNodeStack() {return aNodeStack;}
    SmNode *GetTree() { return aNodeStack.Pop();}
    sal_Bool GetSuccess() { return bSuccess; }
    String &GetText() { return aText;}

    virtual void SetViewSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aViewProps);
    virtual void SetConfigurationSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aViewProps);
};

////////////////////////////////////////////////////////////

enum SmXMLMathElemTokenMap
{
    XML_TOK_MATH
};

enum SmXMLPresLayoutElemTokenMap
{
    XML_TOK_SEMANTICS,
    XML_TOK_MSTYLE,
    XML_TOK_MERROR,
    XML_TOK_MPHANTOM,
    XML_TOK_MROW,
    XML_TOK_MFRAC,
    XML_TOK_MSQRT,
    XML_TOK_MROOT,
    XML_TOK_MSUB,
    XML_TOK_MSUP,
    XML_TOK_MSUBSUP,
    XML_TOK_MMULTISCRIPTS,
    XML_TOK_MUNDER,
    XML_TOK_MOVER,
    XML_TOK_MUNDEROVER,
    XML_TOK_MTABLE,
    XML_TOK_MACTION,
    XML_TOK_MFENCED,
    XML_TOK_MPADDED
};

enum SmXMLPresLayoutAttrTokenMap
{
    XML_TOK_FONTWEIGHT,
    XML_TOK_FONTSTYLE,
    XML_TOK_FONTSIZE,
    XML_TOK_FONTFAMILY,
    XML_TOK_COLOR
};


enum SmXMLFencedAttrTokenMap
{
    XML_TOK_OPEN,
    XML_TOK_CLOSE
};


enum SmXMLPresTableElemTokenMap
{
    XML_TOK_MTR,
    XML_TOK_MTD
};

enum SmXMLPresElemTokenMap
{
    XML_TOK_ANNOTATION,
    XML_TOK_MI,
    XML_TOK_MN,
    XML_TOK_MO,
    XML_TOK_MTEXT,
    XML_TOK_MSPACE,
    XML_TOK_MS,
    XML_TOK_MALIGNGROUP
};

enum SmXMLPresScriptEmptyElemTokenMap
{
    XML_TOK_MPRESCRIPTS,
    XML_TOK_NONE
};

enum SmXMLOperatorAttrTokenMap
{
    XML_TOK_STRETCHY
};

enum SmXMLAnnotationAttrTokenMap
{
    XML_TOK_ENCODING
};

////////////////////////////////////////////////////////////

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
