/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optlingu.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:03:02 $
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
#ifndef _SVX_OPTLINGU_HXX
#define _SVX_OPTLINGU_HXX

// include ---------------------------------------------------------------

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SVX_CHECKLBX_HXX //autogen
#include <svx/checklbx.hxx>
#endif
#ifndef _SVX_LANGBOX_HXX
#include <svx/langbox.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

namespace com{namespace sun{namespace star{
namespace beans{
    class XPropertySet;
}
namespace linguistic2{
    class XDictionary;
    class XDictionaryList;
    class XSpellChecker1;
}}}}

class SvLBoxEntry;
class SvxLinguData_Impl;

// define ----------------------------------------------------------------

#define GROUP_MODULES   ((sal_uInt16)0x0008)

// forward ---------------------------------------------------------------

class SvxEditModulesDlg : public ModalDialog
{
    OKButton            aClosePB;
    HelpButton          aHelpPB;

    FixedLine           aModulesFL;

    FixedText           aLanguageFT;
    SvxLanguageBox      aLanguageLB;

    SvxCheckListBox     aModulesCLB;
    PushButton          aPrioUpPB;
    PushButton          aPrioDownPB;
    PushButton          aBackPB;

    String              sSpell;
    String              sHyph;
    String              sThes;

    SvxLinguData_Impl*  pDefaultLinguData;
    SvxLinguData_Impl&  rLinguData;

    SvLBoxButtonData*   pCheckButtonData;

    SvLBoxEntry*    CreateEntry(String& rTxt, USHORT nCol);

#ifdef _SVX_OPTLINGU_CXX
    DECL_LINK( SelectHdl_Impl, SvxCheckListBox * );
    DECL_LINK( UpDownHdl_Impl, PushButton * );
    DECL_LINK( ClickHdl_Impl, PushButton * );
    DECL_LINK( BackHdl_Impl, PushButton * );
    DECL_LINK( SelectHdlLB_Impl, ListBox * );
    DECL_LINK( LangSelectHdl_Impl, ListBox* );
    DECL_LINK( BoxCheckButtonHdl_Impl, SvTreeListBox * );
#endif

public:
    SvxEditModulesDlg(Window* pParent, SvxLinguData_Impl& rData);
    virtual ~SvxEditModulesDlg();
};

// class SvxLinguTabPage -------------------------------------------------

class SVX_DLLPUBLIC SvxLinguTabPage : public SfxTabPage
{
private:
    FixedLine           aLinguisticFL;
    FixedText           aLinguModulesFT;
    SvxCheckListBox     aLinguModulesCLB;
    PushButton          aLinguModulesEditPB;
    FixedText           aLinguDicsFT;
    SvxCheckListBox     aLinguDicsCLB;
    PushButton          aLinguDicsNewPB;
    PushButton          aLinguDicsEditPB;
    PushButton          aLinguDicsDelPB;
    FixedText           aLinguOptionsFT;
    SvxCheckListBox     aLinguOptionsCLB;
    PushButton          aLinguOptionsEditPB;

    String              sCapitalWords;
    String              sWordsWithDigits;
    String              sCapitalization;
    String              sSpellSpecial;
    String              sAllLanguages;
    String              sSpellAuto;
    String              sHideMarkings;
    String              sOldGerman;
    String              sNumMinWordlen;
    String              sNumPreBreak;
    String              sNumPostBreak;
    String              sHyphAuto;
    String              sHyphSpecial;

    com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet >   xProp;

    com::sun::star::uno::Reference<
        com::sun::star::linguistic2::XDictionaryList >      xDicList;
    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::linguistic2::XDictionary > >    aDics;

    SvLBoxButtonData*   pCheckButtonData;

    SvxLinguData_Impl*  pLinguData;

    SVX_DLLPRIVATE SvxLinguTabPage( Window* pParent, const SfxItemSet& rCoreSet );
    SVX_DLLPRIVATE SvLBoxEntry* CreateEntry(String& rTxt, USHORT nCol);

    SVX_DLLPRIVATE void    AddDicBoxEntry( const com::sun::star::uno::Reference< com::sun::star::linguistic2::XDictionary > &rxDic, USHORT nIdx );
    SVX_DLLPRIVATE ULONG   GetDicUserData( const com::sun::star::uno::Reference< com::sun::star::linguistic2::XDictionary > &rxDic, USHORT nIdx );

#ifdef _SVX_OPTLINGU_CXX
    DECL_LINK( SelectHdl_Impl, SvxCheckListBox * );
    DECL_LINK( ClickHdl_Impl, PushButton * );
    DECL_LINK( BoxDoubleClickHdl_Impl, SvTreeListBox * );
    DECL_LINK( BoxCheckButtonHdl_Impl, SvTreeListBox * );
    DECL_LINK( PostDblClickHdl_Impl, SvTreeListBox * );

    SVX_DLLPRIVATE void                UpdateModulesBox_Impl();
    SVX_DLLPRIVATE void                UpdateDicBox_Impl();
#endif

public:
    virtual             ~SvxLinguTabPage();
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*  GetRanges();

    virtual sal_Bool    FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                HideGroups( sal_uInt16 nGrp );
};

#endif

