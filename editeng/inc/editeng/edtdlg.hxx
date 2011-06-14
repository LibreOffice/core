/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _EDT_ABSTDLG_HXX
#define _EDT_ABSTDLG_HXX

// include ---------------------------------------------------------------

#include <tools/solar.h>
#include <tools/string.hxx>
#include <editeng/editengdllapi.h>
#include <i18npool/lang.h>
#include <com/sun/star/uno/Reference.hxx>
#include <vcl/abstdlg.hxx>
#include <editeng/hangulhanja.hxx>

namespace com { namespace sun { namespace star { namespace linguistic2
{
    class XThesaurus;
    class XHyphenator;
} } } }

class Window;
class ResId;
class Link;
class SvxSpellWrapper;

class AbstractThesaurusDialog : public VclAbstractDialog
{
public:
    virtual String      GetWord() = 0;
    virtual sal_uInt16  GetLanguage() const = 0;
    virtual Window*     GetWindow() = 0;
};

class AbstractHyphenWordDialog : public VclAbstractDialog
{
public:
    virtual void    SelLeft() = 0;
    virtual void    SelRight() = 0;
    virtual Window* GetWindow() = 0;
};

class AbstractHangulHanjaConversionDialog : public VclAbstractTerminatedDialog
{
 public:
    virtual void EnableRubySupport( sal_Bool _bVal ) = 0;
     virtual void SetByCharacter( sal_Bool _bByCharacter ) = 0;
    virtual void SetConversionDirectionState( sal_Bool _bTryBothDirections, editeng::HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection ) = 0;
     virtual void SetConversionFormat( editeng::HangulHanjaConversion::ConversionFormat _eType ) = 0;
    virtual void    SetOptionsChangedHdl( const Link& _rHdl ) = 0;
     virtual void   SetIgnoreHdl( const Link& _rHdl ) = 0;
     virtual void   SetIgnoreAllHdl( const Link& _rHdl ) = 0;
     virtual void   SetChangeHdl( const Link& _rHdl ) = 0;
     virtual void   SetChangeAllHdl( const Link& _rHdl ) = 0;
    virtual void    SetClickByCharacterHdl( const Link& _rHdl ) = 0;
     virtual void   SetConversionFormatChangedHdl( const Link& _rHdl ) = 0;
     virtual void   SetFindHdl( const Link& _rHdl ) = 0;
    virtual sal_Bool        GetUseBothDirections( ) const= 0;
    virtual editeng::HangulHanjaConversion::ConversionDirection    GetDirection( editeng::HangulHanjaConversion::ConversionDirection _eDefaultDirection ) const = 0;
    virtual void    SetCurrentString(
                    const String& _rNewString,
                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rSuggestions,
                    bool _bOriginatesFromDocument = true )=0;
    virtual String  GetCurrentString( ) const =0;
    virtual editeng::HangulHanjaConversion::ConversionFormat    GetConversionFormat( ) const =0;
    virtual void    FocusSuggestion( )= 0;
    virtual String  GetCurrentSuggestion( ) const =0;
};

class EDITENG_DLLPUBLIC EditAbstractDialogFactory : virtual public VclAbstractDialogFactory
{
public:
                                        virtual ~EditAbstractDialogFactory();   // needed for export of vtable
    static EditAbstractDialogFactory*   Create();
    virtual AbstractThesaurusDialog*        CreateThesaurusDialog( Window*, ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XThesaurus >  xThesaurus,
                                                const String &rWord, sal_Int16 nLanguage ) = 0;

    virtual AbstractHyphenWordDialog*       CreateHyphenWordDialog( Window*,
                                                const String &rWord, LanguageType nLang,
                                                ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenator >  &xHyphen,
                                                SvxSpellWrapper* pWrapper ) = 0;
    virtual AbstractHangulHanjaConversionDialog * CreateHangulHanjaConversionDialog( Window* _pParent,
                                            editeng::HangulHanjaConversion::ConversionDirection _ePrimaryDirection ) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
