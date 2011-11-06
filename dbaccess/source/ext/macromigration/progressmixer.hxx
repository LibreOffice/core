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
