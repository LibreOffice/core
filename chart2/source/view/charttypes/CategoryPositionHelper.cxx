#ifndef _CHART2_CATEGORYPOSITIONHELPER_HXX
#include "CategoryPositionHelper.hxx"
#endif

//.............................................................................
namespace chart
{
//.............................................................................
//using namespace ::com::sun::star;
//using namespace ::drafts::com::sun::star::chart2;

CategoryPositionHelper::CategoryPositionHelper( double fSeriesCount, double fCategoryWidth )
    : m_fSeriesCount(fSeriesCount)
    , m_fCategoryWidth(fCategoryWidth)
    , m_fInnerDistance(0.0)
    , m_fOuterDistance(1.0)
{
}

CategoryPositionHelper::CategoryPositionHelper()
    : m_fSeriesCount(1.0)
    , m_fCategoryWidth(1.0)
    , m_fInnerDistance(0.0)
    , m_fOuterDistance(0.0)
{
}

CategoryPositionHelper::~CategoryPositionHelper()
{
}

double CategoryPositionHelper::getSlotWidth() const
{
    double fWidth = m_fCategoryWidth /
                (  m_fSeriesCount
                 + m_fOuterDistance
                 + m_fInnerDistance*( m_fSeriesCount - 1.0) );
    return fWidth;
}

double CategoryPositionHelper::getSlotPos( double fCategoryX, double fSeriesNumber ) const
{
    //the returned position is in the middle of the rect
    //fSeriesNumber 0...n-1
    double fPos = fCategoryX - (m_fCategoryWidth/2.0)
           + (m_fOuterDistance/2.0 + fSeriesNumber*(1.0+m_fInnerDistance)) * getSlotWidth()
           + getSlotWidth()/2.0;

    return fPos;
}

//.............................................................................
} //namespace chart
//.............................................................................
