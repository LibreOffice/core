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
    MorkTableMap *Tables = 0;
    MorkTableMap::iterator tableIter;
    MorkRowMap::iterator rowIter;
    Tables = mork.getTables( defaultScope );
    if ( Tables )
    {
        // Iterate all tables
        for ( tableIter = Tables->begin(); tableIter != Tables->end(); tableIter++ )
        {
            if ( 0 == tableIter->first ) continue;
            SAL_INFO("connectivity.mork", "table->first : " << tableIter->first);
            std::string column = mork.getColumn( tableIter->first );
            std::string value = mork.getValue( tableIter->first );
            SAL_INFO("connectivity.mork", "table.column : " << column);
            SAL_INFO("connectivity.mork", "table.value : " << value);
#if 0
            MorkRowMap *Rows = 0;
            // Get rows
            Rows = mork.getRows( defaultScope, &tableIter->second );
            if ( Rows ) {
                // Iterate all rows
                for ( rowIter = Rows->begin(); rowIter != Rows->end(); rowIter++ )
                {
                    if ( 0 == rowIter->first ) continue;
                    RawAbeMap ram;
                    std::string column;
                    std::string value;
                    char buffer[20];
                    sprintf( buffer, "%d", rowIter->first );
                    abe.id = std::string( buffer );
                    // Get cells
                    for ( MorkCells::iterator cellsIter = rowIter->second.begin();
                          cellsIter != rowIter->second.end(); cellsIter++ )
                    {
                        column = mork.getColumn( cellsIter->first );
                        value = mork.getValue( cellsIter->second );
                        ram[ column ] = value;
                    }
                    AbeMap::iterator abeIter;
                    abes_[ rowIter->first ] = abe;
                    abeIter = abes_.find( rowIter->first );
                    addEntry( ram, abeIter->second );
                }
            }
#endif
        }
    }

    mork.dump();

    return true;
}

int main( int argc, const char* argv[] )
{
  int x = argc;
  x++;
  argv++;
  ProfileAccess* access = new ProfileAccess();
  ::rtl::OUString defaultProfile = access->getDefaultProfile(::com::sun::star::mozilla::MozillaProductType_Thunderbird);
  SAL_INFO("connectivity.mork", "DefaultProfile: " << defaultProfile);

  ::rtl::OUString profilePath = access->getProfilePath(::com::sun::star::mozilla::MozillaProductType_Thunderbird, defaultProfile);
  SAL_INFO("connectivity.mork", "ProfilePath: " << profilePath);

  profilePath += rtl::OUString( "/abook.mab" );

  SAL_INFO("connectivity.mork", "abook.mab: " << profilePath);
  rtl::OString aOString = ::rtl::OUStringToOString( profilePath, RTL_TEXTENCODING_UTF8 );
  openAddressBook(aOString.getStr());
}
