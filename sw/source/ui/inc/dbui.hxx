/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dbui.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _DBUI_HXX
#define _DBUI_HXX

#include <svx/stddlg.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>


class PrintMonitor: public ModelessDialog
{
public:
    FixedText       aDocName;
    FixedText       aPrinting;
    FixedText       aPrinter;
    FixedText       aPrintInfo;
    CancelButton    aCancel;

    PrintMonitor( Window *pParent, BOOL bEMail = FALSE );
};

class CreateMonitor : public ModelessDialog
{
public:
    CreateMonitor( Window *pParent );

    void SetTotalCount( sal_Int32 nTotal );
    void SetCurrentPosition( sal_Int32 nCurrent );

    void SetCancelHdl( const Link& rLink );

private: //methods
    void UpdateCountingText();

private: //member
    FixedText       m_aStatus;
    FixedText       m_aProgress;
    FixedText       m_aCreateDocuments;
    FixedText       m_aCounting;
    CancelButton    m_aCancelButton;

    String          m_sCountingPattern;
    String          m_sVariable_Total;
    String          m_sVariable_Position;
    sal_Int32       m_nTotalCount;
    sal_Int32       m_nCurrentPosition;
};

#endif

