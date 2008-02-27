#include <stdio.h>
#include <stdlib.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestSuite.h>

#include <set>
//!io #include <iostream>
#include <sstream>

#if CPPUNIT_USE_TYPEINFO_NAME
#  include "cppunit/extensions/TypeInfoHelper.h"
#endif

#include <cppunit/externcallbackfunc.hxx>
#include <cppunit/tagvalues.hxx>

#undef HELLOWORLD_DEBUGGER

namespace CppUnit {

/** (Implementation) This class manages all the TestFactoryRegistry.
 *
 * Responsible for the life-cycle of the TestFactoryRegistry.
 *
 * TestFactory registry must call wasDestroyed() to indicate that
 * a given TestRegistry was destroyed, and needDestroy() to
 * know if a given TestFactory need to be destroyed (was not already
 * destroyed by another TestFactoryRegistry).
 */
class NamedRegistries
{
public:
  ~NamedRegistries();

  static NamedRegistries &getInstance();

  TestFactoryRegistry &getRegistry( std::string const& name );

  void wasDestroyed( TestFactory *factory );

  bool needDestroy( TestFactory *factory );

  void testRegistries( hTestResult );
private:
    typedef std::map<std::string, TestFactoryRegistry *> MapRegistries;
    // typedef std::pair<std::string, TestFactoryRegistry *> StringTestFactoryRegistry;
    // typedef std::vector< StringTestFactoryRegistry > MapRegistries;
  MapRegistries m_registries;

