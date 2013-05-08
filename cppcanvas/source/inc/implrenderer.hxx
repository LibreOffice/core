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

#ifndef _CPPCANVAS_IMPLRENDERER_HXX
#define _CPPCANVAS_IMPLRENDERER_HXX

#include <sal/types.h>

#include <boost/shared_ptr.hpp>
#include <cppcanvas/renderer.hxx>
#include <cppcanvas/canvas.hxx>

#include <canvasgraphichelper.hxx>
#include <action.hxx>
#include <outdevstate.hxx>

#include <vector>
#include <map>

class GDIMetaFile;
class VirtualDevice;
class Gradient;
class Rectangle;
class Font;
class PolyPolygon;
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
            void pushState(sal_uInt16 nFlags);
            void popState();
            void clearStateStack();
        private:
            ::std::vector< OutDevState > m_aStates;
        };

        // EMF+
        // TODO: replace?
        struct XForm
        {
            float   eM11;
            float   eM12;
            float   eM21;
            float   eM22;
            float   eDx;
            float   eDy;
            XForm()
            {
                SetIdentity ();
            };

            void SetIdentity ()
            {
                eM11 =  eM22 = 1.0f;
                eDx = eDy = eM12 = eM21 = 0.0f;
            }

            void Set (float m11, float m12, float dx, float m21, float m22, float dy)
            {
                eM11 = m11;
                eM12 = m12;
                eDx  = dx;
                eM21 = m21;
                eM22 = m22;
                eDy  = dy;
            }

            void Set (XForm f)
            {
                eM11 = f.eM11;
                eM12 = f.eM12;
                eM21 = f.eM21;
                eM22 = f.eM22;
                eDx  = f.eDx;
                eDy  = f.eDy;
            }

            void Multiply (float m11, float m12, float dx, float m21, float m22, float dy)
            {
                eM11 = eM11*m11 + eM12*m21;
                eM12 = eM11*m12 + eM12*m22;
                eM21 = eM21*m11 + eM22*m21;
                eM22 = eM21*m12 + eM22*m22;
                eDx *= eDx*m11  + eDy*m21 + dx;
                eDy *= eDx*m12  + eDy*m22 + dy;
            }

            void Multiply (XForm f)
            {
                eM11 = eM11*f.eM11 + eM12*f.eM21;
                eM12 = eM11*f.eM12 + eM12*f.eM22;
                eM21 = eM21*f.eM11 + eM22*f.eM21;
                eM22 = eM21*f.eM12 + eM22*f.eM22;
                eDx *= eDx*f.eM11  + eDy*f.eM21 + f.eDx;
                eDy *= eDx*f.eM12  + eDy*f.eM22 + f.eDy;
            }

#ifdef OSL_BIGENDIAN
// currently unused
static float GetSwapFloat( SvStream& rSt )
{
        float   fTmp;
        sal_Int8* pPtr = (sal_Int8*)&fTmp;
        rSt >> pPtr[3] >> pPtr[2] >> pPtr[1] >> pPtr[0];        // Little Endian <-> Big Endian switch
        return fTmp;
}
#endif

            friend SvStream& operator>>( SvStream& rIn, XForm& rXForm )
            {
                if ( sizeof( float ) != 4 )
                {
                    OSL_FAIL( "EnhWMFReader::sizeof( float ) != 4" );
                    rXForm = XForm();
                }
                else
                {
#ifdef OSL_BIGENDIAN
                    rXForm.eM11 = GetSwapFloat( rIn );
                    rXForm.eM12 = GetSwapFloat( rIn );
                    rXForm.eM21 = GetSwapFloat( rIn );
                    rXForm.eM22 = GetSwapFloat( rIn );
                    rXForm.eDx = GetSwapFloat( rIn );
                    rXForm.eDy = GetSwapFloat( rIn );
#else
                    rIn >> rXForm.eM11 >> rXForm.eM12 >> rXForm.eM21 >> rXForm.eM22
                        >> rXForm.eDx >> rXForm.eDy;
#endif
                }
                return rIn;
            }
        };

        // EMF+
        typedef struct {
            XForm aWorldTransform;
            OutDevState aDevState;
        } EmfPlusGraphicState;

        typedef ::std::map<int,EmfPlusGraphicState> GraphicStateMap;

        class ImplRenderer : public virtual Renderer, protected CanvasGraphicHelper
        {
        public:
            ImplRenderer( const CanvasSharedPtr&    rCanvas,
                          const GDIMetaFile&        rMtf,
                          const Parameters&         rParms );

            virtual ~ImplRenderer();

            virtual bool                draw() const;
            virtual bool                drawSubset( sal_Int32   nStartIndex,
                                                    sal_Int32   nEndIndex ) const;
            virtual ::basegfx::B2DRange getSubsetArea( sal_Int32    nStartIndex,
                                                       sal_Int32    nEndIndex ) const;


            // element of the Renderer's action vector. Need to be
            // public, since some functors need it, too.
            struct MtfAction
            {
                MtfAction( const ActionSharedPtr&   rAction,
                           sal_Int32                nOrigIndex ) :
                    mpAction( rAction ),
                    mnOrigIndex( nOrigIndex )
                {
                }

                ActionSharedPtr mpAction;
                sal_Int32       mnOrigIndex;
            };

            // prefetched and prepared canvas actions
            // (externally not visible)
            typedef ::std::vector< MtfAction >      ActionVector;

            /* EMF+ */
            void ReadRectangle (SvStream& s, float& x, float& y, float &width, float& height, bool bCompressed = false);
            void ReadPoint (SvStream& s, float& x, float& y, sal_uInt32 flags);
            void MapToDevice (double &x, double &y);
            ::basegfx::B2DPoint Map (double ix, double iy);
            ::basegfx::B2DSize MapSize (double iwidth, double iheight);
            void GraphicStatePush (GraphicStateMap& map, sal_Int32 index, OutDevState& rState);
            void GraphicStatePop (GraphicStateMap& map, sal_Int32 index, OutDevState& rState);

        private:
            // default: disabled copy/assignment
            ImplRenderer(const ImplRenderer&);
            ImplRenderer& operator=( const ImplRenderer& );

            void updateClipping( const ::basegfx::B2DPolyPolygon&   rClipPoly,
                                 const ActionFactoryParameters&     rParms,
                                 bool                               bIntersect );

            void updateClipping( const ::Rectangle&                 rClipRect,
                                 const ActionFactoryParameters&     rParms,
                                 bool                               bIntersect );

            ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XCanvasFont > createFont( double&                         o_rFontRotation,
                                                                       const ::Font&                   rFont,
                                                                       const ActionFactoryParameters&  rParms ) const;
            bool createActions( GDIMetaFile&                    rMtf,
                                const ActionFactoryParameters&  rParms,
                                bool                            bSubsettableActions );
            bool createFillAndStroke( const ::basegfx::B2DPolyPolygon& rPolyPoly,
                                      const ActionFactoryParameters&   rParms );
            bool createFillAndStroke( const ::basegfx::B2DPolygon&   rPoly,
                                      const ActionFactoryParameters& rParms );
            void skipContent( GDIMetaFile& rMtf,
                              const char*  pCommentString,
                              sal_Int32&   io_rCurrActionIndex ) const;

            bool isActionContained( GDIMetaFile& rMtf,
                                    const char*  pCommentString,
                                    sal_uInt16       nType ) const;

            void createGradientAction( const ::PolyPolygon&           rPoly,
                                       const ::Gradient&              rGradient,
                                       const ActionFactoryParameters& rParms,
                                       bool                           bIsPolygonRectangle,
                                       bool                           bSubsettableActions );

            void createTextAction( const ::Point&                 rStartPoint,
                                   const String                   rString,
                                   int                            nIndex,
                                   int                            nLength,
                                   const sal_Int32*               pCharWidths,
                                   const ActionFactoryParameters& rParms,
                                   bool                           bSubsettable );

            bool getSubsetIndices( sal_Int32&                    io_rStartIndex,
                                   sal_Int32&                    io_rEndIndex,
                                   ActionVector::const_iterator& o_rRangeBegin,
                                   ActionVector::const_iterator& o_rRangeEnd ) const;

            void processObjectRecord(SvMemoryStream& rObjectStream, sal_uInt16 flags, sal_uInt32 dataSize, sal_Bool bUseWholeStream = sal_False);

            /* EMF+ */
            void processEMFPlus( MetaCommentAction* pAct, const ActionFactoryParameters& rFactoryParms, OutDevState& rState, const CanvasSharedPtr& rCanvas );
            double setFont( sal_uInt8 objectId, const ActionFactoryParameters& rParms, OutDevState& rState );
            void EMFPPlusDrawPolygon (::basegfx::B2DPolyPolygon& polygon, const ActionFactoryParameters& rParms, OutDevState& rState, const CanvasSharedPtr& rCanvas, sal_uInt32 penIndex);
            void EMFPPlusFillPolygon (::basegfx::B2DPolyPolygon& polygon, const ActionFactoryParameters& rParms, OutDevState& rState, const CanvasSharedPtr& rCanvas, bool isColor, sal_uInt32 brushIndexOrColor);

            ActionVector maActions;

            /* EMF+ */
            XForm           aBaseTransform;
            XForm           aWorldTransform;
            EMFPObject*     aObjects [256];
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

#endif /* _CPPCANVAS_IMPLRENDERER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
