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

#ifndef INCLUDED_CANVAS_SPRITEREDRAWMANAGER_HXX
#define INCLUDED_CANVAS_SPRITEREDRAWMANAGER_HXX

#include <basegfx/range/b2dconnectedranges.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2drectangle.hxx>

#include <vector>
#include <algorithm>

#include <canvas/base/sprite.hxx>
#include <canvas/canvastoolsdllapi.h>

/* Definition of SpriteRedrawManager class */

namespace canvas
{
    /** This class manages smooth SpriteCanvas updates

        Use this class to handle the ::canvas::SpriteSurface methods,
        that track and process sprite update events. Recorded update
        events are later grouped by connected areas (i.e. all sprites
        that somehow overlap over a rectangular area are grouped
        together); the forEachSpriteArea() method calls the passed
        functor for each of those connected areas.

        Note that, although this class generally works with IEEE
        doubles, the calculation of connected areas happens in the
        integer domain - it is generally expected that repaints can
        only be divided at pixel boundaries, without causing visible
        artifacts. Therefore, sprites that touch the same pixel (but
        don't necessarily have the same floating point coordinates
        there) will reside in a common sprite area and handled
        together in the forEachSpriteArea functor call.
     */
    class CANVASTOOLS_DLLPUBLIC SpriteRedrawManager
    {
    public:
        /** Data container for the connected components list
         */
        class SpriteInfo
        {
        public:
            ~SpriteInfo() {}

            /** Create sprite info

                @param rRef
                Sprite this info represents (might be the NULL ref)

                @param rTrueUpdateArea
                True (un-rounded) update area this sprite has recorded

                @param bNeedsUpdate
                When false, this sprite is not a member of the change
                record list. Thus, it only needs redraw if within the
                update area of other, changed sprites.

                @internal
             */
            SpriteInfo( const Sprite::Reference&    rRef,
                        const ::basegfx::B2DRange&  rTrueUpdateArea,
                        bool                        bNeedsUpdate ) :
                mpSprite( rRef ),
                maTrueUpdateArea( rTrueUpdateArea ),
                mbNeedsUpdate( bNeedsUpdate ),
                mbIsPureMove( false )
            {
            }

            /** Create sprite info, specify move type

                @param rRef
                Sprite this info represents (might be the NULL ref)

                @param rTrueUpdateArea
                True (un-rounded) update area this sprite has recorded

                @param bNeedsUpdate
                When false, this sprite is not a member of the change
                record list. Thus, it only needs redraw if within the
                update area of other, changed sprites.

                @param bIsPureMove
                When true, this sprite is _only_ moved, no other
                changes happened.

                @internal
             */
            SpriteInfo( const Sprite::Reference&    rRef,
                        const ::basegfx::B2DRange&  rTrueUpdateArea,
                        bool                        bNeedsUpdate,
                        bool                        bIsPureMove ) :
                mpSprite( rRef ),
                maTrueUpdateArea( rTrueUpdateArea ),
                mbNeedsUpdate( bNeedsUpdate ),
                mbIsPureMove( bIsPureMove )
            {
            }

            const Sprite::Reference&    getSprite() const { return mpSprite; }

            // #i61843# need to return by value here, to be used safely from bind
            const ::basegfx::B2DRange&  getUpdateArea() const { return maTrueUpdateArea; }
            bool                        needsUpdate() const { return mbNeedsUpdate; }
            bool                        isPureMove() const { return mbIsPureMove; }

        private:
            Sprite::Reference       mpSprite;
            ::basegfx::B2DRange     maTrueUpdateArea;
            bool                    mbNeedsUpdate;
            bool                    mbIsPureMove;
        };


        /** Helper struct for SpriteTracer template

            This struct stores change information to a sprite's visual
            appearance (move, content updated, and the like).
         */
        struct SpriteChangeRecord
        {
            enum class ChangeType { move, update };

            SpriteChangeRecord( const Sprite::Reference&    rSprite,
                                const ::basegfx::B2DPoint&  rOldPos,
                                const ::basegfx::B2DPoint&  rNewPos,
                                const ::basegfx::B2DVector& rSpriteSize ) :
                meChangeType( ChangeType::move ),
                mpAffectedSprite( rSprite ),
                maOldPos( rOldPos ),
                maUpdateArea( rNewPos.getX(),
                              rNewPos.getY(),
                              rNewPos.getX() + rSpriteSize.getX(),
                              rNewPos.getY() + rSpriteSize.getY() )
            {
            }

