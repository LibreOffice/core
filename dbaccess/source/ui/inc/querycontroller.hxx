#ifndef DBAUI_QUERYCONTROLLER_HXX
#define DBAUI_QUERYCONTROLLER_HXX

#ifndef DBAUI_GENERICCONTROLLER_HXX
#include "genericcontroller.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef DBAUI_QUERYVIEW_HXX
#include "queryview.hxx"
#endif
#ifndef _UNDO_HXX
#include <svtools/undo.hxx>
#endif
#ifndef _CONNECTIVITY_PARSE_SQLITERATOR_HXX_
#include <connectivity/sqliterator.hxx>
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif
#ifndef _CONNECTIVITY_SQLNODE_HXX
#include <connectivity/sqlnode.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef DBAUI_QUERYDESIGNCONTEXT_HXX
#include "ParseContext.hxx"
#endif


class VCLXWindow;
namespace dbaui
{
    class OQueryView;
    class OQueryContainerWindow;
    class OTableConnectionData;
    class OTableWindowData;
    class OAddTableDlg;
    class OTableFieldDesc;
    class OQueryTableWindow;
    class OQueryController : public OGenericUnoController
    {
        SfxUndoManager  m_aUndoManager;
        ::std::vector< OTableConnectionData*>   m_vTableConnectionData;
        ::std::vector< OTableWindowData*>       m_vTableData;
        ::std::vector<OTableFieldDesc*>         m_vTableFieldDesc;

        OQueryParseContext*                     m_pParseContext;
        ::connectivity::OSQLParser*             m_pSqlParser;   // to parse sql statements
        ::connectivity::OSQLParseTreeIterator*  m_pSqlIterator; // to iterate through them
        ::std::vector<sal_uInt32>               m_vColumnWidth;
        Fraction                                m_aZoom;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >    m_xComposer;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier

        ::rtl::OUString m_sDataSourceName;  // is set in initialze
        ::rtl::OUString m_sStatement;       // contains the sql statement
        ::rtl::OUString m_sUpdateCatalogName; // catalog for update data
        ::rtl::OUString m_sUpdateSchemaName;  // schema for update data
        ::rtl::OUString m_sUpdateTableName;   // table for update data
        ::rtl::OUString m_sName;            // name of the query

        OQueryContainerWindow*              m_pWindow;          // temporary window
        OAddTableDlg*                       m_pAddTabDlg;       // isa set by the first call of execute, the owner is the design view

        sal_Int32       m_nVisibleRows;     // which rows the selection browse should show
        sal_Int32       m_nSplitPos;        // the position of the splitter
        sal_Bool        m_bEditable;        // is the control readonly or not
        sal_Bool        m_bDesign;          // if design is true then we show the complete design otherwise only the text format
        sal_Bool        m_bDistinct;        // true when you want "select distinct" otherwise false
        sal_Bool        m_bViewAlias;       // show the alias row in the design view
        sal_Bool        m_bViewTable;       // show the table row in the design view
        sal_Bool        m_bViewFunction;    // show the function row in the design view
        sal_Bool        m_bModified;        // is the data modified
        sal_Bool        m_bEsacpeProcessing;// is true when we shouldn't parse the statement
        sal_Bool        m_bOwnConnection;   // is true when we created our own connection

        // creates the querycomposer
        void setQueryComposer();

    protected:
        // all the features which should be handled by this class
        virtual void            AddSupportedFeatures();
        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId);
        // execute a feature
        virtual void            Execute(sal_uInt16 nId);
        virtual ToolBox* CreateToolBox(Window* pParent);

        OQueryView* getQueryView() { return static_cast<OQueryView*>(m_pView); }
        void createNewConnection(sal_Bool _bUI = sal_False);

    public:
        OQueryController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        ~OQueryController();
        // temp
        virtual VCLXWindow* getWindowPeer();
        // removes the connection from the vector and delete it
        void removeConnectionData(const OTableConnectionData* _pData);
        ::std::vector< OTableWindowData*>*      getTableWindowData()        { return &m_vTableData; }
        ::std::vector<OTableFieldDesc*>*        getTableFieldDesc()         { return &m_vTableFieldDesc; }
        ::std::vector< OTableConnectionData*>*  getTableConnectionData()    { return &m_vTableConnectionData;}

        void SaveTabWinsPosSize( OJoinTableView::OTableWindowMap* pTabWinList, long nOffsetX, long nOffsetY );
        void SaveTabWinPosSize(OQueryTableWindow* pTabWin, long nOffsetX, long nOffsetY);
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > getConnection() { return m_xConnection; }

        // should the statement be parsed by our own sql parser
        sal_Bool        isReadOnly()            const { return !m_bEditable; }
        sal_Bool        isModified()            const { return m_bModified; }
        sal_Bool        isEsacpeProcessing()    const { return m_bEsacpeProcessing; }
        sal_Bool        isDesignMode()          const { return m_bDesign; }
        sal_Bool        isDistinct()            const { return m_bDistinct; }

        ::rtl::OUString getStatement()          const { return m_sStatement; }
        sal_Int32       getSplitPos()           const { return m_nSplitPos;}
        sal_Int32       getVisibleRows()        const { return m_nVisibleRows; }

        void            setDistinct(sal_Bool _bDistinct)        { m_bDistinct = _bDistinct;}
        void            setSplitPos(sal_Int32 _nSplitPos)       { m_nSplitPos = _nSplitPos;}
        void            setVisibleRows(sal_Int32 _nVisibleRows) { m_nVisibleRows = _nVisibleRows;}
        void            setModified(sal_Bool _bModified=sal_True);

        //  const ::connectivity::OSQLParseNode* getParseTree() const { return m_aSqlIterator.getParseTree();}
        // need for undo's and redo's
        SfxUndoManager* getUndoMgr();

        ::connectivity::OSQLParser*             getParser()         { return m_pSqlParser;  }
        ::connectivity::OSQLParseTreeIterator&  getParseIterator()  { return *m_pSqlIterator; }
        sal_uInt32 getColWidth(sal_uInt16 _nPos) const
        {
            return m_vColumnWidth.size() < _nPos ? m_vColumnWidth[_nPos] : sal_uInt32(0);
        }

        virtual sal_Bool Construct(Window* pParent);

        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    getNumberFormatter()const   { return m_xFormatter; }
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt);
        // ::com::sun::star::container::XContainerListener
        virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );
        // ::com::sun::star::lang::XComponent
        virtual void        SAL_CALL dispose();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
        // need by registration
        static ::rtl::OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
        // lang::XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        //
        virtual void Load(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxIn);
        virtual void Save(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOut);
    };
}
#endif // DBAUI_QUERYCONTROLLER_HXX

