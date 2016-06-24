#ifndef INCLUDED_SVX_SVXPRESETLISTBOX_HXX
#define INCLUDED_SVX_SVXPRESETLISTBOX_HXX

#include <svtools/svtdllapi.h>
#include <svx/XPropertyTable.hxx>
#include <vcl/ctrl.hxx>
#include <svtools/valueset.hxx>
#include <svx/xtable.hxx>

class XPropertyList;

class SVX_DLLPUBLIC SvxPresetListBox : public ValueSet
{
private:
    sal_uInt32 nColCount = 2;
    sal_uInt32 nRowCount = 5;
    Link<SvxPresetListBox*,void> maRenameHdl;
    Link<SvxPresetListBox*,void> maDeleteHdl;

    DECL_LINK_TYPED( OnMenuItemSelected, Menu*, bool );
public:
    SvxPresetListBox(vcl::Window* pParent, WinBits nWinStyle = WB_ITEMBORDER);

    virtual void Resize() override;
    virtual void Command( const CommandEvent& rEvt ) override;
    static sal_uInt32 getMaxRowCount();
    sal_uInt32 getColumnCount() const { return nColCount; }
    sal_uInt32 getRowCount() const { return nRowCount; }

    void setColumnCount( const sal_uInt32 nCount ) { nColCount = nCount; }
    void setRowCount( const sal_uInt32 nRow ) { nRowCount = nRow; }
    void SetRenameHdl( const Link<SvxPresetListBox*,void>& rLink )
    {
        maRenameHdl = rLink;
    }
    void SetDeleteHdl( const Link<SvxPresetListBox*,void>& rLink )
    {
        maDeleteHdl = rLink;
    }
    void FillPresetListBox(XHatchList& pList, sal_uInt32 nStartIndex = 0);
    void DrawLayout();

};

#endif // INCLUDED_SVX_SVXPRESETLISTBOX_HXX
