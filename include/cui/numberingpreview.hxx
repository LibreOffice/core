#ifndef INCLUDED_CUI_NUMBERINGPREVIEW_HXX
#define INCLUDED_CUI_NUMBERINGPREVIEW_HXX

#include <cui/cuidllapi.h>
#include <editeng/numitem.hxx>
#include <vcl/customweld.hxx>

/// Provides the preview to show how looks bullet or numbering format before the apply.
class CUI_DLLPUBLIC SvxNumberingPreview final : public weld::CustomWidgetController
{
    const SvxNumRule* pActNum;
    vcl::Font aStdFont;
    bool bPosition;
    sal_uInt16 nActLevel;

    virtual void Paint(vcl::RenderContext& rRenderContext,
                       const ::tools::Rectangle& rRect) override;

public:
    SvxNumberingPreview();

    void SetNumRule(const SvxNumRule* pNum)
    {
        pActNum = pNum;
        Invalidate();
    };
    void SetPositionMode() { bPosition = true; }
    void SetLevel(sal_uInt16 nSet) { nActLevel = nSet; }
};

#endif
