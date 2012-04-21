/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// Helper to suppress warnings in lex generated c code, see #i57362#
#include "src_yy.c"

void (*avoid_unused_yyunput_in_src_yy_c)() = yyunput;
int (*avoid_unused_yy_flex_strlen_in_src_yy_c)() = yy_flex_strlen;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
