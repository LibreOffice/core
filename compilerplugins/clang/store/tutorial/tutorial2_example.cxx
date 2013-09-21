/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// This is just an example file to see what AST looks like for return statements.
// To the AST, run :
// clang++ -fsyntax-only -Xclang -ast-dump tutorial1_example.cxx

bool g()
    {
    if( 1 == 2 )
        return false;
    if( 1 == 2 )
        {
        return false;
        }
    if( true )
        return false;
    }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
