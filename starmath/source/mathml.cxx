/*************************************************************************
 *
 *  $RCSfile: mathml.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: cmc $ $Date: 2001-02-27 14:55:15 $
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

/*
 Warning: The SvXMLElementExport helper class creates the beginning and
 closing tags of xml elements in its constructor and destructor, so theres
 hidden stuff going on, on occasion the ordering of these classes declarations
 may be significant
*/

/*todo: Change characters and tcharacters to accumulate the characters together
into one string, xml parser hands them to us line by line rather than all in
one go*/

#ifndef _TOOLS_DEBUG_H
#include <tools/debug.hxx>
#endif
#ifndef _TOOLS_SOLMATH_H
#include <tools/solmath.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif

#ifndef UNOMODEL_HXX
#include <unomodel.hxx>
#endif
#ifndef MATHML_HXX
#include <mathml.hxx>
#endif
#ifndef DOCUMENT_HXX
#include <document.hxx>
#endif
#ifndef UTILITY_HXX
#include <utility.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include <xmloff/attrlist.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#include <com/sun/star/xml/sax/XEntityResolver.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDTDHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>

using namespace com::sun::star;
using namespace rtl;

#ifndef MATHTYPE_HXX
#include "mathtype.hxx"
#endif


#undef WANTEXCEPT
sal_Unicode UnicodeToStarMath(sal_uInt16 rChar)
{
    sal_Unicode cMathChar = rChar;
    for (int i=sizeof(MathType::aMathTypeTable)-1;i>=0;i--)
        if (rChar == MathType::aMathTypeTable[i])
        {
            cMathChar = i | 0xf000;
            break;
        }
    return cMathChar;
}

sal_Bool SmXMLWrapper::Import(SfxMedium &rMedium)
{
    sal_Bool bRet=sal_False;
    uno::Reference<io::XActiveDataSource> xSource;
    xml::sax::InputSource aParserInput;
    SvStorageStreamRef xDocStream;

    uno::Reference<lang::XMultiServiceFactory> xServiceFactory(
        utl::getProcessServiceFactory());
    DBG_ASSERT(xServiceFactory.is(), "XMLReader::Read: got no service manager");

    if (rMedium.IsStorage())
    {
        xDocStream = rMedium.GetStorage()->OpenStream
            (C2S("Content.xml"),STREAM_READ|STREAM_NOCREATE);
        aParserInput.aInputStream = new utl::OInputStreamWrapper(*xDocStream);
    }
    else
    {
        xSource = rMedium.GetDataSource();
        DBG_ASSERT(xSource.is(),"XMLReader::Read: data source missing");

        // get a pipe for connecting the data source to the parser
        uno::Reference<uno::XInterface> xPipe(xServiceFactory->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.Pipe"))));
        DBG_ASSERT(xPipe.is(),"com.sun.star.io.Pipe service missing");

        uno::Reference<io::XOutputStream> xPipeOutput(xPipe, uno::UNO_QUERY);
        xSource->setOutputStream(xPipeOutput);

        aParserInput.aInputStream = uno::Reference<io::XInputStream>(xPipe,
            uno::UNO_QUERY);
    }

    // get parser
    uno::Reference<xml::sax::XParser> xParser(xServiceFactory->createInstance(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser"))),
        uno::UNO_QUERY);
    DBG_ASSERT(xParser.is(),
        "com.sun.star.xml.sax.Parser service missing");

    // get filter
    uno::Sequence < uno::Any > aArgs( 0 );
    uno::Reference< xml::sax::XDocumentHandler > xFilter(
        xServiceFactory->createInstanceWithArguments(
        OUString::createFromAscii("com.sun.star.office.sax.importer.Math"),
        aArgs ), uno::UNO_QUERY );

    DBG_ASSERT( xFilter.is(),
    "XMLReader::Read: com.sun.star.xml.sax.importer.Math service missing");

    // connect parser and filter
    xParser->setDocumentHandler(xFilter);

    //Make a model component from our SmModel
    uno::Reference< lang::XComponent > xModelComp( rModel, uno::UNO_QUERY );
    DBG_ASSERT( xModelComp.is(), "XMLReader::Read: got no model" );

    // connect model and filter
    uno::Reference < document::XImporter > xImporter( xFilter, uno::UNO_QUERY );
    xImporter->setTargetDocument( xModelComp );

    xParser->setDocumentHandler(xFilter);

    if (xSource.is())
    {
        uno::Reference<io::XActiveDataControl> xSourceControl(
            xSource,uno::UNO_QUERY);
        xSourceControl->start();
    }
#ifdef WANTEXCEPT
    try
    {
#endif
        xParser->parseStream(aParserInput);

        uno::Reference<lang::XUnoTunnel> xFilterTunnel;
        xFilterTunnel = uno::Reference<lang::XUnoTunnel>
            ( xFilter, uno::UNO_QUERY );
        SmXMLImport *pFilter = (SmXMLImport *)xFilterTunnel->getSomething(
            SmXMLImport::getUnoTunnelId() );
        if (pFilter)
            bRet = pFilter->GetSuccess();

#ifdef WANTEXCECT
    }
    catch(...)
    {
        pRet = 0;
    }
#endif
    return bRet;
}

const uno::Sequence< sal_Int8 > & SmXMLImport::getUnoTunnelId() throw()
{
    static uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

const uno::Sequence< sal_Int8 > & SmXMLExport::getUnoTunnelId() throw()
{
    static uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

uno::Sequence< OUString > SAL_CALL SmXMLImport_getSupportedServiceNames()
        throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.office.sax.importer.Math" ));
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
        return aSeq;
}

OUString SAL_CALL SmXMLImport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SmXMLImport" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLImport_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SmXMLImport;
}

uno::Sequence< OUString > SAL_CALL SmXMLExport_getSupportedServiceNames()
        throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.office.sax.exporter.Math" ));
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
        return aSeq;
}

OUString SAL_CALL SmXMLExport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SmXMLExport" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLExport_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SmXMLExport;
}


sal_Int64 SAL_CALL SmXMLImport::getSomething( const uno::Sequence< sal_Int8 >&
    rId ) throw(uno::RuntimeException)
{
    if( rId.getLength() == 16 &&
        0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
        rId.getConstArray(), 16 ) )
    return (sal_Int64)this;

    return SvXMLImport::getSomething( rId );
}

sal_Int64 SAL_CALL SmXMLExport::getSomething( const uno::Sequence< sal_Int8 >&
    rId ) throw(uno::RuntimeException)
{
    if( rId.getLength() == 16 &&
        0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
        rId.getConstArray(), 16 ) )
    return (sal_Int64)this;

    return SvXMLExport::getSomething( rId );
}


void SmXMLImport::endDocument(void)
    throw(xml::sax::SAXException, uno::RuntimeException)
{
    //Set the resulted tree into the SmDocShell where it belongs
    SmNode *pTree;
    if (pTree = GetTree())
    {
        uno::Reference <frame::XModel> xModel = GetModel();
        uno::Reference <lang::XUnoTunnel> xTunnel;
        xTunnel = uno::Reference <lang::XUnoTunnel> (xModel,uno::UNO_QUERY);
        SmModel *pModel = reinterpret_cast<SmModel *>
            (xTunnel->getSomething(SmModel::getUnoTunnelId()));

        if (pModel)
        {
            SmDocShell *pDocShell =
                static_cast<SmDocShell*>(pModel->GetObjectShell());
            pDocShell->SetFormulaTree(pTree);
            if (aText.Len()) //If we picked up our annotation text then use it
                pDocShell->GetText() = aText;
            else    //Make up some editable text
            {
                String &rText = pDocShell->GetText();
                pTree->CreateTextFromNode(rText);
                rText.EraseTrailingChars();
                if((rText.GetChar(0) == '{') &&
                    (rText.GetChar(rText.Len()-1) == '}'))
                {
                    rText.Erase(0,1);
                    rText.Erase(rText.Len()-1,1);
                }
            }
        }
        DBG_ASSERT(pModel,"So there *was* a uno problem after all");

        bSuccess = sal_True;
    }
}

sal_Bool SmXMLWrapper::Export(SfxMedium &rMedium)
{
    sal_Bool bRet=sal_False;
    uno::Reference<lang::XMultiServiceFactory>
        xServiceFactory(utl::getProcessServiceFactory());
    DBG_ASSERT(xServiceFactory.is(),"got no service manager");

    uno::Reference<io::XOutputStream> xOut;
    SvStorageStreamRef xDocStream;

    if (!bFlat) //Storage (Package) of Stream
    {
        SvStorage *pStg = rMedium.GetOutputStorage(sal_True);
        xDocStream = pStg->OpenStream
            (C2S("Content.xml"),STREAM_WRITE|STREAM_SHARE_DENYWRITE);
        xOut = new utl::OOutputStreamWrapper(*xDocStream);
    }
    else
    {
        SvStream *pStream = rMedium.GetOutStream();
        xOut = new utl::OOutputStreamWrapper(*pStream);
    }


    uno::Reference<io::XActiveDataSource> xWriter(
        xServiceFactory->createInstance(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer"))),
        uno::UNO_QUERY);
    DBG_ASSERT(xWriter.is(),"com.sun.star.xml.sax.Writer service missing");

    //connect writer and output stream
    xWriter->setOutputStream(xOut);

    //get filter
    uno::Reference< xml::sax::XDocumentHandler > xHandler(xWriter,
        uno::UNO_QUERY);
    uno::Sequence< uno::Any > aArgs(1);
    uno::Any *pArgs = aArgs.getArray();
    *pArgs <<= xHandler;

    uno::Reference<document::XExporter> xExporter(
        xServiceFactory->createInstanceWithArguments(
        OUString::createFromAscii("com.sun.star.office.sax.exporter.Math"),
        aArgs),uno::UNO_QUERY);
    DBG_ASSERT(xExporter.is(),"com.sun.star.office.sax.exporter.Math service missing");
    if (!xExporter.is())
        return bRet;

    //Get model
    uno::Reference< lang::XComponent > xModelComp(rModel, uno::UNO_QUERY );

    // connect model and filter
    xExporter->setSourceDocument( xModelComp );
    uno::Reference< document::XFilter> xFilter(xExporter,uno::UNO_QUERY);

    // would the MathML filter care about having an original file name?
    // I don't think so
    uno::Sequence< beans::PropertyValue > aProps(0);
    xFilter->filter(aProps);


    uno::Reference<lang::XUnoTunnel> xFilterTunnel;
    xFilterTunnel = uno::Reference<lang::XUnoTunnel>
        ( xFilter, uno::UNO_QUERY );
    SmXMLExport *pFilter = (SmXMLExport *)xFilterTunnel->getSomething(
        SmXMLExport::getUnoTunnelId() );
    if (pFilter)
        bRet = pFilter->GetSuccess();

    return bRet;
}

SmXMLExport::SmXMLExport() : SvXMLExport(MAP_INCH,sXML_math) , pTree(0) ,
    bSuccess(sal_False),pText(0)
{}

sal_uInt32 SmXMLExport::exportDoc(const sal_Char *pClass)
{
    uno::Reference <frame::XModel> xModel = GetModel();
    uno::Reference <lang::XUnoTunnel> xTunnel;
    xTunnel = uno::Reference <lang::XUnoTunnel> (xModel,uno::UNO_QUERY);
    SmModel *pModel = reinterpret_cast<SmModel *>
        (xTunnel->getSomething(SmModel::getUnoTunnelId()));

    if (pModel)
    {
        SmDocShell *pDocShell =
            static_cast<SmDocShell*>(pModel->GetObjectShell());
        pTree = pDocShell->GetFormulaTree();
        pText = &(pDocShell->GetText());
    }

    GetDocHandler()->startDocument();

    /*Add xmlns line*/
    SvXMLAttributeList &rList = GetAttrList();
    rList.AddAttribute(GetNamespaceMap().GetAttrNameByIndex(
        XML_NAMESPACE_MATH_IDX),sCDATA,GetNamespaceMap().GetNameByIndex(
        XML_NAMESPACE_MATH_IDX));

    //I think we need something like ImplExportEntities();
    _ExportContent();
    GetDocHandler()->endDocument();
    bSuccess=sal_True;
    return 0;
}

class SmXMLImportContext: public SvXMLImportContext
{
public:
    SmXMLImportContext( SmXMLImport &rImport, sal_uInt16 nPrfx,
        const OUString& rLName)
        : SvXMLImportContext(rImport, nPrfx, rLName) {}
    const SmXMLImport& GetSmImport() const
    {
        return (const SmXMLImport&)GetImport();
    }
    SmXMLImport& GetSmImport()
    {
        return (SmXMLImport&)GetImport();
    }
    virtual void TCharacters(const ::rtl::OUString &rChars) {}
    virtual void Characters(const ::rtl::OUString &rChars)
    {
        /*
        Whitespace occurring within the content of token elements is "trimmed"
        from the ends (i.e. all whitespace at the beginning and end of the
        content is removed), and "collapsed" internally (i.e. each sequence of
        1 or more whitespace characters is replaced with one blank character).
        */
        //collapsing not done yet!
        const ::rtl::OUString &rChars2 = rChars.trim();
        if (rChars2.getLength())
            TCharacters(rChars2/*.collapse()*/);
    }
    virtual SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList) {return 0;}
};

