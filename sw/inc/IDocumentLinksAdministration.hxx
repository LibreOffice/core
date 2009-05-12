/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IDocumentLinksAdministration.hxx,v $
 * $Revision: 1.6 $
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
class SvxLinkManager;

namespace com { namespace sun { namespace star { namespace uno { class Any; } } } }

namespace sfx2 { class SvLinkSource; }

 /** Document links administration interface
 */
 class IDocumentLinksAdministration
 {
 public:
    /** Links un-/sichtbar in LinkManager einfuegen (gelinkte Bereiche)
    */
    virtual bool IsVisibleLinks() const = 0;

    /**
    */
    virtual void SetVisibleLinks(bool bFlag) = 0;

    /**
    */
    virtual SvxLinkManager& GetLinkManager() = 0;

    /**
    */
    virtual const SvxLinkManager& GetLinkManager() const = 0;

    /** FME 2005-02-25 #i42634# Moved common code of SwReader::Read() and
        SwDocShell::UpdateLinks() to new SwDoc::UpdateLinks():
    */
    virtual void UpdateLinks(BOOL bUI) = 0;

    /** SS fuers Linken von Dokumentteilen
    */
    virtual bool GetData(const String& rItem, const String& rMimeType, ::com::sun::star::uno::Any& rValue) const = 0;

    /**
    */
    virtual bool SetData(const String& rItem, const String& rMimeType, const ::com::sun::star::uno::Any& rValue) = 0;

    /**
    */
    virtual ::sfx2::SvLinkSource* CreateLinkSource(const String& rItem) = 0;

    /** embedded alle lokalen Links (Bereiche/Grafiken)
    */
    virtual bool EmbedAllLinks() = 0;

    /**
    */
    virtual void SetLinksUpdated(const bool bNewLinksUpdated) = 0;

    /**
    */
    virtual bool LinksUpdated() const = 0;

protected:
    virtual ~IDocumentLinksAdministration() {};
 };

 #endif // IDOCUMENTLINKSADMINISTRATION_HXX_INCLUDED
