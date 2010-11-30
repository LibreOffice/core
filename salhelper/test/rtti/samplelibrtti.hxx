/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef __SAMPLELIBRTTI_HXX_
#define __SAMPLELIBRTTI_HXX_

class MyClassA
{
public:
    virtual void  funcA();
    virtual void  funcB();
protected:
    virtual void  funcC();
};


class MyClassB: public MyClassA
{
public:
    virtual void  funcA();
    virtual void  funcB();
protected:
    virtual void  funcC();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
