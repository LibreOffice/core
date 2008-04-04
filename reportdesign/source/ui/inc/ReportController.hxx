#ifndef RPTUI_REPORTCONTROLLER_HXX
#define RPTUI_REPORTCONTROLLER_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportController.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 15:10:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef rptui_SINGLEDOCCONTROLLER_HXX
#include <dbaccess/singledoccontroller.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XSECTION_HPP_
#include <com/sun/star/report/XSection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTDEFINITION_HPP_
#include <com/sun/star/report/XReportDefinition.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTCONTROLMODEL_HPP_
#include <com/sun/star/report/XReportControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTENGINE_HPP_
#include <com/sun/star/report/XReportEngine.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef _SVDEDTV_HXX
#include <svx/svdedtv.hxx>
#endif
#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#include "ModuleHelper.hxx"
#endif
#ifndef _COMPHELPER_IMPLEMENTATIONREFERENCE_HXX
#include <comphelper/implementationreference.hxx>
#endif
#include "RptDef.hxx"
#include <functional>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/frame/XFrame.hpp>


class TransferableHelper;
class TransferableClipboardListener;
class VclWindowEvent;
namespace rptui
{
    class ODesignView;
    class OGroupsSortingDialog;
    class OPropertyMediator;
    class OReportModel;
    class OSectionView;

    typedef ::dbaui::OSingleDocumentController  OReportController_BASE;
    typedef ::cppu::ImplHelper2 <   ::com::sun::star::container::XContainerListener,
                                    ::com::sun::star::beans::XPropertyChangeListener
                                >   OReportController_Listener;

    class OReportController :    public OReportController_BASE
                                ,public OReportController_Listener
                                ,public SfxListener
    {
    private:
        OModuleClient           m_aModuleClient;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
                                m_aCollapsedSections;
        ODesignView*            m_pMyOwnView;           // we want to avoid casts
        TransferableDataHelper  m_aSystemClipboard;     // content of the clipboard
        TransferableClipboardListener*
                                m_pClipbordNotifier;    /// notifier for changes in the clipboard
        OGroupsSortingDialog*   m_pGroupsFloater;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>          m_xReportDefinition;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportEngine>              m_xReportEngine;
        ::com::sun::star::uno::Reference < ::com::sun::star::frame::XComponentLoader>           m_xFrameLoader;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >            m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >                     m_xRowSet;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >    m_xRowSetMediator;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >            m_xFormatter;   // a number formatter working with the report's NumberFormatsSupplier

        ::boost::shared_ptr<rptui::OReportModel>
                                m_aReportModel;
        ::rtl::OUString         m_sName;                /// name for the report definition
        ::rtl::OUString         m_sLastActivePage;      /// last active property browser page
        sal_Int32               m_nSplitPos;            /// the position of the splitter
        sal_Int32               m_nPageNum;             /// the page number from the restoreView call
        //sal_Int32               m_nExecuteReportEvent;
        sal_Bool                m_bShowRuler;
        sal_Bool                m_bGridVisible;
        sal_Bool                m_bGridUse;
        sal_Bool                m_bShowProperties;
        sal_Bool                m_bGroupFloaterWasVisible;
        sal_Bool                m_bHelplinesMove;
        bool                    m_bChartEnabled;
        bool                    m_bChartEnabledAsked;


        /** creates a formatted field in the given section with the given formula as data field
        *
        * \param _aArgs
        * \param _xSection the section where to create the formatted field
        * \param _sFunction the function which will be set at the data field.
        */
        void createControl(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _aArgs,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection,const ::rtl::OUString& _sFunction ,sal_uInt16 _nObjectId = OBJ_DLG_FORMATTEDFIELD);
        /** switch the report header/footer sectionon off with undo or without depending on the given id.
        *
        * \param _nId   Can either be SID_REPORTHEADER_WITHOUT_UNDO or SID_REPORTFOOTER_WITHOUT_UNDO or SID_REPORTHEADERFOOTER.
        */
        void switchReportSection(const sal_Int16 _nId);

        /** switch the report header/footer sectionon off with undo or without depending on the given id.
        *
        * \param _nId   Can either be SID_PAGEHEADER_WITHOUT_UNDO or SID_PAGEFOOTER_WITHOUT_UNDO or SID_PAGEHEADERFOOTER.
        */
        void switchPageSection(const sal_Int16 _nId);

        /** append a new group or remove it with undo.
        *
        * \param _bAppend
        * \param _aArgs The args which contains a element named PROPERTY_GROUP of type report::XGroup.
        */
        void modifyGroup(const bool _bAppend, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _aArgs);

        /** creates a group section.
        *
        * \param _bUndo true when undo action should be created
        * \param _bHeader true when it is a header otherwise it is a footer
        * \param _aArgs The args which contains a element named PROPERTY_GROUP of type report::XGroup.
        */
        void createGroupSection(const bool _bUndo,const bool _bHeader,const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&_aArgs);

        /** add or remove me as listener at the report definition
        *
        * \param _bAdd
        */
        void listen(const bool _bAdd);

        /** opens the common page dialog
        */
        void openPageDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection);

