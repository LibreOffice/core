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
    /** Links un-/sichtbar in LinkManager einfuegen (gelinkte Bereiche)
    */
    virtual bool IsVisibleLinks() const = 0;

    /**
    */
    virtual void SetVisibleLinks(bool bFlag) = 0;

    /**
    */
    virtual sfx2::LinkManager& GetLinkManager() = 0;

    /**
    */
    virtual const sfx2::LinkManager& GetLinkManager() const = 0;

    /** FME 2005-02-25 #i42634# Moved common code of SwReader::Read() and
        SwDocShell::UpdateLinks() to new SwDoc::UpdateLinks():
    */
    virtual void UpdateLinks(sal_Bool bUI) = 0;

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
