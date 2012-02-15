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



#ifndef SVT_TABLAYOUTER_HXX
#define SVT_TABLAYOUTER_HXX

#include "svtools/svtdllapi.h"
#include "svtools/toolpanel/decklayouter.hxx"
#include "svtools/toolpanel/tabalignment.hxx"
#include "svtools/toolpanel/tabitemcontent.hxx"
#include "svtools/toolpanel/refbase.hxx"

#include <memory>

#include <boost/noncopyable.hpp>

class Window;

//........................................................................
namespace svt
{
//........................................................................

    class IToolPanelDeck;

    struct TabDeckLayouter_Data;

    //====================================================================
    //= TabDeckLayouter
    //====================================================================
    class SVT_DLLPUBLIC TabDeckLayouter :public RefBase
                                        ,public IDeckLayouter
                                        ,public ::boost::noncopyable
    {
    public:
        /** creates a new layouter
            @param i_rParent
                is the parent window for any VCL windows the layouter needs to create.
            @param i_rPanels
                is the panel deck which the layouter is responsible for.
            @param i_eAlignment
                specifies the alignment of the panel selector
            @param TabItemContent
                specifies the content to show on the tab items
        */
        TabDeckLayouter(
            Window& i_rParent,
            IToolPanelDeck& i_rPanels,
            const TabAlignment i_eAlignment,
            const TabItemContent i_eItemContent
        );
        ~TabDeckLayouter();

        // attribute access
        TabItemContent  GetTabItemContent() const;
        void            SetTabItemContent( const TabItemContent& i_eItemContent );
        TabAlignment    GetTabAlignment() const;

        // helpers for the A11Y implementation
        ::boost::optional< size_t >
                        GetFocusedPanelItem() const;
        void            FocusPanelItem( const size_t i_nItemPos );
        bool            IsPanelSelectorEnabled() const;
        bool            IsPanelSelectorVisible() const;
        Rectangle       GetItemScreenRect( const size_t i_nItemPos ) const;

        // IDeckLayouter
        virtual Rectangle   Layout( const Rectangle& i_rDeckPlayground );
        virtual void        Destroy();
        virtual void        SetFocusToPanelSelector();
        virtual size_t      GetAccessibleChildCount() const;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                            GetAccessibleChild(
                                const size_t i_nChildIndex,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rParentAccessible
                            );

        // IReference
        DECLARE_IREFERENCE()

    private:
        ::std::auto_ptr< TabDeckLayouter_Data > m_pData;
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // SVT_TABLAYOUTER_HXX
