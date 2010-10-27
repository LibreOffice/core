/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
************************************************************************/

#ifndef DBACCESS_PROGRESSMIXER_HXX
#define DBACCESS_PROGRESSMIXER_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <sal/types.h>

#include <memory>

//........................................................................
namespace dbmm
{
//........................................................................

    typedef sal_uInt32  PhaseID;
    typedef sal_uInt32  PhaseWeight;

    //====================================================================
    //= IProgressConsumer
    //====================================================================
    class SAL_NO_VTABLE IProgressConsumer
    {
    public:
        virtual void    start( sal_uInt32 _nRange ) = 0;
        virtual void    advance( sal_uInt32 _nValue ) = 0;
        virtual void    end() = 0;
    };

    //====================================================================
    //= ProgressMixer
    //====================================================================
    struct ProgressMixer_Data;
    /** a class which mixes (i.e. concatenates) progress values from different
        sources/phases, with different weight
    */
    class ProgressMixer
    {
    public:
        ProgressMixer( IProgressConsumer& _rConsumer );
        ~ProgressMixer();

        /** registers a phase of the process, which has the given weight
            in the overall process
            @precond
                the progress is not runnig, yet
        */
        void    registerPhase( const PhaseID _nID, const PhaseWeight _nWeight );

        /** enters the phase with the given ID, with the phase having
            the given overall range
        */
        void    startPhase( const PhaseID _nID, const sal_uInt32 _nPhaseRange );

        /** announces a new progress in the current phase.

            The given phase progress must be between 0 and the overall phase range
            as specified in ->startPhase.
        */
        void    advancePhase( const sal_uInt32 _nPhaseProgress );

        /** leaves the current phase, which has been started with ->startPhase previously
        */
        void    endPhase();

    private:
        ::std::auto_ptr< ProgressMixer_Data >   m_pData;
    };

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_PROGRESSMIXER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
