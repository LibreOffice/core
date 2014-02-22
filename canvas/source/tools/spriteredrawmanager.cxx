/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/spriteredrawmanager.hxx>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/vector/b2dsize.hxx>

#include <algorithm>
#include <o3tl/compat_functional.hxx>
#include <boost/bind.hpp>


namespace canvas
{
    namespace
    {
        /** Helper class to condense sprite updates into a single action

            This class tracks the sprite changes over the recorded
            change list, and generates a single update action from
            that (note that per screen update, several moves,
            visibility changes and content updates might happen)
         */
        class SpriteTracer
        {
        public:
            SpriteTracer( const Sprite::Reference& rAffectedSprite ) :
                mpAffectedSprite(rAffectedSprite),
                maMoveStartArea(),
                maMoveEndArea(),
                mbIsMove( false ),
                mbIsGenericUpdate( false )
            {
            }

            void operator()( const SpriteRedrawManager::SpriteChangeRecord& rSpriteRecord )
            {
                
                
                if( rSpriteRecord.mpAffectedSprite == mpAffectedSprite )
                {
                    switch( rSpriteRecord.meChangeType )
                    {
                        case SpriteRedrawManager::SpriteChangeRecord::move:
                            if( !mbIsMove )
                            {
                                
                                maMoveStartArea = ::basegfx::B2DRectangle(
                                    rSpriteRecord.maOldPos,
                                    rSpriteRecord.maOldPos + rSpriteRecord.maUpdateArea.getRange() );
                                mbIsMove        = true;
                            }

                            maMoveEndArea   = rSpriteRecord.maUpdateArea;
                            break;

                        case SpriteRedrawManager::SpriteChangeRecord::update:
                            
                            
                            
                            
                            
                            
                            
                            
                            maMoveEndArea.expand( rSpriteRecord.maUpdateArea );
                            mbIsGenericUpdate = true;
                            break;

                        default:
                            ENSURE_OR_THROW( false,
                                              "Unexpected case in SpriteUpdater::operator()" );
                            break;
                    }
                }
            }

            void commit( SpriteRedrawManager::SpriteConnectedRanges& rUpdateCollector ) const
            {
                if( mbIsMove )
                {
                    if( !maMoveStartArea.isEmpty() ||
                        !maMoveEndArea.isEmpty() )
                    {
                        
                        
                        
                        
                        const bool bIsPureMove( !mbIsGenericUpdate );

                        
                        
                        
                        
                        

                        
                        
                        

                        
                        rUpdateCollector.addRange(
                            ::basegfx::unotools::b2DSurroundingIntegerRangeFromB2DRange( maMoveEndArea ),
                            SpriteRedrawManager::SpriteInfo(
                                mpAffectedSprite,
                                maMoveEndArea,
                                true,
                                bIsPureMove ) );

                        
                        
                        rUpdateCollector.addRange(
                            ::basegfx::unotools::b2DSurroundingIntegerRangeFromB2DRange( maMoveStartArea ),
                            SpriteRedrawManager::SpriteInfo(
                                Sprite::Reference(),
                                maMoveStartArea,
                                true,
                                bIsPureMove ) );
                    }
                }
                else if( mbIsGenericUpdate &&
                         !maMoveEndArea.isEmpty() )
                {
                    rUpdateCollector.addRange(
                        ::basegfx::unotools::b2DSurroundingIntegerRangeFromB2DRange( maMoveEndArea ),
                        SpriteRedrawManager::SpriteInfo(
                            mpAffectedSprite,
                            maMoveEndArea,
                            true ) );
                }
            }

        private:
            Sprite::Reference       mpAffectedSprite;
            ::basegfx::B2DRectangle maMoveStartArea;
            ::basegfx::B2DRectangle maMoveEndArea;

            
            bool                    mbIsMove;

            
            bool                    mbIsGenericUpdate;
        };


        /** SpriteChecker functor, which for every sprite checks the
            given update vector for necessary screen updates
         */
        class SpriteUpdater
        {
        public:
            /** Generate update area list

                @param rUpdater
                Reference to an updater object, which will receive the
                update areas.

                @param rChangeContainer
                Container with all sprite change requests

             */
            SpriteUpdater( SpriteRedrawManager::SpriteConnectedRanges&          rUpdater,
                           const SpriteRedrawManager::VectorOfChangeRecords&    rChangeContainer ) :
                mrUpdater( rUpdater ),
                mrChangeContainer( rChangeContainer )
            {
            }

            /** Call this method for every sprite on your screen

                This method scans the change container, collecting all
                update info for the given sprite into one or two
                update operations, which in turn are inserted into the
                connected ranges processor.

                @param rSprite
                Current sprite to collect update info for.
             */
            void operator()( const Sprite::Reference& rSprite )
            {
                const SpriteTracer aSpriteTracer(
                    ::std::for_each( mrChangeContainer.begin(),
                                     mrChangeContainer.end(),
                                     SpriteTracer( rSprite ) ) );

                aSpriteTracer.commit( mrUpdater );
            }

        private:
            SpriteRedrawManager::SpriteConnectedRanges&         mrUpdater;
            const SpriteRedrawManager::VectorOfChangeRecords&   mrChangeContainer;
        };
    }

