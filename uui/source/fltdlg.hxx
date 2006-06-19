/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fltdlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:59:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef UUI_FLTDLG_HXX
#define UUI_FLTDLG_HXX

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

#ifndef _MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

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
