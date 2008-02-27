#ifndef CPPUNIT_TYPEINFOHELPER_H
#define CPPUNIT_TYPEINFOHELPER_H

#include <cppunit/Portability.h>

#if CPPUNIT_USE_TYPEINFO_NAME

#include <typeinfo>


namespace CppUnit {

  /** Helper to use type_info.
   */
  class CPPUNIT_API TypeInfoHelper
  {
  public:
    /** Get the class name of the specified type_info.
     * \param info Info which the class name is extracted from.
     * \return The string returned by type_info::name() without
     *         the "class" prefix. If the name is not prefixed
     *         by "class", it is returned as this.
     */
    static std::string getClassName( const std::type_info &info );
  };

}  // namespace CppUnit

#endif

#endif  // CPPUNIT_TYPEINFOHELPER_H
