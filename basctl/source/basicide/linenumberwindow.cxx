#include "baside2.hxx"
#include "linenumberwindow.hxx"

#include <svtools/xtextedt.hxx>
#include <svtools/textview.hxx>

LineNumberWindow::LineNumberWindow( Window* pParent, ModulWindow* pModulWin ) :
    Window( pParent, WB_BORDER ),
    m_pModulWindow(pModulWin),
    m_nCurYOffset(0)
{
    SetBackground(Wallpaper(GetSettings().GetStyleSettings().GetFieldColor()));
    m_nBaseWidth = GetTextWidth(String('8'));
    m_nWidth = m_nBaseWidth * 3 + m_nBaseWidth / 2;
}

LineNumberWindow::~LineNumberWindow() { }

void LineNumberWindow::Paint( const Rectangle& )
{
    if(SyncYOffset())
        return;

    ExtTextEngine* txtEngine = m_pModulWindow->GetEditEngine();
    if(!txtEngine)
        return;

    TextView* txtView = m_pModulWindow->GetEditView();
    if(!txtView)
        return;

    GetParent()->Resize();

    int windowHeight = GetOutputSize().Height();
    int nLineHeight = GetTextHeight();

    int startY = txtView->GetStartDocPos().Y();
    int nStartLine = startY / nLineHeight + 1;
    int nEndLine = (startY + windowHeight) / nLineHeight + 1;

    if(txtEngine->GetParagraphCount() + 1 < (unsigned int)nEndLine)
        nEndLine = txtEngine->GetParagraphCount() + 1;

    // FIXME: it would be best if we could get notified of a font change
    // rather than doing that re-calculation at each Paint event
    m_nBaseWidth = GetTextWidth(String('8'));

    // reserve enough for 3 sigit minimum, with a bit to spare for confort
    m_nWidth = m_nBaseWidth * 3 + m_nBaseWidth / 2;
    int i = (nEndLine + 1) / 1000;
    while(i)
    {
        i /= 10;
        m_nWidth += m_nBaseWidth;
    }

    sal_Int64 y = (nStartLine - 1) * nLineHeight;
    for(int i = nStartLine; i <= nEndLine; ++i, y += nLineHeight)
        DrawText(Point(0, y - m_nCurYOffset), String::CreateFromInt32(i));
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
    m_nCurYOffset -= nVertScroll;
    Window::Scroll(nHorzScroll, nVertScroll);
}

long& LineNumberWindow::GetCurYOffset()
{
    return m_nCurYOffset;
}

bool LineNumberWindow::SyncYOffset()
{
    TextView* pView = m_pModulWindow->GetEditView();
    if (!pView)
        return false;

    long nViewYOffset = pView->GetStartDocPos().Y();
    if (m_nCurYOffset == nViewYOffset)
        return false;

    m_nCurYOffset = nViewYOffset;
    Invalidate();
    return true;
}

int LineNumberWindow::GetWidth()
{
    return m_nWidth;
}
