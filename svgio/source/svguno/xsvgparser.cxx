/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svgio.hxx"

#include <com/sun/star/graphic/XSvgParser.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase2.hxx>
#include <svgio/svgreader/svgdocumenthandler.hxx>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <comphelper/processfactory.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class XSvgParser : public ::cppu::WeakAggImplHelper2< graphic::XSvgParser, lang::XServiceInfo >
        {
        private:
            XSvgParser(const XSvgParser&);
            XSvgParser& operator=(const XSvgParser&);

        protected:
        public:
            XSvgParser();
            virtual ~XSvgParser();

            // XSvgParser
            virtual uno::Sequence< uno::Reference< ::graphic::XPrimitive2D > > SAL_CALL getDecomposition(
                const uno::Reference< ::io::XInputStream >& xSVGStream,
                const ::rtl::OUString& aAbsolutePath) throw (uno::RuntimeException);

            // XServiceInfo
            virtual rtl::OUString SAL_CALL getImplementationName() throw(uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL supportsService(const rtl::OUString&) throw(uno::RuntimeException);
            virtual uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw(uno::RuntimeException);
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// uno functions

namespace svgio
{
    namespace svgreader
    {
        uno::Sequence< rtl::OUString > XSvgParser_getSupportedServiceNames()
        {
            static rtl::OUString aServiceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.graphic.SvgTools" ) );
            static uno::Sequence< rtl::OUString > aServiceNames( &aServiceName, 1 );

            return( aServiceNames );
        }

        rtl::OUString XSvgParser_getImplementationName()
        {
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "svgio::svgreader::XSvgParser" ) );
        }

        uno::Reference< uno::XInterface > SAL_CALL XSvgParser_createInstance(const uno::Reference< lang::XMultiServiceFactory >&)
        {
            return static_cast< ::cppu::OWeakObject* >(new XSvgParser);
        }
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        XSvgParser::XSvgParser()
        {
        }

        XSvgParser::~XSvgParser()
        {
        }

        uno::Sequence< uno::Reference< ::graphic::XPrimitive2D > > XSvgParser::getDecomposition(
            const uno::Reference< ::io::XInputStream >& xSVGStream,
            const ::rtl::OUString& aAbsolutePath ) throw (uno::RuntimeException)
        {
            drawinglayer::primitive2d::Primitive2DSequence aRetval;

            if(xSVGStream.is())
            {
                // local document handler
                SvgDocHdl* pSvgDocHdl = new SvgDocHdl(aAbsolutePath);
                uno::Reference< xml::sax::XDocumentHandler > xSvgDocHdl(pSvgDocHdl);

                try
                {
                    // prepare ParserInputSrouce
                    xml::sax::InputSource myInputSource;
                    myInputSource.aInputStream = xSVGStream;

                    // get parser
                    uno::Reference< xml::sax::XParser > xParser(
                        comphelper::getProcessServiceFactory()->createInstance(
                            rtl::OUString::createFromAscii("com.sun.star.xml.sax.Parser") ),
                        uno::UNO_QUERY_THROW );

                    // connect parser and filter
                    xParser->setDocumentHandler(xSvgDocHdl);

                    // finally, parse the stream to a hierarchy of
                    // SVGGraphicPrimitive2D which will be embedded to the
                    // primitive sequence. Their decompositions will in the
                    // end create local low-level primitives, thus SVG will
                    // be processable from all our processors
                    xParser->parseStream(myInputSource);
                }
                catch(uno::Exception&)
                {
                    OSL_ENSURE(false, "Parse error (!)");
                }

                // decompose to primitives
                const SvgNodeVector& rResults = pSvgDocHdl->getSvgDocument().getSvgNodeVector();
                const sal_uInt32 nCount(rResults.size());

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    SvgNode* pCandidate = rResults[a];

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

            return aRetval;
        }

        rtl::OUString SAL_CALL XSvgParser::getImplementationName() throw(uno::RuntimeException)
        {
            return(XSvgParser_getImplementationName());
        }

        sal_Bool SAL_CALL XSvgParser::supportsService(const rtl::OUString& rServiceName) throw(uno::RuntimeException)
        {
            const uno::Sequence< rtl::OUString > aServices(XSvgParser_getSupportedServiceNames());

            for(sal_Int32 nService(0); nService < aServices.getLength(); nService++)
            {
                if(rServiceName == aServices[nService])
                {
                    return sal_True;
                }
            }

            return sal_False;
        }

        uno::Sequence< rtl::OUString > SAL_CALL XSvgParser::getSupportedServiceNames() throw(uno::RuntimeException)
        {
            return XSvgParser_getSupportedServiceNames();
        }

    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof
