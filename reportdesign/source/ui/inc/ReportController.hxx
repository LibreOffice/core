/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef RPTUI_REPORTCONTROLLER_HXX
#define RPTUI_REPORTCONTROLLER_HXX

#include "DesignView.hxx"
#include "ModuleHelper.hxx"
#include "ReportControllerObserver.hxx"
#include "RptDef.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/report/XReportControlModel.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/report/XReportEngine.hpp>
#include <com/sun/star/report/XSection.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModeSelector.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/implementationreference.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertystatecontainer.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase5.hxx>
#include <dbaccess/dbsubcomponentcontroller.hxx>
#include <svl/lstner.hxx>
#include <svtools/transfer.hxx>
#include <svx/svdedtv.hxx>
#include <sfx2/zoomitem.hxx>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <functional>

class TransferableClipboardListener;
class VclWindowEvent;
class SfxUndoManager;
namespace rptui
{
    class OGroupsSortingDialog;
    class OReportModel;
    class OSectionView;
    class OAddFieldWindow;
    class OSectionWindow;

    typedef ::dbaui::DBSubComponentController   OReportController_BASE;
    typedef ::cppu::ImplHelper5 <   ::com::sun::star::container::XContainerListener
                                ,   ::com::sun::star::beans::XPropertyChangeListener
                                ,   ::com::sun::star::view::XSelectionSupplier
                                ,   ::com::sun::star::util::XModeSelector
                                ,   ::com::sun::star::embed::XVisualObject
                                >   OReportController_Listener;

    class OReportController :    public OReportController_BASE
                                ,public OReportController_Listener
                                ,public SfxListener
                                ,public ::comphelper::OPropertyStateContainer
                                ,public ::comphelper::OPropertyArrayUsageHelper < OReportController_BASE >
                                ,public ::boost::noncopyable
    {
    private:
        OModuleClient           m_aModuleClient;
        ::cppu::OInterfaceContainerHelper
                                m_aSelectionListeners;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
                                m_aCollapsedSections;
        TransferableDataHelper  m_aSystemClipboard;     // content of the clipboard
        TransferableClipboardListener*
                                m_pClipbordNotifier;    /// notifier for changes in the clipboard
        OGroupsSortingDialog*   m_pGroupsFloater;

        OXReportControllerObserver* m_pReportControllerObserver;

        ODesignView*  getDesignView() const   { return static_cast< ODesignView* >( getView() ); }

        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >         m_xReportDefinition;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportEngine >             m_xReportEngine;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XComponentLoader >           m_xFrameLoader;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >            m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >                     m_xRowSet;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >    m_xRowSetMediator;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >            m_xFormatter;   // a number formatter working with the report's NumberFormatsSupplier
        mutable ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >          m_xHoldAlive;
        mutable ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xColumns;
        ::com::sun::star::awt::Size                                                             m_aVisualAreaSize;

        ::boost::shared_ptr<rptui::OReportModel>
                                m_aReportModel;
        ::rtl::OUString         m_sName;                /// name for the report definition
        ::rtl::OUString         m_sLastActivePage;      /// last active property browser page
        ::rtl::OUString         m_sMode;                /// the current mode of the controller
        sal_Int32               m_nSplitPos;            /// the position of the splitter
        sal_Int32               m_nPageNum;             /// the page number from the restoreView call
        sal_Int32               m_nSelectionCount;
        ::sal_Int64             m_nAspect;
        sal_Int16               m_nZoomValue;
        SvxZoomType             m_eZoomType;
        sal_Bool                m_bShowRuler;
        sal_Bool                m_bGridVisible;
        sal_Bool                m_bGridUse;
        sal_Bool                m_bShowProperties;
        sal_Bool                m_bGroupFloaterWasVisible;
        sal_Bool                m_bHelplinesMove;
        bool                    m_bChartEnabled;
        bool                    m_bChartEnabledAsked;
        bool                    m_bInGeneratePreview;

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

        /** opens the zoom dialog
        */
        void openZoomDialog();

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

        void executeMethodWithUndo(sal_uInt16 _nUndoStrId,const ::std::mem_fun_t<void,ODesignView>& _pMemfun);
        void alignControlsWithUndo(sal_uInt16 _nUndoStrId,sal_Int32 _nControlModification,bool _bAlignAtSection = false);

        // open the help agent of report designer at start time
        void doOpenHelpAgent();

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > getXFrame();

        /** shrink a section
        @param _nUndoStrId the string id of the string which is shown in undo menu
        @param _nShrinkId  ID of what you would like to shrink.
        */
        void shrinkSectionBottom(::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > _xSection);
        void shrinkSectionTop(::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > _xSection);

    public:
        void shrinkSection(sal_uInt16 _nUndoStrId, ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > _xSection, sal_Int32 _nShrinkId);

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

        /** collapse or expand the currently selected section.
        *
        * \param _bCollapse collapse if sal_True otherwise expand
        */
        void collapseSection(const bool _bCollapse);

        /** fills the member that chart is enabled or not
        *
        */
        void checkChartEnabled();

        /** set the zoom factor at the design view
        */
        void impl_zoom_nothrow();

        virtual void impl_onModifyChanged();

        virtual void onLoadedMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& _xLayoutManager );
        virtual void impl_initialize( );
        bool isUiVisible() const;

