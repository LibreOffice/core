#include "rtl/ustring.hxx"

namespace transex
{

class File
{
    private:
    rtl::OUString sFileName;
    rtl::OUString sFullName;

    public:
    rtl::OUString getFileName(){ return sFileName; }
    rtl::OUString getFullName(){ return sFullName; }

    File( const rtl::OUString sFile );
    File( const rtl::OUString sFullName , const rtl::OUString sFile );

    static bool lessFile ( const File& rKey1, const File& rKey2 );

};

}
