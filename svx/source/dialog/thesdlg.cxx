/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: thesdlg.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:38:36 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SVTOOLS_LNGMISC_HXX_
#include <svtools/lngmisc.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _COM_SUN_STAR_LINGUISTIC2_XTHESAURUS_HPP_
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XMEANING_HPP_
#include <com/sun/star/linguistic2/XMeaning.hpp>
#endif


#define _SVX_THESDLG_CXX

#include <svx/thesdlg.hxx>
#include "dlgutil.hxx"
#include <svx/dialmgr.hxx>
#include "svxerr.hxx"

#include <svx/dialogs.hrc>
#include "thesdlg.hrc"

#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _SVX_LANGBOX_HXX
#include <svx/langbox.hxx>
#endif


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

#undef S2U
#undef U2S
//#define S2U(s)                        StringToOUString(s, CHARSET_SYSTEM)
//#define U2S(s)                        OUStringToString(s, CHARSET_SYSTEM)


// GetReplaceEditString -------------------------------

static void GetReplaceEditString( String &rText )
{
    // The strings returned by the thesaurus saometimes have some
    // explanation text put in between '(' and ')' or a trailing '*'.
    // These parts should not be put in the ReplaceEdit Text that may get
    // inserted into the document. Thus we strip them from the text.

    xub_StrLen nPos = rText.Search( sal_Unicode('(') );
    while (STRING_NOTFOUND != nPos)
    {
        xub_StrLen nEnd = rText.Search( sal_Unicode(')'), nPos );
        if (STRING_NOTFOUND != nEnd)
            rText.Erase( nPos, nEnd-nPos+1 );
        else
            break;
        nPos = rText.Search( sal_Unicode('(') );
    }

    nPos = rText.Search( sal_Unicode('*') );
    if (STRING_NOTFOUND != nPos)
        rText.Erase( nPos );

    // remove any possible remaining ' ' that may confuse the thesaurus
    // when it gets called with the text
    rText.EraseLeadingAndTrailingChars( sal_Unicode(' ') );
}

// struct ThesDlg_Impl ---------------------------------------------------

struct ThesDlg_Impl
{
    Reference< XThesaurus > xThesaurus;
    OUString                aLookUpText;
    sal_Int16               nLookUpLanguage;

    ThesDlg_Impl( Reference< XThesaurus > & xThes );
    SfxErrorContext*    pErrContext;    // ErrorContext,
                                        // w"ahrend der Dialog oben ist
};

ThesDlg_Impl::ThesDlg_Impl(Reference< XThesaurus > & xThes) :
    xThesaurus  (xThes)
{
    pErrContext = NULL;
    nLookUpLanguage = LANGUAGE_NONE;
}


// class SvxThesaurusLanguageDlg_Impl ------------------------------------

class SvxThesaurusLanguageDlg_Impl : public ModalDialog
{
private:
    SvxLanguageBox  aLangLB;
    FixedLine       aLangFL;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;

    DECL_LINK( DoubleClickHdl_Impl, ListBox * );

public:
    SvxThesaurusLanguageDlg_Impl( Window* pParent );

    sal_uInt16      GetLanguage() const;
    void            SetLanguage( sal_uInt16 nLang );
};

// -----------------------------------------------------------------------


SvxThesaurusLanguageDlg_Impl::SvxThesaurusLanguageDlg_Impl( Window* pParent ) :

    ModalDialog( pParent, SVX_RES( RID_SVXDLG_THES_LANGUAGE ) ),

    aLangLB     ( this, SVX_RES( LB_THES_LANGUAGE ) ),
    aLangFL     ( this, SVX_RES( FL_THES_LANGUAGE ) ),
    aOKBtn      ( this, SVX_RES( BTN_LANG_OK ) ),
    aCancelBtn  ( this, SVX_RES( BTN_LANG_CANCEL ) ),
    aHelpBtn    ( this, SVX_RES( BTN_LANG_HELP ) )
{
    FreeResource();

    aLangLB.SetLanguageList( LANG_LIST_THES_USED, FALSE, FALSE );
    aLangLB.SetDoubleClickHdl(
        LINK( this, SvxThesaurusLanguageDlg_Impl, DoubleClickHdl_Impl ) );
}


