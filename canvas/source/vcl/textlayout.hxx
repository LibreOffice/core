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

#ifndef INCLUDED_CANVAS_SOURCE_VCL_TEXTLAYOUT_HXX
#define INCLUDED_CANVAS_SOURCE_VCL_TEXTLAYOUT_HXX

#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/StringContext.hpp>
#include <com/sun/star/rendering/XTextLayout.hpp>

#include <canvas/vclwrapper.hxx>

#include "canvasfont.hxx"
#include "impltools.hxx"

#include <boost/noncopyable.hpp>


/* Definition of TextLayout class */

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper< css::rendering::XTextLayout,
                                             css::lang::XServiceInfo > TextLayout_Base;

    class TextLayout : public ::comphelper::OBaseMutex,
                       public TextLayout_Base,
                       private ::boost::noncopyable
    {
    public:
        TextLayout( const css::rendering::StringContext&                aText,
                    sal_Int8                                            nDirection,
                    sal_Int64                                           nRandomSeed,
                    const CanvasFont::Reference&                        rFont,
                    const css::uno::Reference<
                          css::rendering::XGraphicDevice>&              xDevice,
                    const OutDevProviderSharedPtr&                      rOutDev );

        /// Dispose all internal references
        virtual void SAL_CALL disposing() override;

        // XTextLayout
        virtual css::uno::Sequence< css::uno::Reference< css::rendering::XPolyPolygon2D > > SAL_CALL queryTextShapes(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::geometry::RealRectangle2D > SAL_CALL queryInkMeasures(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::geometry::RealRectangle2D > SAL_CALL queryMeasures(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< double > SAL_CALL queryLogicalAdvancements(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL applyLogicalAdvancements( const css::uno::Sequence< double >& aAdvancements ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::geometry::RealRectangle2D SAL_CALL queryTextBounds(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual double SAL_CALL justify( double nSize ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual double SAL_CALL combinedJustify( const css::uno::Sequence< css::uno::Reference< css::rendering::XTextLayout > >& aNextLayouts, double nSize ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::rendering::TextHit SAL_CALL getTextHit( const css::geometry::RealPoint2D& aHitPoint ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::rendering::Caret SAL_CALL getCaret( sal_Int32 nInsertionIndex, sal_Bool bExcludeLigatures ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getNextInsertionIndex( sal_Int32 nStartIndex, sal_Int32 nCaretAdvancement, sal_Bool bExcludeLigatures ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::rendering::XPolyPolygon2D > SAL_CALL queryVisualHighlighting( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::rendering::XPolyPolygon2D > SAL_CALL queryLogicalHighlighting( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual double SAL_CALL getBaselineOffset(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int8 SAL_CALL getMainTextDirection(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::rendering::XCanvasFont > SAL_CALL getFont(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::rendering::StringContext SAL_CALL getText(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw( css::uno::RuntimeException, std::exception ) override;

        bool draw( OutputDevice&                                   rOutDev,
                   const Point&                                    rOutpos,
                   const css::rendering::ViewState&                viewState,
                   const css::rendering::RenderState&              renderState ) const;

    private:
        void setupTextOffsets( long* outputOffsets,
                               const css::uno::Sequence< double >& inputOffsets,
                               const css::rendering::ViewState&    viewState,
                               const css::rendering::RenderState&  renderState     ) const;

        css::rendering::StringContext                    maText;
        css::uno::Sequence< double >                     maLogicalAdvancements;
        CanvasFont::Reference                            mpFont;
        css::uno::Reference< css::rendering::XGraphicDevice> mxDevice;
        OutDevProviderSharedPtr                          mpOutDevProvider;
        sal_Int8                                         mnTextDirection;
    };

}

#endif // INCLUDED_CANVAS_SOURCE_VCL_TEXTLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
