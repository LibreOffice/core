#ifndef __SAMPLELIBRTTI_HXX_
#define __SAMPLELIBRTTI_HXX_

class MyClassA
{
public:
    virtual void  funcA();
    virtual void  funcB();
protected:
    virtual void  funcC();
private:
    virtual void  funcD();
};


class MyClassB: public MyClassA
{
public:
    virtual void  funcA();
    virtual void  funcB();
protected:
    virtual void  funcC();
private:
    virtual void  funcD();

};

#endif