class SmXMLDocContext_Impl : public SmXMLImportContext
{
public:
    SmXMLDocContext_Impl( SmXMLImport &rImport, sal_uInt16 nPrfx,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrfx,rLName) {}
    virtual SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList);
    void EndElement();
};

/*avert thy gaze from the proginator*/
class SmXMLRowContext_Impl : public SmXMLDocContext_Impl
{
public:
    SmXMLRowContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLDocContext_Impl(rImport,nPrefix,rLName)
        { nElementCount = GetSmImport().GetNodeStack().Count(); }
    virtual SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList);
    SvXMLImportContext *StrictCreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList);
    void EndElement();
protected:
    ULONG nElementCount;
};

class SmXMLFracContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLFracContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}
    void EndElement();
};

class SmXMLSqrtContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLSqrtContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}
    void EndElement();
};

class SmXMLRootContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLRootContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}
    void EndElement();
};

struct SmXMLContext_Helper
{
    SmXMLContext_Helper(SmXMLImportContext &rImport) : rContext(rImport),
    nIsBold(-1), nIsItalic(-1),nFontSize(0.0) {}
    void RetrieveAttrs(const uno::Reference< xml::sax::XAttributeList > &
        xAttrList );
    void ApplyAttrs();

    sal_Int8 nIsBold;
    sal_Int8 nIsItalic;
    double nFontSize;
    sal_Bool bFontNodeNeeded;
    OUString sFontFamily;
    OUString sColor;

    SmXMLImportContext rContext;
};

void SmXMLContext_Helper::RetrieveAttrs(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    sal_Int8 nOldIsBold=nIsBold;
    sal_Int8 nOldIsItalic=nIsItalic;
    double nOldFontSize=nFontSize;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    OUString sOldFontFamily = sFontFamily;
    for (sal_Int16 i=0;i<nAttrCount;i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = rContext.GetSmImport().GetNamespaceMap().
            GetKeyByAttrName(sAttrName,&aLocalName);
        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap &rAttrTokenMap =
            rContext.GetSmImport().GetPresLayoutAttrTokenMap();
        switch(rAttrTokenMap.Get(nPrefix,aLocalName))
        {
            case XML_TOK_FONTWEIGHT:
                nIsBold = sValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(
                    sXML_bold)));
                break;
            case XML_TOK_FONTSTYLE:
                nIsItalic = sValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(
                    sXML_italic)));
                break;
            case XML_TOK_FONTSIZE:
                SvXMLUnitConverter::convertDouble(nFontSize,sValue);
                rContext.GetSmImport().GetMM100UnitConverter().
                    setXMLMeasureUnit(MAP_POINT);
                if (-1 == sValue.indexOf(OUString(
                        RTL_CONSTASCII_USTRINGPARAM(sXML_unit_pt))))
                    if (-1 == sValue.indexOf('%'))
                        nFontSize=0.0;
                    else
                    {
                        rContext.GetSmImport().GetMM100UnitConverter().
                            setXMLMeasureUnit(MAP_RELATIVE);
                    }
                break;
            case XML_TOK_FONTFAMILY:
                sFontFamily = sValue;
                break;
            case XML_TOK_COLOR:
                sColor = sValue;
                break;
            default:
                break;
        }
    }

    if ((nOldIsBold!=nIsBold) || (nOldIsItalic!=nIsItalic) ||
        (nOldFontSize!=nFontSize) || (sOldFontFamily!=sFontFamily)
        || sColor.getLength())
        bFontNodeNeeded=sal_True;
    else
        bFontNodeNeeded=sal_False;
}

void SmXMLContext_Helper::ApplyAttrs()
{
    SmNodeStack &rNodeStack = rContext.GetSmImport().GetNodeStack();

    if (bFontNodeNeeded)
    {
        SmToken aToken;
        aToken.cMathChar = '\0';
        aToken.nGroup = 0;
        aToken.nLevel = 5;

        if (nIsBold != -1)
        {
            if (nIsBold)
                aToken.eType = TBOLD;
            else
                aToken.eType = TNBOLD;
            SmStructureNode *pFontNode = static_cast<SmStructureNode *>
                (new SmFontNode(aToken));
            pFontNode->SetSubNodes(0,rNodeStack.Pop());
            rNodeStack.Push(pFontNode);
        }
        if (nIsItalic != -1)
        {
            if (nIsItalic)
                aToken.eType = TITALIC;
            else
                aToken.eType = TNITALIC;
            SmStructureNode *pFontNode = static_cast<SmStructureNode *>
                (new SmFontNode(aToken));
            pFontNode->SetSubNodes(0,rNodeStack.Pop());
            rNodeStack.Push(pFontNode);
        }
        if (nFontSize != 0.0)
        {
            aToken.eType = TSIZE;
            SmFontNode *pFontNode = new SmFontNode(aToken);

            if (MAP_RELATIVE == rContext.GetSmImport().GetMM100UnitConverter().
                getXMLMeasureUnit())
            {
                if (nFontSize < 100.00)
                    pFontNode->SetSizeParameter(Fraction(100.00/nFontSize),
                        FNTSIZ_DIVIDE);
                else
                    pFontNode->SetSizeParameter(Fraction(nFontSize/100.00),
                        FNTSIZ_MULTIPLY);
            }
            else
                pFontNode->SetSizeParameter(Fraction(nFontSize),FNTSIZ_ABSOLUT);

            pFontNode->SetSubNodes(0,rNodeStack.Pop());
            rNodeStack.Push(pFontNode);
        }
        if (sFontFamily.getLength())
        {
            if (sFontFamily.equalsIgnoreCase(OUString(
                RTL_CONSTASCII_USTRINGPARAM(sXML_fixed))))
                aToken.eType = TFIXED;
            else if (sFontFamily.equalsIgnoreCase(OUString(
                RTL_CONSTASCII_USTRINGPARAM("sans"))))
                aToken.eType = TSANS;
            else if (sFontFamily.equalsIgnoreCase(OUString(
                RTL_CONSTASCII_USTRINGPARAM("serif"))))
                aToken.eType = TSERIF;
            else //Just give up, we need to extend our font mechanism to be
                //more general
                return;

            aToken.aText = sFontFamily;
            SmFontNode *pFontNode = new SmFontNode(aToken);
            pFontNode->SetSubNodes(0,rNodeStack.Pop());
            rNodeStack.Push(pFontNode);
        }
        if (sColor.getLength())
        {
            //Again we can only handle a small set of colours in
            //StarMath for now.
            const SvXMLTokenMap& rTokenMap =
                rContext.GetSmImport().GetColorTokenMap();
            aToken.eType = static_cast<SmTokenType>(rTokenMap.Get(
                XML_NAMESPACE_MATH, sColor));
            if (aToken.eType != -1)
            {
                SmFontNode *pFontNode = new SmFontNode(aToken);
                pFontNode->SetSubNodes(0,rNodeStack.Pop());
                rNodeStack.Push(pFontNode);
            }
        }

    }
}

class SmXMLStyleContext_Impl : public SmXMLRowContext_Impl
{
public:
    /*Right now the style tag is completely ignored*/
    SmXMLStyleContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) : SmXMLRowContext_Impl(rImport,nPrefix,rLName),
        aStyleHelper(*this) {}
    void EndElement();
    void StartElement(const uno::Reference< xml::sax::XAttributeList > &
        xAttrList );
protected:
    SmXMLContext_Helper aStyleHelper;
};

void SmXMLStyleContext_Impl::StartElement(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
#if 1
    aStyleHelper.RetrieveAttrs(xAttrList);
#else
    sal_Int8 nOldIsBold=nIsBold;
    sal_Int8 nOldIsItalic=nIsItalic;
    double nOldFontSize=nFontSize;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    OUString sOldFontFamily = sFontFamily;
    for (sal_Int16 i=0;i<nAttrCount;i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName(sAttrName,&aLocalName);
        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap &rAttrTokenMap =
            GetSmImport().GetPresLayoutAttrTokenMap();
        switch(rAttrTokenMap.Get(nPrefix,aLocalName))
        {
            case XML_TOK_FONTWEIGHT:
                nIsBold = sValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(
                    sXML_bold)));
                break;
            case XML_TOK_FONTSTYLE:
                nIsItalic = sValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(
                    sXML_italic)));
                break;
            case XML_TOK_FONTSIZE:
                SvXMLUnitConverter::convertDouble(nFontSize,sValue);
                GetSmImport().GetMM100UnitConverter().
                    setXMLMeasureUnit(MAP_POINT);
                if (-1 == sValue.indexOf(OUString(
                        RTL_CONSTASCII_USTRINGPARAM(sXML_unit_pt))))
                    if (-1 == sValue.indexOf('%'))
                        nFontSize=0.0;
                    else
                    {
                        GetSmImport().GetMM100UnitConverter().
                            setXMLMeasureUnit(MAP_RELATIVE);
                    }
                break;
            case XML_TOK_FONTFAMILY:
                sFontFamily = sValue;
                break;
            case XML_TOK_COLOR:
                sColor = sValue;
                break;
            default:
                break;
        }
    }

    if ((nOldIsBold!=nIsBold) || (nOldIsItalic!=nIsItalic) ||
        (nOldFontSize!=nFontSize) || (sOldFontFamily!=sFontFamily)
        || sColor.getLength())
        bFontNodeNeeded=sal_True;
    else
        bFontNodeNeeded=sal_False;
#endif
}


