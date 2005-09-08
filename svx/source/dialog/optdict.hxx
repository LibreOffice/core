/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optdict.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:41:10 $
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
#ifndef _SVX_OPTDICT_HXX
#define _SVX_OPTDICT_HXX

// include ---------------------------------------------------------------

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _SV_DECOVIEW_HXX //autogen
#include <vcl/decoview.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_LANGUAGE_HPP_
#include <com/sun/star/util/Language.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif


#ifndef _SVX_SIMPTABL_HXX
#include "simptabl.hxx"
#endif
#ifndef _SVX_LANGBOX_HXX
#include <langbox.hxx>
#endif

namespace com{namespace sun{namespace star{
namespace linguistic2{
    class XDictionary;
    class XDictionary1;
    class XSpellChecker1;
    class XSpellChecker;
}}}}

// forward ---------------------------------------------------------------


// class SvxNewDictionaryDialog ------------------------------------------

class SvxNewDictionaryDialog : public ModalDialog
{
private:
    FixedText           aNameText;
    Edit                aNameEdit;
    FixedText           aLanguageText;
    SvxLanguageBox      aLanguageLB;
    CheckBox            aExceptBtn;
    FixedLine            aNewDictBox;
    OKButton            aOKBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >     xSpell;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary1 >   xNewDic;

#ifdef _SVX_OPTDICT_CXX
    DECL_LINK( OKHdl_Impl, Button * );
    DECL_LINK( ModifyHdl_Impl, Edit * );
#endif

public:
    SvxNewDictionaryDialog( Window* pParent,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XSpellChecker1 >  &xSpl );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary1 >
                GetNewDictionary() { return xNewDic; }
};

// class SvxDictEdit ----------------------------------------------------

class SvxDictEdit : public Edit
{
    Link    aActionLink;
    sal_Bool    bSpaces;

    public:
                    SvxDictEdit(Window* pParent, const ResId& rResId) :
                        Edit(pParent, rResId), bSpaces(sal_False){}

    void            SetActionHdl( const Link& rLink )
                                { aActionLink = rLink;}

    void            SetSpaces(sal_Bool bSet)
                                {bSpaces = bSet;}

    virtual void    KeyInput( const KeyEvent& rKEvent );
};

// class SvxEditDictionaryDialog -----------------------------------------

class SvxEditDictionaryDialog : public ModalDialog
{
private:

    FixedText               aBookFT;
    ListBox                 aAllDictsLB;
    FixedText               aLangFT;
    SvxLanguageBox          aLangLB;

    FixedText               aWordFT;
    SvxDictEdit             aWordED;
    FixedText               aReplaceFT;
    SvxDictEdit             aReplaceED;
    SvTabListBox            aWordsLB;
    PushButton              aNewReplacePB;
    PushButton              aDeletePB;
    FixedLine                aEditDictsBox;

    CancelButton            aCloseBtn;
    HelpButton              aHelpBtn;
    String                  sModify;
    String                  sNew;
    DecorationView          aDecoView;

    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XDictionary >  > aDics;  //! snapshot copy to work on
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >     xSpell;

    short               nOld;
    long                nWidth;
    sal_Bool            bFirstSelect;
    sal_Bool            bDoNothing;
    BOOL                bDicIsReadonly;

#ifdef _SVX_OPTDICT_CXX
    DECL_LINK( SelectBookHdl_Impl, ListBox * );
    DECL_LINK( SelectLangHdl_Impl, ListBox * );
    DECL_LINK(SelectHdl, SvTabListBox*);
    DECL_LINK(NewDelHdl, PushButton*);
    DECL_LINK(ModifyHdl, Edit*);


    void            ShowWords_Impl( sal_uInt16 nId );
    void            SetLanguage_Impl( ::com::sun::star::util::Language nLanguage );
    sal_Bool            IsDicReadonly_Impl() const { return bDicIsReadonly; }
    void            SetDicReadonly_Impl( ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XDictionary1 >  &xDic );

    void            RemoveDictEntry(SvLBoxEntry* pEntry);
    USHORT          GetLBInsertPos(const String &rDicWord);

#endif

protected:

    virtual void    Paint( const Rectangle& rRect );

public:
    SvxEditDictionaryDialog( Window* pParent,
            const String& rName,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XSpellChecker1> &xSpl );
    ~SvxEditDictionaryDialog();

    sal_uInt16 GetSelectedDict() {return aAllDictsLB.GetSelectEntryPos();}
};


#endif

