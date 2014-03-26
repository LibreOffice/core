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
#include "ControlFocusHelper.hxx"

class SvTreeListEntry;
namespace svx
{
    class OptHeaderTabListBox;
}

namespace svx
{



    //= DbRegistrationOptionsPage

    class DbRegistrationOptionsPage : public SfxTabPage
    {
    private:
        OUString            aTypeText;
        OUString            aPathText;

        SvSimpleTableContainer* m_pPathCtrl;
        PushButton*         m_pNew;
        PushButton*         m_pEdit;
        PushButton*         m_pDelete;

        ::svx::OptHeaderTabListBox* pPathBox;
        SvTreeListEntry*        m_pCurEntry;
        sal_uLong               m_nOldCount;
        sal_Bool                m_bModified;

#ifdef INCLUDED_CUI_SOURCE_INC_DBREGISTER_HXX
        DECL_LINK( NewHdl, void * );
        DECL_LINK( EditHdl, void * );
        DECL_LINK( DeleteHdl, void * );

        DECL_LINK(PathSelect_Impl, void *);

        DECL_LINK( HeaderSelect_Impl, HeaderBar * );
        DECL_LINK( HeaderEndDrag_Impl, HeaderBar * );
        DECL_LINK( NameValidator, OUString*);


        /** inserts a new entry in the tablistbox
            @param  _sName
                The name of the entry.
            @param  _sLocation
                The location of the file.
        */
        void insertNewEntry( const OUString& _sName,const OUString& _sLocation, const bool bReadOnly );

        /** opens the LinkDialog to create a register pair
            @param  _sOldName
                The old name of the entry may be empty.
            @param  _sOldLocation
                The old location of the entry may be empty.
            @param  _pEntry
                The entry to remove if the entry will be changed
        */
        void openLinkDialog(const OUString& _sOldName,const OUString& _sOldLocation,SvTreeListEntry* _pEntry = NULL);

#endif

    public:
        DbRegistrationOptionsPage( Window* pParent, const SfxItemSet& rSet );
        virtual ~DbRegistrationOptionsPage();

        static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
        static sal_uInt16*      GetRanges();

        virtual bool        FillItemSet( SfxItemSet& rSet );
        virtual void        Reset( const SfxItemSet& rSet );
        virtual void        FillUserData();
    };


    //= RegistrationItemSetHolder

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


    //= DatabaseRegistrationDialog

    class DatabaseRegistrationDialog    :public RegistrationItemSetHolder
                                        ,public SfxSingleTabDialog
    {
    public:
        DatabaseRegistrationDialog( Window* pParent, const SfxItemSet& rAttr );
        ~DatabaseRegistrationDialog();

        virtual short   Execute();
    };


}   // namespace svx


#endif // INCLUDED_CUI_SOURCE_INC_DBREGISTER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
