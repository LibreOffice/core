/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: atkaction.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "atkwrapper.hxx"

#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleKeyBinding.hpp>

#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>

#include <rtl/strbuf.hxx>
#include <algorithm>
#include <map>

#include <stdio.h>

using namespace ::com::sun::star;

// FIXME
static G_CONST_RETURN gchar *
getAsConst( const rtl::OString& rString )
{
    static const int nMax = 10;
    static rtl::OString aUgly[nMax];
    static int nIdx = 0;
    nIdx = (nIdx + 1) % nMax;
    aUgly[nIdx] = rString;
    return aUgly[ nIdx ];
}

static accessibility::XAccessibleAction*
        getAction( AtkAction *action ) throw (uno::RuntimeException)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( action );

    if( pWrap )
    {
        if( !pWrap->mpAction && pWrap->mpContext )
        {
            uno::Any any = pWrap->mpContext->queryInterface( accessibility::XAccessibleAction::static_type(NULL) );
            pWrap->mpAction = reinterpret_cast< accessibility::XAccessibleAction * > (any.pReserved);
            pWrap->mpAction->acquire();
        }

        return pWrap->mpAction;
    }

    return NULL;
}

extern "C" {

static gboolean
action_wrapper_do_action (AtkAction *action,
                          gint       i)
{
    try {
        accessibility::XAccessibleAction* pAction = getAction( action );
        if( pAction )
            return pAction->doAccessibleAction( i );
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in doAccessibleAction()" );
    }

    return FALSE;
}

static gint
action_wrapper_get_n_actions (AtkAction *action)
{
    try {
        accessibility::XAccessibleAction* pAction = getAction( action );
        if( pAction )
            return pAction->getAccessibleActionCount();
    }
    catch(const uno::Exception& e) {
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
    std::pair< const rtl::OUString, const gchar * > ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OOoName ) ), AtkName )

static G_CONST_RETURN gchar *
action_wrapper_get_name (AtkAction *action,
                         gint       i)
{
    static std::map< rtl::OUString, const gchar * > aNameMap;

    if( aNameMap.empty() )
    {
        aNameMap.insert( ACTION_NAME_PAIR( "click", "click" ) );
        aNameMap.insert( ACTION_NAME_PAIR( "select", "click" ) );
        aNameMap.insert( ACTION_NAME_PAIR( "togglePopup", "push" ) );
    }

    try {
        accessibility::XAccessibleAction* pAction = getAction( action );
        if( pAction )
        {
            std::map< rtl::OUString, const gchar * >::iterator iter;

            rtl::OUString aDesc( pAction->getAccessibleActionDescription( i ) );

            iter = aNameMap.find( aDesc );
            if( iter != aNameMap.end() )
                return iter->second;

            std::pair< const rtl::OUString, const gchar * > aNewVal( aDesc,
                g_strdup( OUStringToConstGChar(aDesc) ) );

            if( aNameMap.insert( aNewVal ).second );
                return aNewVal.second;
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getAccessibleActionDescription()" );
    }

    return "";
}

/*
*  GNOME Expects a string in the format:
*
*  <nmemonic>;<full-path>;<accelerator>
*
*  The keybindings in <full-path> should be separated by ":"
*/

static inline void
appendKeyStrokes(rtl::OStringBuffer& rBuffer, const uno::Sequence< awt::KeyStroke >& rKeyStrokes)
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
        accessibility::XAccessibleAction* pAction = getAction( action );
        if( pAction )
        {
            uno::Reference< accessibility::XAccessibleKeyBinding > xBinding( pAction->getAccessibleActionKeyBinding( i ));

            if( xBinding.is() )
            {
                rtl::OStringBuffer aRet;

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
    catch(const uno::Exception& e) {
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
