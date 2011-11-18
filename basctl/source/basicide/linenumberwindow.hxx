#ifndef BASICIDE_LINENUMBERWINDOW_HXX
#define BASICIDE_LINENUMBERWINDOW_HXX

#include <vcl/window.hxx>

class ModulWindow;

class LineNumberWindow : public Window
{
private:
    ModulWindow* m_pModulWindow;
    int m_nWidth;
    long m_nCurYOffset;
    int m_nCharWidth;
    int m_nBaseWidth;
    virtual void DataChanged(DataChangedEvent const & rDCEvt);

protected:
    virtual void Paint( const Rectangle& );

public:
    LineNumberWindow( Window* pParent, ModulWindow* pModulWin );
    ~LineNumberWindow();

    void DoScroll( long nHorzScroll, long nVertScroll );

    bool SyncYOffset();
    long& GetCurYOffset();

    int GetWidth();
};

#endif // BASICIDE_LINENUMBERWINDOW_HXX
