/*************************************************************************
 *
 *  $RCSfile: envfmt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:35 $
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
#ifndef _ENVFMT_HXX
#define _ENVFMT_HXX


#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#include "envlop.hxx"

// class SwEnvFmtPage ---------------------------------------------------------

class SwEnvFmtPage : public SfxTabPage
{
    FixedInfo    aAddrInfo;
    FixedText    aAddrLeftText;
    MetricField  aAddrLeftField;
    FixedText    aAddrTopText;
    MetricField  aAddrTopField;
    MenuButton   aAddrEditButton;
    FixedInfo    aSendInfo;
    FixedText    aSendLeftText;
    MetricField  aSendLeftField;
    FixedText    aSendTopText;
    MetricField  aSendTopField;
    MenuButton   aSendEditButton;
    SwEnvPreview aPreview;
    GroupBox     aPositionsGroup;
    FixedText    aFormatText;
    ListBox      aFormatBox;
    FixedText    aWidthText;
    MetricField  aWidthField;
    FixedText    aHeightText;
    MetricField  aHeightField;
    GroupBox     aSizeGroup;

    SvUShorts  aIDs;
    Timer      aPreviewTimer;
    Edit*      pLastEdit;

     SwEnvFmtPage(Window* pParent, const SfxItemSet& rSet);
    ~SwEnvFmtPage();

    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( EditHdl, MenuButton * );
    DECL_LINK( FormatHdl, ListBox * );
    DECL_LINK( PreviewHdl, Timer * );

    void SetMinMax();

    SfxItemSet  *GetCollItemSet(SwTxtFmtColl* pColl, BOOL bSender);
    SwEnvDlg    *GetParent() {return (SwEnvDlg*) SfxTabPage::GetParent()->GetParent();}

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
            void FillItem(SwEnvItem& rItem);
    virtual BOOL FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);
};

#endif


