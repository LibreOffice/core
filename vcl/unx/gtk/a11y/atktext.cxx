/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: atktext.cxx,v $
 * $Revision: 1.9 $
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
#include "atktextattributes.hxx"

#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>

// #define ENABLE_TRACING

#ifdef ENABLE_TRACING
#include <stdio.h>
#endif

using namespace ::com::sun::star;

static sal_Int16
text_type_from_boundary(AtkTextBoundary boundary_type)
{
    switch(boundary_type)
    {
        case ATK_TEXT_BOUNDARY_CHAR:
            return accessibility::AccessibleTextType::CHARACTER;
        case ATK_TEXT_BOUNDARY_WORD_START:
        case ATK_TEXT_BOUNDARY_WORD_END:
            return accessibility::AccessibleTextType::WORD;
        case ATK_TEXT_BOUNDARY_SENTENCE_START:
        case ATK_TEXT_BOUNDARY_SENTENCE_END:
            return accessibility::AccessibleTextType::SENTENCE;
        case ATK_TEXT_BOUNDARY_LINE_START:
        case ATK_TEXT_BOUNDARY_LINE_END:
            return accessibility::AccessibleTextType::LINE;
        default:
            return -1;
    }
}

/*****************************************************************************/

static gchar *
adjust_boundaries( accessibility::XAccessibleText* pText,
                   accessibility::TextSegment& rTextSegment,
                   AtkTextBoundary  boundary_type,
                   gint * start_offset, gint * end_offset )
{
    accessibility::TextSegment aTextSegment;
    rtl::OUString aString;
    gint start = 0, end = 0;

    if( rTextSegment.SegmentText.getLength() > 0 )
    {
        switch(boundary_type)
        {
        case ATK_TEXT_BOUNDARY_CHAR:
        case ATK_TEXT_BOUNDARY_LINE_START:
        case ATK_TEXT_BOUNDARY_LINE_END:
        case ATK_TEXT_BOUNDARY_SENTENCE_START:
            start = rTextSegment.SegmentStart;
            end = rTextSegment.SegmentEnd;
            aString = rTextSegment.SegmentText;
            break;

        // the OOo break iterator behaves as SENTENCE_START
        case ATK_TEXT_BOUNDARY_SENTENCE_END:
            start = rTextSegment.SegmentStart;
            end = rTextSegment.SegmentEnd;

            if( start > 0 )
                --start;
            if( end > 0 && end < pText->getCharacterCount() - 1 )
                --end;

            aString = pText->getTextRange(start, end);
            break;

        case ATK_TEXT_BOUNDARY_WORD_START:
            start = rTextSegment.SegmentStart;

            // Determine the start index of the next segment
            aTextSegment = pText->getTextBehindIndex(rTextSegment.SegmentEnd,
                                                     text_type_from_boundary(boundary_type));
            if( aTextSegment.SegmentText.getLength() > 0 )
                end = aTextSegment.SegmentStart;
            else
                end = pText->getCharacterCount();

            aString = pText->getTextRange(start, end);
            break;

        case ATK_TEXT_BOUNDARY_WORD_END:
            end = rTextSegment.SegmentEnd;

            // Determine the end index of the previous segment
            aTextSegment = pText->getTextBeforeIndex(rTextSegment.SegmentStart,
                                                     text_type_from_boundary(boundary_type));
            if( aTextSegment.SegmentText.getLength() > 0 )
                start = aTextSegment.SegmentEnd;
            else
                start = 0;

            aString = pText->getTextRange(start, end);
            break;

        default:
            return NULL;
        }
    }

    *start_offset = start;
    *end_offset   = end;

#ifdef ENABLE_TRACING
    fprintf(stderr, "adjust_boundaries( %d, %d, %d ) returns %d, %d\n",
        rTextSegment.SegmentStart, rTextSegment.SegmentEnd, boundary_type,
        start, end);
#endif

    return OUStringToGChar(aString);
}

/*****************************************************************************/

static accessibility::XAccessibleText*
    getText( AtkText *pText ) throw (uno::RuntimeException)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( pText );
    if( pWrap )
    {
        if( !pWrap->mpText && pWrap->mpContext )
        {
            uno::Any any = pWrap->mpContext->queryInterface( accessibility::XAccessibleText::static_type(NULL) );
            pWrap->mpText = reinterpret_cast< accessibility::XAccessibleText * > (any.pReserved);
            pWrap->mpText->acquire();
        }

        return pWrap->mpText;
    }

    return NULL;
}

