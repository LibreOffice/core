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

#ifndef INCLUDED_CPPCANVAS_SOURCE_INC_IMPLRENDERER_HXX
#define INCLUDED_CPPCANVAS_SOURCE_INC_IMPLRENDERER_HXX

#include <sal/config.h>

#include <basegfx/vector/b2dsize.hxx>
#include <sal/types.h>
#include <tools/stream.hxx>
#include <cppcanvas/renderer.hxx>
#include <cppcanvas/canvas.hxx>

#include "canvasgraphichelper.hxx"
#include "action.hxx"
#include "outdevstate.hxx"

#include <com/sun/star/rendering/FontRequest.hpp>
#include <com/sun/star/rendering/StrokeAttributes.hpp>
#include <osl/diagnose.h>
#include <osl/endian.h>

#include <map>
#include <memory>
#include <vector>

class GDIMetaFile;
class VirtualDevice;
class Gradient;
namespace tools { class Rectangle; }
namespace vcl { class Font; }
namespace tools { class PolyPolygon; }
class Point;
class MetaCommentAction;

namespace basegfx {
    class B2DPolyPolygon;
    class B2DPolygon;
}

namespace cppcanvas
{

    namespace internal
    {
        struct OutDevState;
        struct ActionFactoryParameters;
        struct XForm;

        struct EMFPObject
        {
            virtual ~EMFPObject() {}
        };

        // state stack of OutputDevice, to correctly handle
        // push/pop actions
        class VectorOfOutDevStates
        {
        public:
            OutDevState& getState();
            const OutDevState& getState() const;
            void pushState(PushFlags nFlags);
            void popState();
            void clearStateStack();
        private:
            std::vector< OutDevState > m_aStates;
        };

        // EMF+
        // Transformation matrix (used for Affine Transformation)
        //      [ eM11, eM12, eDx ]
        //      [ eM21, eM22, eDy ]
        //      [ 0,    0,    1   ]
        // that consists of a linear map (eM11, eM12, eM21, eM22)
        // More info: https://en.wikipedia.org/wiki/Linear_map
        // followed by a translation (eDx, eDy)

        struct XForm
        {
            float   eM11; // M1,1 value in the matrix. Increases or decreases the size of the pixels horizontally.
            float   eM12; // M1,2 value in the matrix. This effectively angles the X axis up or down.
            float   eM21; // M2,1 value in the matrix. This effectively angles the Y axis left or right.
            float   eM22; // M2,2 value in the matrix. Increases or decreases the size of the pixels vertically.
            float   eDx;  // Delta x (Dx) value in the matrix. Moves the whole coordinate system horizontally.
            float   eDy;  // Delta y (Dy) value in the matrix. Moves the whole coordinate system vertically.
            XForm()
            {
                SetIdentity ();
            }

            void SetIdentity ()
            {
                eM11 =  eM22 = 1.0f;
                eDx = eDy = eM12 = eM21 = 0.0f;
            }

            void Set (const XForm& f)
            {
                eM11 = f.eM11;
                eM12 = f.eM12;
                eM21 = f.eM21;
                eM22 = f.eM22;
                eDx  = f.eDx;
                eDy  = f.eDy;
            }

            // Multiple two square matrices
            //      [ eM11, eM12, eDx ]   [ f.eM11, f.eM12, f.eDx ]
            //      [ eM21, eM22, eDy ] x [ f.eM21, f.eM22, f.eDy ]
            //      [ 0,    0,    1   ]   [ 0,      0,      1     ]
            // More information: https://en.wikipedia.org/wiki/Matrix_multiplication#Square_matrices
            // FIXME We shouldn't modify source matrix during computation
            void Multiply (const XForm& f)
            {
                eM11 = eM11*f.eM11 + eM12*f.eM21;
                eM12 = eM11*f.eM12 + eM12*f.eM22;
                eM21 = eM21*f.eM11 + eM22*f.eM21;
                eM22 = eM21*f.eM12 + eM22*f.eM22;
                eDx  = eDx*f.eM11  + eDy*f.eM21 + f.eDx;
                eDy  = eDx*f.eM12  + eDy*f.eM22 + f.eDy;
            }

