#include "MNSProfileDiscover.hxx"

using namespace connectivity::mork;

int main( int argc, const char* argv[] )
{
  int x = argc;
  x++;
  argv++;
  ProfileAccess* access = new ProfileAccess();
  ::rtl::OUString profile = access->getDefaultProfile(::com::sun::star::mozilla::MozillaProductType_Thunderbird);
}
