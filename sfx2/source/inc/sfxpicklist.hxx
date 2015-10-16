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

#ifndef INCLUDED_SFX2_SOURCE_INC_SFXPICKLIST_HXX
#define INCLUDED_SFX2_SOURCE_INC_SFXPICKLIST_HXX

#include <osl/mutex.hxx>
#include <vcl/menu.hxx>
#include <svl/lstner.hxx>
#include <com/sun/star/util/XStringWidth.hpp>

#include <vector>

#define PICKLIST_MAXSIZE  100

class SfxPickList : public SfxListener
{
    struct PickListEntry
    {
        PickListEntry( const OUString& _aName, const OUString& _aFilter, const OUString& _aTitle ) :
            aName( _aName ), aFilter( _aFilter ), aTitle( _aTitle ) {}

        OUString aName;
        OUString aFilter;
        OUString aTitle;
        OUString aOptions;
    };

    std::vector< PickListEntry* >   m_aPicklistVector;
    sal_uInt32                      m_nAllowedMenuSize;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XStringWidth > m_xStringLength;

                            SfxPickList( sal_uInt32 nMenuSize );
                            virtual ~SfxPickList();

    void                    CreatePicklistMenuTitle( Menu* pMenu, sal_uInt16 nItemId, const OUString& aURL, sal_uInt32 nNo );
    PickListEntry*          GetPickListEntry( sal_uInt32 nIndex );
    void                    CreatePickListEntries();
    void                    RemovePickListEntries();
    /**
     * Adds the given document to the pick list (recent documents) if it satisfies
       certain requirements, e.g. being writable. Check implementation for requirement
       details.
     */
    static void             AddDocumentToPickList( SfxObjectShell* pDocShell );

    public:
        static SfxPickList& Get();
        static void ensure() { Get(); }

        void                CreateMenuEntries( Menu* pMenu );
        static void         ExecuteMenuEntry( sal_uInt16 nId );
        static void         ExecuteEntry( sal_uInt32 nIndex );

        virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
};

#endif // INCLUDED_SFX2_SOURCE_INC_SFXPICKLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