            friend SvStream& ReadXForm( SvStream& rIn, XForm& rXForm )
            {
                if ( sizeof( float ) != 4 )
                {
                    OSL_FAIL( "EnhWMFReader::sizeof( float ) != 4" );
                    rXForm = XForm();
                }
                else
                {
                    rIn.ReadFloat( rXForm.eM11 ).ReadFloat( rXForm.eM12 ).ReadFloat( rXForm.eM21 ).ReadFloat( rXForm.eM22 )
                       .ReadFloat( rXForm.eDx ).ReadFloat( rXForm.eDy );
                }
                return rIn;
            }
        };

        // EMF+
        typedef struct {
            XForm aWorldTransform;
            OutDevState aDevState;
        } EmfPlusGraphicState;

        typedef std::map<int,EmfPlusGraphicState> GraphicStateMap;

        class ImplRenderer : public virtual Renderer, protected CanvasGraphicHelper
        {
        public:
            ImplRenderer( const CanvasSharedPtr&    rCanvas,
                          const GDIMetaFile&        rMtf,
                          const Parameters&         rParms );

            virtual ~ImplRenderer() override;

            virtual bool                draw() const override;
            virtual bool                drawSubset( sal_Int32   nStartIndex,
                                                    sal_Int32   nEndIndex ) const override;
            virtual ::basegfx::B2DRange getSubsetArea( sal_Int32    nStartIndex,
                                                       sal_Int32    nEndIndex ) const override;


            // element of the Renderer's action vector. Need to be
            // public, since some functors need it, too.
            struct MtfAction
            {
                MtfAction( const std::shared_ptr<Action>&   rAction,
                           sal_Int32                nOrigIndex ) :
                    mpAction( rAction ),
                    mnOrigIndex( nOrigIndex )
                {
                }

                std::shared_ptr<Action> mpAction;
                sal_Int32       mnOrigIndex;
            };

            // prefetched and prepared canvas actions
            // (externally not visible)
            typedef std::vector< MtfAction >      ActionVector;

            /* EMF+ */
            static void ReadRectangle (SvStream& s, float& x, float& y, float &width, float& height, bool bCompressed = false);
            static void ReadPoint (SvStream& s, float& x, float& y, sal_uInt32 flags);
            void MapToDevice (double &x, double &y) const;
            ::basegfx::B2DPoint Map (double ix, double iy) const;
            ::basegfx::B2DSize MapSize (double iwidth, double iheight) const;
            void GraphicStatePush (GraphicStateMap& map, sal_Int32 index, OutDevState const & rState);
            void GraphicStatePop (GraphicStateMap& map, sal_Int32 index, OutDevState& rState);

        private:
            ImplRenderer(const ImplRenderer&) = delete;
            ImplRenderer& operator=( const ImplRenderer& ) = delete;

            static void updateClipping( const ::basegfx::B2DPolyPolygon&   rClipPoly,
                                 const ActionFactoryParameters&     rParms,
                                 bool                               bIntersect );

            static void updateClipping( const ::tools::Rectangle&                 rClipRect,
                                 const ActionFactoryParameters&     rParms,
                                 bool                               bIntersect );

            static css::uno::Reference<
                css::rendering::XCanvasFont > createFont( double&                         o_rFontRotation,
                                                          const vcl::Font&                rFont,
                                                          const ActionFactoryParameters&  rParms );
            bool createActions( GDIMetaFile&                    rMtf,
                                const ActionFactoryParameters&  rParms,
                                bool                            bSubsettableActions );
            bool createFillAndStroke( const ::basegfx::B2DPolyPolygon& rPolyPoly,
                                      const ActionFactoryParameters&   rParms );
            bool createFillAndStroke( const ::basegfx::B2DPolygon&   rPoly,
                                      const ActionFactoryParameters& rParms );
            static void skipContent( GDIMetaFile& rMtf,
                              const char*  pCommentString,
                              sal_Int32&   io_rCurrActionIndex );

            static bool isActionContained( GDIMetaFile& rMtf,
                                    const char*     pCommentString,
                                    MetaActionType  nType );

