/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <stdio.h>
#include <typeinfo>
#include "samplelibrtti.hxx"

int main( void )
{
    MyClassB b;
    MyClassA* pA= &b;
    // test the virtual function
    pA->funcA();

    if( typeid( b) == typeid( pA))
        printf("\nsame types");

    MyClassB* pB= dynamic_cast<MyClassB* >( pA);
    pB->funcA();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
