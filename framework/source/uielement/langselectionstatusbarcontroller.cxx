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

#include <classes/fwkresid.hxx>
#include <services.h>
#include <classes/resource.hrc>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/status.hxx>
#include <toolkit/helper/convert.hxx>

#include <boost/noncopyable.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/awt/PopupMenu.hpp>
#include <com/sun/star/awt/PopupMenuDirection.hpp>
#include <svtools/langtab.hxx>
#include <svtools/statusbarcontroller.hxx>
#include <sal/types.h>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/document/XDocumentLanguages.hpp>
#include <i18nlangtag/mslangid.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <comphelper/processfactory.hxx>

#include <tools/gen.hxx>
#include <com/sun/star/awt/Command.hpp>
#include <svl/languageoptions.hxx>

#include "helper/mischelper.hxx"

#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>

#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

#include <map>
#include <set>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace css::uno;
using namespace css::lang;
using namespace css::frame;
using namespace css::i18n;
using namespace css::document;
using namespace framework;

namespace {

class LangSelectionStatusbarController:
    public svt::StatusbarController, private boost::noncopyable
{
public:
    explicit LangSelectionStatusbarController( const css::uno::Reference< css::uno::XComponentContext >& xContext );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XStatusbarController
    virtual void SAL_CALL command( const css::awt::Point& aPos,
                                   ::sal_Int32 nCommand,
                                   sal_Bool bMouseEvent,
                                   const css::uno::Any& aData ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL click( const css::awt::Point& aPos ) throw (css::uno::RuntimeException, std::exception) override;

private:
    virtual ~LangSelectionStatusbarController() {}

    bool            m_bShowMenu;        // if the menu is to be displayed or not (depending on the selected object/text)
    SvtScriptType   m_nScriptType;      // the flags for the different script types available in the selection, LATIN = 0x0001, ASIAN = 0x0002, COMPLEX = 0x0004
    OUString        m_aCurLang;         // the language of the current selection, "*" if there are more than one languages
    OUString        m_aKeyboardLang;    // the keyboard language
    OUString        m_aGuessedTextLang;     // the 'guessed' language for the selection, "" if none could be guessed
    LanguageGuessingHelper      m_aLangGuessHelper;

    void LangMenu( const css::awt::Point& aPos ) throw (css::uno::RuntimeException, std::exception);
};

LangSelectionStatusbarController::LangSelectionStatusbarController( const uno::Reference< uno::XComponentContext >& xContext ) :
    svt::StatusbarController( xContext, uno::Reference< frame::XFrame >(), OUString(), 0 ),
    m_bShowMenu( true ),
    m_nScriptType( SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX ),
    m_aLangGuessHelper( xContext )
{
}

void SAL_CALL LangSelectionStatusbarController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
throw (css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarMutexGuard;

    svt::StatusbarController::initialize( aArguments );

    if ( m_xStatusbarItem.is() )
    {
        m_xStatusbarItem->setText( FWK_RESSTR(STR_LANGSTATUS_MULTIPLE_LANGUAGES) );
        m_xStatusbarItem->setQuickHelpText(FWK_RESSTR(STR_LANGSTATUS_HINT));
    }
}

void LangSelectionStatusbarController::LangMenu(
    const css::awt::Point& aPos )
throw (css::uno::RuntimeException, std::exception)
{
    if (!m_bShowMenu)
        return;

    //add context menu
    Reference< awt::XPopupMenu > xPopupMenu( awt::PopupMenu::create( m_xContext ) );
    //sub menu that contains all items except the last two items: Separator + Set Language for Paragraph
    Reference< awt::XPopupMenu > subPopupMenu( awt::PopupMenu::create( m_xContext ) );

    // get languages to be displayed in the menu
    std::set< OUString > aLangItems;
    FillLangItems( aLangItems, m_xFrame, m_aLangGuessHelper,
            m_nScriptType, m_aCurLang, m_aKeyboardLang, m_aGuessedTextLang );

    // add first few entries to main menu
    sal_Int16 nItemId = static_cast< sal_Int16 >(MID_LANG_SEL_1);
    const OUString sAsterisk("*");  // multiple languages in current selection
    const OUString sNone( SvtLanguageTable::GetLanguageString( LANGUAGE_NONE ));
    std::map< sal_Int16, OUString > aLangMap;
    std::set< OUString >::const_iterator it;
    for (it = aLangItems.begin(); it != aLangItems.end(); ++it)
    {
        const OUString & rStr( *it );
        if ( rStr != sNone &&
             rStr != sAsterisk &&
             !rStr.isEmpty()) // 'no language found' from language guessing
        {
            DBG_ASSERT( MID_LANG_SEL_1 <= nItemId && nItemId <= MID_LANG_SEL_9,
                    "nItemId outside of expected range!" );
            xPopupMenu->insertItem( nItemId, rStr, 0, nItemId );
            if ( rStr == m_aCurLang )
            {
                //make a sign for the current language
                xPopupMenu->checkItem( nItemId, sal_True );
            }
            aLangMap[ nItemId ] = rStr;
            ++nItemId;
        }
    }

    xPopupMenu->insertItem( MID_LANG_SEL_NONE,  FWK_RESSTR(STR_LANGSTATUS_NONE), 0, MID_LANG_SEL_NONE );
    if ( sNone == m_aCurLang )
        xPopupMenu->checkItem( MID_LANG_SEL_NONE, sal_True );
    xPopupMenu->insertItem( MID_LANG_SEL_RESET, FWK_RESSTR(STR_RESET_TO_DEFAULT_LANGUAGE), 0, MID_LANG_SEL_RESET );
    xPopupMenu->insertItem( MID_LANG_SEL_MORE,  FWK_RESSTR(STR_LANGSTATUS_MORE), 0, MID_LANG_SEL_MORE );

    // add entries to submenu ('set language for paragraph')
    nItemId = static_cast< sal_Int16 >(MID_LANG_PARA_1);
    for (it = aLangItems.begin(); it != aLangItems.end(); ++it)
    {
        const OUString & rStr( *it );
        if( rStr != sNone &&
            rStr != sAsterisk &&
            !rStr.isEmpty()) // 'no language found' from language guessing
        {
            DBG_ASSERT( MID_LANG_PARA_1 <= nItemId && nItemId <= MID_LANG_PARA_9,
                    "nItemId outside of expected range!" );
            subPopupMenu->insertItem( nItemId, rStr, 0, nItemId );
            aLangMap[nItemId] = rStr;
            ++nItemId;
        }
    }
    subPopupMenu->insertItem( MID_LANG_PARA_NONE,  FWK_RESSTR(STR_LANGSTATUS_NONE), 0, MID_LANG_PARA_NONE );
    subPopupMenu->insertItem( MID_LANG_PARA_RESET, FWK_RESSTR(STR_RESET_TO_DEFAULT_LANGUAGE), 0, MID_LANG_PARA_RESET );
    subPopupMenu->insertItem( MID_LANG_PARA_MORE,  FWK_RESSTR(STR_LANGSTATUS_MORE), 0, MID_LANG_PARA_MORE );

    // add last two entries to main menu
    xPopupMenu->insertSeparator( MID_LANG_PARA_SEPARATOR );
    xPopupMenu->insertItem( MID_LANG_PARA_STRING, FWK_RESSTR(STR_SET_LANGUAGE_FOR_PARAGRAPH), 0, MID_LANG_PARA_STRING );
    xPopupMenu->setPopupMenu( MID_LANG_PARA_STRING, subPopupMenu );

    // now display the popup menu and execute every command ...

    Reference< awt::XWindowPeer > xParent( m_xParentWindow, UNO_QUERY );
    css::awt::Rectangle aRect( aPos.X, aPos.Y, 0, 0 );
    sal_Int16 nId = xPopupMenu->execute( xParent, aRect, css::awt::PopupMenuDirection::EXECUTE_UP+16 );
    //click "More..."
    if ( nId && m_xFrame.is() )
    {
        OUStringBuffer aBuff;
        //set selected language as current language for selection
        const OUString aSelectedLang = aLangMap[nId];

        if (MID_LANG_SEL_1 <= nId && nId <= MID_LANG_SEL_9)
        {
            aBuff.append( ".uno:LanguageStatus?Language:string=Current_" );
            aBuff.append( aSelectedLang );
        }
        else if (nId == MID_LANG_SEL_NONE)
        {
            //set None as current language for selection
            aBuff.append( ".uno:LanguageStatus?Language:string=Current_LANGUAGE_NONE" );
        }
        else if (nId == MID_LANG_SEL_RESET)
        {
            // reset language attributes for selection
            aBuff.append( ".uno:LanguageStatus?Language:string=Current_RESET_LANGUAGES" );
        }
        else if (nId == MID_LANG_SEL_MORE)
        {
            //open the dialog "format/character" for current selection
            aBuff.append( ".uno:FontDialog?Page:string=font" );
        }
        else if (MID_LANG_PARA_1 <= nId && nId <= MID_LANG_PARA_9)
        {
            aBuff.append( ".uno:LanguageStatus?Language:string=Paragraph_" );
            aBuff.append( aSelectedLang );
        }
        else if (nId == MID_LANG_PARA_NONE)
        {
            //set None as language for current paragraph
            aBuff.append( ".uno:LanguageStatus?Language:string=Paragraph_LANGUAGE_NONE" );
        }
        else if (nId == MID_LANG_PARA_RESET)
        {
            // reset language attributes for paragraph
            aBuff.append( ".uno:LanguageStatus?Language:string=Paragraph_RESET_LANGUAGES" );
        }
        else if (nId == MID_LANG_PARA_MORE)
        {
            //open the dialog "format/character" for current paragraph
            aBuff.append( ".uno:FontDialogForParagraph" );
        }

        const Sequence< beans::PropertyValue > aDummyArgs;
        execute( aBuff.makeStringAndClear(), aDummyArgs );
    }
}

void SAL_CALL LangSelectionStatusbarController::command(
    const css::awt::Point& aPos,
    ::sal_Int32 nCommand,
    sal_Bool /*bMouseEvent*/,
    const css::uno::Any& /*aData*/ )
throw (css::uno::RuntimeException, std::exception)
{
    if ( nCommand & ::awt::Command::CONTEXTMENU )
    {
        LangMenu( aPos );
    }
}

void SAL_CALL LangSelectionStatusbarController::click(
    const css::awt::Point& aPos )
throw (css::uno::RuntimeException, std::exception)
{
    LangMenu( aPos );
}

// XStatusListener
void SAL_CALL LangSelectionStatusbarController::statusChanged( const FeatureStateEvent& Event )
throw ( RuntimeException, std::exception )
{
    // This function will be called when observed data changes,
    // for example the selection or keyboard language.
    // - It displays the language in use in the status bar
    // - and it stores the relevant data for creating the menu
    //   at some later point in the member variables
    //      m_nScriptType, m_aCurLang, m_aKeyboardLang, m_aGuessedText

    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    m_bShowMenu = true;
    m_nScriptType = SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX;  //set the default value

    if ( m_xStatusbarItem.is() )
    {
        OUString aStrValue;
        Sequence< OUString > aSeq;

        if ( Event.State >>= aStrValue )
            m_xStatusbarItem->setText( aStrValue );
        else if ( Event.State >>= aSeq )
        {
            if ( aSeq.getLength() == 4 )
            {
                OUString aStatusText = aSeq[0];
                if (aStatusText == "*")
                {
                    aStatusText = FWK_RESSTR(STR_LANGSTATUS_MULTIPLE_LANGUAGES);
                }
                m_xStatusbarItem->setText( aStatusText );

                // Retrieve all other values from the sequence and
                // store it members!
                m_aCurLang      = aSeq[0];
                m_nScriptType   = static_cast< SvtScriptType >( aSeq[1].toInt32() );
                m_aKeyboardLang = aSeq[2];
                m_aGuessedTextLang  = aSeq[3];
            }
        }
        else if ( !Event.State.hasValue() )
        {
            m_xStatusbarItem->setText( OUString() );
            m_bShowMenu = false;    // no language -> no menu
        }
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_LangSelectionStatusbarController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new LangSelectionStatusbarController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
