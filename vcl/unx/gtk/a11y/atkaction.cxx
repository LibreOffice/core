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

#include "atkwrapper.hxx"

#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleKeyBinding.hpp>

#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>

#include <rtl/strbuf.hxx>
#include <algorithm>
#include <map>

using namespace ::com::sun::star;

// FIXME
static G_CONST_RETURN gchar *
getAsConst( const OString& rString )
{
    static const int nMax = 10;
    static OString aUgly[nMax];
    static int nIdx = 0;
    nIdx = (nIdx + 1) % nMax;
    aUgly[nIdx] = rString;
    return aUgly[ nIdx ].getStr();
}

static css::uno::Reference<css::accessibility::XAccessibleAction>
        getAction( AtkAction *action ) throw (uno::RuntimeException)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( action );

    if( pWrap )
    {
        if( !pWrap->mpAction.is() )
        {
            pWrap->mpAction.set(pWrap->mpContext, css::uno::UNO_QUERY);
        }

        return pWrap->mpAction;
    }

    return css::uno::Reference<css::accessibility::XAccessibleAction>();
}

extern "C" {

static gboolean
action_wrapper_do_action (AtkAction *action,
                          gint       i)
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleAction> pAction
            = getAction( action );
        if( pAction.is() )
            return pAction->doAccessibleAction( i );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in doAccessibleAction()" );
    }

    return FALSE;
}

static gint
action_wrapper_get_n_actions (AtkAction *action)
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleAction> pAction
            = getAction( action );
        if( pAction.is() )
            return pAction->getAccessibleActionCount();
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleActionCount()" );
    }

    return 0;
}

static G_CONST_RETURN gchar *
action_wrapper_get_description (AtkAction *, gint)
{
    // GAIL implement this only for cells
    g_warning( "Not implemented: get_description()" );
    return "";
}

static G_CONST_RETURN gchar *
action_wrapper_get_localized_name (AtkAction *, gint)
{
    // GAIL doesn't implement this as well
    g_warning( "Not implemented: get_localized_name()" );
    return "";
}

#define ACTION_NAME_PAIR( OOoName, AtkName ) \
    std::pair< const OUString, const gchar * > ( OUString( OOoName ), AtkName )

static G_CONST_RETURN gchar *
action_wrapper_get_name (AtkAction *action,
                         gint       i)
{
    static std::map< OUString, const gchar * > aNameMap;

    if( aNameMap.empty() )
    {
        aNameMap.insert( ACTION_NAME_PAIR( "click", "click" ) );
        aNameMap.insert( ACTION_NAME_PAIR( "select", "click" ) );
        aNameMap.insert( ACTION_NAME_PAIR( "togglePopup", "push" ) );
    }

    try {
        css::uno::Reference<css::accessibility::XAccessibleAction> pAction
            = getAction( action );
        if( pAction.is() )
        {
            std::map< OUString, const gchar * >::iterator iter;

            OUString aDesc( pAction->getAccessibleActionDescription( i ) );

            iter = aNameMap.find( aDesc );
            if( iter != aNameMap.end() )
                return iter->second;

            std::pair< const OUString, const gchar * > aNewVal( aDesc,
                g_strdup( OUStringToConstGChar(aDesc) ) );

            if( aNameMap.insert( aNewVal ).second )
                return aNewVal.second;
        }
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleActionDescription()" );
    }

    return "";
}

/*
*  GNOME Expects a string in the format:
*
*  <mnemonic>;<full-path>;<accelerator>
*
*  The keybindings in <full-path> should be separated by ":"
*/

static inline void
appendKeyStrokes(OStringBuffer& rBuffer, const uno::Sequence< awt::KeyStroke >& rKeyStrokes)
{
    for( sal_Int32 i = 0; i < rKeyStrokes.getLength(); i++ )
    {
        if( rKeyStrokes[i].Modifiers &  awt::KeyModifier::SHIFT )
            rBuffer.append("<Shift>");
        if( rKeyStrokes[i].Modifiers &  awt::KeyModifier::MOD1 )
            rBuffer.append("<Control>");
        if( rKeyStrokes[i].Modifiers &  awt::KeyModifier::MOD2 )
            rBuffer.append("<Alt>");

        if( ( rKeyStrokes[i].KeyCode >= awt::Key::A ) && ( rKeyStrokes[i].KeyCode <= awt::Key::Z ) )
            rBuffer.append( (sal_Char) ( 'a' + ( rKeyStrokes[i].KeyCode - awt::Key::A ) ) );
        else
        {
            sal_Char c = '\0';

            switch( rKeyStrokes[i].KeyCode )
            {
                case awt::Key::TAB:      c = '\t'; break;
                case awt::Key::SPACE:    c = ' '; break;
                case awt::Key::ADD:      c = '+'; break;
                case awt::Key::SUBTRACT: c = '-'; break;
                case awt::Key::MULTIPLY: c = '*'; break;
                case awt::Key::DIVIDE:   c = '/'; break;
                case awt::Key::POINT:    c = '.'; break;
                case awt::Key::COMMA:    c = ','; break;
                case awt::Key::LESS:     c = '<'; break;
                case awt::Key::GREATER:  c = '>'; break;
                case awt::Key::EQUAL:    c = '='; break;
                case 0:
                    break;
                default:
                    g_warning( "Unmapped KeyCode: %d", rKeyStrokes[i].KeyCode );
                    break;
            }

            if( c != '\0' )
                rBuffer.append( c );
        }
    }
}

static G_CONST_RETURN gchar *
action_wrapper_get_keybinding (AtkAction *action,
                               gint       i)
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleAction> pAction
            = getAction( action );
        if( pAction.is() )
        {
            uno::Reference< accessibility::XAccessibleKeyBinding > xBinding( pAction->getAccessibleActionKeyBinding( i ));

            if( xBinding.is() )
            {
                OStringBuffer aRet;

                sal_Int32 nmax = std::min( xBinding->getAccessibleKeyBindingCount(), (sal_Int32) 3 );
                for( sal_Int32 n = 0; n < nmax; n++ )
                {
                    appendKeyStrokes( aRet,  xBinding->getAccessibleKeyBinding( n ) );

                    if( n < 2 )
                        aRet.append( (sal_Char) ';' );
                }

                // !! FIXME !! remember keystroke in wrapper object ?
                return getAsConst( aRet.makeStringAndClear() );
            }
        }
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in get_keybinding()" );
    }

    return "";
}

static gboolean
action_wrapper_set_description (AtkAction *, gint, const gchar *)
{
    return FALSE;
}

} // extern "C"

void
actionIfaceInit (AtkActionIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->do_action = action_wrapper_do_action;
  iface->get_n_actions = action_wrapper_get_n_actions;
  iface->get_description = action_wrapper_get_description;
  iface->get_keybinding = action_wrapper_get_keybinding;
  iface->get_name = action_wrapper_get_name;
  iface->get_localized_name = action_wrapper_get_localized_name;
  iface->set_description = action_wrapper_set_description;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
