#include <cppunit/simpleheader.hxx>

namespace rtl_ByteSequence
{
// -----------------------------------------------------------------------
// call this before any other classes
// -----------------------------------------------------------------------
class OSingleton
{
public:
    static OSingleton * GetSingleton();
    virtual ~OSingleton() {}
protected :
    OSingleton()
        {
            printf("Do something before.\n");
        }

    static OSingleton * m_pSingleton;
};

OSingleton * OSingleton::m_pSingleton = NULL;
OSingleton * OSingleton::GetSingleton()
{
    if(m_pSingleton  == NULL )
    {
        m_pSingleton = new OSingleton;
    }
    return m_pSingleton;
}

// -----------------------------------------------------------------------------

class before : public CppUnit::TestFixture
{
protected:
    before()
        {
            OSingleton::GetSingleton();
        }
};

// -----------------------------------------------------------------------------

class GlobalObject
{
public:
    GlobalObject()
        {
            // do nothing here
        }
    ~GlobalObject()
        {
            printf("shutdown");
        }
};

GlobalObject theGlobalObject;

// -----------------------------------------------------------------------------
class assign : public before
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    // insert your test code here.
    void assign_001()
    {
    }

    CPPUNIT_TEST_SUITE(assign);
    CPPUNIT_TEST(assign_001);
    CPPUNIT_TEST_SUITE_END();
}; // class operator=

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_ByteSequence::assign, "rtl_ByteSequence");
} // namespace ByteSequence


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
void RegisterAdditionalFunctions(FktRegFuncPtr _pFunc)
{
    printf("init your work here\n");
}
// NOADDITIONAL;
