#include "baside2.hxx"
#include "linenumberwindow.hxx"

#include <svtools/xtextedt.hxx>
#include <svtools/textview.hxx>

LineNumberWindow::LineNumberWindow( Window* pParent, ModulWindow* pModulWin ) :
  Window( pParent, WB_BORDER ),
  pModulWindow(pModulWin),
  nWidth(1),
  nCurYOffset(0)
{
  SetBackground(Wallpaper(GetSettings().GetStyleSettings().GetFieldColor()));
}

LineNumberWindow::~LineNumberWindow() { }

void LineNumberWindow::Paint( const Rectangle& )
{
  if(SyncYOffset())
    return;

  ExtTextEngine* txtEngine = pModulWindow->GetEditEngine();
  if(!txtEngine)
    return;

  TextView* txtView = pModulWindow->GetEditView();
  if(!txtView)
    return;

  GetParent()->Resize();

  ulong windowHeight = GetOutputSize().Height();
  ulong nLineHeight = GetTextHeight();

  ulong startY = txtView->GetStartDocPos().Y();
  ulong nStartLine = startY / nLineHeight + 1;
  ulong nEndLine = (startY + windowHeight) / nLineHeight + 1;

  if(txtEngine->GetParagraphCount() + 1 < nEndLine)
    nEndLine = txtEngine->GetParagraphCount() + 1;

  nWidth = String::CreateFromInt64(nEndLine).Len() * 10;

  sal_Int64 y = (nStartLine - 1) * nLineHeight;
  for(ulong i = nStartLine; i <= nEndLine; ++i, y += nLineHeight)
    DrawText(Point(0, y - nCurYOffset), String::CreateFromInt64(i));
}

void LineNumberWindow::DataChanged(DataChangedEvent const & rDCEvt)
{
  Window::DataChanged(rDCEvt);
  if (rDCEvt.GetType() == DATACHANGED_SETTINGS
      && (rDCEvt.GetFlags() & SETTINGS_STYLE) != 0)
  {
    Color aColor(GetSettings().GetStyleSettings().GetFieldColor());
    if (aColor != rDCEvt.GetOldSettings()->GetStyleSettings().GetFieldColor())
    {
      SetBackground(Wallpaper(aColor));
      Invalidate();
    }
  }
}

void LineNumberWindow::DoScroll(long nHorzScroll, long nVertScroll)
{
  nCurYOffset -= nVertScroll;
  Window::Scroll(nHorzScroll, nVertScroll);
}

long& LineNumberWindow::GetCurYOffset()
{
  return nCurYOffset;
}

bool LineNumberWindow::SyncYOffset()
{
  TextView* pView = pModulWindow->GetEditView();
  if (!pView)
    return false;

  long nViewYOffset = pView->GetStartDocPos().Y();
  if (nCurYOffset == nViewYOffset)
    return false;

  nCurYOffset = nViewYOffset;
  Invalidate();
  return true;
}

ulong LineNumberWindow::GetWidth()
{
  return (nWidth < 20 ? 20 : nWidth);
}
