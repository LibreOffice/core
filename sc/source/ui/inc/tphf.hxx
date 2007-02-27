/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tphf.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 13:26:55 $
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

#ifndef SC_TPHF_HXX
#define SC_TPHF_HXX


#ifndef _SVX_HDFT_HXX //autogen
#include <svx/hdft2.hxx>
#endif

class ScStyleDlg;

//========================================================================

class ScHFPage : public SvxHFPage
{
public:
    virtual         ~ScHFPage();

    virtual void    Reset( const SfxItemSet& rSet );
    virtual BOOL    FillItemSet( SfxItemSet& rOutSet );

    void            SetPageStyle( const String& rName )    { aStrPageStyle = rName; }
    void            SetStyleDlg ( const ScStyleDlg* pDlg ) { pStyleDlg = pDlg; }

protected:
                    ScHFPage( Window* pParent,
                              USHORT nResId,
                              const SfxItemSet& rSet,
                              USHORT nSetId );

//    using SvxHFPage::ActivatePage;
//    using SvxHFPage::DeactivatePage;
    virtual void    ActivatePage();
    virtual void    DeactivatePage();
    virtual void    ActivatePage( const SfxItemSet& rSet );
    virtual int     DeactivatePage( SfxItemSet* pSet = 0 );

private:
    PushButton          aBtnEdit;
    SfxItemSet          aDataSet;
    String              aStrPageStyle;
    USHORT              nPageUsage;
    const ScStyleDlg*   pStyleDlg;

#ifdef _TPHF_CXX
private:
    DECL_LINK( BtnHdl, PushButton* );
    DECL_LINK( HFEditHdl, void* );
    DECL_LINK( TurnOnHdl, CheckBox* );
#endif
};

//========================================================================

class ScHeaderPage : public ScHFPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static USHORT*      GetRanges();

private:
    ScHeaderPage( Window* pParent, const SfxItemSet& rSet );
};

//========================================================================

class ScFooterPage : public ScHFPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static USHORT*      GetRanges();

private:
    ScFooterPage( Window* pParent, const SfxItemSet& rSet );
};


#endif // SC_TPHF_HXX

