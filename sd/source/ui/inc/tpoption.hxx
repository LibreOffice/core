/*************************************************************************
 *
 *  $RCSfile: tpoption.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2001-03-22 14:13:39 $
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

#ifndef _SD_TPOPTION_HXX
#define _SD_TPOPTION_HXX


#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SVX_OPTGRID_HXX
#include <svx/optgrid.hxx>
#endif

/*************************************************************************
|*
|* Optionen-Tab-Page: Snap
|*
\************************************************************************/
class SdTpOptionsSnap : public SvxGridTabPage
{
public:
            SdTpOptionsSnap( Window* pParent, const SfxItemSet& rInAttrs  );
            ~SdTpOptionsSnap();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual BOOL FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

//    virtual void ActivatePage( const SfxItemSet& rSet );
//    virtual int  DeactivatePage( SfxItemSet* pSet );
};
/*************************************************************************
|*
|* Optionen-Tab-Page: Contents (Inhalte)
|*
\************************************************************************/
class SdTpOptionsContents : public SfxTabPage
{
private:
    FixedLine aGrpViewSubstitute;
    CheckBox aCbxExternGraphic;
    CheckBox aCbxOutlineMode;
    CheckBox aCbxNoText;
    CheckBox aCbxHairlineMode;

    FixedLine    aGrpDisplay;
    CheckBox    aCbxRuler;
    CheckBox    aCbxDragStripes;
    CheckBox    aCbxHandlesBezier;
    CheckBox    aCbxMoveOutline;


public:
            SdTpOptionsContents( Window* pParent, const SfxItemSet& rInAttrs  );
            ~SdTpOptionsContents();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual BOOL FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );
};

/*************************************************************************
|*
|* Optionen-Tab-Page: View
|*
\************************************************************************/
class SdModule;
class SdTpOptionsMisc : public SfxTabPage
{
 friend class SdOptionsDlg;
 friend class SdModule;

private:
    FixedLine   aGrpText;
    CheckBox    aCbxQuickEdit;
    CheckBox    aCbxPickThrough;

    FixedLine   aGrpProgramStart;
    CheckBox    aCbxStartWithTemplate;

    FixedLine   aGrpOther;
    CheckBox    aCbxMasterPageCache;
    CheckBox    aCbxCopy;
    CheckBox    aCbxMarkedHitMovesAlways;
    CheckBox    aCbxCrookNoContortion;

    FixedLine   aGrpStartWithActualPage;
    CheckBox    aCbxStartWithActualPage;

    FixedLine   aGrpSettings;
    FixedText   aTxtMetric;
    ListBox     aLbMetric;

    FixedText   aTxtTabstop;
    MetricField aMtrFldTabstop;

    DECL_LINK( SelectMetricHdl_Impl, ListBox * );

protected:
    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int DeactivatePage( SfxItemSet* pSet );

public:
            SdTpOptionsMisc( Window* pParent, const SfxItemSet& rInAttrs  );
            ~SdTpOptionsMisc();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual BOOL FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    //hide impress and show draw controls
    void    SetDrawMode();
};


#endif // _SD_TPOPTION_HXX

