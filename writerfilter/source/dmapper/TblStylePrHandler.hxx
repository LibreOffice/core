#ifndef INCLUDED_TBLSTYLEPRHANDLER_HXX
#define INCLUDED_TBLSTYLEPRHANDLER_HXX

#include "TablePropertiesHandler.hxx"

#include <dmapper/DomainMapper.hxx>
#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>

namespace writerfilter {
namespace dmapper {

class DomainMapper;
class PropertyMap;

enum TblStyleType
{
    TBL_STYLE_UNKNOWN,
    TBL_STYLE_WHOLETABLE,
    TBL_STYLE_FIRSTROW,
    TBL_STYLE_LASTROW,
    TBL_STYLE_FIRSTCOL,
    TBL_STYLE_LASTCOL,
    TBL_STYLE_BAND1VERT,
    TBL_STYLE_BAND2VERT,
    TBL_STYLE_BAND1HORZ,
    TBL_STYLE_BAND2HORZ,
    TBL_STYLE_NECELL,
    TBL_STYLE_NWCELL,
    TBL_STYLE_SECELL,
    TBL_STYLE_SWCELL
};

class WRITERFILTER_DLLPRIVATE TblStylePrHandler : public LoggedProperties
{
private:
    DomainMapper &              m_rDMapper;
    TablePropertiesHandler *    m_pTablePropsHandler;

    TblStyleType                m_nType;
    PropertyMapPtr              m_pProperties;

    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

public:
    TblStylePrHandler( DomainMapper & rDMapper );
    virtual ~TblStylePrHandler( );

    inline PropertyMapPtr       getProperties() { return m_pProperties; };
    inline TblStyleType         getType() { return m_nType; };

private:

    void resolveSprmProps(Sprm & rSprm);
};

typedef boost::shared_ptr< TblStylePrHandler > TblStylePrHandlerPtr;

}}

#endif