        /** opens or hides the sorting and grouping dialog
        */
        void openSortingAndGroupingDialog();

        /** returns the position of the group inside the groups collection
        */
        sal_Int32 getGroupPosition(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup >& _xGroup);

        /** calls propertyChanged when the header or footer is really turned on.
            @param  _rEvent the group
            @param  _bShow  when <TRUE/> the header and footer will be shown otherwise not
        */
        void notifyGroupSections(const ::com::sun::star::container::ContainerEvent& _rEvent
                                ,bool _bShow);

        /** change the sections for a group
            @param  _sPropName  the header or footer
            @param  _xGroup the group
            @param  _nGroupPos  the position of the group inside the groups collection or the previous index when it was removed
            @param  _bShow  when <TRUE/> the header and footer will be shown otherwise not
        */
        void groupChange( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup>& _xGroup
                         ,const ::rtl::OUString& _sPropName
                         ,sal_Int32 _nGroupPos
                         ,bool _bShow);

        void executeMethodWithUndo(USHORT _nUndoStrId,const ::std::mem_fun_t<void,ODesignView>& _pMemfun);
        void alignControlsWithUndo(USHORT _nUndoStrId,sal_Int32 _nControlModification,bool _bAlignAtSection = false);

        /** opens the file open dialog to allow the user to select a image which will be
        * bound to a newly created image button.
        */
        void insertGraphic();

        /** resets the floater
        */
        void updateFloater();

        /** creates a new function in the given value context
        *
        * \param _aValue contains a XNameContainer
        */
        void createNewFunction(const ::com::sun::star::uno::Any& _aValue);

        /** inserts a label - field pair into the current selected section
        *
        * \param aArgs
        */
        void addPairControls(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs);

        /** inserts a label - field combination to show the page number and/or page count
        *
        * \param _aArgs
        */
        void createPageNumber(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _aArgs);

        /** creates a formatted filed with TODAY() function and if set also an NOW() function
        *
        * \param _aArgs
        */
        void createDateTime(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _aArgs);

        /** gets the current section (SdrView)
        *
        * \return the currently selected section or <NULL/> if noone is selected
        */
        OSectionView* getCurrentSectionView() const;

        /**change the ZOrder of a current select object.
        *
        * \param _nId The command ID about what to do.
        */
        void changeZOrder(sal_Int32 _nId);

        /** marks the next or previous section, when the first/last section was already selected then the report will be selected.
        *
        * \param _bNext
        */
        void markSection(const bool _bNext);

        /** fills the member that chart is enabled or not
        *
        */
        void checkChartEnabled();

        OReportController(OReportController const&);
        OReportController& operator =(OReportController const&);

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > getXFrame();

