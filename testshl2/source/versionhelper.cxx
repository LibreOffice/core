#include <stdlib.h>
#include "versionhelper.hxx"

#include <rtl/ustring.hxx>

// -----------------------------------------------------------------------------
VersionHelper::VersionHelper(rtl::OUString const& _sDLLName, GetOpt & _aOptions)
        :DynamicLibraryHelper(_sDLLName, _aOptions),
         m_pInfo(NULL)
{
    // try to get the entry pointer
    FktGetVersionInfoPtr pFunc = (FktGetVersionInfoPtr) m_pModule->getSymbol( rtl::OUString::createFromAscii( "GetVersionInfo" ) );

    if (pFunc)
    {
        const VersionInfo *pVersion = (pFunc)();
        m_pInfo = pVersion;
    }
}

void VersionHelper::print(std::ostream &stream)
{
    stream << m_pInfo->aTime   << std::endl;
    stream << m_pInfo->aDate   << std::endl;
    stream << m_pInfo->aUpd    << std::endl;
    stream << m_pInfo->aMinor  << std::endl;
    stream << m_pInfo->aBuild  << std::endl;
    stream << m_pInfo->aInpath << std::endl;
}

std::ostream &
operator <<( std::ostream &stream,
             VersionHelper &_aVersion )
{
    _aVersion.print (stream);
    return stream;
}


