/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macropg_impl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-14 07:18:30 $
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

#ifndef _MACROPG_IMPL_HXX
#define _MACROPG_IMPL_HXX

class _SvxMacroTabPage_Impl
{
public:
                                    _SvxMacroTabPage_Impl( const SfxItemSet& rAttrSet );
                                    ~_SvxMacroTabPage_Impl();

    FixedText*                      pAssignFT;
    PushButton*                     pAssignPB;
    PushButton*                     pAssignComponentPB;
    PushButton*                     pDeletePB;
    Image*                          pMacroImg;
    Image*                          pComponentImg;
    Image*                          pMacroImg_h;
    Image*                          pComponentImg_h;
    String*                         pStrEvent;
    String*                         pAssignedMacro;
    _HeaderTabListBox*              pEventLB;
    BOOL                            bReadOnly;
    BOOL                            bIDEDialogMode;
};

class AssignComponentDialog : public ModalDialog
{
private:
    FixedText       maMethodLabel;
    Edit            maMethodEdit;
    OKButton        maOKButton;
    CancelButton    maCancelButton;
    HelpButton      maHelpButton;

    ::rtl::OUString maURL;

    DECL_LINK(ButtonHandler, Button *);

public:
    AssignComponentDialog( Window * pParent, const ::rtl::OUString& rURL );
    ~AssignComponentDialog();

    ::rtl::OUString getURL( void ) const
        { return maURL; }
};

#endif