            void createGradientAction( const ::tools::PolyPolygon&    rPoly,
                                       const ::Gradient&              rGradient,
                                       const ActionFactoryParameters& rParms,
                                       bool                           bIsPolygonRectangle,
                                       bool                           bSubsettableActions );

            void createTextAction( const ::Point&                 rStartPoint,
                                   const OUString&                rString,
                                   int                            nIndex,
                                   int                            nLength,
                                   const long*                    pCharWidths,
                                   const ActionFactoryParameters& rParms,
                                   bool                           bSubsettable );

            bool getSubsetIndices( sal_Int32&                    io_rStartIndex,
                                   sal_Int32&                    io_rEndIndex,
                                   ActionVector::const_iterator& o_rRangeBegin,
                                   ActionVector::const_iterator& o_rRangeEnd ) const;

            void processObjectRecord(SvMemoryStream& rObjectStream, sal_uInt16 flags, sal_uInt32 dataSize, bool bUseWholeStream = false);

            /* EMF+ */
            void processEMFPlus( MetaCommentAction const * pAct, const ActionFactoryParameters& rFactoryParms, OutDevState& rState, const CanvasSharedPtr& rCanvas );
            double setFont(css::rendering::FontRequest& aFontRequest, sal_uInt8 fontObjectId, const ActionFactoryParameters& rParms, OutDevState& rState );

            /// Render LineCap, like the start or end arrow of a polygon.
            /// @return how much we should shorten the original polygon.
            double EMFPPlusDrawLineCap(const ::basegfx::B2DPolygon& rPolygon, double fPolyLength,
                    const ::basegfx::B2DPolyPolygon& rLineCap, bool isFilled, bool bStart,
                    const css::rendering::StrokeAttributes& rAttributes,
                    const ActionFactoryParameters& rParms, OutDevState& rState);

            void EMFPPlusDrawPolygon (const ::basegfx::B2DPolyPolygon & polygon, const ActionFactoryParameters& rParms, OutDevState& rState, const CanvasSharedPtr& rCanvas, sal_uInt32 penIndex);
            void EMFPPlusFillPolygon (::basegfx::B2DPolyPolygon const & polygon, const ActionFactoryParameters& rParms, OutDevState& rState, const CanvasSharedPtr& rCanvas, bool isColor, sal_uInt32 brushIndexOrColor);

            ActionVector maActions;

            /* EMF+ */
            XForm           aBaseTransform;
            XForm           aWorldTransform;
            std::unique_ptr<EMFPObject> aObjects [256];
            float           fPageScale;
            sal_Int32       nOriginX;
            sal_Int32       nOriginY;
            sal_Int32       nHDPI;
            sal_Int32       nVDPI;
            /* EMF+ emf header info */
            sal_Int32       nFrameLeft;
            sal_Int32       nFrameTop;
            sal_Int32       nFrameRight;
            sal_Int32       nFrameBottom;
            sal_Int32       nPixX;
            sal_Int32       nPixY;
            sal_Int32       nMmX;
            sal_Int32       nMmY;
            /* multipart object data */
            bool            mbMultipart;
            sal_uInt16      mMFlags;
            SvMemoryStream  mMStream;
            /* emf+ graphic state stack */
            GraphicStateMap mGSStack;
            GraphicStateMap mGSContainerStack;
        };


        /// Common parameters when creating actions
        struct ActionFactoryParameters
        {
            ActionFactoryParameters( VectorOfOutDevStates&       rStates,
                                     const CanvasSharedPtr&      rCanvas,
                                     ::VirtualDevice&            rVDev,
                                     const Renderer::Parameters& rParms,
                                     sal_Int32&                  io_rCurrActionIndex ) :
                mrStates(rStates),
                mrCanvas(rCanvas),
                mrVDev(rVDev),
                mrParms(rParms),
                mrCurrActionIndex(io_rCurrActionIndex)
            {}

            VectorOfOutDevStates&       mrStates;
            const CanvasSharedPtr&      mrCanvas;
            ::VirtualDevice&            mrVDev;
            const Renderer::Parameters& mrParms;
            sal_Int32&                  mrCurrActionIndex;
        };
    }
}

#endif // INCLUDED_CPPCANVAS_SOURCE_INC_IMPLRENDERER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
