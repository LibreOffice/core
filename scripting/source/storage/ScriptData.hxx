#ifndef _SCRIPTING_STORAGE_SCRIPTDATA_HXX_
#define _SCRIPTING_STORAGE_SCRIPTDATA_HXX_

#include <vector>
#include <cppu/macros.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <rtl/ustring.hxx>

namespace scripting_impl
{

struct ScriptData
{

inline ScriptData::ScriptData() SAL_THROW( () )
    : parcelURI()
    , functionName()
    , logicalName()
    , scriptDescription()
    , scriptLanguage()
    , scriptRoot()
    , scriptLocation()
    , scriptSource(sal_False)
    , scriptDependencies()
    , parcelDelivers()
{
}

inline ScriptData::ScriptData( const ::rtl::OUString& __parcelURI,
        const ::rtl::OUString& __functionName,
        const ::rtl::OUString& __logicalName,
        const ::rtl::OUString& __scriptDescription,
        const ::rtl::OUString& __scriptLanguage,
        const ::rtl::OUString& __scriptRoot,
        const ::rtl::OUString& __scriptLocation,
        const sal_Bool& __scriptSource,
        const ::std::vector < ::std::pair < ::rtl::OUString, bool > >
            __scriptDependencies,
        const ::std::vector < ::std::pair < ::rtl::OUString,
            ::rtl::OUString > >  __parcelDelivers ) SAL_THROW( () )
        : parcelURI(__parcelURI)
        , functionName(__functionName)
        , logicalName(__logicalName)
        , scriptDescription(__scriptDescription)
        , scriptLanguage(__scriptLanguage)
        , scriptRoot(__scriptRoot)
        , scriptLocation(__scriptLocation)
        , scriptSource(__scriptSource)
        , scriptDependencies(__scriptDependencies)
        , parcelDelivers(__parcelDelivers)
{
}

    ::rtl::OUString parcelURI;
    ::rtl::OUString functionName;
    ::rtl::OUString logicalName;
    ::rtl::OUString scriptDescription;
    ::rtl::OUString scriptLanguage;
    ::rtl::OUString scriptRoot;
    ::rtl::OUString scriptLocation;
    sal_Bool scriptSource;
    ::std::vector < ::std::pair < ::rtl::OUString, bool > > scriptDependencies;
    ::std::vector < ::std::pair < ::rtl::OUString, ::rtl::OUString > >  parcelDelivers;
};

}

#endif // _SCRIPTING_STORAGE_SCRIPTDATA_HXX_
