#ifndef OOX_XLS_HYPERLINKBUFFER_HXX
#define OOX_XLS_HYPERLINKBUFFER_HXX
#include "workbookhelper.hxx"
#include "worksheethelper.hxx"
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <vector>
#include <map>

namespace oox {
namespace xls {
typedef ::std::list< HyperlinkModel > HyperlinkModelList;
typedef ::std::map< sal_Int16, HyperlinkModelList > SheetToHyperlinkModelList;

class HyperlinkBuffer : WorkbookHelper
{
    SheetToHyperlinkModelList mHyperlinkModelLists;
    rtl::OUString getHyperlinkUrl( const HyperlinkModel& rHyperlink ) const;
    void insertHyperlink( const com::sun::star::uno::Reference< com::sun::star::sheet::XSpreadsheet > rCurrSheet, const com::sun::star::table::CellAddress& rAddress, const rtl::OUString& rUrl ) const;
public:
    HyperlinkBuffer( const WorkbookHelper& rHelper );
    void finalizeImport();
    void createHyperlinkMapEntry( sal_Int32 nTab, const HyperlinkModel& rModel );
};
}
}
#endif