void SmXMLStyleContext_Impl::EndElement()
{
    /*
    <mstyle> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    if (rNodeStack.Count() - nElementCount > 1)
        SmXMLRowContext_Impl::EndElement();
#if 1
    aStyleHelper.ApplyAttrs();
#else
    if (bFontNodeNeeded)
    {
        SmToken aToken;
        aToken.cMathChar = '\0';
        aToken.nGroup = 0;
        aToken.nLevel = 5;

        if (nIsBold != -1)
        {
            if (nIsBold)
                aToken.eType = TBOLD;
            else
                aToken.eType = TNBOLD;
            SmStructureNode *pFontNode = static_cast<SmStructureNode *>
                (new SmFontNode(aToken));
            pFontNode->SetSubNodes(0,rNodeStack.Pop());
            rNodeStack.Push(pFontNode);
        }
        if (nIsItalic != -1)
        {
            if (nIsItalic)
                aToken.eType = TITALIC;
            else
                aToken.eType = TNITALIC;
            SmStructureNode *pFontNode = static_cast<SmStructureNode *>
                (new SmFontNode(aToken));
            pFontNode->SetSubNodes(0,rNodeStack.Pop());
            rNodeStack.Push(pFontNode);
        }
        if (nFontSize != 0.0)
        {
            aToken.eType = TSIZE;
            SmFontNode *pFontNode = new SmFontNode(aToken);

            if (MAP_RELATIVE == GetSmImport().GetMM100UnitConverter().
                getXMLMeasureUnit())
            {
                if (nFontSize < 100.00)
                    pFontNode->SetSizeParameter(Fraction(100.00/nFontSize),
                        FNTSIZ_DIVIDE);
                else
                    pFontNode->SetSizeParameter(Fraction(nFontSize/100.00),
                        FNTSIZ_MULTIPLY);
            }
            else
                pFontNode->SetSizeParameter(Fraction(nFontSize),FNTSIZ_ABSOLUT);

            pFontNode->SetSubNodes(0,rNodeStack.Pop());
            rNodeStack.Push(pFontNode);
        }
        if (sFontFamily.getLength())
        {
            if (sFontFamily.equalsIgnoreCase(OUString(
                RTL_CONSTASCII_USTRINGPARAM(sXML_fixed))))
                aToken.eType = TFIXED;
            else if (sFontFamily.equalsIgnoreCase(OUString(
                RTL_CONSTASCII_USTRINGPARAM("sans"))))
                aToken.eType = TSANS;
            else if (sFontFamily.equalsIgnoreCase(OUString(
                RTL_CONSTASCII_USTRINGPARAM("serif"))))
                aToken.eType = TSERIF;
            else //Just give up, we need to extend our font mechanism to be
                //more general
                return;

            aToken.aText = sFontFamily;
            SmFontNode *pFontNode = new SmFontNode(aToken);
            pFontNode->SetSubNodes(0,rNodeStack.Pop());
            rNodeStack.Push(pFontNode);
        }
        if (sColor.getLength())
        {
            //Again we can only handle a small set of colours in
            //StarMath for now.
            const SvXMLTokenMap& rTokenMap =
                GetSmImport().GetColorTokenMap();
            aToken.eType = static_cast<SmTokenType>(rTokenMap.Get(
                XML_NAMESPACE_MATH, sColor));
            if (aToken.eType != -1)
            {
                SmFontNode *pFontNode = new SmFontNode(aToken);
                pFontNode->SetSubNodes(0,rNodeStack.Pop());
                rNodeStack.Push(pFontNode);
            }
        }

    }
#endif
}

class SmXMLPaddedContext_Impl : public SmXMLRowContext_Impl
{
public:
    /*Right now the style tag is completely ignored*/
    SmXMLPaddedContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}
    void EndElement();
};

void SmXMLPaddedContext_Impl::EndElement()
{
    /*
    <mpadded> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().Count() - nElementCount > 1)
        SmXMLRowContext_Impl::EndElement();
}

class SmXMLPhantomContext_Impl : public SmXMLRowContext_Impl
{
public:
    /*Right now the style tag is completely ignored*/
    SmXMLPhantomContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}
    void EndElement();
};

void SmXMLPhantomContext_Impl::EndElement()
{
    /*
    <mphantom> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().Count() - nElementCount > 1)
        SmXMLRowContext_Impl::EndElement();

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.nLevel = 5;
    aToken.eType = TPHANTOM;

    SmStructureNode *pPhantom = static_cast<SmStructureNode *>
        (new SmFontNode(aToken));
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    pPhantom->SetSubNodes(0,rNodeStack.Pop());
    rNodeStack.Push(pPhantom);
}

class SmXMLFencedContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLFencedContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName),
        cBegin('('), cEnd(')') {}
    void StartElement(const uno::Reference<
        xml::sax::XAttributeList > & xAttrList );
    void EndElement();
protected:
    sal_Unicode cBegin;
    sal_Unicode cEnd;
};


void SmXMLFencedContext_Impl::StartElement(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0;i<nAttrCount;i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName(sAttrName,&aLocalName);
        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap &rAttrTokenMap =
            GetSmImport().GetFencedAttrTokenMap();
        switch(rAttrTokenMap.Get(nPrefix,aLocalName))
        {
            //temp, starmath cannot handle multichar brackets (I think)
            case XML_TOK_OPEN:
                cBegin = sValue[0];
                break;
            case XML_TOK_CLOSE:
                cEnd = sValue[0];
                break;
            default:
                /*Go to superclass*/
                break;
        }
    }
}


void SmXMLFencedContext_Impl::EndElement()
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.aText = ',';
    aToken.eType = TLEFT;
    aToken.nLevel = 5;

    aToken.eType = TLPARENT;
    aToken.cMathChar = UnicodeToStarMath(cBegin);
    SmStructureNode *pSNode = new SmBraceNode(aToken);
    SmNode *pLeft = new SmMathSymbolNode(aToken);

    aToken.cMathChar = UnicodeToStarMath(cEnd);
    aToken.eType = TRPARENT;
    SmNode *pRight = new SmMathSymbolNode(aToken);


    SmNodeArray aRelationArray;
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    aToken.cMathChar = '\0';
    aToken.aText = ',';
    aToken.eType = TIDENT;

    ULONG i=rNodeStack.Count()-nElementCount;
    if (rNodeStack.Count()-nElementCount > 1)
        i+=rNodeStack.Count()-1-nElementCount;
    aRelationArray.SetSize(i);
    while(rNodeStack.Count() > nElementCount)
    {
        aRelationArray.Put(--i,rNodeStack.Pop());
        if (rNodeStack.Count() > 1)
            aRelationArray.Put(--i,new SmGlyphSpecialNode(aToken));
    }

    SmToken aDummy;
    SmStructureNode *pBody = new SmExpressionNode(aDummy);
    pBody->SetSubNodes(aRelationArray);


    pSNode->SetSubNodes(pLeft,pBody,pRight);
    pSNode->SetScaleMode(SCALE_HEIGHT);
    GetSmImport().GetNodeStack().Push(pSNode);
}


class SmXMLErrorContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLErrorContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}
    void EndElement();
};

void SmXMLErrorContext_Impl::EndElement()
{
    /*Right now the error tag is completely ignored, what
     can I do with it in starmath, ?, maybe we need a
     report window ourselves, do a test for validity of
     the xml input, use merrors, and then generate
     the markup inside the merror with a big red colour
     of something. For now just throw them all away.
     */
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    while(rNodeStack.Count() > nElementCount)
    {
        SmNode *pNode = rNodeStack.Pop();
        delete pNode;
    }
}



class SmXMLNumberContext_Impl : public SmXMLImportContext
{
public:
    SmXMLNumberContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName) {}
    virtual void TCharacters(const ::rtl::OUString &rChars);
};

void SmXMLNumberContext_Impl::TCharacters(const ::rtl::OUString &rChars)
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.aText = rChars;
    aToken.nLevel = 5;
    aToken.eType = TNUMBER;
    GetSmImport().GetNodeStack().Push(new SmTextNode(aToken,FNT_NUMBER));
}

class SmXMLAnnotationContext_Impl : public SmXMLImportContext
{
public:
    SmXMLAnnotationContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName), bIsStarMath(sal_False) {}
    virtual void Characters(const ::rtl::OUString &rChars);
    void StartElement(const uno::Reference<xml::sax::XAttributeList > &
        xAttrList );
private:
    sal_Bool bIsStarMath;
};

void SmXMLAnnotationContext_Impl::StartElement(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0;i<nAttrCount;i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName(sAttrName,&aLocalName);

        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap &rAttrTokenMap =
            GetSmImport().GetAnnotationAttrTokenMap();
        switch(rAttrTokenMap.Get(nPrefix,aLocalName))
        {
            case XML_TOK_ENCODING:
                bIsStarMath= sValue.equals(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("StarMath 5.0")));
                break;
            default:
                break;
        }
    }
}

void SmXMLAnnotationContext_Impl::Characters(const ::rtl::OUString &rChars)
{
    if (bIsStarMath)
        GetSmImport().GetText().Append(String(rChars));
}

class SmXMLTextContext_Impl : public SmXMLImportContext
{
public:
    SmXMLTextContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName) {}
    virtual void TCharacters(const ::rtl::OUString &rChars);
};



void SmXMLTextContext_Impl::TCharacters(const ::rtl::OUString &rChars)
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.aText = rChars;
    aToken.nLevel = 5;
    aToken.eType = TTEXT;
    GetSmImport().GetNodeStack().Push(new SmTextNode(aToken,FNT_TEXT));
}

class SmXMLStringContext_Impl : public SmXMLImportContext
{
public:
    SmXMLStringContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName) {}
    virtual void TCharacters(const ::rtl::OUString &rChars);
};

void SmXMLStringContext_Impl::TCharacters(const ::rtl::OUString &rChars)
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;

    /*
    The content of <ms> elements should be rendered with visible "escaping" of
    certain characters in the content, including at least "double quote"
    itself, and preferably whitespace other than individual blanks. The intent
    is for the viewer to see that the expression is a string literal, and to
    see exactly which characters form its content. For example, <ms>double
    quote is "</ms> might be rendered as "double quote is \"".

    Obviously this isn't fully done here.
    */
    aToken.aText.Erase();
    aToken.aText += '\"';
    aToken.aText += String(rChars);
    aToken.aText += '\"';

    aToken.nLevel = 5;
    aToken.eType = TTEXT;
    GetSmImport().GetNodeStack().Push(new SmTextNode(aToken,FNT_FIXED));
}

class SmXMLIdentifierContext_Impl : public SmXMLImportContext
{
public:
    SmXMLIdentifierContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName),aStyleHelper(*this) {}
    void TCharacters(const ::rtl::OUString &rChars);
    void StartElement(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList ) {aStyleHelper.RetrieveAttrs(xAttrList);};
protected:
    SmXMLContext_Helper aStyleHelper;
};

void SmXMLIdentifierContext_Impl::TCharacters(const ::rtl::OUString &rChars)
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.aText = rChars;
    aToken.nLevel = 5;
    aToken.eType = TIDENT;
    SmTextNode *pNode = 0;

    //we will handle identifier italic/normal here instead of with a standalone
    //font node
    if (((aStyleHelper.nIsItalic == -1) && (rChars.getLength() > 1))
        || ((aStyleHelper.nIsItalic == 0) && (rChars.getLength() == 1)))
    {
        pNode = new SmTextNode(aToken,FNT_FUNCTION);
        pNode->GetFont().SetItalic(ITALIC_NONE);
        aStyleHelper.nIsItalic = -1;
    }
    else
        pNode = new SmTextNode(aToken,FNT_VARIABLE);
    if (aStyleHelper.bFontNodeNeeded && aStyleHelper.nIsItalic != -1)
    {
        if (aStyleHelper.nIsItalic)
            pNode->GetFont().SetItalic(ITALIC_NORMAL);
        else
            pNode->GetFont().SetItalic(ITALIC_NONE);
    }

    if ((-1!=aStyleHelper.nIsBold) || (0.0!=aStyleHelper.nFontSize) ||
        (aStyleHelper.sFontFamily.getLength()) ||
        aStyleHelper.sColor.getLength())
        aStyleHelper.bFontNodeNeeded=sal_True;
    else
        aStyleHelper.bFontNodeNeeded=sal_False;
    if (aStyleHelper.bFontNodeNeeded)
        aStyleHelper.ApplyAttrs();
    GetSmImport().GetNodeStack().Push(pNode);
}

class SmXMLOperatorContext_Impl : public SmXMLImportContext
{
public:
    SmXMLOperatorContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName), bIsStretchy(sal_False) {}
    void TCharacters(const ::rtl::OUString &rChars);
    void StartElement(const uno::Reference<
        xml::sax::XAttributeList > &xAttrList );
private:
    sal_Bool bIsStretchy;
};

void SmXMLOperatorContext_Impl::TCharacters(const ::rtl::OUString &rChars)
{
    SmToken aToken;
    aToken.nGroup = 0;

    aToken.cMathChar = UnicodeToStarMath(rChars[0]);

    aToken.eType = TSPECIAL;
    aToken.nLevel = 5;
    SmMathSymbolNode *pNode = new SmMathSymbolNode(aToken);
    //For stretchy scaling the scaling must be retrieved from this node
    //and applied to the expression itself so as to get the expression
    //to scale the operator to the height of the expression itself
    if (bIsStretchy)
        pNode->SetScaleMode(SCALE_HEIGHT);
    GetSmImport().GetNodeStack().Push(pNode);
}


void SmXMLOperatorContext_Impl::StartElement(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0;i<nAttrCount;i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName(sAttrName,&aLocalName);

        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap &rAttrTokenMap =
            GetSmImport().GetOperatorAttrTokenMap();
        switch(rAttrTokenMap.Get(nPrefix,aLocalName))
        {
            case XML_TOK_STRETCHY:
                bIsStretchy = sValue.equals(
                    OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)));
                break;
            default:
                break;
        }
    }
}



