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

#include <com/sun/star/graphic/XEmfParser.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <vcl/wmf.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>

//#include <com/sun/star/xml/sax/XParser.hpp>
//#include <com/sun/star/xml/sax/Parser.hpp>
//#include <com/sun/star/xml/sax/InputSource.hpp>
//#include <drawinglayer/geometry/viewinformation2d.hxx>
//#include <svgdocumenthandler.hxx>

#include "xemfparser.hxx"

using namespace ::com::sun::star;

namespace emfio
{
    namespace emfreader
    {
        class XEmfParser : public ::cppu::WeakAggImplHelper2< graphic::XEmfParser, lang::XServiceInfo >
        {
        private:
            uno::Reference< uno::XComponentContext > context_;

        protected:
        public:
            explicit XEmfParser(
                uno::Reference< uno::XComponentContext > const & context);
            XEmfParser(const XEmfParser&) = delete;
            XEmfParser& operator=(const XEmfParser&) = delete;

            // XEmfParser
            virtual uno::Sequence< uno::Reference< ::graphic::XPrimitive2D > > SAL_CALL getDecomposition(
                const uno::Reference< ::io::XInputStream >& xEmfStream,
                const OUString& aAbsolutePath) override;

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName() override;
            virtual sal_Bool SAL_CALL supportsService(const OUString&) override;
            virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
        };
    } // end of namespace emfreader
} // end of namespace emfio

// uno functions
namespace emfio
{
    namespace emfreader
    {
        uno::Sequence< OUString > XEmfParser_getSupportedServiceNames()
        {
            return uno::Sequence< OUString > { "com.sun.star.graphic.EmfTools" };
        }

        OUString XEmfParser_getImplementationName()
        {
            return OUString( "emfio::emfreader::XEmfParser" );
        }

        uno::Reference< uno::XInterface > SAL_CALL XEmfParser_createInstance(const uno::Reference< uno::XComponentContext >& context)
        {
            return static_cast< ::cppu::OWeakObject* >(new XEmfParser(context));
        }
    } // end of namespace emfreader
} // end of namespace emfio

namespace emfio
{
    namespace emfreader
    {
        XEmfParser::XEmfParser(
            uno::Reference< uno::XComponentContext > const & context):
            context_(context)
        {
        }

        uno::Sequence< uno::Reference< ::graphic::XPrimitive2D > > XEmfParser::getDecomposition(
            const uno::Reference< ::io::XInputStream >& xEmfStream,
            const OUString& aAbsolutePath )
        {
            drawinglayer::primitive2d::Primitive2DContainer aRetval;

            if (xEmfStream.is())
            {
                static bool bTestCode(true);

                if (bTestCode)
                {
                    static bool bUseOldFilterEmbedded(true);

                    if (bUseOldFilterEmbedded)
                    {
                        GDIMetaFile aMtf;
                        std::unique_ptr<SvStream> pStream(::utl::UcbStreamHelper::CreateStream(xEmfStream));

                        if (pStream && ConvertWMFToGDIMetaFile(*pStream, aMtf, nullptr, nullptr))
                        {
                            const basegfx::B2DHomMatrix aMetafileTransform(
                                basegfx::tools::createScaleTranslateB2DHomMatrix(
                                    5000.0, 5000.0,
                                    1000.0, 1000.0));

                            aRetval.push_back(
                                new drawinglayer::primitive2d::MetafilePrimitive2D(
                                    aMetafileTransform,
                                    aMtf));
                        }
                    }

                    if(aRetval.empty())
                    {
                        // for test, just create some graphic data that will get visualized
                        const basegfx::B2DRange aRange(1000, 1000, 5000, 5000);
                        const basegfx::BColor aColor(1.0, 0.0, 0.0);
                        const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aRange));

                        aRetval.push_back(new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aOutline), aColor));
                    }
                }
                else
                {
                    // new parser here
                    bool bBla = true;




                    //                // local document handler
                    //                SvgDocHdl* pSvgDocHdl = new SvgDocHdl(aAbsolutePath);
                    //                uno::Reference< xml::sax::XDocumentHandler > xSvgDocHdl(pSvgDocHdl);
                    //
                    //                try
                    //                {
                    //                    // prepare ParserInputSrouce
                    //                    xml::sax::InputSource myInputSource;
                    //                    myInputSource.aInputStream = xEmfStream;
                    //
                    //                    // get parser
                    //                    uno::Reference< xml::sax::XParser > xParser(
                    //                        xml::sax::Parser::create(context_));
                    //                    // fdo#60471 need to enable internal entities because
                    //                    // certain ... popular proprietary products write SVG files
                    //                    // that use entities to define XML namespaces.
                    //                    uno::Reference<lang::XInitialization> const xInit(xParser,
                    //                            uno::UNO_QUERY_THROW);
                    //                    uno::Sequence<uno::Any> args(1);
                    //                    args[0] <<= OUString("DoSmeplease");
                    //                    xInit->initialize(args);
                    //
                    //                    // connect parser and filter
                    //                    xParser->setDocumentHandler(xSvgDocHdl);
                    //
                    //                    // finally, parse the stream to a hierarchy of
                    //                    // SVGGraphicPrimitive2D which will be embedded to the
                    //                    // primitive sequence. Their decompositions will in the
                    //                    // end create local low-level primitives, thus SVG will
                    //                    // be processable from all our processors
                    //                    xParser->parseStream(myInputSource);
                    //                }
                    //                catch(const uno::Exception& e)
                    //                {
                    //                    SAL_WARN( "svg", "Parse error! : " << e.Message);
                    //                }
                    //
                    //                // decompose to primitives
                    //                const SvgNodeVector& rResults = pSvgDocHdl->getSvgDocument().getSvgNodeVector();
                    //                const sal_uInt32 nCount(rResults.size());
                    //
                    //                for(sal_uInt32 a(0); a < nCount; a++)
                    //                {
                    //                    SvgNode* pCandidate = rResults[a];
                    //
                    //                    if(Display_none != pCandidate->getDisplay())
                    //                    {
                    //                        pCandidate->decomposeSvgNode(aRetval, false);
                    //                    }
                    //                }
                }
            }
            else
            {
                OSL_ENSURE(false, "Invalid stream (!)");
            }

            return comphelper::containerToSequence(aRetval);
        }

        OUString SAL_CALL XEmfParser::getImplementationName()
        {
            return(XEmfParser_getImplementationName());
        }

        sal_Bool SAL_CALL XEmfParser::supportsService(const OUString& rServiceName)
        {
            return cppu::supportsService(this, rServiceName);
        }

        uno::Sequence< OUString > SAL_CALL XEmfParser::getSupportedServiceNames()
        {
            return XEmfParser_getSupportedServiceNames();
        }

    } // end of namespace emfreader
} // end of namespace emfio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
