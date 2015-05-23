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
#include "atktextattributes.hxx"

#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>

#include <string.h>

using namespace ::com::sun::star;

static accessibility::XAccessibleEditableText*
    getEditableText( AtkEditableText *pEditableText ) throw (uno::RuntimeException)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( pEditableText );
    if( pWrap )
    {
        if( !pWrap->mpEditableText && pWrap->mpContext )
        {
            uno::Any any = pWrap->mpContext->queryInterface( cppu::UnoType<accessibility::XAccessibleEditableText>::get() );
            pWrap->mpEditableText = static_cast< accessibility::XAccessibleEditableText * > (any.pReserved);
            pWrap->mpEditableText->acquire();
        }

        return pWrap->mpEditableText;
    }

    return NULL;
}

/*****************************************************************************/

extern "C" {

static gboolean
editable_text_wrapper_set_run_attributes( AtkEditableText  *text,
                                          AtkAttributeSet  *attribute_set,
                                          gint              nStartOffset,
                                          gint              nEndOffset)
{
    try {
        accessibility::XAccessibleEditableText* pEditableText = getEditableText( text );
        if( pEditableText )
        {
            uno::Sequence< beans::PropertyValue > aAttributeList;

            if( attribute_set_map_to_property_values( attribute_set, aAttributeList ) )
                return pEditableText->setAttributes(nStartOffset, nEndOffset, aAttributeList);
        }
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in setAttributes()" );
    }

    return FALSE;
}

static void
editable_text_wrapper_set_text_contents( AtkEditableText  *text,
                                         const gchar      *string )
{
    try {
        accessibility::XAccessibleEditableText* pEditableText = getEditableText( text );
        if( pEditableText )
        {
            OUString aString ( string, strlen(string), RTL_TEXTENCODING_UTF8 );
            pEditableText->setText( aString );
        }
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in setText()" );
    }
}

static void
editable_text_wrapper_insert_text( AtkEditableText  *text,
                                   const gchar      *string,
                                   gint             length,
                                   gint             *pos )
{
    try {
        accessibility::XAccessibleEditableText* pEditableText = getEditableText( text );
        if( pEditableText )
        {
            OUString aString ( string, length, RTL_TEXTENCODING_UTF8 );
            if( pEditableText->insertText( aString, *pos ) )
                *pos += length;
        }
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in insertText()" );
    }
}

static void
editable_text_wrapper_cut_text( AtkEditableText  *text,
                                gint             start,
                                gint             end )
{
    try {
        accessibility::XAccessibleEditableText* pEditableText = getEditableText( text );
        if( pEditableText )
            pEditableText->cutText( start, end );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in cutText()" );
    }
}

static void
editable_text_wrapper_delete_text( AtkEditableText  *text,
                                   gint             start,
                                   gint             end )
{
    try {
        accessibility::XAccessibleEditableText* pEditableText = getEditableText( text );
        if( pEditableText )
            pEditableText->deleteText( start, end );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in deleteText()" );
    }
}

static void
editable_text_wrapper_paste_text( AtkEditableText  *text,
                                  gint             pos )
{
    try {
        accessibility::XAccessibleEditableText* pEditableText = getEditableText( text );
        if( pEditableText )
            pEditableText->pasteText( pos );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in pasteText()" );
    }
}

static void
editable_text_wrapper_copy_text( AtkEditableText  *text,
                                 gint             start,
                                 gint             end )
{
    try {
        accessibility::XAccessibleEditableText* pEditableText = getEditableText( text );
        if( pEditableText )
            pEditableText->copyText( start, end );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in copyText()" );
    }
}

} // extern "C"

void
editableTextIfaceInit (AtkEditableTextIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->set_text_contents = editable_text_wrapper_set_text_contents;
  iface->insert_text = editable_text_wrapper_insert_text;
  iface->copy_text = editable_text_wrapper_copy_text;
  iface->cut_text = editable_text_wrapper_cut_text;
  iface->delete_text = editable_text_wrapper_delete_text;
  iface->paste_text = editable_text_wrapper_paste_text;
  iface->set_run_attributes = editable_text_wrapper_set_run_attributes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
