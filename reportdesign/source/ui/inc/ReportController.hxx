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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_REPORTCONTROLLER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_REPORTCONTROLLER_HXX

#include "DesignView.hxx"
#include "ModuleHelper.hxx"
#include "ReportControllerObserver.hxx"
#include "RptDef.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
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

#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertystatecontainer.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase5.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <dbaccess/dbsubcomponentcontroller.hxx>
#include <svl/lstner.hxx>
#include <svtools/transfer.hxx>
#include <svx/svdedtv.hxx>
#include <sfx2/zoomitem.hxx>

#include <functional>
#include <memory>

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
    typedef ::cppu::ImplHelper5 <   css::container::XContainerListener
                                ,   css::beans::XPropertyChangeListener
                                ,   css::view::XSelectionSupplier
                                ,   css::util::XModeSelector
                                ,   css::embed::XVisualObject
                                >   OReportController_Listener;

    class OReportController :    public OReportController_BASE
                                ,public OReportController_Listener
                                ,public SfxListener
                                ,public ::comphelper::OPropertyStateContainer
                                ,public ::comphelper::OPropertyArrayUsageHelper < OReportController_BASE >
    {
    private:
        OModuleClient           m_aModuleClient;
        ::comphelper::OInterfaceContainerHelper2
                                m_aSelectionListeners;
        css::uno::Sequence< css::beans::PropertyValue>
                                m_aCollapsedSections;
        TransferableDataHelper  m_aSystemClipboard;     // content of the clipboard
        rtl::Reference<TransferableClipboardListener>
                                m_pClipboardNotifier;    /// notifier for changes in the clipboard
        VclPtr<OGroupsSortingDialog>   m_pGroupsFloater;

        rtl::Reference<OXReportControllerObserver> m_pReportControllerObserver;

        ODesignView*  getDesignView() const   { return static_cast< ODesignView* >( getView() ); }

        css::uno::Reference< css::report::XReportDefinition >         m_xReportDefinition;
        css::uno::Reference< css::report::XReportEngine >             m_xReportEngine;
        css::uno::Reference< css::frame::XDesktop2 >                  m_xFrameLoader;
        css::uno::Reference< css::sdbc::XRowSet >                     m_xRowSet;
        css::uno::Reference< css::beans::XPropertyChangeListener >    m_xRowSetMediator;
        css::uno::Reference< css::util::XNumberFormatter >            m_xFormatter;   // a number formatter working with the report's NumberFormatsSupplier
        mutable css::uno::Reference< css::lang::XComponent >          m_xHoldAlive;
        mutable css::uno::Reference< css::container::XNameAccess >    m_xColumns;
        css::awt::Size                                                m_aVisualAreaSize;

        std::shared_ptr<rptui::OReportModel>
                                m_aReportModel;
        OUString                m_sName;                /// name for the report definition
        OUString                m_sLastActivePage;      /// last active property browser page
        OUString                m_sMode;                /// the current mode of the controller
        sal_Int32               m_nSplitPos;            /// the position of the splitter
        sal_Int32               m_nPageNum;             /// the page number from the restoreView call
        sal_Int32               m_nSelectionCount;
        ::sal_Int64             m_nAspect;
        sal_Int16               m_nZoomValue;
        SvxZoomType             m_eZoomType;
        bool                    m_bShowRuler;
        bool                    m_bGridVisible;
        bool                    m_bGridUse;
        bool                    m_bShowProperties;
        bool                    m_bGroupFloaterWasVisible;
        bool                    m_bHelplinesMove;
        bool                    m_bChartEnabled;
        bool                    m_bChartEnabledAsked;
        bool                    m_bInGeneratePreview;

        /** creates a formatted field in the given section with the given formula as data field
        *
        * \param _aArgs
        * \param _xSection the section where to create the formatted field
        * \param _sFunction the function which will be set at the data field.
        */
        void createControl(const css::uno::Sequence< css::beans::PropertyValue >& _aArgs,const css::uno::Reference< css::report::XSection>& _xSection,const OUString& _sFunction ,sal_uInt16 _nObjectId = OBJ_DLG_FORMATTEDFIELD);
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
        void modifyGroup(const bool _bAppend, const css::uno::Sequence< css::beans::PropertyValue >& _aArgs);

        /** creates a group section.
        *
        * \param _bUndo true when undo action should be created
        * \param _bHeader true when it is a header otherwise it is a footer
        * \param _aArgs The args which contains a element named PROPERTY_GROUP of type report::XGroup.
        */
        void createGroupSection(const bool _bUndo,const bool _bHeader,const css::uno::Sequence< css::beans::PropertyValue >&_aArgs);

        /** add or remove me as listener at the report definition
        *
        * \param _bAdd
        */
        void listen(const bool _bAdd);

        /** opens the common page dialog
        */
        void openPageDialog(const css::uno::Reference< css::report::XSection>& _xSection);

        /** opens or hides the sorting and grouping dialog
        */
        void openSortingAndGroupingDialog();

        /** opens the zoom dialog
        */
        void openZoomDialog();

        /** returns the position of the group inside the groups collection
        */
        sal_Int32 getGroupPosition(const css::uno::Reference< css::report::XGroup >& _xGroup);

        /** calls propertyChanged when the header or footer is really turned on.
            @param  _rEvent the group
            @param  _bShow  when <TRUE/> the header and footer will be shown otherwise not
        */
        void notifyGroupSections(const css::container::ContainerEvent& _rEvent
                                ,bool _bShow);

        /** change the sections for a group
            @param  _sPropName  the header or footer
            @param  _xGroup the group
            @param  _nGroupPos  the position of the group inside the groups collection or the previous index when it was removed
            @param  _bShow  when <TRUE/> the header and footer will be shown otherwise not
        */
        void groupChange( const css::uno::Reference< css::report::XGroup>& _xGroup
                         ,const OUString& _sPropName
                         ,sal_Int32 _nGroupPos
                         ,bool _bShow);

        void executeMethodWithUndo(sal_uInt16 _nUndoStrId,const ::std::mem_fun_t<void,ODesignView>& _pMemfun);
        void alignControlsWithUndo(sal_uInt16 _nUndoStrId,sal_Int32 _nControlModification,bool _bAlignAtSection = false);

        // open the help agent of report designer at start time
        void doOpenHelpAgent();

        css::uno::Reference< css::frame::XFrame > getXFrame();

        /** shrink a section
        @param _nUndoStrId the string id of the string which is shown in undo menu
        @param _nShrinkId  ID of what you would like to shrink.
        */
        static void shrinkSectionBottom(const css::uno::Reference< css::report::XSection >& _xSection);
        static void shrinkSectionTop(const css::uno::Reference< css::report::XSection >& _xSection);

    public:
        void shrinkSection(sal_uInt16 _nUndoStrId, const css::uno::Reference< css::report::XSection >& _xSection, sal_Int32 _nShrinkId);

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
        void createNewFunction(const css::uno::Any& _aValue);

        /** inserts a label - field pair into the current selected section
        *
        * \param aArgs
        */
        void addPairControls(const css::uno::Sequence< css::beans::PropertyValue >& aArgs);

        /** inserts a label - field combination to show the page number and/or page count
        *
        * \param _aArgs
        */
        void createPageNumber(const css::uno::Sequence< css::beans::PropertyValue >& _aArgs);

        /** creates a formatted filed with TODAY() function and if set also an NOW() function
        *
        * \param _aArgs
        */
        void createDateTime(const css::uno::Sequence< css::beans::PropertyValue >& _aArgs);

        /** gets the current section (SdrView)
        *
        * \return the currently selected section or <NULL/> if no one is selected
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

        virtual void impl_onModifyChanged() override;

        virtual void onLoadedMenu( const css::uno::Reference< css::frame::XLayoutManager >& _xLayoutManager ) override;
        virtual void impl_initialize( ) override;
        bool isUiVisible() const;

        /** creates a new default control for the currently set type when the modifier KEY_MOD1 was pressed
        * \param _aArgs must contain a properyvalue with name "KeyModifier" and value KEY_MOD1 when control should be created.
        */
        void createDefaultControl(const css::uno::Sequence< css::beans::PropertyValue>& _aArgs);

        /** fills the state for the feature request.
            @param  _sProperty  the property which should be filled in the value
            @param  _rState     the state to fill
        */
        void impl_fillState_nothrow(const OUString& _sProperty,dbaui::FeatureState& _rState) const;
        void impl_fillCustomShapeState_nothrow(const char* _pCustomShapeType,dbaui::FeatureState& _rState) const;

        /** set the property at all selected controls.
            @return <TRUE/> when the selection is not empty
        */
        bool impl_setPropertyAtControls_throw(const sal_uInt16 _nUndoResId
            ,const OUString& _sProperty
            ,const css::uno::Any& _aValue
            ,const css::uno::Sequence< css::beans::PropertyValue >& _aArgs);

        void OnInvalidateClipboard();
        DECL_LINK_TYPED( OnClipboardChanged, TransferableDataHelper*, void );
        DECL_LINK_TYPED( OnExecuteReport, void*, void );
        DECL_LINK_TYPED( OnOpenHelpAgent, void*, void );
        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures() override;
        // state of a feature. 'feature' may be the handle of a css::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual dbaui::FeatureState GetState(sal_uInt16 nId) const override;
        // execute a feature
        virtual void Execute(sal_uInt16 nId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;

        virtual void getPropertyDefaultByHandle( sal_Int32 _nHandle, css::uno::Any& _rDefault ) const override;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const css::uno::Any& rValue) throw (css::uno::Exception, std::exception) override;

    private:
        virtual ~OReportController();

    public:
        explicit OReportController(css::uno::Reference< css::uno::XComponentContext > const & the_context);
        OReportController(const OReportController&) = delete;
        OReportController& operator=(const OReportController&) = delete;

        DECL_LINK_TYPED( EventLstHdl, VclWindowEvent&, void );
        DECL_LINK_TYPED( OnCreateHdl, OAddFieldWindow&, void);

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // SfxListener
        virtual void Notify(SfxBroadcaster & rBc, SfxHint const & rHint) override;

        /** returns <TRUE/> when the command is enabled
            @param  _nCommand   the command id
            @param  _xControlFormat the report control format
        */
        static bool                         isFormatCommandEnabled(sal_uInt16 _nCommand
                                                ,const css::uno::Reference< css::report::XReportControlFormat>& _xControlFormat);

        virtual bool                        Construct(vcl::Window* pParent) override;
        // XEventListener
        virtual void SAL_CALL               disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

        // css::frame::XController
        virtual sal_Bool SAL_CALL           suspend(sal_Bool bSuspend) throw( css::uno::RuntimeException, std::exception ) override;

        // css::lang::XComponent
        virtual void        SAL_CALL disposing() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
        // need by registration
        static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface > SAL_CALL
            create(css::uno::Reference< css::uno::XComponentContext > const & xContext);

        // css::container::XContainerListener
        virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& Event) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& Event) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& Event) throw( css::uno::RuntimeException, std::exception ) override;

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) throw (css::uno::RuntimeException, std::exception) override;

        // XSelectionSupplier
        virtual sal_Bool SAL_CALL select( const css::uno::Any& xSelection ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getSelection(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

        // css::frame::XController
        virtual sal_Bool SAL_CALL attachModel(const css::uno::Reference< css::frame::XModel > & xModel) throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Any SAL_CALL getViewData() throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL restoreViewData(const css::uno::Any& Data) throw( css::uno::RuntimeException, std::exception ) override;

        /** gives access to the report definition
        * \return the report definition object, may be <NULL/>
        */
        const css::uno::Reference< css::report::XReportDefinition>& getReportDefinition() const { return m_xReportDefinition; }

        // css::frame::XController
        virtual css::uno::Reference< css::frame::XModel >  SAL_CALL getModel() throw( css::uno::RuntimeException, std::exception ) override;

        // XTitle
        virtual OUString SAL_CALL getTitle(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XModeSelector
        virtual void SAL_CALL setMode( const OUString& aMode ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override ;
        virtual OUString SAL_CALL getMode(  ) throw (css::uno::RuntimeException, std::exception) override ;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedModes(  ) throw (css::uno::RuntimeException, std::exception) override ;
        virtual sal_Bool SAL_CALL supportsMode( const OUString& aMode ) throw (css::uno::RuntimeException, std::exception) override ;

        // XVisualObject
        virtual void SAL_CALL setVisualAreaSize( ::sal_Int64 nAspect, const css::awt::Size& aSize ) throw (css::lang::IllegalArgumentException, css::embed::WrongStateException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Size SAL_CALL getVisualAreaSize( ::sal_Int64 nAspect ) throw (css::lang::IllegalArgumentException, css::embed::WrongStateException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual css::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation( ::sal_Int64 nAspect ) throw (css::lang::IllegalArgumentException, css::embed::WrongStateException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getMapUnit( ::sal_Int64 nAspect ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;


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
        css::uno::Reference< css::frame::XModel> executeReport();

        /** returns the RowSet which reflects the current settings of the report definition

            The caller is allowed to hold a reference to the RowSet - it is kept alive as long
            as the controller lives, and it's settings will follow the report definition's settings.
        */
        css::uno::Reference< css::sdbc::XRowSet > const & getRowSet();

        /** returns the number formatter
        */
        const css::uno::Reference< css::util::XNumberFormatter >& getReportNumberFormatter() const { return m_xFormatter;}

        /** return the SdrModel of the real model
        *
        * \return
        */
        const std::shared_ptr<rptui::OReportModel>& getSdrModel() const { return m_aReportModel;}

        const css::uno::Reference< css::uno::XComponentContext >&  getContext() const { return m_xContext; }
        inline sal_Int16   getZoomValue() const     { return m_nZoomValue; }
        inline void         resetZoomType()         { m_eZoomType = SvxZoomType::PERCENT; }

        // css::beans::XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override
        {
            return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
        }
        // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        OSectionWindow* getSectionWindow(const css::uno::Reference< css::report::XSection>& _xSection) const;

        css::uno::Reference< css::container::XNameAccess > const & getColumns() const;
        OUString getColumnLabel_throw(const OUString& i_sColumnName) const;

        SfxUndoManager& getUndoManager() const;
        void            clearUndoManager() const;
        void            addUndoAction( SfxUndoAction* i_pAction );
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_REPORTCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
