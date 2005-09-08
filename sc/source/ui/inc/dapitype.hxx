/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dapitype.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:19:20 $
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

#ifndef SC_DAPITYPE_HXX
#define SC_DAPITYPE_HXX

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

//------------------------------------------------------------------------

class ScDataPilotSourceTypeDlg : public ModalDialog
{
private:
    FixedLine       aFlFrame;
    RadioButton     aBtnSelection;
    RadioButton     aBtnDatabase;
    RadioButton     aBtnExternal;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

public:
            ScDataPilotSourceTypeDlg( Window* pParent, BOOL bEnableExternal );
            ~ScDataPilotSourceTypeDlg();

    BOOL    IsSelection() const;
    BOOL    IsDatabase() const;
    BOOL    IsExternal() const;
};


class ScDataPilotServiceDlg : public ModalDialog
{
private:
    FixedLine       aFlFrame;
    FixedText       aFtService;
    ListBox         aLbService;
    FixedText       aFtSource;
    Edit            aEdSource;
    FixedText       aFtName;
    Edit            aEdName;
    FixedText       aFtUser;
    Edit            aEdUser;
    FixedText       aFtPasswd;
    Edit            aEdPasswd;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

public:
            ScDataPilotServiceDlg( Window* pParent,
                    const com::sun::star::uno::Sequence<rtl::OUString>& rServices );
            ~ScDataPilotServiceDlg();

    String  GetServiceName() const;
    String  GetParSource() const;
    String  GetParName() const;
    String  GetParUser() const;
    String  GetParPass() const;
};


#endif


