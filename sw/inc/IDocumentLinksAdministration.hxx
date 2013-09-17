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

#ifndef IDOCUMENTLINKSADMINISTRATION_HXX_INCLUDED
#define IDOCUMENTLINKSADMINISTRATION_HXX_INCLUDED

#include <tools/solar.h>

namespace com { namespace sun { namespace star { namespace uno { class Any; } } } }
namespace sfx2 { class SvLinkSource;  class LinkManager; }

 /** Document links administration interface
 */
 class IDocumentLinksAdministration
 {
 public:
    /** Insert links in-/visibly into LinkManager (linked ranges).
    */
    virtual bool IsVisibleLinks() const = 0;

    virtual void SetVisibleLinks(bool bFlag) = 0;

    virtual sfx2::LinkManager& GetLinkManager() = 0;

    virtual const sfx2::LinkManager& GetLinkManager() const = 0;

    /** #i42634# Moved common code of SwReader::Read() and
        SwDocShell::UpdateLinks() to new SwDoc::UpdateLinks():
    */
    virtual void UpdateLinks(bool bUI) = 0;

    /** SS fuers Linken von Dokumentteilen  / ?? for linking of parts of documents.
    */
    virtual bool GetData(const OUString& rItem, const String& rMimeType, ::com::sun::star::uno::Any& rValue) const = 0;

    virtual bool SetData(const OUString& rItem, const String& rMimeType, const ::com::sun::star::uno::Any& rValue) = 0;

    virtual ::sfx2::SvLinkSource* CreateLinkSource(const OUString& rItem) = 0;

    /** Embed all local links (ranges/graphics).
    */
    virtual bool EmbedAllLinks() = 0;

    virtual void SetLinksUpdated(const bool bNewLinksUpdated) = 0;

    virtual bool LinksUpdated() const = 0;

protected:
    virtual ~IDocumentLinksAdministration() {};
 };

 #endif // IDOCUMENTLINKSADMINISTRATION_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
