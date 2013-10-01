
#include "recentthumbnailtimer.hxx"
#include <sfx2/recentdocsview.hxx>

void RecentThumbnailTimer::Timeout()
{
    if (pDocsView->maThumbStack.empty())
    {
        Stop();
        return;
    }
    else
    {
        pDocsView->insertItem( pDocsView->maThumbStack.back() )
        pDocsView->maThumbStack.pop_back();

    }
}
