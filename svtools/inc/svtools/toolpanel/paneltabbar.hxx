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



#ifndef SVT_PANELTABBAR_HXX
#define SVT_PANELTABBAR_HXX

#include "svtools/svtdllapi.h"
#include "svtools/toolpanel/tabalignment.hxx"
#include "svtools/toolpanel/tabitemcontent.hxx"

#include <vcl/ctrl.hxx>

#include <memory>
#include <boost/optional.hpp>

class PushButton;

//........................................................................
namespace svt
{
//........................................................................

    class PanelTabBar_Impl;
    class IToolPanelDeck;

    //====================================================================
    //= PanelTabBar
    //====================================================================
    /** a tab bar for selecting panels

        At the moment, this control aligns the tabs vertically, this might be extended to also support a horizontal
        layout in the future.
    */
    class SVT_DLLPUBLIC PanelTabBar : public Control
    {
    public:
        PanelTabBar( Window& i_rParentWindow, IToolPanelDeck& i_rPanelDeck, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent );
        ~PanelTabBar();

        // attribute access
        TabItemContent  GetTabItemContent() const;
        void            SetTabItemContent( const TabItemContent& i_eItemContent );

        ::boost::optional< size_t > GetFocusedPanelItem() const;
        void                        FocusPanelItem( const size_t i_nItemPos );
        Rectangle                   GetItemScreenRect( const size_t i_nItemPos ) const;
        bool                        IsVertical() const;
        IToolPanelDeck&             GetPanelDeck() const;
        PushButton&                 GetScrollButton( const bool i_bForward );

        // Window overridables
        virtual Size    GetOptimalSize( WindowSizeType i_eType ) const;

    protected:
        // Window overridables
        virtual void    Paint( const Rectangle& i_rRect );
        virtual void    Resize();
        virtual void    MouseMove( const MouseEvent& i_rMouseEvent );
        virtual void    MouseButtonDown( const MouseEvent& i_rMouseEvent );
        virtual void    MouseButtonUp( const MouseEvent& i_rMouseEvent );
        virtual void    RequestHelp( const HelpEvent& i_rHelpEvent );
        virtual void    GetFocus();
        virtual void    LoseFocus();
        virtual void    KeyInput( const KeyEvent& i_rKeyEvent );
        virtual void    DataChanged( const DataChangedEvent& i_rDataChanedEvent );

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >
                        GetComponentInterface( sal_Bool i_bCreate );

    private:
        ::std::auto_ptr< PanelTabBar_Impl > m_pImpl;
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // SVT_PANELTABBAR_HXX

