#include "baside2.hxx"
#include "linenumberwindow.hxx"

#include <svtools/xtextedt.hxx>
#include <svtools/textview.hxx>

LineNumberWindow::LineNumberWindow( Window* pParent, ModulWindow* pModulWin ) :
  Window( pParent, WB_BORDER ),
  pModulWindow(pModulWin),
  nWidth(1)
{
  SetBackground(Wallpaper(GetSettings().GetStyleSettings().GetFieldColor()));
}

LineNumberWindow::~LineNumberWindow() { }

void LineNumberWindow::Paint( const Rectangle& )
{
  ExtTextEngine* txtEngine = pModulWindow->GetEditEngine();
  if(!txtEngine)
    return;

  TextView* txtView = pModulWindow->GetEditView();
  if(!txtView)
    return;

  GetParent()->Resize();

  ulong windowHeight = GetOutputSize().Height();
  ulong startY = txtView->GetStartDocPos().Y();
  ulong nLineHeight = GetTextHeight();

  ulong nStartLine = startY / nLineHeight + 1;
  ulong nEndLine = (startY + windowHeight) / nLineHeight + 1;

  if(txtEngine->GetParagraphCount() + 1 < nEndLine)
    nEndLine = txtEngine->GetParagraphCount() + 1;

  nWidth = String::CreateFromInt64(nEndLine).Len() * 10;

  for(ulong i = nStartLine, y = 0; i < nEndLine; ++i, y += nLineHeight)
    DrawText(Point(0, y), String::CreateFromInt64(i));
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
