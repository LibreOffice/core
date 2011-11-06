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



#ifndef _SIDEBARWINACC_HXX
#define _SIDEBARWINACC_HXX

#include <toolkit/awt/vclxwindow.hxx>

class ViewShell;
class SwSidebarItem;
class SwFrm;

namespace sw { namespace sidebarwindows {

class SwSidebarWin;

class SidebarWinAccessible : public VCLXWindow
{
    public:
        explicit SidebarWinAccessible( SwSidebarWin& rSidebarWin,
                                       ViewShell& rViewShell,
                                       const SwSidebarItem& rSidebarItem );
        virtual ~SidebarWinAccessible();

        virtual com::sun::star::uno::Reference< com::sun::star::accessibility::XAccessibleContext >
                CreateAccessibleContext();

        void ChangeSidebarItem( const SwSidebarItem& rSidebarItem );

    private:
        SwSidebarWin& mrSidebarWin;
        ViewShell& mrViewShell;
        const SwFrm* mpAnchorFrm;
        bool bAccContextCreated;
};

} } // end of namespace sw::sidebarwindows

#endif
