/*************************************************************************
 *
 *  $RCSfile: optgrid.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:00:58 $
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
#ifndef _SVX_OPTGRID_HXX
#define _SVX_OPTGRID_HXX

// include ---------------------------------------------------------------

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

class SvxGridTabPage;

// class SvxOptionsGrid --------------------------------------------------

class SvxOptionsGrid
{
protected:
    UINT32  nFldDrawX;
    UINT32  nFldDivisionX;
    UINT32  nFldDrawY;
    UINT32  nFldDivisionY;
    UINT32  nFldSnapX;
    UINT32  nFldSnapY;
    BOOL    bUseGridsnap:1;
    BOOL    bSynchronize:1;
    BOOL    bGridVisible:1;
    BOOL    bEqualGrid:  1;

public:
    SvxOptionsGrid();
    ~SvxOptionsGrid();

    void    SetFldDrawX(    UINT32 nSet){nFldDrawX      = nSet;}
    void    SetFldDivisionX(UINT32 nSet){nFldDivisionX  = nSet;}
    void    SetFldDrawY   ( UINT32 nSet){nFldDrawY      = nSet;}
    void    SetFldDivisionY(UINT32 nSet){nFldDivisionY  = nSet;}
    void    SetFldSnapX(    UINT32 nSet){nFldSnapX      = nSet;}
    void    SetFldSnapY   ( UINT32 nSet){nFldSnapY      = nSet;}
    void    SetUseGridSnap( BOOL bSet ) {bUseGridsnap   = bSet;}
    void    SetSynchronize( BOOL bSet ) {bSynchronize   = bSet;}
    void    SetGridVisible( BOOL bSet ) {bGridVisible   = bSet;}
    void    SetEqualGrid( BOOL bSet )   {bEqualGrid     = bSet;}

    UINT32  GetFldDrawX(    ) const {  return nFldDrawX;    }
    UINT32  GetFldDivisionX() const {  return nFldDivisionX;}
    UINT32  GetFldDrawY   ( ) const {  return nFldDrawY;    }
    UINT32  GetFldDivisionY() const {  return nFldDivisionY;}
    UINT32  GetFldSnapX(    ) const {  return nFldSnapX;    }
    UINT32  GetFldSnapY   ( ) const {  return nFldSnapY;    }
    BOOL    GetUseGridSnap( ) const {  return bUseGridsnap; }
    BOOL    GetSynchronize( ) const {  return bSynchronize; }
    BOOL    GetGridVisible( ) const {  return bGridVisible; }
    BOOL    GetEqualGrid()    const {  return bEqualGrid;   }
};

// class SvxGridItem -----------------------------------------------------

class SvxGridItem : public SvxOptionsGrid, public SfxPoolItem
{
    friend SvxGridTabPage;

public:
    SvxGridItem( USHORT nWhich) : SfxPoolItem(nWhich){};
    SvxGridItem( const SvxGridItem& pTestItem );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const International * = 0 ) const;

};

// class SvxGridTabPage --------------------------------------------------

class SvxGridTabPage : public SfxTabPage
{
public:
    SvxGridTabPage( Window* pParent, const SfxItemSet& rSet );

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet );

    void                HideSnapGrid();

private:
    CheckBox            aCbxUseGridsnap;
    CheckBox            aCbxGridVisible;
    CheckBox            aCbxSynchronize;
    GroupBox            aGrpOptions; // Neu

    FixedText           aFtResolution;
    FixedText           aFtDrawX;
    MetricField         aMtrFldDrawX;
    FixedText           aFtDrawY;
    MetricField         aMtrFldDrawY;
    FixedText           aFtDivision;
    MetricField         aMtrFldDivisionX;
    MetricField         aMtrFldDivisionY;
    GroupBox            aGrpDrawGrid; // Neu

    CheckBox            aCbxEqualGrid; // Neu
    FixedText           aFtSnapX; // Neu
    MetricField         aMtrFldSnapX; // Neu
    FixedText           aFtSnapY; // Neu
    MetricField         aMtrFldSnapY; // Neu
    GroupBox            aGrpSnapGrid; // Neu

    BOOL                bAttrModified;
    BOOL                bEqualGrid; // Neu

#ifdef _SVX_OPTGRID_CXX
    DECL_LINK( ChangeDrawXHdl_Impl, void * );
    DECL_LINK( ChangeDrawYHdl_Impl, void * );
    DECL_LINK( ChangeSnapXHdl_Impl, void * );
    DECL_LINK( ChangeSnapYHdl_Impl, void * );
    DECL_LINK( ChangeGridsnapHdl_Impl, void * );
    DECL_LINK( ChangeEqualGridHdl_Impl, void * );
    DECL_LINK( ChangeDivisionXHdl_Impl, void * );
    DECL_LINK( ChangeDivisionYHdl_Impl, void * );

    //void      Init();
    //void      AutoClickHdl(CheckBox* pBox);
    //void      ModifyHdl(Edit* );
#endif
};


#endif