  typedef std::set<TestFactory *> Factories;
  Factories m_factoriesToDestroy;
  Factories m_destroyedFactories;
};


NamedRegistries::~NamedRegistries()
{
#ifdef HELLOWORLD_DEBUGGER
    printf("~NamedRegistries.\n");
#endif
    MapRegistries::iterator it = m_registries.begin();
    while ( it != m_registries.end() )
    {
        TestFactoryRegistry *registry = (it++)->second;
        if ( needDestroy( registry ) )
            delete registry;
    }
/*
    for (MapRegistries::iterator it = m_registries.begin();
         it != m_registries.end();
         ++it)
    {
        StringTestFactoryRegistry stfr = *it;
        TestFactoryRegistry *registry = stfr.second;
        if ( needDestroy( registry ) )
            delete registry;
    }
*/
}


NamedRegistries &
NamedRegistries::getInstance()
{
  static NamedRegistries namedRegistries;
  return namedRegistries;
}


TestFactoryRegistry &
NamedRegistries::getRegistry( std::string const& name )
{
#ifdef HELLOWORLD_DEBUGGER
    printf("getRegistry '%s', ", name.c_str());
#endif
    MapRegistries::const_iterator foundIt = m_registries.find( name );
    if ( foundIt == m_registries.end() )
    {
        TestFactoryRegistry *factory = new TestFactoryRegistry( name );
        // m_registries.insert( std::make_pair( name, factory ) );
        m_registries[name] = factory;
        m_factoriesToDestroy.insert( factory );

#ifdef HELLOWORLD_DEBUGGER
        printf("factory not found, create.\n");
#endif
        return *factory;
    }
#ifdef HELLOWORLD_DEBUGGER
    printf("factory found.\n");
#endif
    return *foundIt->second;
/*
    // MapRegistries::const_iterator foundIt;
    TestFactoryRegistry *pFound = NULL;
    for (MapRegistries::const_iterator it = m_registries.begin();
         it != m_registries.end();
         ++it)
    {
        StringTestFactoryRegistry stfr = *it;
        if (name == stfr.first)
        {
            pFound = stfr.second;
            break;
        }
    }

    // MapRegistries::const_iterator foundIt = m_registries.find( name );
  if ( pFound != NULL )
  {
    TestFactoryRegistry *factory = new TestFactoryRegistry( name );
    // m_registries.insert( std::make_pair( name, factory ) );
    StringTestFactoryRegistry stfr = std::make_pair(name, factory);
    m_registries.push_back(stfr);
    m_factoriesToDestroy.insert( factory );
    return *factory;
  }
  return *pFound;
*/
}


void
NamedRegistries::wasDestroyed( TestFactory *factory )
{
  m_factoriesToDestroy.erase( factory );
  m_destroyedFactories.insert( factory );
}


bool
NamedRegistries::needDestroy( TestFactory *factory )
{
  return m_destroyedFactories.count( factory ) == 0;
}

void NamedRegistries::testRegistries( hTestResult _pResult)
{
    // initialise the test handling
    (pCallbackFunc)(0 /* NULL */, TAG_TYPE, INIT_TEST, TAG_RESULT_PTR, _pResult, TAG_DONE);

    // Start signal based Check
    for (MapRegistries::const_iterator it = m_registries.begin();
         it != m_registries.end();
         ++it)
    {
        std::string sName = (*it).first;

        CppUnit::Test* tp = CppUnit::TestFactoryRegistry::getRegistry(sName).makeTest();
        tp->run(_pResult);
    }

    // release test handling
    (pCallbackFunc)(0 /* NULL */, TAG_TYPE, RELEASE_TEST, TAG_RESULT_PTR, _pResult, TAG_DONE);
    // start normal based tests, leave signal signed tests out.

    //! Allfunction in the inclusion list which has a flag, not found, has to call here.
}

// -----------------------------------------------------------------------------

TestFactoryRegistry::TestFactoryRegistry( std::string const& name ) :
    m_name( name )
{
}


TestFactoryRegistry::~TestFactoryRegistry()
{
  // The wasDestroyed() and needDestroy() is used to prevent
  // a double destruction of a factory registry.
  // registerFactory( "All Tests", getRegistry( "Unit Tests" ) );
  // => the TestFactoryRegistry "Unit Tests" is owned by both
  // the "All Tests" registry and the NamedRegistries...
  NamedRegistries::getInstance().wasDestroyed( this );

  for ( Factories::iterator it = m_factories.begin(); it != m_factories.end(); ++it )
  {
    TestFactory *factory = it->second;
    if ( NamedRegistries::getInstance().needDestroy( factory ) )
      delete factory;
  }
}

// static
TestFactoryRegistry &
TestFactoryRegistry::getRegistry()
{
  return getRegistry( "All Tests" );
}

// static
TestFactoryRegistry &
TestFactoryRegistry::getRegistry( const std::string &name )
{
  return NamedRegistries::getInstance().getRegistry( name );
}

// static
void TestFactoryRegistry::testRegistries( hTestResult _pResult )
{
    NamedRegistries::getInstance().testRegistries( _pResult );
}


void
TestFactoryRegistry::registerFactory( const std::string &name,
                                      TestFactory *factory )
{
#ifdef HELLOWORLD_DEBUGGER
    printf("registerFactory %s\n", name.c_str());
#endif
    m_factories[name] = factory;
#ifdef HELLOWORLD_DEBUGGER
    printf("FactorySize: %d\n ", m_factories.size());
#endif
}


void
TestFactoryRegistry::registerFactory( TestFactory *factory )
{
    static int serialNumber = 1;

    std::string aStr("@Dummy@_");

    sal_Char cBuf[20];
    sal_Char *pBuf = cBuf;
    sprintf(pBuf, "%d", serialNumber++ );

    aStr += std::string(pBuf);

    // LLA: this does not work within Solaris, could be a problem of the osl::Module::load?
    // std::ostringstream ost;
    // ost << "@Dummy@" << nValue;
    // registerFactory( ost.str(), factory );

#ifdef HELLOWORLD_DEBUGGER
    printf("aStr: %s\n", aStr.c_str());
#endif
    registerFactory( aStr, factory );
}


Test *
TestFactoryRegistry::makeTest()
{
  TestSuite *suite = new TestSuite( m_name );
  addTestToSuite( suite );
  return suite;
}


void
TestFactoryRegistry::addTestToSuite( TestSuite *suite )
{
#ifdef HELLOWORLD_DEBUGGER
  printf("addTestToSuite %s\n", suite->getName().c_str());
#endif
  for ( Factories::iterator it = m_factories.begin();
        it != m_factories.end();
        ++it )
  {
    TestFactory *factory = (*it).second;
    suite->addTest( factory->makeTest() );
  }
}


}  // namespace CppUnit
