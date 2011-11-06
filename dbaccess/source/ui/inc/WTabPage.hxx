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



#ifndef DBAUI_WIZ_TABBPAGE_HXX
#define DBAUI_WIZ_TABBPAGE_HXX

#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif

namespace dbaui
{
    // ========================================================
    // Wizard Page
    // ========================================================
    class OCopyTableWizard;
    class OWizardPage       : public TabPage
    {
    protected:
        OCopyTableWizard*           m_pParent;
        sal_Bool                    m_bFirstTime;   // Page wird das erste mal gerufen ; should be set in the reset methode

        OWizardPage( Window* pParent, const ResId& rResId );// : TabPage(pParent,rResId),m_pParent((OCopyTableWizard*)pParent),m_bFirstTime(sal_True) {};

    public:
        virtual void        Reset ( )           = 0;
        virtual sal_Bool    LeavePage()         = 0;
        virtual String      GetTitle() const    = 0;

        sal_Bool            IsFirstTime() const { return m_bFirstTime; }
    };
}
#endif // DBAUI_WIZ_TABBPAGE_HXX



