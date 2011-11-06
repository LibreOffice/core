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



#ifndef SVT_PANELTABBARPEER_HXX
#define SVT_PANELTABBARPEER_HXX

#include "svtaccessiblefactory.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <toolkit/awt/vclxwindow.hxx>

//......................................................................................................................
namespace svt
{
//......................................................................................................................

    class PanelTabBar;
    //====================================================================
    //= PanelTabBarPeer
    //====================================================================
    class PanelTabBarPeer : public VCLXWindow
    {
    public:
        PanelTabBarPeer( PanelTabBar& i_rTabBar );

    protected:
        ~PanelTabBarPeer();

        // VCLXWindow overridables
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > CreateAccessibleContext();

        // XComponent
        void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    private:
        AccessibleFactoryAccess m_aAccessibleFactory;
        PanelTabBar*            m_pTabBar;
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // SVT_PANELTABBARPEER_HXX
