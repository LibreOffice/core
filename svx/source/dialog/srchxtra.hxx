/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: srchxtra.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:05:25 $
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
#ifndef _SVX_SRCHXTRA_HXX
#define _SVX_SRCHXTRA_HXX

// include ---------------------------------------------------------------

#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif

#include "checklbx.hxx"
#include "srchdlg.hxx"

// class SvxSearchFormatDialog -------------------------------------------

class SvxSearchFormatDialog : public SfxTabDialog
{
public:
    SvxSearchFormatDialog( Window* pParent, const SfxItemSet& rSet );
    ~SvxSearchFormatDialog();

protected:
    virtual void    PageCreated( USHORT nId, SfxTabPage &rPage );

private:
    FontList*       pFontList;
};

// class SvxSearchFormatDialog -------------------------------------------

class SvxSearchAttributeDialog : public ModalDialog
{
public:
    SvxSearchAttributeDialog( Window* pParent, SearchAttrItemList& rLst,
                              const USHORT* pWhRanges );

private:
    FixedText           aAttrFL;
    SvxCheckListBox     aAttrLB;

    OKButton            aOKBtn;
    CancelButton        aEscBtn;
    HelpButton          aHelpBtn;

    SearchAttrItemList& rList;

    DECL_LINK( OKHdl, Button * );
};

// class SvxSearchSimilarityDialog ---------------------------------------

class SvxSearchSimilarityDialog : public ModalDialog
{
private:
    FixedLine           aFixedLine;
    FixedText           aOtherTxt;
    NumericField        aOtherFld;
    FixedText           aLongerTxt;
    NumericField        aLongerFld;
    FixedText           aShorterTxt;
    NumericField        aShorterFld;
    CheckBox            aRelaxBox;

    OKButton            aOKBtn;
    CancelButton        aEscBtn;
    HelpButton          aHelpBtn;

public:
    SvxSearchSimilarityDialog(  Window* pParent,
                                BOOL bRelax,
                                USHORT nOther,
                                USHORT nShorter,
                                USHORT nLonger );

    USHORT  GetOther()      { return (USHORT)aOtherFld.GetValue(); }
    USHORT  GetShorter()    { return (USHORT)aShorterFld.GetValue(); }
    USHORT  GetLonger()     { return (USHORT)aLongerFld.GetValue(); }
    BOOL    IsRelaxed()     { return aRelaxBox.IsChecked(); }
};


#endif