        /** creates a new default control for the currently set type when the modifier KEY_MOD1 was pressed
        * \param _aArgs must contain a properyvalue with name "KeyModifier" and value KEY_MOD1 when control should be created.
        */
        void createDefaultControl(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& _aArgs);

        /** fills the state for the feture request.
            @param  _sProperty  the property which should be filled in the value
            @param  _rState     the state to fill
        */
        void impl_fillState_nothrow(const ::rtl::OUString& _sProperty,dbaui::FeatureState& _rState) const;
        void impl_fillCustomShapeState_nothrow(const char* _pCustomShapeType,dbaui::FeatureState& _rState) const;

        /** set the property at all selected controls.
            @return <TRUE/> when the selection is not empty
        */
        bool impl_setPropertyAtControls_throw(const sal_uInt16 _nUndoResId
            ,const ::rtl::OUString& _sProperty
            ,const ::com::sun::star::uno::Any& _aValue
            ,const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _aArgs);

        void OnInvalidateClipboard();
        DECL_LINK( OnClipboardChanged, void* );
        DECL_LINK( OnExecuteReport, void* );
        DECL_LINK( OnOpenHelpAgent, void* );
        short saveModified();
        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures();
        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual dbaui::FeatureState GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        virtual void getPropertyDefaultByHandle( sal_Int32 _nHandle, ::com::sun::star::uno::Any& _rDefault ) const;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue) throw (::com::sun::star::uno::Exception);

    private:
        virtual ~OReportController();

    public:
        OReportController(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & the_context);

        DECL_LINK( EventLstHdl, VclWindowEvent* );
        DECL_LINK( OnCreateHdl, OAddFieldWindow*);

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // SfxListener
        virtual void Notify(SfxBroadcaster & rBc, SfxHint const & rHint);

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

        // XSelectionSupplier
        virtual ::sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& xSelection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

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

        // XTitle
        virtual ::rtl::OUString SAL_CALL getTitle(  ) throw (::com::sun::star::uno::RuntimeException);

        // XModeSelector
        virtual void SAL_CALL setMode( const ::rtl::OUString& aMode ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException) ;
        virtual ::rtl::OUString SAL_CALL getMode(  ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedModes(  ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual ::sal_Bool SAL_CALL supportsMode( const ::rtl::OUString& aMode ) throw (::com::sun::star::uno::RuntimeException) ;

        // XVisualObject
        virtual void SAL_CALL setVisualAreaSize( ::sal_Int64 nAspect, const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Size SAL_CALL getVisualAreaSize( ::sal_Int64 nAspect ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation( ::sal_Int64 nAspect ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMapUnit( ::sal_Int64 nAspect ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);


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

        /** returns the number formatter
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    getReportNumberFormatter() const;

        /** return the SdrModel of the real model
        *
        * \return
        */
        ::boost::shared_ptr<rptui::OReportModel> getSdrModel() const;

        inline ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >  getContext() const { return m_xContext; }
        inline sal_Int16   getZoomValue() const     { return m_nZoomValue; }
        inline void         resetZoomType()         { m_eZoomType = SVX_ZOOM_PERCENT; }

        // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
        {
            return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
        }
        // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        ::boost::shared_ptr<OSectionWindow> getSectionWindow(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) const;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > getColumns() const;
        ::rtl::OUString getColumnLabel_throw(const ::rtl::OUString& i_sColumnName) const;

        SfxUndoManager& getUndoManager() const;
        void            clearUndoManager() const;
        void            addUndoAction( SfxUndoAction* i_pAction );
    };
}
#endif // RPTUI_REPORTCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
