/*************************************************************************
 *
 *  $RCSfile: IDocumentLinksAdministration.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:13:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef IDOCUMENTLINKSADMINISTRATION_HXX_INCLUDED
#define IDOCUMENTLINKSADMINISTRATION_HXX_INCLUDED

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
    virtual void UpdateLinks() = 0;

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
