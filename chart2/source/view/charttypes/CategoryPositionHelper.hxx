#ifndef _CHART2_CATEGORYPOSITIONHELPER_HXX
#define _CHART2_CATEGORYPOSITIONHELPER_HXX

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class CategoryPositionHelper
{
public:
    CategoryPositionHelper( double fSeriesCount, double CategoryWidth = 1.0);
    CategoryPositionHelper();
    virtual ~CategoryPositionHelper();

    double getSlotWidth() const;
    double getSlotPos( double fCategoryX, double fSeriesNumber ) const;

protected:
    double m_fSeriesCount;
    double m_fCategoryWidth;
    //Distance between two neighboring bars in same category, seen relative to width of the bar:
    double m_fInnerDistance; //[-1,1] m_fInnerDistance=1 --> distance == width; m_fInnerDistance=-1-->all rects are painted on the same position
    //Distance between two neighboring bars in different category, seen relative to width of the bar:
    double m_fOuterDistance; //>=0 m_fOuterDistance=1 --> distance == width
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