static __FAR_DATA SvXMLTokenMapEntry aMathElemTokenMap[] =
{
    { XML_NAMESPACE_MATH,   sXML_math,              XML_TOK_MATH   },
    XML_TOKEN_MAP_END
};

class SmXMLSpaceContext_Impl : public SmXMLImportContext
{
public:
    SmXMLSpaceContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName) {}
    void StartElement(const uno::Reference<
        xml::sax::XAttributeList >& xAttrList );
};

void SmXMLSpaceContext_Impl::StartElement(
    const uno::Reference<xml::sax::XAttributeList > &xAttrList )
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.eType = TBLANK;
    aToken.nLevel = 5;
    SmBlankNode *pBlank = new SmBlankNode(aToken);
    pBlank->IncreaseBy(aToken);
    GetSmImport().GetNodeStack().Push(pBlank);
}

class SmXMLSubContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLSubContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}
    void EndElement() { GenericEndElement(TRSUB,RSUB); }
protected:
    void GenericEndElement(SmTokenType eType,SmSubSup aSubSup);
};


void SmXMLSubContext_Impl::GenericEndElement(SmTokenType eType,SmSubSup aSubSup)
{
    /*The <msub> element requires exactly 2 arguments.*/
    DBG_ASSERT(GetSmImport().GetNodeStack().Count() - nElementCount == 2,
        "Sub has not two arguments");
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = eType;
    SmSubSupNode *pNode = new SmSubSupNode(aToken);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    // initialize subnodes array
    SmNodeArray  aSubNodes;
    aSubNodes.SetSize(1 + SUBSUP_NUM_ENTRIES);
    for (ULONG i = 1;  i < aSubNodes.GetSize();  i++)
        aSubNodes.Put(i, NULL);

    aSubNodes.Put(aSubSup+1,rNodeStack.Pop());
    aSubNodes.Put(0, rNodeStack.Pop());
    pNode->SetSubNodes(aSubNodes);
    rNodeStack.Push(pNode);
}

class SmXMLSupContext_Impl : public SmXMLSubContext_Impl
{
public:
    SmXMLSupContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLSubContext_Impl(rImport,nPrefix,rLName) {}
    void EndElement() {GenericEndElement(TRSUP,RSUP);}
};

class SmXMLSubSupContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLSubSupContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}
    void EndElement() { GenericEndElement(TRSUB,RSUB,RSUP); }
protected:
    void GenericEndElement(SmTokenType eType,
        SmSubSup aSub,SmSubSup aSup);

};

void SmXMLSubSupContext_Impl::GenericEndElement(SmTokenType eType,
        SmSubSup aSub,SmSubSup aSup)
{
    /*The <msub> element requires exactly 3 arguments.*/
    DBG_ASSERT(GetSmImport().GetNodeStack().Count() - nElementCount == 3,
        "SubSup has not three arguments");

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = eType;
    SmSubSupNode *pNode = new SmSubSupNode(aToken);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    // initialize subnodes array
    SmNodeArray  aSubNodes;
    aSubNodes.SetSize(1 + SUBSUP_NUM_ENTRIES);
    for (ULONG i = 1;  i < aSubNodes.GetSize();  i++)
        aSubNodes.Put(i, NULL);

    aSubNodes.Put(aSup+1,rNodeStack.Pop());
    aSubNodes.Put(aSub+1,rNodeStack.Pop());
    aSubNodes.Put(0, rNodeStack.Pop());
    pNode->SetSubNodes(aSubNodes);
    rNodeStack.Push(pNode);
}

class SmXMLUnderContext_Impl : public SmXMLSubContext_Impl
{
public:
    SmXMLUnderContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLSubContext_Impl(rImport,nPrefix,rLName) {}
    void StartElement(const uno::Reference< xml::sax::XAttributeList > &
        xAttrList );
    void EndElement();
    void HandleAccent();
protected:
    sal_Int16 nAttrCount;
};

void SmXMLUnderContext_Impl::StartElement(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
}

void SmXMLUnderContext_Impl::HandleAccent()
{
    DBG_ASSERT(GetSmImport().GetNodeStack().Count() - nElementCount == 2,
        "Sub has not two arguments");

    /*Just one special case for the underline thing*/
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    SmNode *pTest = rNodeStack.Pop();
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = TUNDERLINE;


    SmNodeArray aSubNodes;
    aSubNodes.SetSize(2);

    SmStructureNode *pNode = new SmAttributNode(aToken);
    if (pTest->GetToken().cMathChar == (0x0332|0xf000))
    {
        aSubNodes.Put(0, new SmRectangleNode(aToken));
        delete pTest;
    }
    else
        aSubNodes.Put(0, pTest);

    aSubNodes.Put(1, rNodeStack.Pop());
    pNode->SetSubNodes(aSubNodes);
    pNode->SetScaleMode(SCALE_WIDTH);
    rNodeStack.Push(pNode);
}


void SmXMLUnderContext_Impl::EndElement()
{
    if (!nAttrCount)
        GenericEndElement(TCSUB,CSUB);
    else
        HandleAccent();
#if 0
    //UnderBrace trick
    SmStructureNode *pNode = rNodeStack.Pop();
    if (pNode->GetSubNode(1)->GetToken().cMathChar == (0x0332|0xf000))
    if (pNode->GetSubNode(0)->GetToken().cMathChar == (0x0332|0xf000))
#endif
}

class SmXMLOverContext_Impl : public SmXMLSubContext_Impl
{
public:
    SmXMLOverContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLSubContext_Impl(rImport,nPrefix,rLName), nAttrCount(0) {}
    void EndElement();
    void StartElement(const uno::Reference< xml::sax::XAttributeList > &
        xAttrList );
    void HandleAccent();
protected:
    sal_Int16 nAttrCount;
};


void SmXMLOverContext_Impl::StartElement(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
}


void SmXMLOverContext_Impl::EndElement()
{
    if (!nAttrCount)
        GenericEndElement(TCSUP,CSUP);
    else
        HandleAccent();
}


void SmXMLOverContext_Impl::HandleAccent()
{
    DBG_ASSERT(GetSmImport().GetNodeStack().Count() - nElementCount == 2,
        "Sub has not two arguments");
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = TACUTE;

    SmAttributNode *pNode = new SmAttributNode(aToken);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    SmNodeArray aSubNodes;
    aSubNodes.SetSize(2);
    aSubNodes.Put(0, rNodeStack.Pop());
    aSubNodes.Put(1, rNodeStack.Pop());
    pNode->SetSubNodes(aSubNodes);
    pNode->SetScaleMode(SCALE_WIDTH);
    rNodeStack.Push(pNode);

}

class SmXMLUnderOverContext_Impl : public SmXMLSubSupContext_Impl
{
public:
    SmXMLUnderOverContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLSubSupContext_Impl(rImport,nPrefix,rLName) {}
    void EndElement() { GenericEndElement(TCSUB,CSUB,CSUP); }
};

class SmXMLMultiScriptsContext_Impl : public SmXMLSubSupContext_Impl
{
public:
    SmXMLMultiScriptsContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLSubSupContext_Impl(rImport,nPrefix,rLName),
        bHasPrescripts(FALSE) {}
    void EndElement();
    void MiddleElement();
    SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList);
private:
    sal_Bool bHasPrescripts;
};

class SmXMLNoneContext_Impl : public SmXMLImportContext
{
public:
    SmXMLNoneContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName) {}
    void EndElement();
};


void SmXMLNoneContext_Impl::EndElement(void)
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.aText.Erase();
    aToken.nLevel = 5;
    aToken.eType = TIDENT;
    GetSmImport().GetNodeStack().Push(
        new SmTextNode(aToken,FNT_VARIABLE));
}

class SmXMLPrescriptsContext_Impl : public SmXMLImportContext
{
public:
    SmXMLPrescriptsContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName) {}
};

class SmXMLTableRowContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLTableRowContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLRowContext_Impl(rImport,nPrefix,rLName)
        {}
    SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList);
};


class SmXMLTableContext_Impl : public SmXMLTableRowContext_Impl
{
public:
    SmXMLTableContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLTableRowContext_Impl(rImport,nPrefix,rLName)
        {}
    void EndElement();
    SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList);
};


class SmXMLTableCellContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLTableCellContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLRowContext_Impl(rImport,nPrefix,rLName)
        {}
};

class SmXMLAlignGroupContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLAlignGroupContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLRowContext_Impl(rImport,nPrefix,rLName)
        {}
    /*Don't do anything with alignment for now*/
    void EndElement() {}
};

class SmXMLActionContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLActionContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLRowContext_Impl(rImport,nPrefix,rLName)
        {}
    void EndElement();
};


static __FAR_DATA SvXMLTokenMapEntry aPresLayoutElemTokenMap[] =
{
    { XML_NAMESPACE_MATH,   sXML_semantics, XML_TOK_SEMANTICS },
    { XML_NAMESPACE_MATH,   sXML_math,      XML_TOK_MATH   },
    { XML_NAMESPACE_MATH,   sXML_mstyle,    XML_TOK_MSTYLE  },
    { XML_NAMESPACE_MATH,   sXML_merror,    XML_TOK_MERROR },
    { XML_NAMESPACE_MATH,   sXML_mphantom,  XML_TOK_MPHANTOM },
    { XML_NAMESPACE_MATH,   sXML_mrow,      XML_TOK_MROW },
    { XML_NAMESPACE_MATH,   sXML_mfrac,     XML_TOK_MFRAC },
    { XML_NAMESPACE_MATH,   sXML_msqrt,     XML_TOK_MSQRT },
    { XML_NAMESPACE_MATH,   sXML_mroot,     XML_TOK_MROOT },
    { XML_NAMESPACE_MATH,   sXML_msub,      XML_TOK_MSUB },
    { XML_NAMESPACE_MATH,   sXML_msup,      XML_TOK_MSUP },
    { XML_NAMESPACE_MATH,   sXML_msubsup,   XML_TOK_MSUBSUP },
    { XML_NAMESPACE_MATH,   sXML_munder,    XML_TOK_MUNDER },
    { XML_NAMESPACE_MATH,   sXML_mover,     XML_TOK_MOVER },
    { XML_NAMESPACE_MATH,   sXML_munderover,    XML_TOK_MUNDEROVER },
    { XML_NAMESPACE_MATH,   sXML_mmultiscripts, XML_TOK_MMULTISCRIPTS },
    { XML_NAMESPACE_MATH,   sXML_mtable,    XML_TOK_MTABLE },
    { XML_NAMESPACE_MATH,   sXML_maction,   XML_TOK_MACTION },
    { XML_NAMESPACE_MATH,   sXML_mfenced,   XML_TOK_MFENCED },
    { XML_NAMESPACE_MATH,   sXML_mpadded,   XML_TOK_MPADDED },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPresLayoutAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   sXML_fontweight,      XML_TOK_FONTWEIGHT    },
    { XML_NAMESPACE_MATH,   sXML_fontstyle,       XML_TOK_FONTSTYLE     },
    { XML_NAMESPACE_MATH,   sXML_fontsize,        XML_TOK_FONTSIZE      },
    { XML_NAMESPACE_MATH,   sXML_fontfamily,      XML_TOK_FONTFAMILY    },
    { XML_NAMESPACE_MATH,   sXML_color,           XML_TOK_COLOR },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aFencedAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   sXML_open,       XML_TOK_OPEN },
    { XML_NAMESPACE_MATH,   sXML_close,      XML_TOK_CLOSE },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aOperatorAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   sXML_stretchy,      XML_TOK_STRETCHY },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aAnnotationAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   sXML_encoding,      XML_TOK_ENCODING },
    XML_TOKEN_MAP_END
};


