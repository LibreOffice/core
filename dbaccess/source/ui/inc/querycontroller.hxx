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
#ifndef DBAUI_QUERYVIEW_HXX
#include "queryview.hxx"
#endif
#ifndef _UNDO_HXX
#include <svtools/undo.hxx>
#endif

class VCLXWindow;
namespace dbaui
{
    class OQueryView;
    class OQueryContainerWindow;
    class OQueryController : public OGenericUnoController
    {
        SfxUndoManager  m_aUndoManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >    m_xComposer;

        ::rtl::OUString m_sDataSourceName;  // is set in initialze
        ::rtl::OUString m_sStatement;       // contains the sql statement
        ::rtl::OUString m_sUpdateCatalogName; // catalog for update data
        ::rtl::OUString m_sUpdateSchemaName;  // schema for update data
        ::rtl::OUString m_sUpdateTableName;   // table for update data
        ::rtl::OUString m_sName;            // name of the query

        OQueryContainerWindow* m_pWindow;   // temporary window

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

        void setModified(sal_Bool _bModified);
        sal_Bool isModified() const { return m_bModified; }
        // need for undo's and redo's
        SfxUndoManager* getUndoMgr();

        virtual sal_Bool Construct(Window* pParent);

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
    };
}
#endif // DBAUI_QUERYCONTROLLER_HXX

