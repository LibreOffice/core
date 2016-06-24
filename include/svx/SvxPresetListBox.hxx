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
public:
    SvxPresetListBox(vcl::Window* pParent, WinBits nWinStyle = WB_ITEMBORDER);

    virtual void Resize() override;
    static sal_uInt32 getMaxRowCount();
    static sal_uInt32 getEntryEdgeLength();
    static sal_uInt32 getColumnCount();

    void setColumnCount( sal_uInt32 nPos );
    void FillPresetListBox(const XPropertyList &pList, sal_uInt32 nStartIndex = 1);
    void layoutToGivenHeight(sal_uInt32 nHeight, sal_uInt32 nEntryCount);

};

#endif // INCLUDED_SVX_SVXPRESETLISTBOX_HXX