static __FAR_DATA SvXMLTokenMapEntry aPresElemTokenMap[] =
{
    { XML_NAMESPACE_MATH,   sXML_annotation,    XML_TOK_ANNOTATION },
    { XML_NAMESPACE_MATH,   sXML_mi,    XML_TOK_MI },
    { XML_NAMESPACE_MATH,   sXML_mn,    XML_TOK_MN },
    { XML_NAMESPACE_MATH,   sXML_mo,    XML_TOK_MO },
    { XML_NAMESPACE_MATH,   sXML_mtext, XML_TOK_MTEXT },
    { XML_NAMESPACE_MATH,   sXML_mspace,XML_TOK_MSPACE },
    { XML_NAMESPACE_MATH,   sXML_ms,    XML_TOK_MS },
    { XML_NAMESPACE_MATH,   sXML_maligngroup,   XML_TOK_MALIGNGROUP },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPresScriptEmptyElemTokenMap[] =
{
    { XML_NAMESPACE_MATH,   sXML_mprescripts,   XML_TOK_MPRESCRIPTS },
    { XML_NAMESPACE_MATH,   sXML_none,  XML_TOK_NONE },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPresTableElemTokenMap[] =
{
    { XML_NAMESPACE_MATH,   sXML_mtr,       XML_TOK_MTR },
    { XML_NAMESPACE_MATH,   sXML_mtd,       XML_TOK_MTD },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aColorTokenMap[] =
{
    { XML_NAMESPACE_MATH,   sXML_black,        TBLACK},
    { XML_NAMESPACE_MATH,   sXML_white,        TWHITE},
    { XML_NAMESPACE_MATH,   sXML_red,          TRED},
    { XML_NAMESPACE_MATH,   sXML_green,        TGREEN},
    { XML_NAMESPACE_MATH,   sXML_blue,         TBLUE},
    { XML_NAMESPACE_MATH,   sXML_aqua,         TCYAN},
    { XML_NAMESPACE_MATH,   sXML_fuchsia,      TMAGENTA},
    { XML_NAMESPACE_MATH,   sXML_yellow,       TYELLOW},
    XML_TOKEN_MAP_END
};


const SvXMLTokenMap& SmXMLImport::GetMathElemTokenMap()
{
    if(!pMathElemTokenMap)
        pMathElemTokenMap = new SvXMLTokenMap(aMathElemTokenMap);
    return *pMathElemTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetPresLayoutElemTokenMap()
{
    if(!pPresLayoutElemTokenMap)
        pPresLayoutElemTokenMap = new SvXMLTokenMap(aPresLayoutElemTokenMap);
    return *pPresLayoutElemTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetPresLayoutAttrTokenMap()
{
    if(!pPresLayoutAttrTokenMap)
        pPresLayoutAttrTokenMap = new SvXMLTokenMap(aPresLayoutAttrTokenMap);
    return *pPresLayoutAttrTokenMap;
}


const SvXMLTokenMap& SmXMLImport::GetFencedAttrTokenMap()
{
    if(!pFencedAttrTokenMap)
        pFencedAttrTokenMap = new SvXMLTokenMap(aFencedAttrTokenMap);
    return *pFencedAttrTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetOperatorAttrTokenMap()
{
    if(!pOperatorAttrTokenMap)
        pOperatorAttrTokenMap = new SvXMLTokenMap(aOperatorAttrTokenMap);
    return *pOperatorAttrTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetAnnotationAttrTokenMap()
{
    if(!pAnnotationAttrTokenMap)
        pAnnotationAttrTokenMap = new SvXMLTokenMap(aAnnotationAttrTokenMap);
    return *pAnnotationAttrTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetPresElemTokenMap()
{
    if(!pPresElemTokenMap)
        pPresElemTokenMap = new SvXMLTokenMap(aPresElemTokenMap);
    return *pPresElemTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetPresScriptEmptyElemTokenMap()
{
    if(!pPresScriptEmptyElemTokenMap)
        pPresScriptEmptyElemTokenMap = new
            SvXMLTokenMap(aPresScriptEmptyElemTokenMap);
    return *pPresScriptEmptyElemTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetPresTableElemTokenMap()
{
    if(!pPresTableElemTokenMap)
        pPresTableElemTokenMap = new SvXMLTokenMap(aPresTableElemTokenMap);
    return *pPresTableElemTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetColorTokenMap()
{
    if(!pColorTokenMap)
        pColorTokenMap = new SvXMLTokenMap(aColorTokenMap);
    return *pColorTokenMap;
}

SvXMLImportContext *SmXMLDocContext_Impl::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = 0L;

    const SvXMLTokenMap& rTokenMap = GetSmImport().GetPresLayoutElemTokenMap();

    UINT32 nTest = rTokenMap.Get(nPrefix, rLocalName);

    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        //Consider semantics a dummy except for any starmath annotations
        case XML_TOK_SEMANTICS:
            pContext = GetSmImport().CreateRowContext(nPrefix,rLocalName,
                xAttrList);
            break;
        /*General Layout Schemata*/
        case XML_TOK_MROW:
            pContext = GetSmImport().CreateRowContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MFRAC:
            pContext = GetSmImport().CreateFracContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MSQRT:
            pContext = GetSmImport().CreateSqrtContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MROOT:
            pContext = GetSmImport().CreateRootContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MSTYLE:
            pContext = GetSmImport().CreateStyleContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MERROR:
            pContext = GetSmImport().CreateErrorContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MPADDED:
            pContext = GetSmImport().CreatePaddedContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MPHANTOM:
            pContext = GetSmImport().CreatePhantomContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MFENCED:
            pContext = GetSmImport().CreateFencedContext(nPrefix,rLocalName,
                xAttrList);
            break;
        /*Script and Limit Schemata*/
        case XML_TOK_MSUB:
            pContext = GetSmImport().CreateSubContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MSUP:
            pContext = GetSmImport().CreateSupContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MSUBSUP:
            pContext = GetSmImport().CreateSubSupContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MUNDER:
            pContext = GetSmImport().CreateUnderContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MOVER:
            pContext = GetSmImport().CreateOverContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MUNDEROVER:
            pContext = GetSmImport().CreateUnderOverContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MMULTISCRIPTS:
            pContext = GetSmImport().CreateMultiScriptsContext(nPrefix,
                rLocalName, xAttrList);
            break;
        case XML_TOK_MTABLE:
            pContext = GetSmImport().CreateTableContext(nPrefix,
                rLocalName, xAttrList);
            break;
        case XML_TOK_MACTION:
            pContext = GetSmImport().CreateActionContext(nPrefix,
                rLocalName, xAttrList);
            break;
        default:
            /*Basically theres an implicit mrow around certain bare
             *elements, use a RowContext to see if this is one of
             *those ones*/
            SmXMLRowContext_Impl aTempContext(GetSmImport(),nPrefix,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_mrow)));

            pContext = aTempContext.StrictCreateChildContext(nPrefix,
                rLocalName, xAttrList);
            break;
    }
    return pContext;
}

void SmXMLDocContext_Impl::EndElement()
{
    SmNodeArray ContextArray;
    ContextArray.SetSize(1);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    for(ULONG i=0;i< 1;i++)
        ContextArray.Put(i, rNodeStack.Pop());

    SmToken aDummy;
    SmStructureNode *pSNode = new SmLineNode(aDummy);
    pSNode->SetSubNodes(ContextArray);
    rNodeStack.Push(pSNode);

    SmNodeArray  LineArray;
    ULONG n = rNodeStack.Count();
    LineArray.SetSize(n);
    for (ULONG j = 0; j < n; j++)
        LineArray.Put(n - (j + 1), rNodeStack.Pop());
    SmStructureNode *pSNode2 = new SmTableNode(aDummy);
    pSNode2->SetSubNodes(LineArray);
    rNodeStack.Push(pSNode2);
}

void SmXMLFracContext_Impl::EndElement()
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = TOVER;
    SmStructureNode *pSNode = new SmBinVerNode(aToken);
    SmNode *pOper = new SmRectangleNode(aToken);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    DBG_ASSERT(GetSmImport().GetNodeStack().Count() - nElementCount == 2,
        "Fraction (mfrac) tag is missing component");

    SmNode *pSecond = rNodeStack.Pop();
    SmNode *pFirst = rNodeStack.Pop();
    pSNode->SetSubNodes(pFirst,pOper,pSecond);
    rNodeStack.Push(pSNode);
}

void SmXMLRootContext_Impl::EndElement()
{
    /*The <mroot> element requires exactly 2 arguments.*/
    DBG_ASSERT(GetSmImport().GetNodeStack().Count() - nElementCount == 2,
        "Root tag is missing component");

    SmToken aToken;
    aToken.cMathChar = 0xF087;  //Temporary: alert, based on old starmath font
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = TNROOT;
    SmStructureNode *pSNode = new SmRootNode(aToken);
    SmNode *pOper = new SmRootSymbolNode(aToken);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    SmNode *pIndex = rNodeStack.Pop();
    SmNode *pBase = rNodeStack.Pop();
    pSNode->SetSubNodes(pIndex,pOper,pBase);
    rNodeStack.Push(pSNode);
}

void SmXMLSqrtContext_Impl::EndElement()
{
    /*
    <msqrt> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().Count() - nElementCount > 1)
        SmXMLRowContext_Impl::EndElement();

    SmToken aToken;
    aToken.cMathChar = 0xF087;  //Temporary: alert, based on old starmath font
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = TSQRT;
    SmStructureNode *pSNode = new SmRootNode(aToken);
    SmNode *pOper = new SmRootSymbolNode(aToken);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    pSNode->SetSubNodes(0,pOper,rNodeStack.Pop());
    rNodeStack.Push(pSNode);
}

void SmXMLRowContext_Impl::EndElement()
{
    SmNodeArray aRelationArray;
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    ULONG nSize = rNodeStack.Count()-nElementCount;

    if (nSize)
    {
        aRelationArray.SetSize(nSize);
        for(ULONG j=rNodeStack.Count()-nElementCount;j > 0;j--)
            aRelationArray.Put(j-1,rNodeStack.Pop());


        //If the first or last element is an operator with stretchyness
        //set then we must create a brace node here from those elements,
        //removing the stretchness from the operators and applying it to
        //ourselves, and creating the appropiate dummy StarMath none bracket
        //to balance the arrangement
        if (((aRelationArray.Get(0)->GetScaleMode() == SCALE_HEIGHT)
            && (aRelationArray.Get(0)->GetType() == NMATH))
        || ((aRelationArray.Get(nSize-1)->GetScaleMode() == SCALE_HEIGHT)
            && (aRelationArray.Get(nSize-1)->GetType() == NMATH)))
        {
            SmToken aToken;
            aToken.cMathChar = '\0';
            aToken.nGroup = 0;
            aToken.nLevel = 5;

            int nLeft=0,nRight=0;
            if ((aRelationArray.Get(0)->GetScaleMode() == SCALE_HEIGHT)
                && (aRelationArray.Get(0)->GetType() == NMATH))
            {
                aToken = aRelationArray.Get(0)->GetToken();
                nLeft=1;
            }
            else
                aToken.cMathChar = '\0';

            aToken.eType = TLPARENT;
            SmNode *pLeft = new SmMathSymbolNode(aToken);

            if ((aRelationArray.Get(nSize-1)->GetScaleMode() == SCALE_HEIGHT)
                && (aRelationArray.Get(nSize-1)->GetType() == NMATH))
            {
                aToken = aRelationArray.Get(nSize-1)->GetToken();
                nRight=1;
            }
            else
                aToken.cMathChar = '\0';

            aToken.eType = TRPARENT;
            SmNode *pRight = new SmMathSymbolNode(aToken);

            SmNodeArray aRelationArray2;

            aRelationArray2.SetSize(nSize-nLeft-nRight);

            for(USHORT i=0;i < nSize-nLeft-nRight;i++)
                aRelationArray2.Put(i,aRelationArray.Get(i+nLeft));

            SmToken aDummy;
            SmStructureNode *pSNode = new SmBraceNode(aToken);
            SmStructureNode *pBody = new SmExpressionNode(aDummy);
            pBody->SetSubNodes(aRelationArray2);

            pSNode->SetSubNodes(pLeft,pBody,pRight);
            pSNode->SetScaleMode(SCALE_HEIGHT);
            rNodeStack.Push(pSNode);
            return;
        }
    }
    else //Multiple newlines result in empty row elements
    {
        aRelationArray.SetSize(1);
        SmToken aToken;
        aToken.cMathChar = '\0';
        aToken.nGroup = 0;
        aToken.nLevel = 5;
        aToken.eType = TNEWLINE;
        aRelationArray.Put(0,new SmLineNode(aToken));
    }

    SmToken aDummy;
    SmStructureNode *pSNode = new SmExpressionNode(aDummy);
    pSNode->SetSubNodes(aRelationArray);
    rNodeStack.Push(pSNode);
}





SvXMLImportContext *SmXMLRowContext_Impl::StrictCreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = 0L;

    const SvXMLTokenMap& rTokenMap = GetSmImport().GetPresElemTokenMap();
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        /*Note that these should accept malignmark subelements, but do not*/
        case XML_TOK_MN:
            pContext = GetSmImport().CreateNumberContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MI:
            pContext = GetSmImport().CreateIdentifierContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MO:
            pContext = GetSmImport().CreateOperatorContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MTEXT:
            pContext = GetSmImport().CreateTextContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MSPACE:
            pContext = GetSmImport().CreateSpaceContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MS:
            pContext = GetSmImport().CreateStringContext(nPrefix,rLocalName,
                xAttrList);
            break;

        /*Note: The maligngroup should only be seen when the row
         * (or decendants) are in a table*/
        case XML_TOK_MALIGNGROUP:
            pContext = GetSmImport().CreateAlignGroupContext(nPrefix,rLocalName,
                xAttrList);
            break;

        case XML_TOK_ANNOTATION:
            pContext = GetSmImport().CreateAnnotationContext(nPrefix,rLocalName,
                xAttrList);
            break;

        default:
            break;
    }
    return pContext;
}



SvXMLImportContext *SmXMLRowContext_Impl::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = StrictCreateChildContext(nPrefix,
    rLocalName, xAttrList);

    if (!pContext)
    {
        //Hmm, unrecognized for this level, check to see if its
        //an element that can have an implicit schema around it
        pContext = SmXMLDocContext_Impl::CreateChildContext(nPrefix,
            rLocalName,xAttrList);
    }
    return pContext;
}


SvXMLImportContext *SmXMLMultiScriptsContext_Impl::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = 0L;

    const SvXMLTokenMap& rTokenMap = GetSmImport().
        GetPresScriptEmptyElemTokenMap();
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_MPRESCRIPTS:
            MiddleElement();
            pContext = GetSmImport().CreatePrescriptsContext(nPrefix,
                rLocalName, xAttrList);
            break;
        case XML_TOK_NONE:
            pContext = GetSmImport().CreateNoneContext(nPrefix,rLocalName,
                xAttrList);
            break;
        default:
            pContext = SmXMLRowContext_Impl::CreateChildContext(nPrefix,
                rLocalName,xAttrList);
            break;
    }
    return pContext;
}

void SmXMLMultiScriptsContext_Impl::MiddleElement()
{
    bHasPrescripts=sal_True;

    DBG_ASSERT(GetSmImport().GetNodeStack().Count() - nElementCount > 0,
        "Sub has no arguments");
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    if (rNodeStack.Count()-nElementCount > 1)
    {
        SmToken aToken;
        aToken.cMathChar = '\0';
        aToken.nGroup = 0;
        aToken.nLevel = 0;
        aToken.eType = TRSUB;
        ULONG nFinalCount = rNodeStack.Count()-nElementCount-1;

        SmNodeStack aReverseStack;
        while (rNodeStack.Count()-nElementCount)
        {
            SmNode *pThing = rNodeStack.Pop();
            aReverseStack.Push(pThing);
        }

        for (ULONG nCount=0;nCount < nFinalCount;nCount+=2)
        {
            SmSubSupNode *pNode = new SmSubSupNode(aToken);

            // initialize subnodes array
            SmNodeArray  aSubNodes;
            aSubNodes.SetSize(1 + SUBSUP_NUM_ENTRIES);
            for (ULONG i = 1;  i < aSubNodes.GetSize();  i++)
                aSubNodes.Put(i, NULL);

            /*On each loop the base and its sub sup pair becomes the
             base for the next loop to which the next sub sup pair is
             attached, i.e. wheels within wheels*/
            //if (nCount == 0)
            aSubNodes.Put(0, aReverseStack.Pop());

            SmNode *pScriptNode = aReverseStack.Pop();

            if ((pScriptNode->GetToken().eType != TIDENT) ||
                (pScriptNode->GetToken().aText.Len()))
                aSubNodes.Put(RSUB+1,pScriptNode);
            pScriptNode = aReverseStack.Pop();
            if ((pScriptNode->GetToken().eType != TIDENT) ||
                (pScriptNode->GetToken().aText.Len()))
                aSubNodes.Put(RSUP+1,pScriptNode);

            pNode->SetSubNodes(aSubNodes);
            aReverseStack.Push(pNode);
        }
    rNodeStack.Push(aReverseStack.Pop());
    }
}


void SmXMLTableContext_Impl::EndElement()
{
    SmNodeArray aExpressionArray;
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    SmNodeStack aReverseStack;
    aExpressionArray.SetSize(rNodeStack.Count()-nElementCount);

    ULONG nRows = rNodeStack.Count()-nElementCount;
    USHORT nCols = 0;

    SmStructureNode *pArray;
    for(ULONG i=rNodeStack.Count()-nElementCount;i > 0;i--)
    {
        pArray = (SmStructureNode *)rNodeStack.Pop();
        if (pArray->GetNumSubNodes() == 0)
        {
            //This is a little tricky, it is possible that there was
            //be elements that were not inside a <mtd> pair, in which
            //case they will not be in a row, i.e. they will not have
            //SubNodes, so we have to wait until here before we can
            //resolve the situation. Implicitsurrounding tags are
            //surprisingly difficult to get right within this
            //architecture

            SmNodeArray aRelationArray;
            aRelationArray.SetSize(1);
            aRelationArray.Put(0,pArray);
            SmToken aDummy;
            pArray = new SmExpressionNode(aDummy);
            pArray->SetSubNodes(aRelationArray);
        }

        if (pArray->GetNumSubNodes() > nCols)
            nCols = pArray->GetNumSubNodes();
        aReverseStack.Push(pArray);
    }
    aExpressionArray.SetSize(nCols*nRows);
    ULONG j=0;
    while (aReverseStack.Count())
    {
        pArray = (SmStructureNode *)aReverseStack.Pop();
        for (USHORT i=0;i<pArray->GetNumSubNodes();i++)
            aExpressionArray.Put(j++,pArray->GetSubNode(i));
    }

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = TRGROUP;
    aToken.nLevel = 0;
    aToken.eType = TMATRIX;
    SmMatrixNode *pSNode = new SmMatrixNode(aToken);
    pSNode->SetSubNodes(aExpressionArray);
    pSNode->SetRowCol(static_cast<USHORT>(nRows),nCols);
    rNodeStack.Push(pSNode);
}

SvXMLImportContext *SmXMLTableRowContext_Impl::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = 0L;

    const SvXMLTokenMap& rTokenMap = GetSmImport().
        GetPresTableElemTokenMap();
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_MTD:
            pContext = GetSmImport().CreateTableCellContext(nPrefix,
                rLocalName, xAttrList);
            break;
        default:
            pContext = SmXMLRowContext_Impl::CreateChildContext(nPrefix,
                rLocalName,xAttrList);
            break;
    }
    return pContext;
}

