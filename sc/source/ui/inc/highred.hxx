/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: highred.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:32:19 $
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

#ifndef SC_HIGHRED_HXX
#define SC_HIGHRED_HXX

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _HEADBAR_HXX //autogen
#include <svtools/headbar.hxx>
#endif

#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif


#ifndef SC_RANGENAM_HXX
#include "rangenam.hxx"
#endif

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"
#endif

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SVX_ACREDLIN_HXX
#include <svx/ctredlin.hxx>
#endif

#ifndef _SVX_SIMPTABL_HXX
#include <svx/simptabl.hxx>
#endif

#ifndef SC_CHGTRACK_HXX
#include "chgtrack.hxx"
#endif

#ifndef SC_CHGVISET_HXX
#include "chgviset.hxx"
#endif

class ScViewData;
class ScDocument;

#ifndef FLT_DATE_BEFORE
#define FLT_DATE_BEFORE     0
#define FLT_DATE_SINCE      1
#define FLT_DATE_EQUAL      2
#define FLT_DATE_NOTEQUAL   3
#define FLT_DATE_BETWEEN    4
#define FLT_DATE_SAVE       5
#endif

//==================================================================

class ScHighlightChgDlg : public ScAnyRefDlg
{
private:

    CheckBox                aHighlightBox;
    FixedLine               aFlFilter;
    SvxTPFilter             aFilterCtr;
    CheckBox                aCbAccept;
    CheckBox                aCbReject;

    OKButton                aOkButton;
    CancelButton            aCancelButton;
    HelpButton              aHelpButton;

    ScRefEdit               aEdAssign;
    ScRefButton             aRbAssign;

    ScViewData*             pViewData;
    ScDocument*             pDoc;
    ScRangeName             aLocalRangeName;
    Selection               theCurSel;
    Size                    MinSize;
    ScRangeList             aRangeList;
    ScChangeViewSettings    aChangeViewSet;

    void                    Init();

    DECL_LINK( RefHandle, SvxTPFilter* );
    DECL_LINK(HighLightHandle, CheckBox*);
    DECL_LINK(OKBtnHdl, PushButton*);


protected:

    virtual void    RefInputDone( BOOL bForced = FALSE );

public:
                    ScHighlightChgDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                               ScViewData*      ptrViewData);

                    ~ScHighlightChgDlg();

    virtual void    SetActive();
    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual BOOL    Close();
    virtual BOOL    IsRefInputMode() const;

};


#endif // SC_NAMEDLG_HXX

