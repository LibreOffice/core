/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: atkeditabletext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:26:15 $
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
#include "precompiled_vcl.hxx"

#include "atkwrapper.hxx"
#include "atktextattributes.hxx"

#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>

// #include <functional>
// #include <hash_map>

#include <stdio.h>
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
            uno::Any any = pWrap->mpContext->queryInterface( accessibility::XAccessibleEditableText::static_type(NULL) );
            pWrap->mpEditableText = reinterpret_cast< accessibility::XAccessibleEditableText * > (any.pReserved);
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
    catch(const uno::Exception& e) {
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
            rtl::OUString aString ( string, strlen(string), RTL_TEXTENCODING_UTF8 );
            pEditableText->setText( aString );
        }
    }
    catch(const uno::Exception& e) {
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
            rtl::OUString aString ( string, length, RTL_TEXTENCODING_UTF8 );
            if( pEditableText->insertText( aString, *pos ) )
                *pos += length;
        }
    }
    catch(const uno::Exception& e) {
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
    catch(const uno::Exception& e) {
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
    catch(const uno::Exception& e) {
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
    catch(const uno::Exception& e) {
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
    catch(const uno::Exception& e) {
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