SvXMLImportContext *SmXMLTableContext_Impl::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = 0L;

    const SvXMLTokenMap& rTokenMap = GetSmImport().
        GetPresTableElemTokenMap();
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_MTR:
            pContext = GetSmImport().CreateTableRowContext(nPrefix,rLocalName,
                xAttrList);
            break;
        default:
            pContext = SmXMLTableRowContext_Impl::CreateChildContext(nPrefix,
                rLocalName,xAttrList);
            break;
    }
    return pContext;
}

void SmXMLMultiScriptsContext_Impl::EndElement()
{
    if (!bHasPrescripts)
        MiddleElement();

    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    if (rNodeStack.Count()-nElementCount > 1)
    {
        SmToken aToken;
        aToken.cMathChar = '\0';
        aToken.nGroup = 0;
        aToken.nLevel = 0;
        aToken.eType = TLSUB;
        ULONG nFinalCount = rNodeStack.Count()-nElementCount-1;

        SmNodeStack aReverseStack;
        while (rNodeStack.Count()-nElementCount)
            aReverseStack.Push(rNodeStack.Pop());
        for (ULONG nCount=0;nCount < nFinalCount;nCount+=2)
        {
            SmSubSupNode *pNode = new SmSubSupNode(aToken);

            // initialize subnodes array
            SmNodeArray  aSubNodes;
            aSubNodes.SetSize(1 + SUBSUP_NUM_ENTRIES);
            for (ULONG i = 1;  i < aSubNodes.GetSize();  i++)
                aSubNodes.Put(i, NULL);

            /*On each loop the base and its sub sup pair becomes the
             base for the next loop to which the next sub sup pair is
             attached, i.e. wheels within wheels*/
            //if (nCount == 0)
            aSubNodes.Put(0, aReverseStack.Pop());

            SmNode *pScriptNode = aReverseStack.Pop();
            if (pScriptNode->GetToken().aText.Len())
                aSubNodes.Put(LSUB+1,pScriptNode);
            pScriptNode = aReverseStack.Pop();
            if (pScriptNode->GetToken().aText.Len())
                aSubNodes.Put(LSUP+1,pScriptNode);

            pNode->SetSubNodes(aSubNodes);
            aReverseStack.Push(pNode);
        }
    rNodeStack.Push(aReverseStack.Pop());
    }

}
void SmXMLActionContext_Impl::EndElement()
{
    /*For now we will just assume that the
     selected attribute is one, and then just display
     that expression alone, i.e. remove all expect the
     first pushed one*/

    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    for(ULONG i=rNodeStack.Count()-nElementCount;i > 1;i--)
    {
        delete rNodeStack.Pop();
    }
}

SvXMLImportContext *SmXMLImport::CreateContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLDocContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateRowContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLRowContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateTextContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLTextContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateAnnotationContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLAnnotationContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateStringContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLStringContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateNumberContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLNumberContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateIdentifierContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLIdentifierContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateOperatorContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
    return new SmXMLOperatorContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateSpaceContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
    return new SmXMLSpaceContext_Impl(*this,nPrefix,rLocalName);
}


SvXMLImportContext *SmXMLImport::CreateFracContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLFracContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateSqrtContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLSqrtContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateRootContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLRootContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateStyleContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLStyleContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreatePaddedContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLPaddedContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreatePhantomContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLPhantomContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateFencedContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLFencedContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateErrorContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLErrorContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateSubContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLSubContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateSubSupContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLSubSupContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateSupContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLSupContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateUnderContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLUnderContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateOverContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLOverContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateUnderOverContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLUnderOverContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateMultiScriptsContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLMultiScriptsContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateTableContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLTableContext_Impl(*this,nPrefix,rLocalName);
}
SvXMLImportContext *SmXMLImport::CreateTableRowContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLTableRowContext_Impl(*this,nPrefix,rLocalName);
}
SvXMLImportContext *SmXMLImport::CreateTableCellContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLTableCellContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateNoneContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLNoneContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreatePrescriptsContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLPrescriptsContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateAlignGroupContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLAlignGroupContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateActionContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> &xAttrList)
{
        return new SmXMLActionContext_Impl(*this,nPrefix,rLocalName);
}

SmXMLImport::~SmXMLImport()
{
    delete pMathElemTokenMap;
    delete pPresLayoutElemTokenMap;
    delete pPresElemTokenMap;
    delete pPresScriptEmptyElemTokenMap;
    delete pPresTableElemTokenMap;
    delete pPresLayoutAttrTokenMap;
    delete pFencedAttrTokenMap;
    delete pColorTokenMap;
    delete pOperatorAttrTokenMap;
    delete pAnnotationAttrTokenMap;
}

