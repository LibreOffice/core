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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTLINKSADMINISTRATIONMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTLINKSADMINISTRATIONMANAGER_HXX

#include <IDocumentLinksAdministration.hxx>

#include <memory>

namespace sfx2 { class LinkManager; }
class SwDoc;
class SwPaM;
class SwNodeRange;

namespace sw
{

class DocumentLinksAdministrationManager : public IDocumentLinksAdministration
{
public:

    DocumentLinksAdministrationManager( SwDoc& i_rSwdoc );

    bool IsVisibleLinks() const override;

    void SetVisibleLinks(bool bFlag) override;

    sfx2::LinkManager& GetLinkManager() override;

    const sfx2::LinkManager& GetLinkManager() const override;

    void UpdateLinks() override;

    bool GetData(const OUString& rItem, const OUString& rMimeType, css::uno::Any& rValue) const override;

    bool SetData(const OUString& rItem, const OUString& rMimeType, const css::uno::Any& rValue) override;

    ::sfx2::SvLinkSource* CreateLinkSource(const OUString& rItem) override;

    bool EmbedAllLinks() override;

    void SetLinksUpdated(const bool bNewLinksUpdated) override;

    bool LinksUpdated() const override;

    //Non-Interface method
    bool SelectServerObj( const OUString& rStr, SwPaM*& rpPam, std::unique_ptr<SwNodeRange>& rpRange ) const;

    virtual ~DocumentLinksAdministrationManager() override;

private:

    DocumentLinksAdministrationManager(DocumentLinksAdministrationManager const&) = delete;
    DocumentLinksAdministrationManager& operator=(DocumentLinksAdministrationManager const&) = delete;

    bool mbVisibleLinks; //< TRUE: Links are inserted visibly.
    bool mbLinksUpdated; //< #i38810# flag indicating, that the links have been updated.
    std::unique_ptr<sfx2::LinkManager> m_pLinkMgr; //< List of linked stuff (graphics/DDE/OLE).

    SwDoc& m_rDoc;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
