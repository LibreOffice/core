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

#ifndef SD_SIDEBAR_PANELS_CURRENT_MASTER_PAGES_SELECTOR_HXX
#define SD_SIDEBAR_PANELS_CURRENT_MASTER_PAGES_SELECTOR_HXX

#include "MasterPagesSelector.hxx"
#include <com/sun/star/lang/XComponent.hpp>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;


namespace sd { namespace tools { class EventMultiplexerEvent; } }


namespace sd { namespace sidebar {


/** Show the master pages currently used by a SdDrawDocument.
*/
class CurrentMasterPagesSelector
    : public MasterPagesSelector,
      public SfxListener
{
public:
    static MasterPagesSelector* Create (
        ::Window* pParent,
        ViewShellBase& rViewShellBase,
        const cssu::Reference<css::ui::XSidebar>& rxSidebar);

    /** Set the selection so that the master page is selected that is
        used by the currently selected page of the document in the
        center pane.
    */
    virtual void UpdateSelection (void);

    /** Copy all master pages that are to be shown into the given list.
    */
    virtual void Fill (ItemList& rItemList);

    using MasterPagesSelector::Fill;

protected:
    virtual ResId GetContextMenuResId (void) const;

    virtual void ProcessPopupMenu (Menu& rMenu);
    virtual void ExecuteCommand (const sal_Int32 nCommandId);

private:
    cssu::Reference<css::lang::XComponent> mxListener;

    CurrentMasterPagesSelector (
        ::Window* pParent,
        SdDrawDocument& rDocument,
        ViewShellBase& rBase,
        const ::boost::shared_ptr<MasterPageContainer>& rpContainer,
        const cssu::Reference<css::ui::XSidebar>& rxSidebar);
    virtual ~CurrentMasterPagesSelector (void);

    virtual void LateInit (void);

    DECL_LINK(EventMultiplexerListener,sd::tools::EventMultiplexerEvent*);
    void Notify (SfxBroadcaster&, const SfxHint& rHint);
};

} } // end of namespace sd::sidebar

#endif
