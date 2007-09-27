/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optcomp.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:06:30 $
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
    DECL_LINK(              UseAsDefaultHdl, PushButton* );

    // private methods
    void                    InitControls( const SfxItemSet& rSet );
    void                    ReplaceFormatName( String& rEntry );
    void                    SetCurrentOptions( ULONG nOptions );
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

