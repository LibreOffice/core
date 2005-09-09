/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbui.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:09:37 $
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
#ifndef _DBUI_HXX
#define _DBUI_HXX

#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif


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

