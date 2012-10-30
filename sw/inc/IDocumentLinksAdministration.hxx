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

#ifndef IDOCUMENTLINKSADMINISTRATION_HXX_INCLUDED
#define IDOCUMENTLINKSADMINISTRATION_HXX_INCLUDED

#include <tools/solar.h>

class String;
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
    virtual bool GetData(const rtl::OUString& rItem, const String& rMimeType, ::com::sun::star::uno::Any& rValue) const = 0;

    virtual bool SetData(const rtl::OUString& rItem, const String& rMimeType, const ::com::sun::star::uno::Any& rValue) = 0;

    virtual ::sfx2::SvLinkSource* CreateLinkSource(const rtl::OUString& rItem) = 0;

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
