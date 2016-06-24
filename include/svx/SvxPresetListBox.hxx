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
    Size rSize;
public:
    SvxPresetListBox(vcl::Window* pParent, WinBits nWinStyle = WB_ITEMBORDER);

    virtual void Resize() override;
    virtual void Command( const CommandEvent& rEvt ) override;
    static sal_uInt32 getMaxRowCount();
    sal_uInt32 getColumnCount() const { return nColCount; }
    sal_uInt32 getRowCount() const { return nRowCount; }

    void setColumnCount( const sal_uInt32 nCount ) { nColCount = nCount; }
    void setRowCount( const sal_uInt32 nRow ) { nRowCount = nRow; }
    //void FillPresetListBox(const XPropertyList &pList, sal_uInt32 nStartIndex = 1);
    void FillPresetListBox(XHatchList& pList, sal_uInt32 nStartIndex = 0);
    void layoutToGivenHeight(sal_uInt32 nHeight, sal_uInt32 nEntryCount);

};

#endif // INCLUDED_SVX_SVXPRESETLISTBOX_HXX
