/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: splittbl.hxx,v $
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
#ifndef _SPLITTBL_HXX
#define _SPLITTBL_HXX

#include <svx/stddlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>

class SwWrtShell;

class SwSplitTblDlg : public SvxStandardDialog
{
    OKButton        aOKPB;
    CancelButton    aCancelPB;
    HelpButton      aHelpPB;
    FixedLine       aSplitFL;
    RadioButton     aCntntCopyRB;
    RadioButton     aBoxAttrCopyWithParaRB ;
    RadioButton     aBoxAttrCopyNoParaRB ;
    RadioButton     aBorderCopyRB;

    SwWrtShell      &rShell;
    USHORT          m_nSplit;

protected:
    virtual void Apply();

public:
    SwSplitTblDlg( Window *pParent, SwWrtShell &rSh );

    USHORT GetSplitMode() const { return m_nSplit; }
};

#endif