            SpriteChangeRecord( const Sprite::Reference&    rSprite,
                                const ::basegfx::B2DPoint&  rPos,
                                const ::basegfx::B2DRange&  rUpdateArea ) :
                meChangeType( ChangeType::update ),
                mpAffectedSprite( rSprite ),
                maOldPos( rPos ),
                maUpdateArea( rUpdateArea )
            {
            }

            const Sprite::Reference& getSprite() const { return mpAffectedSprite; }

            ChangeType          meChangeType;
            Sprite::Reference   mpAffectedSprite;
            ::basegfx::B2DPoint maOldPos;
            ::basegfx::B2DRange maUpdateArea;
        };

        typedef ::std::vector< SpriteChangeRecord >             VectorOfChangeRecords;
        typedef ::basegfx::B2DConnectedRanges< SpriteInfo >     SpriteConnectedRanges;
        typedef SpriteConnectedRanges::ComponentType            AreaComponent;
        typedef SpriteConnectedRanges::ConnectedComponents      UpdateArea;
        typedef ::std::vector< Sprite::Reference >              VectorOfSprites;

        SpriteRedrawManager();
        SpriteRedrawManager(const SpriteRedrawManager&) = delete;
        SpriteRedrawManager& operator=( const SpriteRedrawManager& ) = delete;

        /** Must be called when user of this object gets
            disposed. Frees all internal references.
         */
        void disposing();

        /** Functor, to be used from forEachSpriteArea
         */
        template< typename Functor > struct AreaUpdateCaller
        {
            AreaUpdateCaller( Functor&                      rFunc,
                              const SpriteRedrawManager&    rManager ) :
                mrFunc( rFunc ),
                mrManager( rManager )
            {
            }

            void operator()( const UpdateArea& rUpdateArea )
            {
                mrManager.handleArea( mrFunc, rUpdateArea );
            }

            Functor&                    mrFunc;
            const SpriteRedrawManager&  mrManager;
        };

        /** Call given functor for each sprite area that needs an
            update.

            This method calls the given functor for each update area
            (calculated from the sprite change records).

            @tpl Functor
            Must provide the following four methods:
            <pre>
            void backgroundPaint( ::basegfx::B2DRange aUpdateRect );
            void scrollUpdate( ::basegfx::B2DRange& o_rMoveStart,
                               ::basegfx::B2DRange& o_rMoveEnd,
                               UpdateArea           aUpdateArea );
            void opaqueUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                               const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites );
            void genericUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                                const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites );
            </pre>
            The backgroundPaint() method is called to simply repaint
            background content, the scrollUpdate() method is used to
            scroll a given area, and paint background in the uncovered
            areas, the opaqueUpdate() method is called when a sprite
            can be painted in the given area without taking background
            content into account, and finally, genericUpdate() is
            called for complex updates, where first the background and
            then all sprites consecutively have to be repainted.
         */
        template< typename Functor > void forEachSpriteArea( Functor& rFunc ) const
        {
            SpriteConnectedRanges aUpdateAreas;

            setupUpdateAreas( aUpdateAreas );

            aUpdateAreas.forEachAggregate(
                AreaUpdateCaller< Functor >( rFunc, *this ) );
        }

        /** Call given functor for each active sprite.

            This method calls the given functor for each active
            sprite, in the order of sprite priority.

            @tpl Functor
            Must provide a Functor::operator( Sprite::Reference& )
            method.
         */
        template< typename Functor > void forEachSprite( const Functor& rFunc ) const
        {
            ::std::for_each( maSprites.begin(),
                             maSprites.end(),
                             rFunc );
        }

        /// Clear sprite change records (typically directly after a screen update)
        void clearChangeRecords();

        // SpriteSurface interface, is delegated to e.g. from SpriteCanvas
        void showSprite( const Sprite::Reference& rSprite );
        void hideSprite( const Sprite::Reference& rSprite );
        void moveSprite( const Sprite::Reference&       rSprite,
                         const ::basegfx::B2DPoint&     rOldPos,
                         const ::basegfx::B2DPoint&     rNewPos,
                         const ::basegfx::B2DVector&    rSpriteSize );
        void updateSprite( const Sprite::Reference&     rSprite,
                           const ::basegfx::B2DPoint&   rPos,
                           const ::basegfx::B2DRange&   rUpdateArea );

