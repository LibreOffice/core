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
#include <sal/log.hxx>

#include <com/sun/star/graphic/XSvgParser.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <svgdocumenthandler.hxx>

#include <svgvisitor.hxx>

#include "xsvgparser.hxx"

using namespace ::com::sun::star;

namespace svgio
{
    namespace svgreader
    {
        class XSvgParser : public ::cppu::WeakAggImplHelper2< graphic::XSvgParser, lang::XServiceInfo >
        {
        private:
            std::shared_ptr<SvgDrawVisitor> mpVisitor;

            uno::Reference< uno::XComponentContext > context_;
            bool parseSvgXML(uno::Reference<io::XInputStream> const & xSVGStream,
                             uno::Reference<xml::sax::XDocumentHandler> const & xSvgDocHdl);
        public:
            explicit XSvgParser(
                uno::Reference< uno::XComponentContext > const & context);
            XSvgParser(const XSvgParser&) = delete;
            XSvgParser& operator=(const XSvgParser&) = delete;

            // XSvgParser
            virtual uno::Sequence< uno::Reference< ::graphic::XPrimitive2D > > SAL_CALL getDecomposition(
                const uno::Reference< ::io::XInputStream >& xSVGStream,
                const OUString& aAbsolutePath) override;

            virtual uno::Any SAL_CALL getDrawCommands(
                uno::Reference<io::XInputStream> const & xSvgStream,
                const OUString& aAbsolutePath) override;

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName() override;
            virtual sal_Bool SAL_CALL supportsService(const OUString&) override;
            virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
        };
    } // end of namespace svgreader
} // end of namespace svgio

// uno functions
namespace svgio
{
    namespace svgreader
    {
        uno::Sequence< OUString > XSvgParser_getSupportedServiceNames()
        {
            return uno::Sequence< OUString > { "com.sun.star.graphic.SvgTools" };
        }

        OUString XSvgParser_getImplementationName()
        {
            return OUString( "svgio::svgreader::XSvgParser" );
        }

        uno::Reference< uno::XInterface > XSvgParser_createInstance(const uno::Reference< uno::XComponentContext >& context)
        {
            return static_cast< ::cppu::OWeakObject* >(new XSvgParser(context));
        }
    } // end of namespace svgreader
} // end of namespace svgio

namespace svgio
{
    namespace svgreader
    {
        XSvgParser::XSvgParser(
            uno::Reference< uno::XComponentContext > const & context):
            context_(context)
        {
        }

        bool XSvgParser::parseSvgXML(uno::Reference<io::XInputStream> const & xSVGStream, uno::Reference<xml::sax::XDocumentHandler> const & xSvgDocHdl)
        {
            try
            {
                // prepare ParserInputSrouce
                xml::sax::InputSource myInputSource;
                myInputSource.aInputStream = xSVGStream;

                // get parser
                uno::Reference< xml::sax::XParser > xParser(
                    xml::sax::Parser::create(context_));
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
            catch(const uno::Exception& e)
            {
                SAL_WARN( "svg", "Parse error! : " << e);
                return false;
            }

            return true;
        }

        uno::Sequence< uno::Reference< ::graphic::XPrimitive2D > > XSvgParser::getDecomposition(
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

        uno::Any SAL_CALL XSvgParser::getDrawCommands(
                uno::Reference<io::XInputStream> const & xSvgStream,
                const OUString& aAbsolutePath)
        {
            uno::Any aAnyResult;

            if (!xSvgStream.is())
                return aAnyResult;

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
                    std::shared_ptr<gfx::DrawRoot> pDrawRoot(mpVisitor->getDrawRoot());
                    sal_uInt64 nPointer = reinterpret_cast<sal_uInt64>(pDrawRoot.get());
                    aAnyResult <<= sal_uInt64(nPointer);
                }
            }

            return aAnyResult;
        }

        OUString SAL_CALL XSvgParser::getImplementationName()
        {
            return XSvgParser_getImplementationName();
        }

        sal_Bool SAL_CALL XSvgParser::supportsService(const OUString& rServiceName)
        {
            return cppu::supportsService(this, rServiceName);
        }

        uno::Sequence< OUString > SAL_CALL XSvgParser::getSupportedServiceNames()
        {
            return XSvgParser_getSupportedServiceNames();
        }

    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
