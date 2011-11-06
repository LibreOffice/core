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



#ifndef UUI_FLTDLG_HXX
#define UUI_FLTDLG_HXX

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include <vcl/dialog.hxx>

#ifndef _MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#include <vcl/lstbox.hxx>

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
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
