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

#include "progressmixer.hxx"

#include <osl/diagnose.h>

#include <map>

namespace dbmm
{
#define OVERALL_RANGE   100000

    // misc types
    struct PhaseData
    {
        // the weight of the phase, relative to all other phases
        PhaseWeight nWeight;
        // the "local"  range of the phase
        sal_uInt32  nRange;
        // this is the point in the "overall range" at which this phase starts
        sal_uInt32  nGlobalStart;
        /** the "global" range of the phase, i.e. its range after weighting with all other
            phases
        */
        sal_uInt32  nGlobalRange;

        PhaseData()
            :nWeight(1)
            ,nRange(100)
            ,nGlobalStart(0)
            ,nGlobalRange(100)
        {
        }

        PhaseData( const PhaseWeight _nWeight )
            :nWeight( _nWeight )
            ,nRange(100)
            ,nGlobalStart(0)
            ,nGlobalRange(100)
        {
        }
    };

    typedef ::std::map< PhaseID, PhaseData >   Phases;

    // ProgressMixer_Data
    struct ProgressMixer_Data
    {
        Phases              aPhases;
        Phases::iterator    pCurrentPhase;
        sal_uInt32          nWeightSum;         /// the cached sum of the weights
        double              nOverallStretch;
        IProgressConsumer&  rConsumer;

        ProgressMixer_Data( IProgressConsumer& _rConsumer )
            :aPhases()
            ,pCurrentPhase( aPhases.end() )
            ,nWeightSum( 0 )
            ,nOverallStretch( 0 )
            ,rConsumer( _rConsumer )
        {
        }
    };

    namespace
    {
#if OSL_DEBUG_LEVEL > 0
        bool lcl_isRunning( const ProgressMixer_Data& _rData )
        {
            return _rData.pCurrentPhase != _rData.aPhases.end();
        }
#endif
        void lcl_ensureInitialized( ProgressMixer_Data& _rData )
        {
            OSL_PRECOND( _rData.nWeightSum, "lcl_ensureInitialized: we have no phases, this will crash!" );

            if ( _rData.nOverallStretch )
                return;

            _rData.nOverallStretch = 1.0 * OVERALL_RANGE / _rData.nWeightSum;

            // tell the single phases their "overall starting point"
            PhaseWeight nRunningWeight( 0 );
            for (   Phases::iterator phase = _rData.aPhases.begin();
                    phase != _rData.aPhases.end();
                    ++phase
                )
            {
                phase->second.nGlobalStart = (sal_uInt32)( nRunningWeight * _rData.nOverallStretch );
                nRunningWeight += phase->second.nWeight;

                sal_uInt32 nNextPhaseStart = (sal_uInt32)( nRunningWeight * _rData.nOverallStretch );
                phase->second.nGlobalRange = nNextPhaseStart - phase->second.nGlobalStart;
            }

            _rData.rConsumer.start( OVERALL_RANGE );
        }
    }

    // ProgressMixer
    ProgressMixer::ProgressMixer( IProgressConsumer& _rConsumer )
        :m_pData( new ProgressMixer_Data( _rConsumer ) )
    {
    }

    ProgressMixer::~ProgressMixer()
    {
    }

    void ProgressMixer::registerPhase( const PhaseID _nID, const PhaseWeight _nWeight )
    {
        OSL_PRECOND( !lcl_isRunning( *m_pData ), "ProgressMixer::registerPhase: already running!" );
        OSL_ENSURE( m_pData->aPhases.find( _nID ) == m_pData->aPhases.end(),
            "ProgressMixer::registerPhase: ID already used!" );
        m_pData->aPhases[ _nID ] = PhaseData( _nWeight );
        m_pData->nWeightSum += _nWeight;
    }

    void ProgressMixer::startPhase( const PhaseID _nID, const sal_uInt32 _nPhaseRange )
    {
        OSL_ENSURE( m_pData->aPhases.find( _nID ) != m_pData->aPhases.end(),
            "ProgresMixer::startPhase: unknown phase!" );

        m_pData->aPhases[ _nID ].nRange = _nPhaseRange;
        m_pData->pCurrentPhase = m_pData->aPhases.find( _nID );
    }

    void ProgressMixer::advancePhase( const sal_uInt32 _nPhaseProgress )
    {
        OSL_PRECOND( lcl_isRunning( *m_pData ), "ProgresMixer::advancePhase: not running!" );

        // in case this is the first call, ensure all the ranges/weights are calculated
        // correctly
        lcl_ensureInitialized( *m_pData );

        const PhaseData& rPhase( m_pData->pCurrentPhase->second );

        double nLocalProgress = 1.0 * _nPhaseProgress / rPhase.nRange;
        sal_uInt32 nOverallProgress = (sal_uInt32)
            ( rPhase.nGlobalStart + nLocalProgress * rPhase.nGlobalRange );

        m_pData->rConsumer.advance( nOverallProgress );
    }

    void ProgressMixer::endPhase()
    {
        OSL_PRECOND( lcl_isRunning( *m_pData ), "ProgresMixer::endPhase: not running!" );

        // in case this is the first call, ensure all the ranges/weights are calculated
        // correctly
        lcl_ensureInitialized( *m_pData );

        // simply assume the phase's complete range is over
        advancePhase( m_pData->pCurrentPhase->second.nRange );

        // if that's the last phase, this is the "global end", too
        Phases::const_iterator pNextPhase( m_pData->pCurrentPhase );
        ++pNextPhase;
        if ( pNextPhase == m_pData->aPhases.end() )
            m_pData->rConsumer.end();
    }

} // namespace dbmm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
