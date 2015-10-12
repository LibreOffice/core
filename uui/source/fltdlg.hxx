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

#ifndef INCLUDED_UUI_SOURCE_FLTDLG_HXX
#define INCLUDED_UUI_SOURCE_FLTDLG_HXX

#include <vcl/dialog.hxx>

#include <vcl/msgbox.hxx>
#include <vcl/lstbox.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

#include <vector>

namespace uui
{

struct FilterNamePair
{
    OUString sInternal;
    OUString sUI      ;
};

typedef ::std::vector< FilterNamePair > FilterNameList   ;
typedef FilterNameList::const_iterator  FilterNameListPtr;

class FilterDialog : public ModalDialog
{
    // public interface
    public:
        explicit FilterDialog(vcl::Window* pParentWindow);
        virtual ~FilterDialog();
        virtual void dispose() override;
        void SetURL       ( const OUString&          sURL           );
        void ChangeFilters( const FilterNameList*    pFilterNames   );
        bool AskForFilter (       FilterNameListPtr& pSelectedItem  );

    // helper (or hided functions!)
    private:
        short Execute() override { return RET_CANCEL; };
        OUString impl_buildUIFileName( const OUString& sURL );

    // member
    private:
              VclPtr<FixedText>       m_pFtURL       ;
              VclPtr<ListBox>         m_pLbFilters   ;
        const FilterNameList* m_pFilterNames;

};  // class FilterDialog

}   // namespace uui

#endif // INCLUDED_UUI_SOURCE_FLTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
