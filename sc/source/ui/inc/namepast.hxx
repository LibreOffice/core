/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: namepast.hxx,v $
 * $Revision: 1.4 $
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

#ifndef SC_NAMEPAST_HXX
#define SC_NAMEPAST_HXX

#include <vcl/dialog.hxx>
#include <vcl/imagebtn.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

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
