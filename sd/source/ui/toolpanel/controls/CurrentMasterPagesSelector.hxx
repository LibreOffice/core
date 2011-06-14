/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
