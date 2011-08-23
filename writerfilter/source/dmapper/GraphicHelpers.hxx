#ifndef INCLUDED_GRAPHICHELPERS_HXX
#define INCLUDED_GRAPHICHELPERS_HXX

#include "PropertyMap.hxx"

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>

#include <boost/shared_ptr.hpp>

namespace writerfilter {
namespace dmapper
{

class WRITERFILTER_DLLPRIVATE PositionHandler: public Properties
{
public:
    PositionHandler( );
    ~PositionHandler( );

    sal_Int16 m_nOrient;
    sal_Int16 m_nRelation;
    sal_Int32 m_nPosition;

    virtual void attribute( Id aName, Value& rVal );
    virtual void sprm( Sprm& rSprm );
}; 
typedef boost::shared_ptr<PositionHandler> PositionHandlerPtr;

class WRITERFILTER_DLLPRIVATE WrapHandler: public Properties
{
public:
    WrapHandler( );
    ~WrapHandler( );

    sal_Int32 m_nType;
    sal_Int32 m_nSide;

    sal_Int32 getWrapMode( );

    virtual void attribute( Id aName, Value& rVal );
    virtual void sprm( Sprm& rSprm );
};
typedef boost::shared_ptr<WrapHandler> WrapHandlerPtr;

} }

#endif
