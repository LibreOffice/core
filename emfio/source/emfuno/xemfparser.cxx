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
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wmfexternal.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <sal/log.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <wmfreader.hxx>
#include <emfreader.hxx>

using namespace ::com::sun::star;

namespace emfio::emfreader
{
        namespace {

        class XEmfParser : public ::cppu::WeakAggImplHelper2< graphic::XEmfParser, lang::XServiceInfo >
        {
        private:
            uno::Reference< uno::XComponentContext > context_;
            basegfx::B2DTuple maSizeHint;

        public:
            explicit XEmfParser(
                uno::Reference< uno::XComponentContext > const & context);
            XEmfParser(const XEmfParser&) = delete;
            XEmfParser& operator=(const XEmfParser&) = delete;

            // XEmfParser
            virtual uno::Sequence< uno::Reference< ::graphic::XPrimitive2D > > SAL_CALL getDecomposition(
                const uno::Reference< ::io::XInputStream >& xEmfStream,
                const OUString& aAbsolutePath,
                const uno::Sequence< ::beans::PropertyValue >& rProperties) override;
            void SAL_CALL setSizeHint(const geometry::RealPoint2D& rSize) override;

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName() override;
            virtual sal_Bool SAL_CALL supportsService(const OUString&) override;
            virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
        };

        }

        XEmfParser::XEmfParser(
            uno::Reference< uno::XComponentContext > const & context):
            context_(context)
        {
        }

        uno::Sequence< uno::Reference< ::graphic::XPrimitive2D > > XEmfParser::getDecomposition(
            const uno::Reference< ::io::XInputStream >& xEmfStream,
            const OUString& /*aAbsolutePath*/,
            const uno::Sequence< ::beans::PropertyValue >& rProperties)
        {
            drawinglayer::primitive2d::Primitive2DContainer aRetval;

            if (xEmfStream.is())
            {
                WmfExternal aExternalHeader;
                const bool bExternalHeaderUsed(aExternalHeader.setSequence(rProperties));
                bool bEnableEMFPlus = true;
                comphelper::SequenceAsHashMap aMap(rProperties);
                auto it = aMap.find("EMFPlusEnable");
                if (it != aMap.end())
                {
                    bool bValue;
                    if (it->second >>= bValue)
                    {
                        bEnableEMFPlus = bValue;
                    }
                }

                // rough check - import and conv to primitive
                GDIMetaFile aMtf;
                std::unique_ptr<SvStream> pStream(::utl::UcbStreamHelper::CreateStream(xEmfStream));
                sal_uInt32 nOrgPos = pStream->Tell();

                SvStreamEndian nOrigNumberFormat = pStream->GetEndian();
                pStream->SetEndian(SvStreamEndian::LITTLE);

                sal_uInt32 nMetaType(0);
                if (checkSeek(*pStream, 0x28))
                    pStream->ReadUInt32(nMetaType);
                pStream->Seek(nOrgPos);

                bool bReadError(false);

                try
                {
                    if (nMetaType == 0x464d4520)
                    {
                        // read and get possible failure/error, ReadEnhWMF returns success
                        emfio::EmfReader aReader(*pStream, aMtf);
                        aReader.SetSizeHint(maSizeHint);
                        if (!bEnableEMFPlus)
                        {
                            aReader.SetEnableEMFPlus(bEnableEMFPlus);
                        }
                        bReadError = !aReader.ReadEnhWMF();
                    }
                    else
                    {
                        emfio::WmfReader aReader(*pStream, aMtf, bExternalHeaderUsed ? &aExternalHeader : nullptr);
                        if (!bEnableEMFPlus)
                            aReader.SetEnableEMFPlus(bEnableEMFPlus);
                        aReader.ReadWMF();

                        // Need to check for ErrCode at stream to not lose former work.
                        // This may contain important information and will behave the
                        // same as before. When we have an error, do not create content
                        ErrCode aErrCode(pStream->GetError());

                        bReadError = aErrCode.IsError();
                    }
                }
                catch (...)
                {
                    bReadError = true;
                }

                pStream->SetEndian(nOrigNumberFormat);

                if (!bReadError)
                {
                    Size aSize(aMtf.GetPrefSize());

                    if (aMtf.GetPrefMapMode().GetMapUnit() == MapUnit::MapPixel)
                    {
                        aSize = Application::GetDefaultDevice()->PixelToLogic(aSize, MapMode(MapUnit::Map100thMM));
                    }
                    else
                    {
                        aSize = OutputDevice::LogicToLogic(aSize, aMtf.GetPrefMapMode(), MapMode(MapUnit::Map100thMM));
                    }

                    // use size
                    const basegfx::B2DHomMatrix aMetafileTransform(
                        basegfx::utils::createScaleB2DHomMatrix(
                            aSize.Width(),
                            aSize.Height()));

                    // ...and create a single MetafilePrimitive2D containing the Metafile.
                    // CAUTION: Currently, ReadWindowMetafile uses the local VectorGraphicData
                    // and a MetafileAccessor hook at the MetafilePrimitive2D inside of
                    // ImpGraphic::ImplGetGDIMetaFile to get the Metafile. Thus, the first
                    // and only primitive in this case *has to be* a MetafilePrimitive2D.
                    aRetval.push_back(
                        new drawinglayer::primitive2d::MetafilePrimitive2D(
                            aMetafileTransform,
                            aMtf));

                    // // force to use decomposition directly to get rid of the metafile
                    // const css::uno::Sequence< css::beans::PropertyValue > aViewParameters;
                    // drawinglayer::primitive2d::MetafilePrimitive2D aMetafilePrimitive2D(
                    //     aMetafileTransform,
                    //     aMtf);
                    // aRetval.append(aMetafilePrimitive2D.getDecomposition(aViewParameters));

                    // if (aRetval.empty())
                    // {
                    //     // for test, just create some graphic data that will get visualized
                    //     const basegfx::B2DRange aRange(1000, 1000, 5000, 5000);
                    //     const basegfx::BColor aColor(1.0, 0.0, 0.0);
                    //     const basegfx::B2DPolygon aOutline(basegfx::utils::createPolygonFromRect(aRange));
                    //
                    //     aRetval.push_back(new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aOutline), aColor));
                    // }
                }
            }
            else
            {
                SAL_WARN("emfio", "Invalid stream (!)");
            }

            return aRetval.toSequence();
        }

        void XEmfParser::setSizeHint(const geometry::RealPoint2D& rSize)
        {
            maSizeHint.setX(rSize.X);
            maSizeHint.setY(rSize.Y);
        }

        OUString SAL_CALL XEmfParser::getImplementationName()
        {
            return "emfio::emfreader::XEmfParser";
        }

        sal_Bool SAL_CALL XEmfParser::supportsService(const OUString& rServiceName)
        {
            return cppu::supportsService(this, rServiceName);
        }

        uno::Sequence< OUString > SAL_CALL XEmfParser::getSupportedServiceNames()
        {
            return { "com.sun.star.graphic.EmfTools" };
        }

} // end of namespace emfio::emfreader



extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
emfio_emfreader_XEmfParser_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new emfio::emfreader::XEmfParser(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
