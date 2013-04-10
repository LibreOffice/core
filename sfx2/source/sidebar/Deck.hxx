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

#ifndef SFX_SIDEBAR_DECK_HXX
#define SFX_SIDEBAR_DECK_HXX

#include "Panel.hxx"

#include <vcl/window.hxx>
#include <vcl/image.hxx>
#include <com/sun/star/ui/LayoutSize.hpp>

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

class ScrollBar;

namespace css = ::com::sun::star;

namespace sfx2 { namespace sidebar {

class DeckDescriptor;
class DeckTitleBar;


/** This is the parent window of the panels.
    It displays the deck title.
*/
class Deck
    : public Window
{
public:
    Deck (
        const DeckDescriptor& rDeckDescriptor,
        Window* pParentWindow,
        const ::boost::function<void(void)>& rCloserAction);
    virtual ~Deck (void);

    void Dispose (void);

    const ::rtl::OUString& GetId (void) const;
    DeckTitleBar* GetTitleBar (void) const;
    Rectangle GetContentArea (void) const;
    ::rtl::OUString GetIconURL (const bool bIsHighContrastModeActive) const;
    void SetPanels (const SharedPanelContainer& rPanels);
    const SharedPanelContainer& GetPanels (void) const;
    void RequestLayout (void);
    ::Window* GetPanelParentWindow (void);

    virtual void Paint (const Rectangle& rUpdateArea);
    virtual void DataChanged (const DataChangedEvent& rEvent);

    void PrintWindowTree (void);
    void PrintWindowTree (const ::std::vector<Panel*>& rPanels);
    static void PrintWindowSubTree (Window* pRoot, int nIndentation);

    class ScrollContainerWindow : public Window
    {
    public:
        ScrollContainerWindow (Window* pParentWindow);
        virtual ~ScrollContainerWindow (void);
        virtual void Paint (const Rectangle& rUpdateArea);
        void SetSeparators (const ::std::vector<sal_Int32>& rSeparators);
    private:
        ::std::vector<sal_Int32> maSeparators;
    };

private:
    const ::rtl::OUString msTitle;
    const ::rtl::OUString msId;
    Image maIcon;
    const ::rtl::OUString msIconURL;
    const ::rtl::OUString msHighContrastIconURL;
    SharedPanelContainer maPanels;
    ::boost::scoped_ptr<DeckTitleBar> mpTitleBar;
    ::boost::scoped_ptr<Window> mpScrollClipWindow;
    ::boost::scoped_ptr<ScrollContainerWindow> mpScrollContainer;
    ::boost::scoped_ptr<Window> mpFiller;
    ::boost::scoped_ptr<ScrollBar> mpVerticalScrollBar;

    DECL_LINK(HandleVerticalScrollBarChange,void*);
};


} } // end of namespace sfx2::sidebar

#endif