/*****************************************************************************/

static accessibility::XAccessibleTextAttributes*
    getTextAttributes( AtkText *pText ) throw (uno::RuntimeException)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( pText );
    if( pWrap )
    {
        if( !pWrap->mpTextAttributes && pWrap->mpContext )
        {
            uno::Any any = pWrap->mpContext->queryInterface( accessibility::XAccessibleTextAttributes::static_type(NULL) );
            /* Since this not a dedicated interface in Atk and thus has not
             * been queried during wrapper initialization, we need to check
             * the return value here.
             */
            if( typelib_TypeClass_INTERFACE == any.pType->eTypeClass )
            {
                pWrap->mpTextAttributes = reinterpret_cast< accessibility::XAccessibleTextAttributes * > (any.pReserved);
                pWrap->mpTextAttributes->acquire();
            }
        }

        return pWrap->mpTextAttributes;
    }

    return NULL;
}

/*****************************************************************************/

extern "C" {

static gchar *
text_wrapper_get_text (AtkText *text,
                       gint     start_offset,
                       gint     end_offset)
{
    gchar * ret = NULL;

    g_return_val_if_fail( (end_offset == -1) || (end_offset >= start_offset), NULL );

    /* at-spi expects the delete event to be send before the deletion happened
     * so we save the deleted string object in the UNO event notification and
     * fool libatk-bridge.so here ..
     */
    void * pData = g_object_get_data( G_OBJECT(text), "ooo::text_changed::delete" );
    if( pData != NULL )
    {
        accessibility::TextSegment * pTextSegment =
            reinterpret_cast <accessibility::TextSegment *> (pData);

        if( pTextSegment->SegmentStart == start_offset &&
            pTextSegment->SegmentEnd == end_offset )
        {
            rtl::OString aUtf8 = rtl::OUStringToOString( pTextSegment->SegmentText, RTL_TEXTENCODING_UTF8 );
            return g_strdup( aUtf8.getStr() );
        }
    }

    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
        {
            rtl::OUString aText;
            sal_Int32 n = pText->getCharacterCount();

            if( -1 == end_offset )
                aText = pText->getText();
            else if( start_offset < n )
                aText = pText->getTextRange(start_offset, end_offset);

            ret = g_strdup( rtl::OUStringToOString(aText, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getText()" );
    }

#ifdef ENABLE_TRACING
    fprintf(stderr, "text_wrapper_get_text( %d,%d ) returns %s\n", start_offset, end_offset, ret ? ret : "null" );
#endif
    return ret;
}

static gchar *
text_wrapper_get_text_after_offset (AtkText          *text,
                                    gint             offset,
                                    AtkTextBoundary  boundary_type,
                                    gint             *start_offset,
                                    gint             *end_offset)
{
    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
        {
            accessibility::TextSegment aTextSegment = pText->getTextBehindIndex(offset, text_type_from_boundary(boundary_type));
            return adjust_boundaries(pText, aTextSegment, boundary_type, start_offset, end_offset);
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in get_text_after_offset()" );
    }

    return NULL;
}

static gchar *
text_wrapper_get_text_at_offset (AtkText          *text,
                                 gint             offset,
                                 AtkTextBoundary  boundary_type,
                                 gint             *start_offset,
                                 gint             *end_offset)
{
    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
        {
            accessibility::TextSegment aTextSegment = pText->getTextAtIndex(offset, text_type_from_boundary(boundary_type));
            return adjust_boundaries(pText, aTextSegment, boundary_type, start_offset, end_offset);
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in get_text_at_offset()" );
    }

    return NULL;
}

static gunichar
text_wrapper_get_character_at_offset (AtkText          *text,
                                      gint             offset)
{
    gint start, end;
    gunichar uc = 0;

    gchar * char_as_string =
        text_wrapper_get_text_at_offset(text, offset, ATK_TEXT_BOUNDARY_CHAR,
                                        &start, &end);
    if( char_as_string )
    {
        uc = g_utf8_get_char( char_as_string );
        g_free( char_as_string );
    }

    return uc;
}

static gchar *
text_wrapper_get_text_before_offset (AtkText          *text,
                                     gint             offset,
                                     AtkTextBoundary  boundary_type,
                                     gint             *start_offset,
                                     gint             *end_offset)
{
    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
        {
            accessibility::TextSegment aTextSegment = pText->getTextBeforeIndex(offset, text_type_from_boundary(boundary_type));
            return adjust_boundaries(pText, aTextSegment, boundary_type, start_offset, end_offset);
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in text_before_offset()" );
    }

    return NULL;
}

static gint
text_wrapper_get_caret_offset (AtkText          *text)
{
    gint offset = -1;

    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
            offset = pText->getCaretPosition();
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getCaretPosition()" );
    }

#ifdef ENABLE_TRACING
    fprintf(stderr, "get_caret_offset(%p) returns %d\n", text, offset);
#endif

    return offset;
}

static gboolean
text_wrapper_set_caret_offset (AtkText *text,
                               gint     offset)
{
    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
            return pText->setCaretPosition( offset );
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in setCaretPosition()" );
    }

    return FALSE;
}

static AtkAttributeSet *
text_wrapper_get_run_attributes( AtkText        *text,
                                 gint           offset,
                                 gint           *start_offset,
                                 gint           *end_offset)
{
    AtkAttributeSet *pSet = NULL;

    try {
        accessibility::XAccessibleText* pText = getText( text );
        accessibility::XAccessibleTextAttributes* pTextAttributes = getTextAttributes( text );
        if( pText && pTextAttributes )
        {
            uno::Sequence< beans::PropertyValue > aAttributeList =
                pTextAttributes->getRunAttributes( offset, uno::Sequence< rtl::OUString > () );

            pSet = attribute_set_new_from_property_values( aAttributeList, true, text );

            accessibility::TextSegment aTextSegment =
                pText->getTextAtIndex(offset, accessibility::AccessibleTextType::ATTRIBUTE_RUN);

            *start_offset = aTextSegment.SegmentStart;
            *end_offset = aTextSegment.SegmentEnd + 1; // FIXME: TESTME
        }
    }
    catch(const uno::Exception& e) {

        g_warning( "Exception in get_run_attributes()" );

        if( pSet )
        {
            atk_attribute_set_free( pSet );
            pSet = NULL;
        }
    }

    return pSet;
}

/*****************************************************************************/

static AtkAttributeSet *
text_wrapper_get_default_attributes( AtkText *text )
{
    AtkAttributeSet *pSet = NULL;

    try {
        accessibility::XAccessibleTextAttributes* pTextAttributes = getTextAttributes( text );
        if( pTextAttributes )
        {
            uno::Sequence< beans::PropertyValue > aAttributeList =
                pTextAttributes->getDefaultAttributes( uno::Sequence< rtl::OUString > () );

            pSet = attribute_set_new_from_property_values( aAttributeList, false, text );
        }
    }
    catch(const uno::Exception& e) {

        g_warning( "Exception in get_default_attributes()" );

        if( pSet )
        {
            atk_attribute_set_free( pSet );
            pSet = NULL;
        }
    }

    return pSet;
}

/*****************************************************************************/

static void
text_wrapper_get_character_extents( AtkText          *text,
                                    gint             offset,
                                    gint             *x,
                                    gint             *y,
                                    gint             *width,
                                    gint             *height,
                                    AtkCoordType      coords )
{
    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
        {
            *x = *y = *width = *height = 0;
            awt::Rectangle aRect = pText->getCharacterBounds( offset );

            gint origin_x = 0;
            gint origin_y = 0;

            if( coords == ATK_XY_SCREEN )
            {
                g_return_if_fail( ATK_IS_COMPONENT( text ) );
                atk_component_get_position( ATK_COMPONENT( text ), &origin_x, &origin_y, coords);
            }

            *x = aRect.X + origin_x;
            *y = aRect.Y + origin_y;
            *width = aRect.Width;
            *height = aRect.Height;

#ifdef ENABLE_TRACING
            fprintf(stderr, "get_character_extents(%d, %d) returns: %d,%d,%d,%d ",
                offset, coords, *x, *y, *width, *height);
#endif
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getCharacterBounds" );
    }
}

static gint
text_wrapper_get_character_count (AtkText *text)
{
    gint rv = 0;

    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
            rv = pText->getCharacterCount();
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getCharacterCount" );
    }

#ifdef ENABLE_TRACING
    fprintf(stderr, "get_character_count(%p) returns: %d\n", text, rv);
#endif

    return rv;
}

