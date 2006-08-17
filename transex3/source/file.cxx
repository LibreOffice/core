#include "file.hxx"

namespace transex
{

File::File( const rtl::OUString sFile )
{
    sFileName = sFile;
}

File::File( const rtl::OUString sFullPath , const rtl::OUString sFile )
{
    sFileName = sFile;
    sFullName = sFullPath;
}

bool File::lessFile ( const File& rKey1, const File& rKey2 )
{
    rtl::OUString sName1( ( static_cast< File >( rKey1 ) ).getFileName() );
    rtl::OUString sName2( ( static_cast< File >( rKey2 ) ).getFileName() );

    return sName1.compareTo( sName2 ) < 0 ;
}

}
