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



// SMARTTAGS

#ifndef _STMENU_HXX
#define _STMENU_HXX

#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif

#include <vector>
#include <com/sun/star/smarttags/XSmartTagAction.hpp>

#ifndef _COM_SUN_STAR_SMARTTAGS_XSTRINGKEYMAP_HPP_
#include <com/sun/star/container/XStringKeyMap.hpp>
#endif
#include <com/sun/star/text/XTextRange.hpp>

class SwView;

/** Class: SwSmartTagPopup

   This class contains the implementation of the smarttag popup
   menu that is opened if a user clicks on an underlined word.

   The menu is built in the constructor and the actions for each
   menu entry are invoked in the excute-method.
*/

class SwSmartTagPopup : public PopupMenu
{
    SwView*  mpSwView;
    com::sun::star::uno::Reference< com::sun::star::text::XTextRange > mxTextRange;

    struct InvokeAction
    {
        com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > mxAction;
        com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > mxSmartTagProperties;
        sal_uInt32 mnActionID;
        InvokeAction( com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > xAction,
                      com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xSmartTagProperties,
                      sal_uInt32 nActionID ) : mxAction( xAction ), mxSmartTagProperties( xSmartTagProperties ), mnActionID( nActionID ) {}
    };

    std::vector< InvokeAction > maInvokeActions;

    using PopupMenu::Execute;

public:
    SwSmartTagPopup( SwView* _pSwView,
                     ::com::sun::star::uno::Sequence< rtl::OUString >& rSmartTagTypes,
                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::container::XStringKeyMap > >& rStringKeyMaps,
                     ::com::sun::star::uno::Reference< com::sun::star::text::XTextRange > xTextRange );

    sal_uInt16  Execute( const Rectangle& rPopupPos, Window* pWin );
};

#endif

