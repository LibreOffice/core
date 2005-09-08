/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connect.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:44:18 $
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
#ifndef _SVX_CONNECT_HXX
#define _SVX_CONNECT_HXX

// include ---------------------------------------------------------------

#ifndef _SVX_CONNCTRL_HXX
#include "connctrl.hxx"
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

class SdrView;

/*************************************************************************
|*
|* Dialog zum Aendern von Konnektoren (Connectors)
|*
\************************************************************************/

class SvxConnectionPage : public SfxTabPage
{
private:
    FixedText           aFtType;
    ListBox             aLbType;

    FixedLine           aFlDelta;
    FixedText           aFtLine1;
    MetricField         aMtrFldLine1;
    FixedText           aFtLine2;
    MetricField         aMtrFldLine2;
    FixedText           aFtLine3;
    MetricField         aMtrFldLine3;

    FixedLine           aFlDistance;
    FixedText           aFtHorz1;
    MetricField         aMtrFldHorz1;
    FixedText           aFtVert1;
    MetricField         aMtrFldVert1;
    FixedText           aFtHorz2;
    MetricField         aMtrFldHorz2;
    FixedText           aFtVert2;
    MetricField         aMtrFldVert2;

    SvxXConnectionPreview   aCtlPreview;

    const SfxItemSet&   rOutAttrs;
    SfxItemSet          aAttrSet;
    const SdrView*      pView;
    SfxMapUnit          eUnit;

#ifdef _SVX_CONNECT_CXX
    void                FillTypeLB();

                        DECL_LINK( ChangeAttrHdl_Impl, void * );
#endif

public:

    SvxConnectionPage( Window* pWindow, const SfxItemSet& rInAttrs );
    ~SvxConnectionPage();

    static SfxTabPage*  Create( Window*, const SfxItemSet& );
    static  USHORT*     GetRanges();

    virtual BOOL        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet & );

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
    virtual void PageCreated (SfxAllItemSet aSet); //add CHINA001
};

/*************************************************************************
|*
|* Von SfxSingleTabDialog abgeleitet, um vom Control "uber virtuelle Methode
|* benachrichtigt werden zu k"onnen.
|*
\************************************************************************/

class SvxConnectionDialog : public SfxSingleTabDialog
{
public:
    SvxConnectionDialog( Window* pParent, const SfxItemSet& rAttr,
                       const SdrView* pView );
    ~SvxConnectionDialog();
};


#endif // _SVX_CONNECT_HXX

