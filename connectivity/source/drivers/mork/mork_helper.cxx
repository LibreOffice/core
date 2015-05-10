/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "MNSProfileDiscover.hxx"
#include "MorkParser.hxx"


using namespace connectivity::mork;

bool openAddressBook(const std::string& path)
{
    MorkParser mork;
    // Open and parse mork file
    if (!mork.open(path))
    {
        return false;
    }
    const int defaultScope = 0x80;
    MorkTableMap::Map::iterator tableIter;
    MorkTableMap *Tables = mork.getTables( defaultScope );
    if ( Tables )
    {
        // Iterate all tables
        for ( tableIter = Tables->map.begin(); tableIter != Tables->map.end(); ++tableIter )
        {
            if ( 0 == tableIter->first ) continue;
            SAL_INFO("connectivity.mork", "table->first : " << tableIter->first);
            std::string column = mork.getColumn( tableIter->first );
            std::string value = mork.getValue( tableIter->first );
            SAL_INFO("connectivity.mork", "table.column : " << column);
            SAL_INFO("connectivity.mork", "table.value : " << value);
        }
    }

    mork.dump();

    return true;
}

int main(void)
{
  ProfileAccess* access = new ProfileAccess();
  OUString defaultProfile = access->getDefaultProfile(::com::sun::star::mozilla::MozillaProductType_Thunderbird);
  SAL_INFO("connectivity.mork", "DefaultProfile: " << defaultProfile);

  OUString profilePath = access->getProfilePath(::com::sun::star::mozilla::MozillaProductType_Thunderbird, defaultProfile);
  SAL_INFO("connectivity.mork", "ProfilePath: " << profilePath);

  profilePath += OUString( "/abook.mab" );

  SAL_INFO("connectivity.mork", "abook.mab: " << profilePath);
  OString aOString = OUStringToOString( profilePath, RTL_TEXTENCODING_UTF8 );
  openAddressBook(aOString.getStr());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
