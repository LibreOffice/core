/*************************************************************************
 *
 *  $RCSfile: labdlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-28 14:04:20 $
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
#ifndef _SVX_LABDLG_HXX
#define _SVX_LABDLG_HXX

// include ---------------------------------------------------------------


#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
class SdrView;

// class SvxCaptionTabPage -----------------------------------------------

const sal_uInt16 CAPTYPE_BITMAPS_COUNT = 3;

class SvxCaptionTabPage : public SfxTabPage
{
private:
    ValueSet        aCT_CAPTTYPE;
    FixedText       aFT_ABSTAND;
    MetricField     aMF_ABSTAND;
    FixedText       aFT_WINKEL;
    ListBox         aLB_WINKEL;
    FixedText       aFT_ANSATZ;
    ListBox         aLB_ANSATZ;
    FixedText       aFT_UM;
    MetricField     aMF_ANSATZ;
    FixedText       aFT_ANSATZ_REL;
    ListBox         aLB_ANSATZ_REL;
    FixedText       aFT_LAENGE;
    MetricField     aMF_LAENGE;
    CheckBox        aCB_LAENGE;

    Image*          mpBmpCapTypes[CAPTYPE_BITMAPS_COUNT];
    Image*          mpBmpCapTypesH[CAPTYPE_BITMAPS_COUNT];

    String          aStrHorzList;
    String          aStrVertList;

    short           nCaptionType;
    BOOL            bFixedAngle;
    INT32           nFixedAngle;
    INT32           nGap;
    short           nEscDir;
    BOOL            bEscRel;
    INT32           nEscAbs;
    INT32           nEscRel;
    INT32           nLineLen;
    BOOL            bFitLineLen;

    USHORT          nAnsatzRelPos;
    USHORT          nAnsatzTypePos;
    USHORT          nWinkelTypePos;

#ifdef _SVX_LABDLG_CXX
    void            SetupAnsatz_Impl( USHORT nType );
    void            SetupType_Impl( USHORT nType );
    DECL_LINK( AnsatzSelectHdl_Impl, ListBox * );
    DECL_LINK( AnsatzRelSelectHdl_Impl, ListBox * );
    DECL_LINK( LineOptHdl_Impl, Button * );
    DECL_LINK( SelectCaptTypeHdl_Impl, void * );
#endif

    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

public:
    SvxCaptionTabPage( Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxCaptionTabPage();

    static SfxTabPage*  Create( Window*, const SfxItemSet& );
    static USHORT*      GetRanges();

    virtual BOOL        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet & );
    void                Construct();
    void                SetView( const SdrView* pSdrView )
                            { pView = pSdrView; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
    void FillValueSet();
};

// class SvxCaptionTabDialog ---------------------------------------------

class SvxCaptionTabDialog : public SfxTabDialog
{
private:
//  const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;
    USHORT              nAnchorCtrls;

    Link                aValidateLink;

    virtual void        PageCreated( USHORT nId, SfxTabPage &rPage );

public:

            SvxCaptionTabDialog(Window* pParent, const SdrView* pView,
                                    USHORT nAnchorTypes = 0 );

            ~SvxCaptionTabDialog();

            //link for the Writer to validate positions
            void SetValidateFramePosLink( const Link& rLink );
};


#endif //_SVX_LABDLG_HXX

