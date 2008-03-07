/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optload.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:33:19 $
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
#ifndef _OPTLOAD_HXX
#define _OPTLOAD_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SVX_STRARRAY_HXX
#include <svx/strarray.hxx>
#endif
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SVX_CHECKLBX_HXX //autogen
#include <svx/checklbx.hxx>
#endif
#ifndef _SWLBOX_HXX
#include <swlbox.hxx>
#endif
#ifndef _CAPTION_HXX
#include <caption.hxx>
#endif

class SwFldMgr;
class SvLBoxEntry;
class SwWrtShell;

/*-----------------31.08.96 10.09-------------------

--------------------------------------------------*/

class SwLoadOptPage : public SfxTabPage
{
private:
    FixedLine   aUpdateFL;
    FixedText   aLinkFT;
    RadioButton aAlwaysRB;
    RadioButton aRequestRB;
    RadioButton aNeverRB;

    FixedText   aFieldFT;
    CheckBox    aAutoUpdateFields;
    CheckBox    aAutoUpdateCharts;

    FixedLine   aSettingsFL;
    FixedText   aMetricFT;
    ListBox     aMetricLB;
    FixedText   aTabFT;
    MetricField aTabMF;
    CheckBox    aUseSquaredPageMode;

    SwWrtShell* pWrtShell;
    sal_Bool    bHTMLMode;
    UINT16      nLastTab;
    sal_Int32   nOldLinkMode;

    DECL_LINK(CaptionHdl, PushButton*);
    DECL_LINK(MetricHdl, ListBox*);

public:
    SwLoadOptPage( Window* pParent, const SfxItemSet& rSet );
    ~SwLoadOptPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/
class SwCaptionOptDlg : public SfxSingleTabDialog
{
public:
     SwCaptionOptDlg(Window* pParent, const SfxItemSet& rSet);
    ~SwCaptionOptDlg();
};
/* -----------------23.10.98 13:19-------------------
 *
 * --------------------------------------------------*/

class CaptionComboBox : public SwComboBox
{
protected:
    virtual void KeyInput( const KeyEvent& );

public:
    CaptionComboBox( Window* pParent, const ResId& rResId)
        : SwComboBox(pParent, rResId)
    {}
};

/*-----------------31.08.96 10.09-------------------

--------------------------------------------------*/

class SwCaptionPreview : public Window
{
private:
    String          maText;
    Point           maDrawPos;
public:
                    SwCaptionPreview( Window* pParent, const ResId& rResId );
    void            SetPreviewText( const String& rText );
    virtual void    Paint( const Rectangle& rRect );
};

class SwCaptionOptPage : public SfxTabPage
{
private:
    FixedText       aCheckFT;
    SvxCheckListBox aCheckLB;

    FixedText       aFtCaptionOrder;
    ListBox         aLbCaptionOrder;

    SwCaptionPreview    aPreview;

    FixedLine       aSettingsGroupFL;
    FixedText       aCategoryText;
    CaptionComboBox aCategoryBox;
    FixedText       aFormatText;
    ListBox         aFormatBox;
    //#i61007# order of captions
    FixedText       aNumberingSeparatorFT;
    Edit            aNumberingSeparatorED;
    FixedText       aTextText;
    Edit            aTextEdit;
    FixedText       aPosText;
    ListBox         aPosBox;

    FixedLine       aNumCaptFL;
    FixedText       aFtLevel;
    ListBox         aLbLevel;
    FixedText       aFtDelim;
    Edit            aEdDelim;

    FixedLine       aCategoryFL;
    FixedText       aCharStyleFT;
    ListBox         aCharStyleLB;
    CheckBox        aApplyBorderCB;

    String          sSWTable;
    String          sSWFrame;
    String          sSWGraphic;
    String          sOLE;

    String          sIllustration;
    String          sTable;
    String          sText;
    String          sDrawing;

    String          sBegin;
    String          sEnd;
    String          sAbove;
    String          sBelow;

    String          sNone;

    SwFldMgr        *pMgr;
    USHORT          eType;
    BOOL            bHTMLMode;

    DECL_LINK( SelectHdl, ListBox *pLB = 0 );
    DECL_LINK( ModifyHdl, Edit *pEdt = 0 );
    DECL_LINK( OrderHdl, ListBox* );
    DECL_LINK( ShowEntryHdl, SvxCheckListBox *pLB = 0 );
    DECL_LINK( SaveEntryHdl, SvxCheckListBox *pLB = 0 );

    void                DelUserData();
    void                SetOptions( const USHORT nPos,
                                    const SwCapObjType eType,
                                    const SvGlobalName *pOleId = 0);
    void                SaveEntry( SvLBoxEntry* pEntry );
    void                DrawSample();

public:
                        SwCaptionOptPage( Window* pParent,
                                         const SfxItemSet& rSet );
                        ~SwCaptionOptPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif


