/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chinese_dictionarydialog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:56:15 $
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

#ifndef _TEXTCONVERSIONDLGS_CHINESE_DICTIONARYDLG_HXX
#define _TEXTCONVERSIONDLGS_CHINESE_DICTIONARYDLG_HXX

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
// header for class FixedLine
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for class RadioButton
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class Edit
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
// header for class ListBox
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
// header for class SvHeaderTabListBox
#ifndef _SVTABBX_HXX
#include <svtools/svtabbx.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTICOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XCONVERSIONDICTIONARY_HPP_
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#endif

#include <vector>

//.............................................................................
namespace textconversiondlgs
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

struct DictionaryEntry
{
    DictionaryEntry();
    DictionaryEntry( const rtl::OUString& rTerm, const rtl::OUString& rMapping
                    , sal_Int16 nConversionPropertyType //linguistic2::ConversionPropertyType
                    , sal_Bool bNewEntry = sal_False );

    virtual ~DictionaryEntry();

    bool operator==( const DictionaryEntry& rE ) const;

    rtl::OUString m_aTerm;
    rtl::OUString m_aMapping;
    sal_Int16     m_nConversionPropertyType; //linguistic2::ConversionPropertyType

    sal_Bool      m_bNewEntry;
};

class DictionaryList : public SvHeaderTabListBox
{
public:
    DictionaryList( Window* pParent, const ResId& );
    DictionaryList( Window* pParent );
    virtual ~DictionaryList();

    HeaderBar* createHeaderBar( const String& rColumn1, const String& rColumn2, const String& rColumn3
                  , long nWidth1, long nWidth2, long nWidth3 );

    void initDictionaryControl( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XConversionDictionary>& xDictionary
                            , ListBox* pPropertyTypeNameListBox );
    void activate( HeaderBar* pHeaderBar );
    void deleteAll();
    void refillFromDictionary( sal_Int32 nTextConversionOptions /*i18n::TextConversionOption*/ );
    void save();

    DictionaryEntry* getTermEntry( const rtl::OUString& rTerm ) const;
    bool hasTerm( const rtl::OUString& rTerm ) const;

    void addEntry( const rtl::OUString& rTerm, const rtl::OUString& rMapping
            , sal_Int16 nConversionPropertyType /*linguistic2::ConversionPropertyType*/, ULONG nPos = LIST_APPEND );
    ULONG deleteEntries( const rtl::OUString& rTerm ); //return lowest position of deleted entries or LIST_APPEND if no entry was deleted
    void deleteEntryOnPos( sal_Int32 nPos  );
    DictionaryEntry* getEntryOnPos( sal_Int32 nPos ) const;
    DictionaryEntry* getFirstSelectedEntry() const;

    void sortByColumn( USHORT nSortColumnIndex, bool bSortAtoZ );
    USHORT getSortColumn() const;

    virtual void Resize();

private:
    String getPropertyTypeName( sal_Int16 nConversionPropertyType /*linguistic2::ConversionPropertyType*/ ) const;
    String makeTabString( const DictionaryEntry& rEntry ) const;

    DECL_LINK( CompareHdl, SvSortData* );
    StringCompare ColumnCompare( SvLBoxEntry* pLeft, SvLBoxEntry* pRight );
    SvLBoxItem* getItemAtColumn( SvLBoxEntry* pEntry, USHORT nColumn ) const;

public:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XConversionDictionary>   m_xDictionary;

private:
    HeaderBar*  m_pHeaderBar;
    ListBox*    m_pPropertyTypeNameListBox;

    std::vector< DictionaryEntry* > m_aToBeDeleted;

    USHORT      m_nSortColumnIndex;
};

class ChineseDictionaryDialog : public ModalDialog
{
public:
    ChineseDictionaryDialog( Window* pParent );
    virtual ~ChineseDictionaryDialog();

    //this method should be called once before calling execute
    void setDirectionAndTextConversionOptions( bool bDirectionToSimplified, sal_Int32 nTextConversionOptions /*i18n::TextConversionOption*/ );

    virtual short   Execute();

private:
    DECL_LINK( DirectionHdl, void* );
    DECL_LINK( EditFieldsHdl, Control* );
    DECL_LINK( MappingSelectHdl, void* );
    DECL_LINK( AddHdl, void* );
    DECL_LINK( ModifyHdl, void* );
    DECL_LINK( DeleteHdl, void* );
    DECL_LINK( HeaderBarClick, void* );

    void updateAfterDirectionChange();
    void updateButtons();

    bool isEditFieldsHaveContent() const;
    bool isEditFieldsContentEqualsSelectedListContent() const;

    DictionaryList& getActiveDictionary();
    DictionaryList& getReverseDictionary();

    const DictionaryList& getActiveDictionary() const;
    const DictionaryList& getReverseDictionary() const;

private:
    sal_Int32   m_nTextConversionOptions; //i18n::TextConversionOption

    RadioButton m_aRB_To_Simplified;
    RadioButton m_aRB_To_Traditional;

    CheckBox    m_aCB_Reverse;

    FixedText   m_aFT_Term;
    Edit        m_aED_Term;

    FixedText   m_aFT_Mapping;
    Edit        m_aED_Mapping;

    FixedText   m_aFT_Property;
    ListBox     m_aLB_Property;

    HeaderBar*      m_pHeaderBar;
    DictionaryList  m_aCT_DictionaryToSimplified;
    DictionaryList  m_aCT_DictionaryToTraditional;

    PushButton  m_aPB_Add;
    PushButton  m_aPB_Modify;
    PushButton  m_aPB_Delete;

    FixedLine   m_aFL_Bottomline;

    OKButton        m_aBP_OK;
    CancelButton    m_aBP_Cancel;
    HelpButton      m_aBP_Help;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >              m_xContext;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiComponentFactory>         m_xFactory;
};

//.............................................................................
} //end namespace
//.............................................................................
#endif
