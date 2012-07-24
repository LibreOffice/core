#include "MNSProfileDiscover.hxx"

using namespace connectivity::mozab;

int main( int argc, const char* argv[] )
{
  ProfileAccess* access = new ProfileAccess();
  ::rtl::OUString profile = access->getDefaultProfile(::com::sun::star::mozilla::MozillaProductType_Thunderbird);
}