void SmXMLExport::_ExportContent()
{
    SvXMLElementExport aEquation(*this,XML_NAMESPACE_MATH,sXML_math, sal_False,
        sal_True);

    if (pText)
    {
        SvXMLElementExport aSemantics(*this,XML_NAMESPACE_MATH,sXML_semantics,
            sal_True, sal_True);
    }

    ExportNodes(pTree,0);

    if (pText)
    {
        AddAttribute(XML_NAMESPACE_MATH,sXML_encoding,
            OUString(RTL_CONSTASCII_USTRINGPARAM("StarMath 5.0")));
        SvXMLElementExport aAnnotation(*this,XML_NAMESPACE_MATH,
            sXML_annotation,sal_True, sal_False);
        GetDocHandler()->characters(*pText);
    }
}

void SmXMLExport::ExportLine(const SmNode *pNode,int nLevel)
{
    ExportExpression(pNode,nLevel);
}

void SmXMLExport::ExportBinaryHorizontal(const SmNode *pNode,int nLevel)
{
    ExportExpression(pNode,nLevel);
}

void SmXMLExport::ExportUnaryHorizontal(const SmNode *pNode,int nLevel)
{
    ExportExpression(pNode,nLevel);
}

void SmXMLExport::ExportExpression(const SmNode *pNode,int nLevel)
{
    SvXMLElementExport *pRow=0;
    ULONG  nSize = pNode->GetNumSubNodes();

    if (nSize > 1)
        pRow = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,sXML_mrow,
        sal_True, sal_True);

    //if (nSize)
    //{
        for (USHORT i = 0; i < nSize; i++)
            if (const SmNode *pTemp = pNode->GetSubNode(i))
                ExportNodes(pTemp,nLevel+1);
    //}
#if 0
    else
    {
        //This saves us from situations like "a newline" where the
        //lack of a term following the newline would otherwise create
        //a incorrect token like <mtr/>
        SvXMLElementExport aDummy(*this,XML_NAMESPACE_MATH,sXML_mi,
            sal_True,sal_True);
        sal_Unicode nArse[2] = {'\n','\0'};
        GetDocHandler()->characters(nArse);
    }
#endif

    delete pRow;
}

void SmXMLExport::ExportBinaryVertical(const SmNode *pNode,int nLevel)
{
    DBG_ASSERT(pNode->GetNumSubNodes()==3,"Bad Fraction");
    SvXMLElementExport aFraction(*this,XML_NAMESPACE_MATH,sXML_mfrac, sal_True,
        sal_True);
    ExportNodes(pNode->GetSubNode(0),nLevel);
    ExportNodes(pNode->GetSubNode(2),nLevel);
}

void SmXMLExport::ExportTable(const SmNode *pNode, int nLevel)
{
    SvXMLElementExport *pTable=0;

    USHORT nSize = pNode->GetNumSubNodes();

    //If the list ends in newline then the last entry has
    //no subnodes, the newline is superfulous so we just drop
    //the last node, inclusion would create a bad MathML
    //table
    if (pNode->GetSubNode(nSize-1)->GetNumSubNodes() == 0)
        nSize--;

    if ( nLevel || (nSize >1))
        pTable = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,sXML_mtable,
        sal_True, sal_True);

    for (USHORT i = 0; i < nSize; i++)
        if (const SmNode *pTemp = pNode->GetSubNode(i))
        {
            SvXMLElementExport *pRow=0;
            if (pTable)
                pRow = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,
                    sXML_mtr, sal_True, sal_True);
            ExportNodes(pTemp,nLevel+1);
            delete pRow;
        }

    delete pTable;
}

void SmXMLExport::ExportMath(const SmNode *pNode, int nLevel)
{
    const SmMathSymbolNode *pTemp = static_cast<const SmMathSymbolNode *>
        (pNode);
    SvXMLElementExport aMath(*this,XML_NAMESPACE_MATH,sXML_mo,
        sal_True,sal_False);
    sal_Unicode nArse[2];
    nArse[0] = MathType::aMathTypeTable[pTemp->GetText().GetChar(0)&0x00FF];
    nArse[1] = 0;
    GetDocHandler()->characters(nArse);
}

void SmXMLExport::ExportText(const SmNode *pNode, int nLevel)
{
    SvXMLElementExport *aText;
    const SmTextNode *pTemp = static_cast<const SmTextNode *>(pNode);
    switch (pNode->GetToken().eType)
    {
        default:
        case TIDENT:
            //Note that we change the fontstyle to italic for strings that
            //are italic and longer than a single character.
            if ((pTemp->GetText().Len() > 1) &&
                (pTemp->GetFont().GetItalic() == ITALIC_NORMAL))
                AddAttribute(XML_NAMESPACE_MATH,sXML_fontstyle,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_italic)));
            else if ((pTemp->GetText().Len() == 1) &&
                (pTemp->GetFont().GetItalic() == ITALIC_NONE))
                AddAttribute(XML_NAMESPACE_MATH,sXML_fontstyle,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_normal)));
            aText = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,sXML_mi,
            sal_True,sal_False);
            break;
        case TNUMBER:
            aText = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,sXML_mn,
            sal_True,sal_False);
            break;
        case TTEXT:
            aText = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,sXML_mtext,
            sal_True,sal_False);
            break;
        }
    GetDocHandler()->characters(pTemp->GetText());
    delete aText;
}

void SmXMLExport::ExportSubSupScript(const SmNode *pNode,int nLevel)
{
    const SmNode *pSub,*pSup,*pCSub,*pCSup,*pLSub,*pLSup;
    SvXMLElementExport *pThing=0,*pThing2=0;

    //if we have prescripts at all then we must use the tensor notation

    //This is one of those excellent locations where scope is vital to
    //arrange the construction and destruction of the element helper
    //classes correctly
    pLSub = pNode->GetSubNode(LSUB+1);
    pLSup = pNode->GetSubNode(LSUP+1);
    if (pLSub || pLSup)
    {
        SvXMLElementExport aMultiScripts(*this,XML_NAMESPACE_MATH,
            sXML_mmultiscripts, sal_True, sal_True);


        if ((pCSub = pNode->GetSubNode(CSUB+1))
            && (pCSup=pNode->GetSubNode(CSUP+1)))
        {
            pThing2 = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,
                sXML_munderover, sal_True,sal_True);
        }
        else if (pCSub = pNode->GetSubNode(CSUB+1))
        {
            pThing2 = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,
                sXML_munder, sal_True,sal_True);
        }
        else if (pCSup = pNode->GetSubNode(CSUP+1))
        {
            pThing2 = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,
                sXML_mover, sal_True,sal_True);
        }

        ExportNodes(pNode->GetSubNode(0), nLevel+1);    //Main Term

        if (pCSub)
            ExportNodes(pCSub, nLevel+1);
        if (pCSup)
            ExportNodes(pCSup, nLevel+1);
        delete pThing2;

        pSub = pNode->GetSubNode(RSUB+1);
        pSup = pNode->GetSubNode(RSUP+1);
        if (pSub || pSup)
        {
            if (pSub)
                ExportNodes(pSub, nLevel+1);
            else
            {
                SvXMLElementExport aNone(*this,XML_NAMESPACE_MATH, sXML_none,
                    sal_True,sal_True);
            }
            if (pSup)
                ExportNodes(pSup, nLevel+1);
            else
            {
                SvXMLElementExport aNone(*this,XML_NAMESPACE_MATH, sXML_none,
                    sal_True,sal_True);
            }
        }

        //Seperator element between suffix and prefix sub/sup pairs
        {
            SvXMLElementExport aPrescripts(*this,XML_NAMESPACE_MATH,
                sXML_mprescripts, sal_True,sal_True);
        }

        if (pLSub)
            ExportNodes(pLSub, nLevel+1);
        else
        {
            SvXMLElementExport aNone(*this,XML_NAMESPACE_MATH, sXML_none,
                sal_True,sal_True);

        }
        if (pLSup)
            ExportNodes(pLSup, nLevel+1);
        else
        {
            SvXMLElementExport aNone(*this,XML_NAMESPACE_MATH, sXML_none,
                sal_True,sal_True);

        }
    }
    else
    {
        if ((pSub = pNode->GetSubNode(RSUB+1)) &&
            (pSup = pNode->GetSubNode(RSUP+1)))
        {
            pThing = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,
                sXML_msubsup, sal_True,sal_True);
        }
        else if (pSub = pNode->GetSubNode(RSUB+1))
        {
            pThing = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,sXML_msub,
                sal_True,sal_True);
        }
        else if (pSup = pNode->GetSubNode(RSUP+1))
        {
            pThing = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,sXML_msup,
                sal_True,sal_True);
        }

        if ((pCSub = pNode->GetSubNode(CSUB+1))
            && (pCSup=pNode->GetSubNode(CSUP+1)))
        {
            pThing2 = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,
                sXML_munderover, sal_True,sal_True);
        }
        else if (pCSub = pNode->GetSubNode(CSUB+1))
        {
            pThing2 = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,
                sXML_munder, sal_True,sal_True);
        }
        else if (pCSup = pNode->GetSubNode(CSUP+1))
        {
            pThing2 = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,
                sXML_mover, sal_True,sal_True);
        }
        ExportNodes(pNode->GetSubNode(0), nLevel+1);    //Main Term

        if (pCSub)
            ExportNodes(pCSub, nLevel+1);
        if (pCSup)
            ExportNodes(pCSup, nLevel+1);
        delete pThing2;

        if (pSub)
            ExportNodes(pSub, nLevel+1);
        if (pSup)
            ExportNodes(pSup, nLevel+1);
        delete pThing;
    }
}

void SmXMLExport::ExportBrace(const SmNode *pNode, int nLevel)
{
    const SmNode *pTemp;
    const SmMathSymbolNode *pLeft=static_cast<const SmMathSymbolNode *>
        (pNode->GetSubNode(0));
    const SmMathSymbolNode *pRight=static_cast<const SmMathSymbolNode *>
        (pNode->GetSubNode(2));
    SvXMLElementExport *pFences=0,*pRow=0;
    if ( ((pLeft) && (pLeft->GetToken().eType != TNONE)) &&
        ((pRight) && (pRight->GetToken().eType != TNONE)) &&
        (pNode->GetScaleMode() == SCALE_HEIGHT))
    {
        sal_Unicode nArse[2];
        nArse[1] = 0;
        nArse[0] = MathType::aMathTypeTable[pLeft->GetText().GetChar(0)
            & 0x00FF];
        AddAttribute(XML_NAMESPACE_MATH,sXML_open,nArse);
        nArse[0] = MathType::aMathTypeTable[pRight->GetText().GetChar(0)
            & 0x00FF];
        AddAttribute(XML_NAMESPACE_MATH,sXML_close,nArse);
        pFences = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,sXML_mfenced,
            sal_True,sal_True);
    }
    else if (pLeft && (pLeft->GetToken().eType != TNONE))
    {
        pRow = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,sXML_mrow,
            sal_True, sal_True);
        if (pNode->GetScaleMode() == SCALE_HEIGHT)
            AddAttribute(XML_NAMESPACE_MATH,sXML_stretchy,OUString(
            RTL_CONSTASCII_USTRINGPARAM(sXML_true)));
        else
            AddAttribute(XML_NAMESPACE_MATH,sXML_stretchy,OUString(
            RTL_CONSTASCII_USTRINGPARAM(sXML_false)));
        ExportMath(pLeft,nLevel+1);
    }
    else
        pRow = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,sXML_mrow,
            sal_True, sal_True);



    if (pTemp = pNode->GetSubNode(1))
        ExportExpression(pTemp,nLevel+1);
    if (pFences)
        delete pFences;
    else if (pRight && (pRight->GetToken().eType != TNONE))
    {
        if (pNode->GetScaleMode() == SCALE_HEIGHT)
            AddAttribute(XML_NAMESPACE_MATH,sXML_stretchy,OUString(
            RTL_CONSTASCII_USTRINGPARAM(sXML_true)));
        else
            AddAttribute(XML_NAMESPACE_MATH,sXML_stretchy,OUString(
            RTL_CONSTASCII_USTRINGPARAM(sXML_false)));
        ExportMath(pRight,nLevel+1);
    }
    delete pRow;
}

