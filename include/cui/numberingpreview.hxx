#ifndef INCLUDED_CUI_NUMBERINGPREVIEW_HXX
#define INCLUDED_CUI_NUMBERINGPREVIEW_HXX

#include <cui/cuidllapi.h>
#include <editeng/numitem.hxx>
#include <vcl/customweld.hxx>

class CUI_DLLPUBLIC SvxNumberingPreview : public weld::CustomWidgetController
{
    const SvxNumRule* pActNum;
    vcl::Font aStdFont;
    bool bPosition;
    sal_uInt16 nActLevel;

protected:
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
