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


#ifndef __FRAMEWORK_CLASSES_BMKMENU_HXX
#define __FRAMEWORK_CLASSES_BMKMENU_HXX

#include "framework/addonmenu.hxx"
//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/frame/XFrame.hpp>
#include <framework/fwedllapi.h>
//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <vcl/menu.hxx>
#include <vcl/image.hxx>

class String;
class ImageList;

#define BMKMENU_ITEMID_START    20000

namespace framework
{

class BmkMenu_Impl;
class FWE_DLLPUBLIC BmkMenu : public AddonMenu
{
    public:
                        enum BmkMenuType
                        {
                            BMK_NEWMENU,
                            BMK_WIZARDMENU
                        };

                        BmkMenu( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                 BmkMenuType nType );
                        ~BmkMenu();

    void                Initialize(); // Synchrones Laden der Eintraege

    protected:
        BmkMenu::BmkMenuType m_nType;
        sal_uInt16          CreateMenuId();

    private:
                        BmkMenu( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                 BmkMenuType, BmkMenu* pRoot );

        BmkMenu_Impl*   _pImp;
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_CLASSES_BMKMENU_HXX