        // open the help agent of report designer at start time
        void doOpenHelpAgent();

        /** creates a new default control for the currently set type when the modifier KEY_MOD1 was pressed
        * \param _aArgs must contain a properyvalue with name "KeyModifier" and value KEY_MOD1 when control should be created.
        */
        void createDefaultControl(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& _aArgs);

        DECL_LINK( OnInvalidateClipboard, void* );
        DECL_LINK( OnClipboardChanged, void* );
        DECL_LINK( OnExecuteReport, void* );
        DECL_LINK( OnOpenHelpAgent, void* );
        short saveModified();
        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures();
        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual dbaui::FeatureState GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        virtual void losingConnection( );

        virtual ~OReportController();
    public:
        OReportController(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & the_context);

        DECL_LINK( NotifyUndoActionHdl, SfxUndoAction* );
        DECL_LINK( EventLstHdl, VclWindowEvent* );

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // SfxListener
        virtual void Notify(SfxBroadcaster & rBc, SfxHint const & rHint);

        virtual void setModified(sal_Bool _bModified=sal_True);

        //  const ::connectivity::OSQLParseNode* getParseTree() const { return m_aSqlIterator.getParseTree();}
        // need for undo's and redo's
        SfxUndoManager* getUndoMgr();

        /** returns <TRUE/> when the command is enbaled
            @param  _nCommand   the command id
            @param  _xControlFormat the report control format
        */
        sal_Bool                            isFormatCommandEnabled(sal_uInt16 _nCommand
                                                ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportControlFormat>& _xControlFormat) const;

        virtual sal_Bool                    Construct(Window* pParent);
        // XEventListener
        virtual void SAL_CALL               disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL           suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::lang::XComponent
        virtual void        SAL_CALL disposing();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
        // need by registration
        static ::rtl::OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
            create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);

        // ::com::sun::star::container::XContainerListener
        virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL attachModel(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xModel) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Any SAL_CALL getViewData(void) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL restoreViewData(const ::com::sun::star::uno::Any& Data) throw( ::com::sun::star::uno::RuntimeException );

        /** gives access to the report definition
        * \return the report definition object, may be <NULL/>
        */
        inline ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition> getReportDefinition() const { return m_xReportDefinition; }

        // ::com::sun::star::frame::XController
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  SAL_CALL getModel(void) throw( ::com::sun::star::uno::RuntimeException );

        // XSelectionSupplier
        virtual ::sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& xSelection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw (::com::sun::star::uno::RuntimeException);

        // XTitle
        virtual ::rtl::OUString SAL_CALL getTitle(  ) throw (::com::sun::star::uno::RuntimeException);

        /** returns the current position of the splitter
        *
        * \return
        */
        inline sal_Int32    getSplitPos() const { return m_nSplitPos;}
        inline void         setSplitPos(sal_Int32 _nSplitPos)       { m_nSplitPos = _nSplitPos;}

        /** creates a new report from the report definition.
         *
         * \return The model or <NULL/> if the model could not be created.
         */
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel> executeReport();

        /** returns the RowSet which reflects the current settings of the report definition

            The caller is allowed to hold a reference to the RowSet - it is kept alive as long
            as the controller lives, and it's settings will follow the report definition's settings.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > getRowSet();

        /** hides or display all design floaters
        *
        * \param _bShow If <TRUE/> show floaters otherwise hide them.
        */
        void displayDesignFloater(sal_Bool _bShow);

        /** returns the number formatter
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    getReportNumberFormatter() const;

        /** return the SdrModel of the real model
        *
        * \return
        */
        ::boost::shared_ptr<rptui::OReportModel> getSdrModel();

        inline ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >  getContext() const { return m_xContext; }

    private:
        virtual void onLoadedMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& _xLayoutManager );
        virtual void impl_initialize( );
    };
}
#endif // RPTUI_REPORTCONTROLLER_HXX

