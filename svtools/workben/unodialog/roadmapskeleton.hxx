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



#ifndef SVTOOLS_ROADMAPSKELETONDIALOG_HXX
#define SVTOOLS_ROADMAPSKELETONDIALOG_HXX

#include <comphelper/componentcontext.hxx>
#include <svtools/roadmapwizard.hxx>

//........................................................................
namespace udlg
{
//........................................................................

    //====================================================================
    //= RoadmapSkeletonDialog
    //====================================================================
    typedef ::svt::RoadmapWizard    RoadmapSkeletonDialog_Base;
    class RoadmapSkeletonDialog : public RoadmapSkeletonDialog_Base
    {
    public:
        RoadmapSkeletonDialog( const ::comphelper::ComponentContext& _rContext, Window* _pParent );
        virtual ~RoadmapSkeletonDialog();

    protected:
        // OWizardMachine overridables
        virtual void            enterState( WizardState _nState );
        virtual sal_Bool        prepareLeaveCurrentState( CommitPageReason _eReason );
        virtual sal_Bool        leaveState( WizardState _nState );
        virtual WizardState     determineNextState( WizardState _nCurrentState ) const;
        virtual sal_Bool        onFinish();

    private:
        ::comphelper::ComponentContext  m_aContext;
    };

//........................................................................
} // namespace udlg
//........................................................................

#endif // SVTOOLS_ROADMAPSKELETONDIALOG_HXX
