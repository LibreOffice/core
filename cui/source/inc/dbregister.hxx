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

#ifndef SVX_DBREGISTER_HXX
#define SVX_DBREGISTER_HXX

#include "optpath.hxx"
#include <tools/string.hxx>
#include <comphelper/stl_types.hxx>
#include <svl/poolitem.hxx>
#include "ControlFocusHelper.hxx"
#include <sfx2/basedlgs.hxx>

class SvLBoxEntry;
namespace svx
{
    class OptHeaderTabListBox;
}
//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= DbRegistrationOptionsPage
    //====================================================================
    class DbRegistrationOptionsPage : public SfxTabPage
    {
    private:
        FixedLine           aStdBox;
        FixedText           aTypeText;
        FixedText           aPathText;
        SvxControlFocusHelper   aPathCtrl;
        PushButton          m_aNew;
        PushButton          m_aEdit;
        PushButton          m_aDelete;

        HeaderBar*          pHeaderBar;
        ::svx::OptHeaderTabListBox* pPathBox;
        SvLBoxEntry*        m_pCurEntry;
        sal_uLong               m_nOldCount;
        sal_Bool                m_bModified;

#ifdef SVX_DBREGISTER_HXX
        DECL_LINK( NewHdl, void * );
        DECL_LINK( EditHdl, void * );
        DECL_LINK( DeleteHdl, void * );

        DECL_LINK( PathSelect_Impl, SvTabListBox* );

        DECL_LINK( HeaderSelect_Impl, HeaderBar * );
        DECL_LINK( HeaderEndDrag_Impl, HeaderBar * );
        DECL_LINK( NameValidator, String*);


        /** inserts a new entry in the tablistbox
            @param  _sName
                The name of the entry.
            @param  _sLocation
                The location of the file.
        */
        void insertNewEntry( const ::rtl::OUString& _sName,const ::rtl::OUString& _sLocation, const bool bReadOnly );

        /** opens the LinkDialog to create a register pair
            @param  _sOldName
                The old name of the entry may be empty.
            @param  _sOldLocation
                The old location of the entry may be empty.
            @param  _pEntry
                The entry to remove if the entry will be changed
        */
        void openLinkDialog(const String& _sOldName,const String& _sOldLocation,SvLBoxEntry* _pEntry = NULL);

#endif

    public:
        DbRegistrationOptionsPage( Window* pParent, const SfxItemSet& rSet );
        virtual ~DbRegistrationOptionsPage();

        static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
        static sal_uInt16*      GetRanges();

        virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
        virtual void        Reset( const SfxItemSet& rSet );
        virtual void        FillUserData();
    };

    //====================================================================
    //= RegistrationItemSetHolder
    //====================================================================
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

    //====================================================================
    //= DatabaseRegistrationDialog
    //====================================================================
    class DatabaseRegistrationDialog    :public RegistrationItemSetHolder
                                        ,public SfxSingleTabDialog
    {
    public:
        DatabaseRegistrationDialog( Window* pParent, const SfxItemSet& rAttr );
        ~DatabaseRegistrationDialog();

        virtual short   Execute();
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // SVX_DBREGISTER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
