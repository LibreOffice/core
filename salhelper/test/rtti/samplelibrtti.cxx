/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "samplelibrtti.hxx"
#include <stdio.h>

// MyClassA =============================================================
void MyClassA::funcA()
{
    printf("MyClassA::funcA \n");
}

void MyClassA::funcB()
{
}

void MyClassA::funcC()
{
}

void MyClassA::funcD()
{
}

// MyClassB ===============================================================
void MyClassB::funcA()
{

    printf("MyClassA::funcB \n");
}

void MyClassB::funcB()
{
}

void MyClassB::funcC()
{
}

void MyClassB::funcD()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
