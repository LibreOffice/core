/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
#ifndef __FLEX_LEXER_H
// Never included before - need to define base class.
#define __FLEX_LEXER_H
typedef void ostream;
#include <vector>
#include <rtftok/RTFInputSource.hxx>

extern "C++" {

struct yy_buffer_state;
typedef int yy_state_type;
  
}
#include <rtftok/RTFScanner.hxx>
#endif

#if defined(yyFlexLexer) || ! defined(yyFlexLexerOnce)
// Either this is the first time through (yyFlexLexerOnce not defined),
// or this is a repeated include to define a different flavor of
// yyFlexLexer, as discussed in the flex man page.
#define yyFlexLexerOnce

class yyFlexLexer : public writerfilter::rtftok::RTFScanner {
private:
    class RtfTokenizer* tokenizerInstance;

    void split_ctrl(char *yytext, char* token, char *value);
    void raise_ctrl(char* yytext);
    void raise_dest(char* yytext);
    void raise_destOrCtrl(char* yytext);
protected:
    int num_lines;
    sal_uInt64 num_chars;
    int star_flag;

public:
    // arg_yyin and arg_yyout default to the cin and cout, but we
    // only make that assignment when initializing in yylex().
    yyFlexLexer( class writerfilter::rtftok::RTFInputSource* arg_yyin, class writerfilter::rtftok::RTFScannerHandler &eventHandler );

    virtual ~yyFlexLexer();

    void yy_switch_to_buffer( struct yy_buffer_state* new_buffer );
    struct yy_buffer_state* yy_create_buffer( class writerfilter::rtftok::RTFInputSource* s, int size );
    void yy_delete_buffer( struct yy_buffer_state* b );
    void yyrestart( class writerfilter::rtftok::RTFInputSource* s );

    virtual int yylex();
    virtual void switch_streams( class writerfilter::rtftok::RTFInputSource* new_in, ostream* new_out );

protected:

//	int LexerInput( char* buf, int max_size );
    virtual void LexerOutput( const char* buf, int size );
    virtual void LexerError( const char* msg );

    void yyunput( int c, char* buf_ptr );
    int yyinput();

    void yy_load_buffer_state();
    void yy_init_buffer( struct yy_buffer_state* b, class writerfilter::rtftok::RTFInputSource* s );
    void yy_flush_buffer( struct yy_buffer_state* b );

    int yy_start_stack_ptr;
    int yy_start_stack_depth;
    int* yy_start_stack;

    void yy_push_state( int new_state );
    void yy_pop_state();
    int yy_top_state();

    yy_state_type yy_get_previous_state();
    yy_state_type yy_try_NUL_trans( yy_state_type current_state );
    int yy_get_next_buffer();

    class writerfilter::rtftok::RTFInputSource* yyin;	// input source for default LexerInput
    ostream* yyout;	// output sink for default LexerOutput

    struct yy_buffer_state* yy_current_buffer;

    // yy_hold_char holds the character lost when yytext is formed.
    char yy_hold_char;

    // Number of characters read into yy_ch_buf.
    int yy_n_chars;

    // Points to current character in buffer.
    char* yy_c_buf_p;

    int yy_init;		// whether we need to initialize
    int yy_start;		// start state number

    // Flag which is used to allow yywrap()'s to do buffer switches
    // instead of setting up a fresh yyin.  A bit of a hack ...
    int yy_did_buffer_switch_on_eof;

    // The following are not always needed, but may be depending
    // on use of certain flex features (like REJECT or yymore()).

    yy_state_type yy_last_accepting_state;
    char* yy_last_accepting_cpos;

    yy_state_type* yy_state_buf;
    yy_state_type* yy_state_ptr;

    char* yy_full_match;
    int* yy_full_state;
    int yy_full_lp;

    int yy_lp;
    int yy_looking_for_trail_begin;

    int yy_more_flag;
    int yy_more_len;
    int yy_more_offset;
    int yy_prev_more_offset;

 private:

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
