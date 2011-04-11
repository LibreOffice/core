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

#ifndef UUI_FLTDLG_HXX
#define UUI_FLTDLG_HXX

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include <vcl/dialog.hxx>

#include <vcl/msgbox.hxx>
#include <vcl/lstbox.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <tools/string.hxx>

#include <vector>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace uui
{

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

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
