// This is just an example file to see what AST looks like for return statements.
// To the AST, run :
// clang++ -fsyntax-only -Xclang -ast-dump tutorial1_example.cxx

void f()
    {
    return;
    }

bool g()
    {
    return false;
    }

bool h()
    {
    return 3 > 2;
    }
