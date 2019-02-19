/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_EDITENG_EDTDLG_HXX
#define INCLUDED_EDITENG_EDTDLG_HXX

#include <rtl/ustring.hxx>
#include <tools/link.hxx>
#include <editeng/editengdllapi.h>
#include <i18nlangtag/lang.h>
#include <com/sun/star/uno/Reference.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/weld.hxx>
#include <editeng/hangulhanja.hxx>

namespace com { namespace sun { namespace star { namespace linguistic2
{
    class XThesaurus;
    class XHyphenator;
} } } }

namespace vcl { class Window; }
class SvxSpellWrapper;
class Button;
class CheckBox;

class AbstractThesaurusDialog : public VclAbstractDialog
{
protected:
    virtual ~AbstractThesaurusDialog() override = default;
public:
    virtual OUString    GetWord() = 0;
};

class AbstractHyphenWordDialog : public VclAbstractDialog
{
protected:
    virtual ~AbstractHyphenWordDialog() override = default;
};

class AbstractHangulHanjaConversionDialog : public VclAbstractTerminatedDialog
{
protected:
    virtual ~AbstractHangulHanjaConversionDialog() override = default;
public:
    virtual void     EnableRubySupport( bool _bVal ) = 0;
    virtual void     SetByCharacter( bool _bByCharacter ) = 0;
    virtual void     SetConversionDirectionState( bool _bTryBothDirections, editeng::HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection ) = 0;
    virtual void     SetConversionFormat( editeng::HangulHanjaConversion::ConversionFormat _eType ) = 0;
    virtual void     SetOptionsChangedHdl( const Link<LinkParamNone*,void>& _rHdl ) = 0;
    virtual void     SetIgnoreHdl( const Link<weld::Button&,void>& _rHdl ) = 0;
    virtual void     SetIgnoreAllHdl( const Link<weld::Button&,void>& _rHdl ) = 0;
    virtual void     SetChangeHdl( const Link<weld::Button&,void>& _rHdl ) = 0;
    virtual void     SetChangeAllHdl( const Link<weld::Button&,void>& _rHdl ) = 0;
    virtual void     SetClickByCharacterHdl( const Link<weld::ToggleButton&,void>& _rHdl ) = 0;
    virtual void     SetConversionFormatChangedHdl( const Link<weld::Button&,void>& _rHdl ) = 0;
    virtual void     SetFindHdl( const Link<weld::Button&,void>& _rHdl ) = 0;
    virtual bool     GetUseBothDirections() const= 0;
    virtual editeng::HangulHanjaConversion::ConversionDirection
                     GetDirection( editeng::HangulHanjaConversion::ConversionDirection _eDefaultDirection ) const = 0;
    virtual void     SetCurrentString(
                        const OUString& _rNewString,
                        const css::uno::Sequence< OUString >& _rSuggestions,
                        bool _bOriginatesFromDocument = true )=0;
    virtual OUString GetCurrentString( ) const =0;
    virtual editeng::HangulHanjaConversion::ConversionFormat
                     GetConversionFormat( ) const =0;
    virtual void     FocusSuggestion( )= 0;
    virtual OUString GetCurrentSuggestion( ) const =0;
};

class EDITENG_DLLPUBLIC EditAbstractDialogFactory : virtual public VclAbstractDialogFactory
{
public:
                                        virtual ~EditAbstractDialogFactory() override;   // needed for export of vtable
    static EditAbstractDialogFactory*   Create();
    virtual VclPtr<AbstractThesaurusDialog>  CreateThesaurusDialog( vcl::Window*, css::uno::Reference< css::linguistic2::XThesaurus >  xThesaurus,
                                                const OUString &rWord, LanguageType nLanguage ) = 0;

    virtual VclPtr<AbstractHyphenWordDialog> CreateHyphenWordDialog(weld::Window*,
                                                const OUString &rWord, LanguageType nLang,
                                                css::uno::Reference< css::linguistic2::XHyphenator >  &xHyphen,
                                                SvxSpellWrapper* pWrapper) = 0;
    virtual VclPtr<AbstractHangulHanjaConversionDialog> CreateHangulHanjaConversionDialog(weld::Window* pParent) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