static gint
text_wrapper_get_offset_at_point (AtkText     *text,
                                  gint         x,
                                  gint         y,
                                  AtkCoordType coords)
{
    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
        {
            gint origin_x = 0;
            gint origin_y = 0;

            if( coords == ATK_XY_SCREEN )
            {
                g_return_val_if_fail( ATK_IS_COMPONENT( text ), -1 );
                atk_component_get_position( ATK_COMPONENT( text ), &origin_x, &origin_y, coords);
            }

            return pText->getIndexAtPoint( awt::Point(x - origin_x, y - origin_y) );
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getIndexAtPoint" );
    }

    return -1;
}

// FIXME: the whole series of selections API is problematic ...

static gint
text_wrapper_get_n_selections (AtkText *text)
{
    gint rv = 0;

    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
            rv = ( pText->getSelectionEnd() > pText->getSelectionStart() ) ? 1 : 0;
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getSelectionEnd() or getSelectionStart()" );
    }

#ifdef ENABLE_TRACING
    fprintf(stderr, "get_n_selections(%p) returns %d\n", text, rv);
#endif

    return rv;
}

static gchar *
text_wrapper_get_selection (AtkText *text,
                            gint     selection_num,
                            gint    *start_offset,
                            gint    *end_offset)
{
    g_return_val_if_fail( selection_num == 0, FALSE );

    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
        {
            *start_offset = pText->getSelectionStart();
            *end_offset   = pText->getSelectionEnd();

            return OUStringToGChar( pText->getSelectedText() );
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getSelectionEnd(), getSelectionStart() or getSelectedText()" );
    }

    return NULL;
}

