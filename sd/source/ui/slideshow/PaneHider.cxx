#include "PaneHider.hxx"

#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "slideshow.hxx"
#include "PaneManager.hxx"

namespace sd {

PaneHider::PaneHider (const ViewShell& rViewShell)
    : mrViewShell(rViewShell),
      mbWindowVisibilitySaved(false),
      mbOriginalLeftPaneWindowVisibility(false),
      mbOriginalRightPaneWindowVisibility(false)
{
    // Hide the left and right pane windows when a slideshow exists and is
    // not full screen.
    Slideshow* pSlideShow = mrViewShell.GetSlideShow();
    if (pSlideShow!=NULL && !pSlideShow->isFullScreen())
    {
        PaneManager& rPaneManager (mrViewShell.GetViewShellBase().GetPaneManager());
        mbOriginalLeftPaneWindowVisibility = rPaneManager.RequestWindowVisibilityChange(
            PaneManager::PT_LEFT,
            false,
            PaneManager::CM_SYNCHRONOUS);
        mbOriginalRightPaneWindowVisibility = rPaneManager.RequestWindowVisibilityChange(
            PaneManager::PT_RIGHT,
            false,
            PaneManager::CM_SYNCHRONOUS);

        mbWindowVisibilitySaved = true;
    }
}




PaneHider::~PaneHider (void)
{
    if (mbWindowVisibilitySaved)
    {
        PaneManager& rPaneManager (mrViewShell.GetViewShellBase().GetPaneManager());
        rPaneManager.RequestWindowVisibilityChange(
            PaneManager::PT_LEFT,
            mbOriginalLeftPaneWindowVisibility,
            PaneManager::CM_ASYNCHRONOUS);
        rPaneManager.RequestWindowVisibilityChange(
            PaneManager::PT_RIGHT,
            mbOriginalRightPaneWindowVisibility,
            PaneManager::CM_ASYNCHRONOUS);
    }
}

} // end of namespace sd
