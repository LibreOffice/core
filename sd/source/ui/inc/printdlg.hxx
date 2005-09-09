/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: printdlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:49:54 $
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


#ifndef _SD_PRINTDLG_HXX_
#define _SD_PRINTDLG_HXX_


#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif


/*************************************************************************
|*
|*    SdPrintDlg::SdPrintDlg()
|*
|*    Beschreibung  Dialog zum Einstellen von Printoptionen
|*
*************************************************************************/

class SdPrintDlg : public ModalDialog
{
private:
    FixedInfo           aFtInfo;
    RadioButton         aRbtScale;
    RadioButton         aRbtPoster;
    RadioButton         aRbtCut;
    FixedLine           aGrpOptions;
    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

public:

            SdPrintDlg( Window* pWindow );

    USHORT  GetAttr();
};

#endif // _SD_PRINTDLG_HXX_

