/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SD_TOOLPANEL_CONTROLS_CURRENT_MASTER_PAGES_SELECTOR_HXX
#define SD_TOOLPANEL_CONTROLS_CURRENT_MASTER_PAGES_SELECTOR_HXX

#include "MasterPagesSelector.hxx"
#include <com/sun/star/lang/XComponent.hpp>

namespace sd { namespace tools { class EventMultiplexerEvent; } }


namespace sd { namespace toolpanel { namespace controls {


/** Show the master pages currently used by a SdDrawDocument.
*/
class CurrentMasterPagesSelector
    : public MasterPagesSelector,
      public SfxListener
{
public:
    CurrentMasterPagesSelector (
        TreeNode* pParent,
        SdDrawDocument& rDocument,
        ViewShellBase& rBase,
        const ::boost::shared_ptr<MasterPageContainer>& rpContainer);
    virtual ~CurrentMasterPagesSelector (void);

    virtual void LateInit (void);

    /** Set the selection so that the master page is selected that is
        used by the currently selected page of the document in the
        center pane.
    */
    virtual void UpdateSelection (void);

    /** Copy all master pages that are to be shown into the given list.
    */
    virtual void Fill (ItemList& rItemList);

    using sd::toolpanel::controls::MasterPagesSelector::Fill;

protected:
    virtual ResId GetContextMenuResId (void) const;
    virtual void Execute (SfxRequest& rRequest);
    virtual void GetState (SfxItemSet& rItemSet);

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>
        mxListener;

    DECL_LINK(EventMultiplexerListener,sd::tools::EventMultiplexerEvent*);
    void Notify (SfxBroadcaster&, const SfxHint& rHint);
};

} } } // end of namespace ::sd::toolpanel::controls

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