static gboolean
text_wrapper_add_selection (AtkText *text,
                            gint     start_offset,
                            gint     end_offset)
{
    // FIXME: can we try to be more compatible by expanding an
    //        existing adjacent selection ?

    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
            return pText->setSelection( start_offset, end_offset ); // ?
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in setSelection()" );
    }

    return FALSE;
}

static gboolean
text_wrapper_remove_selection (AtkText *text,
                               gint     selection_num)
{
    g_return_val_if_fail( selection_num == 0, FALSE );

    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
            return pText->setSelection( 0, 0 ); // ?
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in setSelection()" );
    }

    return FALSE;
}

static gboolean
text_wrapper_set_selection (AtkText *text,
                            gint     selection_num,
                            gint     start_offset,
                            gint     end_offset)
{
    g_return_val_if_fail( selection_num == 0, FALSE );

    try {
        accessibility::XAccessibleText* pText = getText( text );
        if( pText )
            return pText->setSelection( start_offset, end_offset );
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in setSelection()" );
    }

    return FALSE;
}

} // extern "C"

void
textIfaceInit (AtkTextIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->get_text = text_wrapper_get_text;
  iface->get_character_at_offset = text_wrapper_get_character_at_offset;
  iface->get_text_before_offset = text_wrapper_get_text_before_offset;
  iface->get_text_at_offset = text_wrapper_get_text_at_offset;
  iface->get_text_after_offset = text_wrapper_get_text_after_offset;
  iface->get_caret_offset = text_wrapper_get_caret_offset;
  iface->set_caret_offset = text_wrapper_set_caret_offset;
  iface->get_character_count = text_wrapper_get_character_count;
  iface->get_n_selections = text_wrapper_get_n_selections;
  iface->get_selection = text_wrapper_get_selection;
  iface->add_selection = text_wrapper_add_selection;
  iface->remove_selection = text_wrapper_remove_selection;
  iface->set_selection = text_wrapper_set_selection;
  iface->get_run_attributes = text_wrapper_get_run_attributes;
  iface->get_default_attributes = text_wrapper_get_default_attributes;
  iface->get_character_extents = text_wrapper_get_character_extents;
  iface->get_offset_at_point = text_wrapper_get_offset_at_point;
}
