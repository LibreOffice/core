/*************************************************************************
 *
 *  $RCSfile: optcomp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 10:01:02 $
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
#ifndef _OPTCOMP_HXX
#define _OPTCOMP_HXX

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COMPATIBILITY_HXX
#include <svtools/compatibility.hxx>
#endif
#ifndef _SVX_CHECKLBX_HXX
#include <svx/checklbx.hxx>
#endif

// class SwCompatibilityOptPage ------------------------------------------

class SwWrtShell;
struct SwCompatibilityOptPage_Impl;

class SwCompatibilityOptPage : public SfxTabPage
{
private:
    // controls
    FixedLine               m_aMainFL;
    FixedText               m_aFormattingFT;
    ListBox                 m_aFormattingLB;
    FixedText               m_aOptionsFT;
    SvxCheckListBox         m_aOptionsLB;
    PushButton              m_aResetPB;
    PushButton              m_aDefaultPB;
    // config item
    SvtCompatibilityOptions m_aConfigItem;
    // text of the user entry
    String                  m_sUserEntry;
    // text of the "use as default" querybox
    String                  m_sUseAsDefaultQuery;
    // shell of the current document
    SwWrtShell*             m_pWrtShell;
    // impl object
    SwCompatibilityOptPage_Impl* m_pImpl;
    // saved options after "Reset"; used in "FillItemSet" for comparison
    ULONG                   m_nSavedOptions;

    // handler
    DECL_LINK(              SelectHdl, ListBox* );
    DECL_LINK(              CheckHdl, SvxCheckListBox* );
    DECL_LINK(              UseAsDefaultHdl, PushButton* );

    // private methods
    void                    InitControls( const SfxItemSet& rSet );
    void                    SetCurrentOptions( ULONG nOptions );
    ULONG                   GetCurrentOptions() const;
    void                    SetDocumentOptions( ULONG nOptions );
    ULONG                   GetDocumentOptions() const;
    void                    WriteOptions();

public:
    SwCompatibilityOptPage( Window* pParent, const SfxItemSet& rSet );
    ~SwCompatibilityOptPage();

    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual BOOL            FillItemSet( SfxItemSet& rSet );
    virtual void            Reset( const SfxItemSet& rSet );
};

#endif

