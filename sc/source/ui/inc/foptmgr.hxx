/*************************************************************************
 *
 *  $RCSfile: foptmgr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_FOPTMGR_HXX
#define SC_FOPTMGR_HXX

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

//----------------------------------------------------------------------------

class FixedInfo;
class ScRefButton;
class MoreButton;
class Dialog;
struct ScQueryParam;
class ScDocument;
class ScViewData;

//----------------------------------------------------------------------------

class ScFilterOptionsMgr
{
public:
            ScFilterOptionsMgr( Dialog*             ptrDlg,
                                ScViewData*         ptrViewData,
                                const ScQueryParam& refQueryData,
                                MoreButton&         refBtnMore,
                                CheckBox&           refBtnCase,
                                CheckBox&           refBtnRegExp,
                                CheckBox&           refBtnHeader,
                                CheckBox&           refBtnUnique,
                                CheckBox&           refBtnCopyResult,
                                CheckBox&           refBtnDestPers,
                                ListBox&            refLbCopyArea,
                                Edit&               refEdCopyArea,
                                ScRefButton&        refRbCopyArea,
                                FixedText&          refFtDbAreaLabel,
                                FixedInfo&          refFtDbArea,
                                GroupBox&           refGbOptions,
                                const String&       refStrNoName,
                                const String&       refStrUndefined );
            ~ScFilterOptionsMgr();

    BOOL    VerifyPosStr ( const String& rPosStr ) const;

private:
    Dialog*         pDlg;
    ScViewData*     pViewData;
    ScDocument*     pDoc;

    MoreButton&     rBtnMore;
    CheckBox&       rBtnCase;
    CheckBox&       rBtnRegExp;
    CheckBox&       rBtnHeader;
    CheckBox&       rBtnUnique;
    CheckBox&       rBtnCopyResult;
    CheckBox&       rBtnDestPers;
    ListBox&        rLbCopyPos;
    Edit&           rEdCopyPos;
    ScRefButton&    rRbCopyPos;
    FixedText&      rFtDbAreaLabel;
    FixedInfo&      rFtDbArea;
    GroupBox&       rGbOptions;

    const String&   rStrNoName;
    const String&   rStrUndefined;

    const ScQueryParam& rQueryData;

#ifdef _FOPTMGR_CXX
private:
    void Init();

    // Handler:
    DECL_LINK( EdPosModifyHdl,      Edit* );
    DECL_LINK( LbPosSelHdl,         ListBox* );
    DECL_LINK( BtnCopyResultHdl,    CheckBox* );
#endif
};



#endif // SC_FOPTMGR_HXX
