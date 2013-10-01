
#ifndef __SFX2_RECENTTHUMBNAILTIMER_HXX__
#define __SFX2_RECENTTHUMBNAILTIMER_HXX__

#include <sfx2/recentdocsview.hxx>
#include <vcl/timer.hxx>

class RecentThumbnailTimer : public Timer
{
public:
    RecentThumbnailTimer(RecentDocsView *pDocsView_, long nTimeout_)
        :pDocsView(pDocsView_), nTimeout(nTimeout_)
    {
        SetTimeout( nTimeout );
    }
private:
    RecentDocsView *pDocsView;
    long nTimeout;
    virtual void Timeout();
};

#endif  // __SFX2_RECENTTHUMBNAILTIMER_HXX__
