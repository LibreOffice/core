

#include <vcl/builderfactory.hxx>
#include <svx/favcharmap.hxx>




SvxShowFavCharSet::Paint(vcl::RenderContext& rRenderContext, const Rectangle& )
{
    InitSettings(rRenderContext);
    RecalculateFont(rRenderContext);
    DrawChars_Impl(rRenderContext, FirstInView(), LastInView());
}
VCL_BUILDER_FACTORY(SvxShowFavCharSet)