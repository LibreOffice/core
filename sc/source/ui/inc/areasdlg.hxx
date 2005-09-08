/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: areasdlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:12:08 $
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

#ifndef SC_AREASDLG_HXX
#define SC_AREASDLG_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef SC_ANYREFDG_HXX
#include <anyrefdg.hxx>
#endif

class ScDocument;
class ScViewData;
class ScRangeUtil;
class ScRangeItem;


//============================================================================

class ScPrintAreasDlg : public ScAnyRefDlg
{
public:
                    ScPrintAreasDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent );
                    ~ScPrintAreasDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual void    AddRefEntry();

    virtual BOOL    IsTableLocked() const;

    virtual void    SetActive();
    virtual void    Deactivate();
    virtual BOOL    Close();

private:
    ListBox         aLbPrintArea;
    FixedLine       aFlPrintArea;
    ScRefEdit       aEdPrintArea;
    ScRefButton     aRbPrintArea;

    ListBox         aLbRepeatRow;
    FixedLine       aFlRepeatRow;
    ScRefEdit       aEdRepeatRow;
    ScRefButton     aRbRepeatRow;

    ListBox         aLbRepeatCol;
    FixedLine       aFlRepeatCol;
    ScRefEdit       aEdRepeatCol;
    ScRefButton     aRbRepeatCol;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    BOOL            bDlgLostFocus;
    ScRefEdit*      pRefInputEdit;
    ScDocument*     pDoc;
    ScViewData*     pViewData;
    SCTAB           nCurTab;

#ifdef _AREASDLG_CXX
private:
    void Impl_Reset();
    BOOL Impl_CheckRefStrings();
    void Impl_FillLists();
    BOOL Impl_GetItem( Edit* pEd, SfxStringItem& rItem );

    // Handler:
    DECL_LINK( Impl_SelectHdl,      ListBox*    );
    DECL_LINK( Impl_ModifyHdl,      ScRefEdit*  );
    DECL_LINK( Impl_BtnHdl,         PushButton* );
    DECL_LINK( Impl_GetFocusHdl,    Control*    );
#endif
};



#endif