    void SpriteRedrawManager::setupUpdateAreas( SpriteConnectedRanges& rUpdateAreas ) const
    {
        
        
        
        
        
        
        
        
        
        SpriteWeakOrder aSpriteComparator;

        
        ListOfSprites::const_iterator       aCurrSprite( maSprites.begin() );
        const ListOfSprites::const_iterator aEndSprite ( maSprites.end() );
        while( aCurrSprite != aEndSprite )
        {
            if( (*aCurrSprite)->isContentChanged() )
                const_cast<SpriteRedrawManager*>(this)->updateSprite( *aCurrSprite,
                                                                      (*aCurrSprite)->getPosPixel(),
                                                                      (*aCurrSprite)->getUpdateArea() );
            ++aCurrSprite;
        }

        
        VectorOfSprites aSortedSpriteVector;
        ::std::copy( maSprites.begin(),
                     maSprites.end(),
                     ::std::back_insert_iterator< VectorOfSprites >(aSortedSpriteVector) );
        ::std::sort( aSortedSpriteVector.begin(),
                     aSortedSpriteVector.end(),
                     aSpriteComparator );

        
        
        
        
        VectorOfSprites aUpdatableSprites;
        VectorOfChangeRecords::const_iterator       aCurrRecord( maChangeRecords.begin() );
        const VectorOfChangeRecords::const_iterator aEndRecords( maChangeRecords.end() );
        while( aCurrRecord != aEndRecords )
        {
            const Sprite::Reference& rSprite( aCurrRecord->getSprite() );
            if( rSprite.is() )
                aUpdatableSprites.push_back( rSprite );
            ++aCurrRecord;
        }

        ::std::sort( aUpdatableSprites.begin(),
                     aUpdatableSprites.end(),
                     aSpriteComparator );

        VectorOfSprites::iterator aEnd=
            ::std::unique( aUpdatableSprites.begin(),
                           aUpdatableSprites.end() );

        
        
        
        ::std::for_each( aUpdatableSprites.begin(),
                         aEnd,
                         SpriteUpdater( rUpdateAreas,
                                        maChangeRecords) );

        
        
        

        
        
        
        
        VectorOfSprites aUnchangedSprites;
        ::std::set_difference( aSortedSpriteVector.begin(),
                               aSortedSpriteVector.end(),
                               aUpdatableSprites.begin(),
                               aEnd,
                               ::std::back_insert_iterator< VectorOfSprites >(aUnchangedSprites),
                               aSpriteComparator );

        
        
        VectorOfSprites::const_iterator         aCurr( aUnchangedSprites.begin() );
        const VectorOfSprites::const_iterator   aEnd2( aUnchangedSprites.end() );
        while( aCurr != aEnd2 )
        {
            const ::basegfx::B2DRange& rUpdateArea( (*aCurr)->getUpdateArea() );
            rUpdateAreas.addRange(
                ::basegfx::unotools::b2DSurroundingIntegerRangeFromB2DRange( rUpdateArea ),
                SpriteInfo(*aCurr,
                           rUpdateArea,
                           false) );
            ++aCurr;
        }
    }

#if OSL_DEBUG_LEVEL > 0
    bool impIsEqualB2DRange(const basegfx::B2DRange& rRangeA, const basegfx::B2DRange& rRangeB, double fSmallValue)
    {
        return fabs(rRangeB.getMinX() - rRangeA.getMinX()) <= fSmallValue
            && fabs(rRangeB.getMinY() - rRangeA.getMinY()) <= fSmallValue
            && fabs(rRangeB.getMaxX() - rRangeA.getMaxX()) <= fSmallValue
            && fabs(rRangeB.getMaxY() - rRangeA.getMaxY()) <= fSmallValue;
    }

    bool impIsEqualB2DVector(const basegfx::B2DVector& rVecA, const basegfx::B2DVector& rVecB, double fSmallValue)
    {
        return fabs(rVecB.getX() - rVecA.getX()) <= fSmallValue
            && fabs(rVecB.getY() - rVecA.getY()) <= fSmallValue;
    }
#endif

    bool SpriteRedrawManager::isAreaUpdateScroll( ::basegfx::B2DRectangle&  o_rMoveStart,
                                                  ::basegfx::B2DRectangle&  o_rMoveEnd,
                                                  const UpdateArea&         rUpdateArea,
                                                  ::std::size_t             nNumSprites ) const
    {
        
        
        
        
        
        
        
        
        if( nNumSprites != 2 )
            return false;

        const SpriteConnectedRanges::ComponentListType::const_iterator aFirst(
            rUpdateArea.maComponentList.begin() );
        SpriteConnectedRanges::ComponentListType::const_iterator aSecond(
            aFirst ); ++aSecond;

        if( !aFirst->second.isPureMove() ||
            !aSecond->second.isPureMove() ||
            !aFirst->second.getSprite().is() ||
            
            !aFirst->second.getSprite()->isAreaUpdateOpaque( aFirst->second.getUpdateArea() ) ||
            aSecond->second.getSprite().is() )
        {
            
            
            return false;
        }

        o_rMoveStart      = aSecond->second.getUpdateArea();
        o_rMoveEnd        = aFirst->second.getUpdateArea();

#if OSL_DEBUG_LEVEL > 0
        ::basegfx::B2DRectangle aTotalBounds( o_rMoveStart );
        aTotalBounds.expand( o_rMoveEnd );

        OSL_POSTCOND(impIsEqualB2DRange(rUpdateArea.maTotalBounds, basegfx::unotools::b2DSurroundingIntegerRangeFromB2DRange(aTotalBounds), 0.5),
            "SpriteRedrawManager::isAreaUpdateScroll(): sprite area and total area mismatch");
        OSL_POSTCOND(impIsEqualB2DVector(o_rMoveStart.getRange(), o_rMoveEnd.getRange(), 0.5),
            "SpriteRedrawManager::isAreaUpdateScroll(): scroll start and end area have mismatching size");
#endif

        return true;
    }

