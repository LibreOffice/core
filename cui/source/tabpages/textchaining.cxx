#include <textchaining.hxx>

const sal_uInt16 SvxTextChainingPage::pRanges[] =
{
      SDRATTR_MISC_FIRST
    , SDRATTR_TEXT_HORZADJUST
    , SDRATTR_TEXT_WORDWRAP
    , SDRATTR_TEXT_AUTOGROWSIZE
    , 0
};

SvxTextChainingPage::SvxTextChainingPage(vcl::Window* pWindow, const SfxItemSet& rInAttrs)
    : SfxTabPage(pWindow,"TextChainingPage","cui/ui/textchainingtabpage.ui", &rInAttrs)
{
    SfxItemPool* pPool = rInAttrs.GetPool();
}

SvxTextChainingPage::~SvxTextChainingPage()
{
    disposeOnce();
}

void SvxTextChainingPage::dispose()
{
}

void SvxTextChainingPage::Reset( const SfxItemSet* rAttrs )
{
}

bool SvxTextChainingPage::FillItemSet( SfxItemSet* rAttrs)
{
    return true;
}

VclPtr<SfxTabPage> SvxTextChainingPage::Create( vcl::Window* pWindow,
                                            const SfxItemSet* rAttrs )
{
    return VclPtr<SvxTextChainingPage>::Create( pWindow, *rAttrs );
}

