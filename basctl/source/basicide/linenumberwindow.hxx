#ifndef BASICIDE_LINENUMBERWINDOW_HXX
#define BASICIDE_LINENUMBERWINDOW_HXX

#include <vcl/window.hxx>

class ModulWindow;

class LineNumberWindow : public Window
{
private:
  ModulWindow* pModulWindow;
  ulong nWidth;

  virtual void DataChanged(DataChangedEvent const & rDCEvt);

protected:
  virtual void Paint( const Rectangle& );

public:
  LineNumberWindow( Window* pParent, ModulWindow* pModulWin );
  ~LineNumberWindow();

  ulong GetWidth() { return (nWidth < 20 ? 20 : nWidth); }
};

#endif // BASICIDE_LINENUMBERWINDOW_HXX
