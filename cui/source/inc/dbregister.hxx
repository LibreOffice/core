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

#ifndef INCLUDED_CUI_SOURCE_INC_DBREGISTER_HXX
#define INCLUDED_CUI_SOURCE_INC_DBREGISTER_HXX

#include <rtl/ustring.hxx>
#include <sfx2/basedlgs.hxx>
#include <svl/poolitem.hxx>
#include <svtools/simptabl.hxx>
#include "optpath.hxx"

class SvTreeListEntry;
namespace svx
{
    class OptHeaderTabListBox;
}

namespace svx
{

    class DbRegistrationOptionsPage : public SfxTabPage
    {
    private:
        sal_uLong               m_nOldCount;
        bool                m_bModified;

        std::unique_ptr<weld::Button> m_xNew;
        std::unique_ptr<weld::Button> m_xEdit;
        std::unique_ptr<weld::Button> m_xDelete;
        std::unique_ptr<weld::TreeView> m_xPathBox;
        std::unique_ptr<weld::TreeIter> m_xIter;

        DECL_LINK( NewHdl, weld::Button&, void );
        DECL_LINK( EditHdl, weld::Button&, void );
        DECL_LINK( DeleteHdl, weld::Button&, void );
        DECL_LINK( PathBoxDoubleClickHdl, weld::TreeView&, void);

        DECL_LINK( PathSelect_Impl, weld::TreeView&, void);

        DECL_LINK( HeaderSelect_Impl, int, void );
        DECL_LINK( NameValidator, const OUString&, bool);

        /** inserts a new entry in the tablistbox
            @param  _sName
                The name of the entry.
            @param  _sLocation
                The location of the file.
        */
        void insertNewEntry( const OUString& _sName,const OUString& _sLocation, const bool bReadOnly );

        /** opens the LinkDialog to create a register pair
            @param  sOldName
                The old name of the entry may be empty.
            @param  sOldLocation
                The old location of the entry may be empty.
            @param  nEntry
                The entry to remove if the entry will be changed
        */
        void openLinkDialog(const OUString& sOldName, const OUString& sOldLocation, int nEntry = -1);

    public:
        DbRegistrationOptionsPage(TabPageParent pParent, const SfxItemSet& rSet);
        virtual ~DbRegistrationOptionsPage() override;
        virtual void dispose() override;

        static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rSet );

        virtual bool        FillItemSet( SfxItemSet* rSet ) override;
        virtual void        Reset( const SfxItemSet* rSet ) override;
        virtual void        FillUserData() override;
    };

    /** helper for DatabaseRegistrationDialog

        Necessary so that DatabaseRegistrationDialog is self-contained, i.e. always reflects
        the current registration state.
    */
    class RegistrationItemSetHolder
    {
    private:
        SfxItemSet  m_aRegistrationItems;

    protected:
        RegistrationItemSetHolder( const SfxItemSet& _rMasterSet );
        ~RegistrationItemSetHolder();

    protected:
        const SfxItemSet& getRegistrationItems() const { return m_aRegistrationItems; }
    };

    class DatabaseRegistrationDialog    :public RegistrationItemSetHolder
                                        ,public SfxSingleTabDialogController
    {
    public:
        DatabaseRegistrationDialog(weld::Window* pParent, const SfxItemSet& rAttr);

        virtual short   run() override;
    };
}

#endif // INCLUDED_CUI_SOURCE_INC_DBREGISTER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
