/*************************************************************************
 *
 *  $RCSfile: connect.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-02-03 18:17:42 $
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

