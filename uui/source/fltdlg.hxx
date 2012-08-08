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

#ifndef UUI_FLTDLG_HXX
#define UUI_FLTDLG_HXX

#include <vcl/dialog.hxx>

#include <vcl/msgbox.hxx>
#include <vcl/lstbox.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <tools/string.hxx>

#include <vector>

namespace uui
{

struct FilterNamePair
{
    String sInternal;
    String sUI      ;
};

typedef ::std::vector< FilterNamePair > FilterNameList   ;
typedef FilterNameList::const_iterator  FilterNameListPtr;

class FilterDialog : public ModalDialog
{
    // public interface
    public:
             FilterDialog (       Window*            pParentWindow  ,
                                  ResMgr*            pResMgr        );
        void SetURL       ( const String&            sURL           );
        void ChangeFilters( const FilterNameList*    pFilterNames   );
        bool AskForFilter (       FilterNameListPtr& pSelectedItem  );

    // helper (or hided functions!)
    private:
        short Execute() { return RET_CANCEL; };
        String impl_buildUIFileName( const String& sURL );

    // member
    private:
              FixedText       m_ftURL       ;
              ListBox         m_lbFilters   ;
              OKButton        m_btnOK       ;
              CancelButton    m_btnCancel   ;
              HelpButton      m_btnHelp     ;
        const FilterNameList* m_pFilterNames;

};  // class FilterDialog

}   // namespace uui

#endif // UUI_FLTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