    bool SpriteRedrawManager::isAreaUpdateNotOpaque( const ::basegfx::B2DRectangle& rUpdateRect,
                                                     const AreaComponent&           rComponent ) const
    {
        const Sprite::Reference& pAffectedSprite( rComponent.second.getSprite() );

        if( !pAffectedSprite.is() )
            return true; 

        return !pAffectedSprite->isAreaUpdateOpaque( rUpdateRect );
    }

    bool SpriteRedrawManager::isAreaUpdateOpaque( const UpdateArea& rUpdateArea,
                                                  ::std::size_t     nNumSprites ) const
    {
        
        
        

        
        
        
        

        
        
        
        if( nNumSprites > 3 || nNumSprites < 1 )
            return false;

        
        
        ::basegfx::B2DRange aTrueArea( rUpdateArea.maComponentList.begin()->second.getUpdateArea() );
        ::std::for_each( rUpdateArea.maComponentList.begin(),
                         rUpdateArea.maComponentList.end(),
                         ::boost::bind( (void (basegfx::B2DRange::*)(const basegfx::B2DRange&))(
                                            &basegfx::B2DRange::expand),
                                        aTrueArea,
                                        ::boost::bind( &SpriteInfo::getUpdateArea,
                                                       ::boost::bind( ::o3tl::select2nd<AreaComponent>(),
                                                                      _1 ) ) ) );

        const SpriteConnectedRanges::ComponentListType::const_iterator aEnd(
            rUpdateArea.maComponentList.end() );

        
        
        return (::std::find_if( rUpdateArea.maComponentList.begin(),
                                aEnd,
                                ::boost::bind( &SpriteRedrawManager::isAreaUpdateNotOpaque,
                                               this,
                                               ::boost::cref(aTrueArea),
                                               _1 ) ) == aEnd );
    }

    bool SpriteRedrawManager::areSpritesChanged( const UpdateArea& rUpdateArea ) const
    {
        
        
        //
        
        
        const SpriteConnectedRanges::ComponentListType::const_iterator aEnd(
            rUpdateArea.maComponentList.end() );
        return (::std::find_if( rUpdateArea.maComponentList.begin(),
                                aEnd,
                                ::boost::bind( &SpriteInfo::needsUpdate,
                                               ::boost::bind(
                                                   ::o3tl::select2nd<SpriteConnectedRanges::ComponentType>(),
                                                   _1 ) ) ) != aEnd );
    }

    SpriteRedrawManager::SpriteRedrawManager() :
        maSprites(),
        maChangeRecords()
    {
    }

    void SpriteRedrawManager::disposing()
    {
        
        maChangeRecords.clear();

        
        
        
        

        
        
        
        ListOfSprites::reverse_iterator aCurr( maSprites.rbegin() );
        ListOfSprites::reverse_iterator aEnd( maSprites.rend() );
        while( aCurr != aEnd )
            (*aCurr++)->dispose();

        maSprites.clear();
    }

    void SpriteRedrawManager::clearChangeRecords()
    {
        maChangeRecords.clear();
    }

    void SpriteRedrawManager::showSprite( const Sprite::Reference& rSprite )
    {
        maSprites.push_back( rSprite );
    }

    void SpriteRedrawManager::hideSprite( const Sprite::Reference& rSprite )
    {
        maSprites.remove( rSprite );
    }

    void SpriteRedrawManager::moveSprite( const Sprite::Reference&      rSprite,
                                          const ::basegfx::B2DPoint&    rOldPos,
                                          const ::basegfx::B2DPoint&    rNewPos,
                                          const ::basegfx::B2DVector&   rSpriteSize )
    {
        maChangeRecords.push_back( SpriteChangeRecord( rSprite,
                                                       rOldPos,
                                                       rNewPos,
                                                       rSpriteSize ) );
    }

    void SpriteRedrawManager::updateSprite( const Sprite::Reference&    rSprite,
                                            const ::basegfx::B2DPoint&  rPos,
                                            const ::basegfx::B2DRange&  rUpdateArea )
    {
        maChangeRecords.push_back( SpriteChangeRecord( rSprite,
                                                       rPos,
                                                       rUpdateArea ) );
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
