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



#ifndef __FRAMEWORK_LAYOUTMANAGER_PANEL_HXX_
#define __FRAMEWORK_LAYOUTMANAGER_PANEL_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <macros/generic.hxx>
#include <stdtypes.h>
#include <properties.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <vcl/window.hxx>
#include <vcl/splitwin.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework
{

enum PanelPosition
{
    PANEL_TOP,
    PANEL_LEFT,
    PANEL_RIGHT,
    PANEL_BOTTOM,
    PANEL_COUNT
};

class Panel
{
    public:
        Panel( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMGR,
               const css::uno::Reference< css::awt::XWindow >& rParent,
               PanelPosition nPanel );
        virtual ~Panel();

    private:
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;
        css::uno::Reference< css::awt::XWindow >               m_xPanelWindow;
        PanelPosition                                          m_nPanelPosition;
};

}

#endif // __FRAMEWORK_LAYOUTMANAGER_PANEL_HXX_