        /** Internal, handles each distinct component for forEachAggregate()

            The reason why this must be public is that it needs to be
            accessible from the AreaUpdateCaller functor.

            @internal
         */
        template< typename Functor > void handleArea( Functor&          rFunc,
                                                      const UpdateArea& rUpdateArea ) const
        {
            // check whether this area contains changed sprites at all
            // (if not, just ignore it)
            if( areSpritesChanged( rUpdateArea ) )
            {
                // at least one of the sprites actually needs an
                // update - process whole area.

                // check whether this area could be handled special
                // (background paint, direct update, scroll, etc.)
                ::basegfx::B2DRange aMoveStart;
                ::basegfx::B2DRange aMoveEnd;
                if( rUpdateArea.maComponentList.empty() )
                {
                    rFunc.backgroundPaint( rUpdateArea.maTotalBounds );
                }
                else
                {
                    // cache number of sprites in this area (it's a
                    // list, and both isAreaUpdateScroll() and
                    // isAreaUpdateOpaque() need it).
                    const ::std::size_t nNumSprites(
                        rUpdateArea.maComponentList.size() );

                    if( isAreaUpdateScroll( aMoveStart,
                                            aMoveEnd,
                                            rUpdateArea,
                                            nNumSprites ) )
                    {
                        rFunc.scrollUpdate( aMoveStart,
                                            aMoveEnd,
                                            rUpdateArea );
                    }
                    else
                    {
                        // potentially, more than a single sprite
                        // involved. Have to sort component lists for
                        // sprite prio.
                        VectorOfSprites aSortedUpdateSprites;
                        for (auto const& elem : rUpdateArea.maComponentList)
                        {
                            const Sprite::Reference& rSprite( elem.second.getSprite() );
                            if( rSprite.is() )
                                aSortedUpdateSprites.push_back( rSprite );
                        }

                        ::std::sort( aSortedUpdateSprites.begin(),
                                     aSortedUpdateSprites.end(),
                                     SpriteWeakOrder() );

                        if( isAreaUpdateOpaque( rUpdateArea,
                                                nNumSprites ) )
                        {
                            rFunc.opaqueUpdate( rUpdateArea.maTotalBounds,
                                                aSortedUpdateSprites );
                        }
                        else
                        {
                            rFunc.genericUpdate( rUpdateArea.maTotalBounds,
                                                 aSortedUpdateSprites );
                        }
                    }
                }
            }
        }

    private:
        /** Central method of this class. Calculates the set of
            disjunct components that need an update.
         */
        void setupUpdateAreas( SpriteConnectedRanges& rUpdateAreas ) const;

        bool areSpritesChanged( const UpdateArea& rUpdateArea ) const;

        bool isAreaUpdateNotOpaque( const ::basegfx::B2DRectangle&  rUpdateRect,
                                    const AreaComponent&            rComponent ) const;

        bool isAreaUpdateOpaque( const UpdateArea&  rUpdateArea,
                                 ::std::size_t      nNumSprites ) const;

        /** Check whether given update area can be handled by a simple
            scroll

            @param o_rMoveStart
            Start rect of the move

            @param o_rMoveEnd
            End rect of the move. The content must be moved from start
            to end rect

            @param rUpdateArea
            Area to check for scroll update optimization
         */
        bool isAreaUpdateScroll( ::basegfx::B2DRectangle& o_rMoveStart,
                                 ::basegfx::B2DRectangle& o_rMoveEnd,
                                 const UpdateArea&      rUpdateArea,
                                 ::std::size_t          nNumSprites ) const;


        VectorOfSprites                 maSprites; // list of active
                                                   // sprite
                                                   // objects. this
                                                   // list is only
                                                   // used for full
                                                   // repaints,
                                                   // otherwise, we
                                                   // rely on the
                                                   // active sprites
                                                   // itself to notify
                                                   // us.

        VectorOfChangeRecords           maChangeRecords; // vector of
                                                         // sprites
                                                         // changes
                                                         // since last
                                                         // updateScreen()
                                                         // call
    };
}

#endif /* INCLUDED_CANVAS_SPRITEREDRAWMANAGER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