// -----------------------------------------------------------------------

sal_uInt16 SvxThesaurusLanguageDlg_Impl::GetLanguage() const
{
    sal_uInt16 nLang = aLangLB.GetSelectLanguage();
    return nLang;
}

// -----------------------------------------------------------------------

void SvxThesaurusLanguageDlg_Impl::SetLanguage( sal_uInt16 nLang )
{
    aLangLB.SelectLanguage( nLang );
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxThesaurusLanguageDlg_Impl, DoubleClickHdl_Impl, ListBox *, EMPTYARG )
{
    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( SvxThesaurusLanguageDlg_Impl, DoubleClickHdl_Impl, ListBox *, EMPTYARG )

// class SvxThesaurusDialog ----------------------------------------------


// -----------------------------------------------------------------------


SvxThesaurusDialog::SvxThesaurusDialog( Window* pParent, Reference< XThesaurus >  xThes,
                                        const String &rWord, sal_Int16 nLanguage) :

    SvxStandardDialog( pParent, SVX_RES( RID_SVXDLG_THESAURUS ) ),

    aWordText   ( this, SVX_RES( FT_WORD ) ),
    aWordLB     ( this, SVX_RES( LB_WORD ) ),
    aReplaceText( this, SVX_RES( FT_REPL ) ),
    aReplaceEdit( this, SVX_RES( ED_REPL ) ),
    aMeanText   ( this, SVX_RES( FT_MEAN ) ),
    aMeanLB     ( this, SVX_RES( LB_MEAN ) ),
    aSynonymText( this, SVX_RES( FT_SYNON ) ),
    aSynonymLB  ( this, SVX_RES( LB_SYNON ) ),
    aVarFL      ( this, SVX_RES( FL_VAR ) ),
    aOkBtn      ( this, SVX_RES( BTN_THES_OK ) ),
    aCancelBtn  ( this, SVX_RES( BTN_THES_CANCEL ) ),
    aLookUpBtn  ( this, SVX_RES( BTN_LOOKUP ) ),
    aLangBtn    ( this, SVX_RES( BTN_LANGUAGE ) ),
    aHelpBtn    ( this, SVX_RES( BTN_THES_HELP ) ),
    aErrStr     (       SVX_RES( STR_ERR_WORDNOTFOUND ) )
{
    pImpl = new ThesDlg_Impl( xThes );
    pImpl->aLookUpText = ::rtl::OUString( rWord );
    pImpl->nLookUpLanguage = nLanguage;
    pImpl->pErrContext =
        new SfxErrorContext( ERRCTX_SVX_LINGU_THESAURUS, String(), this,
                             RID_SVXERRCTX, &DIALOG_MGR() );

    aLangBtn.SetClickHdl( LINK( this, SvxThesaurusDialog, LanguageHdl_Impl ) );
    aLookUpBtn.SetClickHdl( LINK( this, SvxThesaurusDialog, LookUpHdl_Impl ) );
    aMeanLB.SetSelectHdl( LINK( this, SvxThesaurusDialog, EntryHdl_Impl ) );
    aSynonymLB.SetSelectHdl( LINK( this, SvxThesaurusDialog, SynonymHdl_Impl ) );
    Link aLink = LINK( this, SvxThesaurusDialog, SelectHdl_Impl );
    aMeanLB.SetDoubleClickHdl( aLink );
    aSynonymLB.SetDoubleClickHdl( aLink );
    aWordLB.SetSelectHdl( aLink );

    FreeResource();

    OUString aTmp( rWord );
    linguistic::RemoveHyphens( aTmp );
    linguistic::ReplaceControlChars( aTmp );
    aReplaceEdit.SetText( aTmp );
    aWordLB.InsertEntry( aTmp );
    aWordLB.SelectEntry( aTmp );

    Init_Impl( nLanguage );

    // disable controls if service is missing
    if (!pImpl->xThesaurus.is())
        Enable( sal_False );
}


// -----------------------------------------------------------------------

SvxThesaurusDialog::~SvxThesaurusDialog()
{
    delete pImpl->pErrContext;
    delete pImpl;
}

// -----------------------------------------------------------------------

uno::Sequence< Reference< XMeaning > > SAL_CALL
    SvxThesaurusDialog::queryMeanings_Impl(
            OUString& rTerm,
            const Locale& rLocale,
            const beans::PropertyValues& rProperties )
        throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    uno::Sequence< Reference< XMeaning > > aMeanings(
            pImpl->xThesaurus->queryMeanings( rTerm, rLocale, rProperties ) );

    // text with '.' at the end?
    if (0 == aMeanings.getLength() && rTerm.getLength() &&
        rTerm.getStr()[ rTerm.getLength() - 1 ] == '.')
    {
        // try again without trailing '.' chars. It may be a word at the
        // end of a sentence and not an abbreviation...
        String aTxt( rTerm );
        aTxt.EraseTrailingChars( '.' );
        aMeanings = pImpl->xThesaurus->queryMeanings( aTxt, rLocale, rProperties );
        if (aMeanings.getLength())
        {
            rTerm = aTxt;
        }
    }

    return aMeanings;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxThesaurusDialog::GetLanguage() const
{
    return pImpl->nLookUpLanguage;
}

// -----------------------------------------------------------------------

void SvxThesaurusDialog::UpdateMeaningBox_Impl( uno::Sequence< Reference< XMeaning >  > *pMeaningSeq )
{
    // create temporary meaning list if not supplied from somewhere else
    sal_Bool bTmpSeq = sal_False;
    if (!pMeaningSeq  &&  pImpl->xThesaurus.is())
    {
        bTmpSeq = sal_True;
        lang::Locale aLocale( SvxCreateLocale( pImpl->nLookUpLanguage ) );
        uno::Sequence< Reference< XMeaning > > aTmpMean = queryMeanings_Impl(
                pImpl->aLookUpText, aLocale, Sequence< PropertyValue >() );

        pMeaningSeq = new Sequence< Reference< XMeaning >  > ( aTmpMean );

        // set new replace edit text if a different look up text was used
        // see: queryMeanings_Impl
        aReplaceEdit.SetText( pImpl->aLookUpText );
    }

    sal_Int32 nMeaningCount = pMeaningSeq ? pMeaningSeq->getLength() : 0;
    const Reference< XMeaning >  *pMeaning = pMeaningSeq ? pMeaningSeq->getConstArray() : NULL;
    aMeanLB.Clear();
    for ( sal_Int32 i = 0;  i < nMeaningCount;  ++i )
        aMeanLB.InsertEntry( pMeaning[i]->getMeaning() );

    // remove temporary meaning list
    if (bTmpSeq)
        delete pMeaningSeq;

    if (aMeanLB.GetEntryCount() > 0)
        aMeanLB.SelectEntryPos(0);
    UpdateSynonymBox_Impl();
}


// -----------------------------------------------------------------------
void SvxThesaurusDialog::UpdateSynonymBox_Impl()
{

    aSynonymLB.Clear();

    sal_uInt16 nPos = aMeanLB.GetSelectEntryPos();  // active meaning pos
    if (nPos != LISTBOX_ENTRY_NOTFOUND  &&  pImpl->xThesaurus.is())
    {
        // get Reference< XMeaning >  for selected meaning
        lang::Locale aLocale( SvxCreateLocale( pImpl->nLookUpLanguage ) );
        Reference< XMeaning >  xMeaning = queryMeanings_Impl(
                    pImpl->aLookUpText, aLocale, Sequence< PropertyValue >() )
                 .getConstArray()[ nPos ];

        uno::Sequence< OUString >   aSynonyms;
        if (xMeaning.is())
            aSynonyms = xMeaning->querySynonyms();

        sal_Int32 nSynonymCount = aSynonyms.getLength();
        const OUString *pSynonym = aSynonyms.getConstArray();
        for ( sal_Int32 i=0;  i < nSynonymCount;  ++i )
            aSynonymLB.InsertEntry( pSynonym[i] );
    }

}

// -----------------------------------------------------------------------

void SvxThesaurusDialog::Apply()
{
}

// -----------------------------------------------------------------------
void SvxThesaurusDialog::Init_Impl(sal_Int16 nLanguage)
{
    // Sprache anpassen
    String aStr( GetText() );
    aStr.Erase( aStr.Search( sal_Unicode( '(' ) ) - 1 );
    aStr.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( " (" ) ) );
    //aStr += GetLanguageString( (LanguageType)pImpl->pThesaurus->GetLanguage() );
    aStr += GetLanguageString( (LanguageType) nLanguage );
    aStr.Append( sal_Unicode( ')' ) );
    SetText( aStr );    // Text der Window-Leiste setzen (Wort + Sprache)

    // adapt meanings according to (new) language
    UpdateMeaningBox_Impl();
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxThesaurusDialog, LookUpHdl_Impl, Button *, pBtn )
{

    EnterWait();

    String aText( aReplaceEdit.GetText() );

    OUString aOldLookUpText = pImpl->aLookUpText;
    pImpl->aLookUpText = ::rtl::OUString( aText );

    uno::Sequence< Reference< XMeaning >  > aMeanings;
    if (pImpl->xThesaurus.is())
        aMeanings = queryMeanings_Impl(
                            pImpl->aLookUpText,
                            SvxCreateLocale( pImpl->nLookUpLanguage ),
                            Sequence< PropertyValue >() );

    LeaveWait();
    if ( aMeanings.getLength() == 0 )
    {
        if( pBtn == &aCancelBtn ) // called via double click
        {
            pImpl->aLookUpText = aOldLookUpText;
        }
        else
        {
            UpdateMeaningBox_Impl( &aMeanings );
            if( pBtn == &aLookUpBtn )
                InfoBox( this, aErrStr ).Execute();
        }
        return 0;
    }

    UpdateMeaningBox_Impl( &aMeanings );

    if ( aWordLB.GetEntryPos( aText ) == LISTBOX_ENTRY_NOTFOUND )
        aWordLB.InsertEntry( aText );

    aWordLB.SelectEntry( aText );
    aMeanLB.SelectEntryPos( 0 );

    String aStr( aMeanLB.GetSelectEntry() );
    GetReplaceEditString( aStr );
    aReplaceEdit.SetText( aStr );
    aSynonymLB.SetNoSelection();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxThesaurusDialog, LanguageHdl_Impl, Button *, EMPTYARG )
{
    if (!pImpl->xThesaurus.is())
        return 0;

    SvxThesaurusLanguageDlg_Impl aDlg( this );
    sal_uInt16 nLang = pImpl->nLookUpLanguage;
    aDlg.SetLanguage( nLang );

    if ( aDlg.Execute() == RET_OK )
    {
        nLang = aDlg.GetLanguage();
        if (pImpl->xThesaurus->hasLocale( SvxCreateLocale( nLang ) ))
            pImpl->nLookUpLanguage = nLang;
        UpdateMeaningBox_Impl();
        Init_Impl( nLang );
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxThesaurusDialog, SynonymHdl_Impl, ListBox *, EMPTYARG )
{
    if ( aSynonymLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        String aStr( aSynonymLB.GetSelectEntry() );
        GetReplaceEditString( aStr );
        aReplaceEdit.SetText( aStr );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxThesaurusDialog, SelectHdl_Impl, ListBox *, pBox )
{
    String aStr( pBox->GetSelectEntry() );
    GetReplaceEditString( aStr );
    aReplaceEdit.SetText( aStr );

    //! 'aCancelBtn' is used to indicate that the handler is called as result
    //! of a double click action.
    LookUpHdl_Impl( &aCancelBtn /* ??? &aLookUpBtn */ );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxThesaurusDialog, EntryHdl_Impl, ListBox *, EMPTYARG )
{

    UpdateSynonymBox_Impl();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxThesaurusDialog, SpellErrorHdl_Impl, void *, pError )
{
    // Der "ubergebene Pointer pError ist die falsche Sprachen-Nummer
    LanguageType eLang = (LanguageType)(sal_uIntPtr)pError;
    String aErr( ::GetLanguageString( eLang ) );
    // Fehlermeldung ausgeben
    ErrorHandler::HandleError(
        *new StringErrorInfo( ERRCODE_SVX_LINGU_LANGUAGENOTEXISTS, aErr ) );
    return 0;
}


