#include <stdlib.h>
#include <memory>
#include <iostream>

#include <stdio.h>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

class AutoPtrTest
{
    int m_nValue;

    // NOCOPY
    AutoPtrTest(AutoPtrTest const& );
    AutoPtrTest& operator =(AutoPtrTest const& );

public:
    AutoPtrTest(int _nValue = 1)
            :m_nValue(_nValue)
        {
            std::cout << "build Value" << std::endl;
        }
    ~AutoPtrTest()
        {
            std::cout << "release Value" << std::endl;
            m_nValue = 0;
        }
//    int operator int() const {return m_nValue;}
    int getValue() const {return m_nValue;}
};

std::auto_ptr<AutoPtrTest> getPtr()
{
    return std::auto_ptr<AutoPtrTest>(new AutoPtrTest(2));
}

// auto_ptr get() gibt den Pointer ohne das ownership
// auto_ptr release()gibt den Pointer und (!) das ownership

void test_autoptr()
{
    AutoPtrTest *pValuePtr;
    std::cout << "start test" << std::endl;
    {
        std::auto_ptr<AutoPtrTest> pValue = getPtr();
        std::cout << "Value with getValue() " << pValue->getValue() << std::endl;
        // std::cout << "Value operator int()  " << *pValue << std::endl;
        // ownership weiterreichen
        pValuePtr = pValue.release();
    }
    std::cout << "ValuePtr with getValue() " << pValuePtr->getValue() << std::endl;

    // ownership an neuen Pointer uebergeben
    std::auto_ptr<AutoPtrTest> pValue2(pValuePtr);

    // AutoPtrTest *pStr = getPtr();
    std::cout << "end test" << std::endl;
}

// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int argc, char* argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
    test_autoptr();
    return 0;
}
