/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namepast.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:39:11 $
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

#ifndef SC_NAMEPAST_HXX
#define SC_NAMEPAST_HXX

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#include "scui_def.hxx" //CHINA001
//CHINA001 #define BTN_PASTE_NAME   100
//CHINA001 #define BTN_PASTE_LIST   101
//CHINA001
class ScRangeName;

class ScNamePasteDlg : public ModalDialog
{
    DECL_LINK( ButtonHdl, Button * );
    DECL_LINK( ListSelHdl, ListBox * );
    DECL_LINK( ListDblClickHdl, ListBox * );

protected:
    FixedText       aLabelText;
    ListBox         aNameList;
    OKButton        aOKButton;
    CancelButton    aCancelButton;
    HelpButton      aHelpButton;
    PushButton      aInsListButton;
public:
    ScNamePasteDlg( Window * pParent, const ScRangeName* pList, BOOL bInsList=TRUE );

    String          GetSelectedName() const;
};


#endif //SC_NAMEPAST_HXX
