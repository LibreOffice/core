/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IDocumentLinksAdministration.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 07:53:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