void SmXMLExport::ExportRoot(const SmNode *pNode, int nLevel)
{
    if (pNode->GetSubNode(0))
    {
        SvXMLElementExport aRoot(*this,XML_NAMESPACE_MATH,sXML_mroot,sal_True,
            sal_True);
        ExportNodes(pNode->GetSubNode(2),nLevel+1);
        ExportNodes(pNode->GetSubNode(0),nLevel+1);
    }
    else
    {
        SvXMLElementExport aSqrt(*this,XML_NAMESPACE_MATH,sXML_msqrt,sal_True,
            sal_True);
        ExportNodes(pNode->GetSubNode(2),nLevel+1);
    }
}

void SmXMLExport::ExportOperator(const SmNode *pNode, int nLevel)
{
    /*we need to either use content or font and size attributes
     *here*/
#if 0
    {
    SvXMLElementExport aMath(*this,XML_NAMESPACE_MATH,sXML_mo,
        sal_True,sal_False);
    SmTextNode *pTemp = (SmTextNode *)pNode->GetSubNode(0);
    GetDocHandler()->characters(pTemp->GetText());
    }
#endif
    SvXMLElementExport aRow(*this,XML_NAMESPACE_MATH,sXML_mrow,
        sal_True, sal_True);
    ExportNodes(pNode->GetSubNode(0),nLevel+1);
    ExportNodes(pNode->GetSubNode(1),nLevel+1);
}

void SmXMLExport::ExportAttributes(const SmNode *pNode, int nLevel)
{
    SvXMLElementExport *pElement;

    if (pNode->GetToken().eType == TUNDERLINE)
    {
        AddAttribute(XML_NAMESPACE_MATH,sXML_accentunder,
            OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)));
        pElement = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,sXML_munder,
            sal_True,sal_True);
    }
    else
    {
        AddAttribute(XML_NAMESPACE_MATH,sXML_accent,
            OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)));
        pElement = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,sXML_mover,
            sal_True,sal_True);
    }

    ExportNodes(pNode->GetSubNode(1),nLevel+1);
    switch (pNode->GetToken().eType)
    {
        case TOVERLINE:
            {
            //proper entity support required
            SvXMLElementExport aMath(*this,XML_NAMESPACE_MATH,sXML_mo,
                sal_True,sal_False);
#if 0
            GetDocHandler()->characters(
                OUString(RTL_CONSTASCII_USTRINGPARAM("&overbar;")));
#else
            sal_Unicode nArse[2] = {0xAF,0x00};
#endif
            GetDocHandler()->characters(nArse);
            }
            break;
        case TUNDERLINE:
            {
            //proper entity support required
            SvXMLElementExport aMath(*this,XML_NAMESPACE_MATH,sXML_mo,
                sal_True,sal_False);
#if 0
            GetDocHandler()->characters(
                OUString(RTL_CONSTASCII_USTRINGPARAM("&underbar;")));
#else
            sal_Unicode nArse[2] = {0x0332,0x00};
#endif
            GetDocHandler()->characters(nArse);
            }
            break;
        default:
            ExportNodes(pNode->GetSubNode(0),nLevel+1);
            break;
    }
    delete pElement;
}

void SmXMLExport::ExportFont(const SmNode *pNode, int nLevel)
{
    SvXMLElementExport *pElement=0;
    switch (pNode->GetToken().eType)
    {
        //wrap a phantom element around everything*/
        case TPHANTOM:
            pElement = new SvXMLElementExport(*this,XML_NAMESPACE_MATH,
                sXML_mphantom, sal_True,sal_True);
            break;
        case TBOLD:
            AddAttribute(XML_NAMESPACE_MATH,sXML_fontweight,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_bold)));
            break;
        case TITALIC:
            AddAttribute(XML_NAMESPACE_MATH,sXML_fontstyle,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_italic)));
            break;
        case TNBOLD:
            AddAttribute(XML_NAMESPACE_MATH,sXML_fontweight,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_weight_normal)));
            break;
        case TNITALIC:
            AddAttribute(XML_NAMESPACE_MATH,sXML_fontstyle,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_weight_normal)));
            break;
        case TBLACK:
            AddAttribute(XML_NAMESPACE_MATH,sXML_color,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_black)));
            break;
        case TWHITE:
            AddAttribute(XML_NAMESPACE_MATH,sXML_color,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_white)));
            break;
        case TRED:
            AddAttribute(XML_NAMESPACE_MATH,sXML_color,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_red)));
            break;
        case TGREEN:
            AddAttribute(XML_NAMESPACE_MATH,sXML_color,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_green)));
            break;
        case TBLUE:
            AddAttribute(XML_NAMESPACE_MATH,sXML_color,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_blue)));
            break;
        case TCYAN:
            AddAttribute(XML_NAMESPACE_MATH,sXML_color,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_aqua)));
            break;
        case TMAGENTA:
            AddAttribute(XML_NAMESPACE_MATH,sXML_color,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_fuchsia)));
            break;
        case TYELLOW:
            AddAttribute(XML_NAMESPACE_MATH,sXML_color,
                OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_yellow)));
            break;
        case TSIZE:
            {
            const SmFontNode *pFontNode = static_cast<const SmFontNode *>
                (pNode);
            const Fraction &aFrac = pFontNode->GetSizeParameter();

            OUStringBuffer sStrBuf;
            switch(pFontNode->GetSizeType())
            {
                case FNTSIZ_MULTIPLY:
                    SvXMLUnitConverter::convertDouble(sStrBuf,
                        static_cast<double>(aFrac*Fraction(100.00)));
                    sStrBuf.append(static_cast<sal_Unicode>('%'));
                    break;
                case FNTSIZ_DIVIDE:
                    SvXMLUnitConverter::convertDouble(sStrBuf,
                        static_cast<double>(Fraction(100.00)/aFrac));
                    sStrBuf.append(static_cast<sal_Unicode>('%'));
                    break;
                case FNTSIZ_ABSOLUT:
                    SvXMLUnitConverter::convertDouble(sStrBuf,
                        static_cast<double>(aFrac));
                    sStrBuf.append(
                        OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_unit_pt)));
                    break;
                default:
                    {
                        //The problem here is that the wheels fall off because
                        //font size is stored in 100th's of a mm not pts, and
                        //rounding errors take their toll on the original
                        //value specified in points.

                        //Must fix StarMath to retain the original pt values
                        Fraction aTemp = Sm100th_mmToPts(pFontNode->GetFont().
                            GetSize().Height());

                        if (pFontNode->GetSizeType() == FNTSIZ_MINUS)
                            aTemp-=aFrac;
                        else
                            aTemp+=aFrac;

                        double mytest = static_cast<double>(aTemp);

                        mytest = SolarMath::Round(mytest,1);
                        SvXMLUnitConverter::convertDouble(sStrBuf,mytest);
                        sStrBuf.append(OUString(
                            RTL_CONSTASCII_USTRINGPARAM(sXML_unit_pt)));
                    }
                    break;
            }

            OUString sStr(sStrBuf.makeStringAndClear());
            AddAttribute(XML_NAMESPACE_MATH,sXML_fontsize,sStr);
            }
            break;
        case TFIXED:
        case TSANS:
        case TSERIF:
            AddAttribute(XML_NAMESPACE_MATH,sXML_fontfamily,
                pNode->GetToken().aText);
            break;

    }
#if 0
    if (pNode->GetNumSubNodes() > 1) //or in the future is a node that
                                     //cannot take the currently supported
                                     //properties
#endif
    //for now we will just always export with a style and not worry about
    //anyone else for the moment.
    {
        //wrap a style around it
        SvXMLElementExport aStyle(*this,XML_NAMESPACE_MATH,
                sXML_mstyle, sal_True,sal_True);
        ExportExpression(pNode,nLevel);
    }
#if 0
    else
        ExportNodes(pNode->GetSubNode(0),nLevel+1);
#endif

    delete pElement;
}


void SmXMLExport::ExportVerticalBrace(const SmNode *pNode, int nLevel)
{
    //Place the overbrace value OVER a vertical brace and then place that
    //expression OVER the overbrace value, If someone can find a
    //dedicated term in MathML to handle this overbrace/underbrace concept
    //let me know. C.
    const sal_Char *pWhich;

    switch (pNode->GetToken().eType)
    {
        case TOVERBRACE:
        default:
            pWhich = sXML_mover;
            break;
        case TUNDERBRACE:
            pWhich = sXML_munder;
            break;
    }

    DBG_ASSERT(pNode->GetNumSubNodes()==3,"Bad Vertical Brace");
    SvXMLElementExport aOver1(*this,XML_NAMESPACE_MATH,pWhich, sal_True,
        sal_True);
    {//Scoping
        AddAttribute(XML_NAMESPACE_MATH,sXML_accent,OUString(
            RTL_CONSTASCII_USTRINGPARAM(sXML_accent)));
        SvXMLElementExport aOver2(*this,XML_NAMESPACE_MATH,pWhich, sal_True,
            sal_True);
        ExportNodes(pNode->GetSubNode(0),nLevel);
        ExportNodes(pNode->GetSubNode(1),nLevel);
    }
    ExportNodes(pNode->GetSubNode(2),nLevel);
}

void SmXMLExport::ExportMatrix(const SmNode *pNode, int nLevel)
{
    SvXMLElementExport aTable(*this,XML_NAMESPACE_MATH,sXML_mtable,
        sal_True, sal_True);
    const SmMatrixNode *pMatrix = static_cast<const SmMatrixNode *>(pNode);
    USHORT i=0;
    for (ULONG y = 0; y < pMatrix->GetNumRows(); y++)
    {
        SvXMLElementExport aRow(*this,XML_NAMESPACE_MATH,sXML_mtr,
            sal_True, sal_True);
        for (ULONG x = 0; x < pMatrix->GetNumCols(); x++)
            if (const SmNode *pTemp = pNode->GetSubNode(i++))
            {
                SvXMLElementExport aCell(*this,XML_NAMESPACE_MATH,
                        sXML_mtd, sal_True, sal_True);
                ExportNodes(pTemp,nLevel+1);
            }
    }
}

void SmXMLExport::ExportNodes(const SmNode *pNode, int nLevel)
{
    if (!pNode)
        return;
    switch(pNode->GetType())
    {
        case NTABLE:
            ExportTable(pNode,nLevel);
            break;
        case NEXPRESSION:
            ExportExpression(pNode,nLevel);
            break;
        case NLINE:
            ExportLine(pNode,nLevel);
            break;
        case NTEXT:
            ExportText(pNode,nLevel);
            break;
        case NSPECIAL: //NSPECIAL requires some sort of Entity preservation in
                    //the XML engine.
        case NMATH:
            //To fully handle generic MathML we need to implement the full
            //operator dictionary, we will generate MathML with explicit
            //stretchiness for now.
            AddAttribute(XML_NAMESPACE_MATH,sXML_stretchy,OUString(
                RTL_CONSTASCII_USTRINGPARAM(sXML_false)));
            ExportMath(pNode,nLevel);
            break;
        case NBINHOR:
            ExportBinaryHorizontal(pNode,nLevel);
            break;
        case NUNHOR:
            ExportUnaryHorizontal(pNode,nLevel);
            break;
        case NBRACE:
            ExportBrace(pNode,nLevel);
            break;
        case NBINVER:
            ExportBinaryVertical(pNode,nLevel);
            break;
        case NSUBSUP:
            ExportSubSupScript(pNode,nLevel);
            break;
        case NROOT:
            ExportRoot(pNode,nLevel);
            break;
        case NOPER:
            ExportOperator(pNode,nLevel);
            break;
        case NATTRIBUT:
            ExportAttributes(pNode,nLevel);
            break;
        case NFONT:
            ExportFont(pNode,nLevel);
            break;
        case NVERTICAL_BRACE:
            ExportVerticalBrace(pNode,nLevel);
            break;
        case NMATRIX:
            ExportMatrix(pNode,nLevel);
            break;
#if 0
        default:
            {
            ULONG  nSize = pNode->GetNumSubNodes();
            for (ULONG i = 0; i < nSize; i++)
                if (SmNode *pTemp = pNode->GetSubNode(i))
                    ExportNodes(pTemp,nLevel+1);
            }
            break;
#endif
    }
}
