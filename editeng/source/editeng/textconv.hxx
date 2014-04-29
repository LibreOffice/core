/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _TEXTCONV_HXX
#define _TEXTCONV_HXX

#include <editeng/splwrap.hxx>
#include <editeng/svxacorr.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <editeng/hangulhanja.hxx>

class EditView;
class ImpEditEngine;
class ContentNode;

class TextConvWrapper : public editeng::HangulHanjaConversion
{
    rtl::OUString   aConvText;      // convertible text part found last time
    LanguageType    nConvTextLang;  // language of aConvText
    sal_uInt16          nLastPos;       // starting position of the last found text portion (word)
    sal_uInt16          nUnitOffset;    // offset of current unit in the current text portion (word)

    ESelection      aConvSel;       // selection to be converted if
                                    // 'HasRange' is true, other conversion
                                    // starts from the cursor position

    EditView *      pEditView;
    Window *        pWin;

    sal_Bool        bStartChk;
    sal_Bool        bStartDone;
    sal_Bool        bEndDone;
    sal_Bool        bAllowChange;   // storage for _bAllowImplicitChangesForNotConvertibleText
                                    // parameters value of function GetNextPortion.
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
                            LanguageType nLang, sal_uInt16 nLangWhichId,
                            const Font *pFont,  sal_uInt16 nFontWhichId );


public:
    TextConvWrapper( Window* pWindow,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMSF,
            const ::com::sun::star::lang::Locale& rSourceLocale,
            const ::com::sun::star::lang::Locale& rTargetLocale,
            const Font* pTargetFont,
            sal_Int32 nOptions,
            sal_Bool bIsInteractive,
            sal_Bool bIsStart, EditView* pView );

    virtual ~TextConvWrapper();

    void    Convert();
};

#endif

