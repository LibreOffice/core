/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textconv.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-08 09:13:33 $
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
#ifndef _TEXTCONV_HXX
#define _TEXTCONV_HXX

#include <splwrap.hxx>
#include <svxacorr.hxx>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef SVX_HANGUL_HANJA_CONVERSION_HXX
#include "hangulhanja.hxx"
#endif

class EditView;
class ImpEditEngine;
class ContentNode;

class TextConvWrapper : public svx::HangulHanjaConversion
{
    rtl::OUString   aConvText;      // convertible text part found last time
    LanguageType    nConvTextLang;  // language of aConvText
    USHORT          nLastPos;       // starting position of the last found text portion (word)
    USHORT          nUnitOffset;    // offset of current unit in the current text portion (word)

    ESelection      aConvSel;       // selection to be converted if
                                    // 'HasRange' is true, other conversion
                                    // starts from the cursor position

    EditView *      pEditView;
    Window *        pWin;

    sal_Bool        bStartChk;
    sal_Bool        bStartDone;
    sal_Bool        bEndDone;
    sal_Bool        bAllowChange;   // storage for _bAllowImplicitChangesForNotConvertibleText
                                    // paramters value of function GetNextPortion.
                                    // used to transport the value to where it is needed.


    // from SvxSpellWrapper copied and modified
    sal_Bool    ConvNext_impl();        // former SpellNext
    sal_Bool    FindConvText_impl();    // former FindSpellError
    sal_Bool    ConvMore_impl();        // former SpellMore

    // from EditSpellWrapper copied and modified
    void        ConvStart_impl( SvxSpellArea eSpell );   // former SpellStart
    void        ConvEnd_impl();                          // former SpellEnd
    sal_Bool    ConvContinue_impl();                     // former SpellContinue

    void        SelectNewUnit_impl( const sal_Int32 nUnitStart,
                                    const sal_Int32 nUnitEnd );

    void        ChangeText( const String &rNewText,
                            const ::rtl::OUString& rOrigText,
                            const ::com::sun::star::uno::Sequence< sal_Int32 > *pOffsets,
                            ESelection *pESelection );
    void        ChangeText_impl( const String &rNewText, sal_Bool bKeepAttributes );

    // Forbidden and not implemented.
    TextConvWrapper (const TextConvWrapper &);
    TextConvWrapper & operator= (const TextConvWrapper &);

protected:
    virtual void    GetNextPortion( ::rtl::OUString& /* [out] */ rNextPortion,
                        LanguageType& /* [out] */ rLangOfPortion,
                        sal_Bool /* [in] */ _bAllowImplicitChangesForNotConvertibleText );
    virtual void    HandleNewUnit( const sal_Int32 nUnitStart,
                                   const sal_Int32 nUnitEnd );
    virtual void    ReplaceUnit(
                        const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd,
                        const ::rtl::OUString& rOrigText,
                        const ::rtl::OUString& rReplaceWith,
                        const ::com::sun::star::uno::Sequence< sal_Int32 > &rOffsets,
                        ReplacementAction eAction,
                        LanguageType *pNewUnitLanguage );

    virtual sal_Bool    HasRubySupport() const;

    void SetLanguageAndFont( const ESelection &rESel,
                            LanguageType nLang, USHORT nLangWhichId,
                            const Font *pFont,  USHORT nFontWhichId );


public:
    TextConvWrapper( Window* pWindow,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMSF,
            const ::com::sun::star::lang::Locale& rSourceLocale,
            const ::com::sun::star::lang::Locale& rTargetLocale,
            const Font* pTargetFont,
            INT32 nOptions,
            sal_Bool bIsInteractive,
            BOOL bIsStart, EditView* pView );

    virtual ~TextConvWrapper();

    void    Convert();
};

#endif

