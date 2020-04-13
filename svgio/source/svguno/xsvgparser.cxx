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

#include <sal/config.h>

#include <com/sun/star/graphic/XSvgParser.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <svgdocumenthandler.hxx>
#include <tools/diagnose_ex.h>

#include <svgvisitor.hxx>

#include <vcl/svgparser.hxx>

using namespace ::com::sun::star;


namespace svgio::svgreader
{
        class SvgParser : public vcl::AbstractSvgParser
        {
        private:
            std::shared_ptr<SvgDrawVisitor> mpVisitor;
            bool parseSvgXML(css::uno::Reference<css::io::XInputStream> const & xSVGStream,
                             css::uno::Reference<css::xml::sax::XDocumentHandler> const & xSvgDocHdl);
        public:
            explicit SvgParser() {}
            virtual ~SvgParser() {}
            SvgParser(const SvgParser&) = delete;
            SvgParser& operator=(const SvgParser&) = delete;

            virtual css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > getDecomposition(
                const css::uno::Reference< css::io::XInputStream >& xSVGStream,
                const OUString& aAbsolutePath) override;

            virtual gfx::DrawRoot* getDrawCommands(
                css::uno::Reference<css::io::XInputStream> const & xSvgStream,
                const OUString& aAbsolutePath) override;
        };

        bool SvgParser::parseSvgXML(uno::Reference<io::XInputStream> const & xSVGStream, uno::Reference<xml::sax::XDocumentHandler> const & xSvgDocHdl)
        {
            try
            {
                uno::Reference<uno::XComponentContext> xContext(
                        comphelper::getProcessComponentContext());
                // prepare ParserInputSrouce
                xml::sax::InputSource myInputSource;
                myInputSource.aInputStream = xSVGStream;

                // get parser
                uno::Reference< xml::sax::XParser > xParser(
                    xml::sax::Parser::create(xContext));
                // fdo#60471 need to enable internal entities because
                // certain ... popular proprietary products write SVG files
                // that use entities to define XML namespaces.
                uno::Reference<lang::XInitialization> const xInit(xParser,
                        uno::UNO_QUERY_THROW);
                uno::Sequence<uno::Any> args(1);
                args[0] <<= OUString("DoSmeplease");
                xInit->initialize(args);

                // connect parser and filter
                xParser->setDocumentHandler(xSvgDocHdl);

                // finally, parse the stream to a hierarchy of
                // SVGGraphicPrimitive2D which will be embedded to the
                // primitive sequence. Their decompositions will in the
                // end create local low-level primitives, thus SVG will
                // be processable from all our processors
                xParser->parseStream(myInputSource);
            }
            catch(const uno::Exception&)
            {
                TOOLS_INFO_EXCEPTION( "svg", "Parse error");
                return false;
            }

            return true;
        }

        uno::Sequence< uno::Reference< ::graphic::XPrimitive2D > > SvgParser::getDecomposition(
            const uno::Reference< ::io::XInputStream >& xSVGStream,
            const OUString& aAbsolutePath )
        {
            drawinglayer::primitive2d::Primitive2DContainer aRetval;

            if(xSVGStream.is())
            {
                // local document handler
                SvgDocHdl* pSvgDocHdl = new SvgDocHdl(aAbsolutePath);
                uno::Reference<xml::sax::XDocumentHandler> xSvgDocHdl(pSvgDocHdl);
                parseSvgXML(xSVGStream, xSvgDocHdl);

                // decompose to primitives
                for(std::unique_ptr<SvgNode> const & pCandidate : pSvgDocHdl->getSvgDocument().getSvgNodeVector())
                {
                    if(Display_none != pCandidate->getDisplay())
                    {
                        pCandidate->decomposeSvgNode(aRetval, false);
                    }
                }
            }
            else
            {
                OSL_ENSURE(false, "Invalid stream (!)");
            }

            return comphelper::containerToSequence(aRetval);
        }

        gfx::DrawRoot* SvgParser::getDrawCommands(
                uno::Reference<io::XInputStream> const & xSvgStream,
                const OUString& aAbsolutePath)
        {
            if (!xSvgStream.is())
                return nullptr;

            SvgDocHdl* pSvgDocHdl = new SvgDocHdl(aAbsolutePath);
            uno::Reference<xml::sax::XDocumentHandler> xSvgDocHdl(pSvgDocHdl);
            parseSvgXML(xSvgStream, xSvgDocHdl);

            // decompose to primitives
            for (std::unique_ptr<SvgNode> const & pCandidate : pSvgDocHdl->getSvgDocument().getSvgNodeVector())
            {
                if (Display_none != pCandidate->getDisplay())
                {
                    mpVisitor = std::make_shared<SvgDrawVisitor>();
                    pCandidate->accept(*mpVisitor);
                    return mpVisitor->getDrawRoot().get();
                }
            }

            return nullptr;
        }

} // end of namespace svgio::svgreader

extern "C"
{
    SAL_DLLPUBLIC_EXPORT vcl::AbstractSvgParser* svgio_create_parser()
    {
        return new svgio::svgreader::SvgParser();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
