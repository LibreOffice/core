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

#pragma once

#include <vcl/weld.hxx>

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

class FilterDialog : public weld::GenericDialogController
{
    // public interface
    public:
        explicit FilterDialog(weld::Window* pParentWindow);
        virtual ~FilterDialog() override;
        void SetURL       ( const OUString&          sURL           );
        void ChangeFilters( const FilterNameList*    pFilterNames   );
        bool AskForFilter (       FilterNameListPtr& pSelectedItem  );

    private:
        OUString impl_buildUIFileName( const OUString& sURL );

    // member
    private:
        const FilterNameList* m_pFilterNames;
        std::unique_ptr<weld::Label> m_xFtURL;
        std::unique_ptr<weld::TreeView> m_xLbFilters;

};  // class FilterDialog

}   // namespace uui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
