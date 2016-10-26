#ifndef INCLUDED_SVX_CHARMAPFAV_HXX
#define INCLUDED_SVX_CHARMAPFAV_HXX

#include <svx/svxdllapi.h>
#include <svx/charmap.hxx>

class SVX_DLLPUBLIC SvxShowFavCharSet : public SvxShowCharSet
{

protected:
  virtual void Paint( vcl::RenderContext& rRenderContext, const Rectangle& ) override;
};
#endif