#ifndef INCLUDED_CUI_SOURCE_INC_TEXTFLOW_HXX
#define INCLUDED_CUI_SOURCE_INC_TEXTFLOW_HXX

#include <sfx2/tabdlg.hxx>
#include <svx/svdattr.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/layout.hxx>

class SdrView;

class SvxTextChainingPage : public SfxTabPage
{
private:
    static const sal_uInt16     pRanges[];

    VclPtr<VclGrid> mpBoxLinks;
    VclPtr<ListBox> mpLBPrevLink;
    VclPtr<ListBox> mpLBNextLink;

public:
    SvxTextChainingPage( vcl::Window* pWindow, const SfxItemSet& rInAttrs );
    virtual ~SvxTextChainingPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window*, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet * ) override;
};

#endif
