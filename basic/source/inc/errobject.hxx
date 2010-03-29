#ifndef ERROBJECT_HXX
#define ERROBJECT_HXX
#include "sbunoobj.hxx"
#include <ooo/vba/XErrObject.hpp>


class SbxErrObject : public SbUnoObject
{
    com::sun::star::uno::Reference< ooo::vba::XErrObject > m_xErr;
    SbxErrObject( const String& aName_, const com::sun::star::uno::Any& aUnoObj_ );
    ~SbxErrObject();
public:
    static SbxVariableRef getErrObject();
    static com::sun::star::uno::Reference< ooo::vba::XErrObject > getUnoErrObject();
};
#endif
