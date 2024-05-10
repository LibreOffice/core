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

#include <config_features.h>

#include <fmscriptingenv.hxx>
#include <svx/fmmodel.hxx>

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/script/XScriptListener.hpp>

#include <comphelper/diagnose_ex.hxx>
#include <tools/debug.hxx>
#include <cppuhelper/implbase.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <mutex>
#include <o3tl/sorted_vector.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include <basic/basmgr.hxx>

#include <memory>
#include <string_view>

using std::pair;

namespace svxform
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::script::XEventAttacherManager;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::script::XScriptListener;
    using ::com::sun::star::script::ScriptEvent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::awt::XControl;
    using ::com::sun::star::beans::XPropertySet;

    //= FormScriptListener

    typedef ::cppu::WeakImplHelper <   XScriptListener
                                    >   FormScriptListener_Base;

    /** implements the XScriptListener interface, is used by FormScriptingEnvironment
    */
    class FormScriptListener    :public FormScriptListener_Base
    {
    private:
        std::mutex m_aMutex;
        FormScriptingEnvironment *m_pScriptExecutor;

    public:
        explicit FormScriptListener( FormScriptingEnvironment * pScriptExecutor );

        // XScriptListener
        virtual void SAL_CALL firing( const ScriptEvent& aEvent ) override;
        virtual Any SAL_CALL approveFiring( const ScriptEvent& aEvent ) override;
        // XEventListener
        virtual void SAL_CALL disposing( const EventObject& Source ) override;

        // lifetime control
        void dispose();

    protected:
        virtual ~FormScriptListener() override;

    private:
        /** determines whether calling a given method at a given listener interface can be done asynchronously

            @param _rListenerType
                the name of the UNO type whose method is to be checked
            @param _rMethodName
                the name of the method at the interface determined by _rListenerType

            @return
                <TRUE/> if and only if the method is declared <code>oneway</code>, i.e. can be called asynchronously
        */
        static bool impl_allowAsynchronousCall_nothrow( std::u16string_view _rListenerType, std::u16string_view _rMethodName );

        /** determines whether the instance is already disposed
        */
        bool    impl_isDisposed_nothrow() const { return !m_pScriptExecutor; }

        /** fires the given script event in a thread-safe manner

            This methods calls our script executor's doFireScriptEvent, with previously releasing the given mutex guard,
            but ensuring that our script executor is not deleted between this release and the actual call.

            @param _rGuard
                a clearable guard to our mutex. Must be the only active guard to our mutex.
            @param _rEvent
                the event to fire
            @param _pSynchronousResult
                a place to take a possible result of the script call.

            @precond
                m_pScriptExecutor is not <NULL/>.
        */
        void    impl_doFireScriptEvent_nothrow( std::unique_lock<std::mutex>& _rGuard, const ScriptEvent& _rEvent, Any* _pSynchronousResult );

    private:
        DECL_LINK( OnAsyncScriptEvent, void*, void );
    };

    FormScriptListener::FormScriptListener( FormScriptingEnvironment* pScriptExecutor )
        :m_pScriptExecutor( pScriptExecutor )
    {
    }


    FormScriptListener::~FormScriptListener()
    {
    }


    bool FormScriptListener::impl_allowAsynchronousCall_nothrow( std::u16string_view _rListenerType, std::u16string_view _rMethodName )
    {
        // This used to be implemented as:
        // is (_rListenerType + "::" + _rMethodName) a oneway function?
        // since we got rid of the notion of oneway, this is the list
        // of oneway methods, autogenerated by postprocessing of
        // commitdiff 90eac3e69749a9227c4b6902b1f3cef1e338c6d1
        static const o3tl::sorted_vector<pair<std::u16string_view, std::u16string_view>> delayed_event_listeners{
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.accessibility.XAccessibleComponent",u"grabFocus"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.accessibility.XAccessibleEventBroadcaster",u"addAccessibleEventListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.accessibility.XAccessibleEventBroadcaster",u"removeAccessibleEventListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.accessibility.XAccessibleSelection",u"clearAccessibleSelection"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.accessibility.XAccessibleSelection",u"selectAllAccessibleChildren"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XActionListener",u"actionPerformed"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XActivateListener",u"windowActivated"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XActivateListener",u"windowDeactivated"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XAdjustmentListener",u"adjustmentValueChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XButton",u"addActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XButton",u"removeActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XButton",u"setLabel"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XButton",u"setActionCommand"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XCheckBox",u"addItemListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XCheckBox",u"removeItemListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XCheckBox",u"setState"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XCheckBox",u"setLabel"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XCheckBox",u"enableTriState"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XComboBox",u"addItemListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XComboBox",u"removeItemListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XComboBox",u"addActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XComboBox",u"removeActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XComboBox",u"addItem"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XComboBox",u"addItems"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XComboBox",u"removeItems"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XComboBox",u"setDropDownLineCount"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XControl",u"setContext"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XControl",u"createPeer"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XControl",u"setDesignMode"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XControlContainer",u"setStatusText"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XControlContainer",u"addControl"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XControlContainer",u"removeControl"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XCurrencyField",u"setValue"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XCurrencyField",u"setMin"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XCurrencyField",u"setMax"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XCurrencyField",u"setFirst"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XCurrencyField",u"setLast"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XCurrencyField",u"setSpinSize"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XCurrencyField",u"setDecimalDigits"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XCurrencyField",u"setStrictFormat"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XDateField",u"setDate"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XDateField",u"setMin"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XDateField",u"setMax"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XDateField",u"setFirst"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XDateField",u"setLast"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XDateField",u"setLongFormat"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XDateField",u"setStrictFormat"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XDialog",u"setTitle"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XDisplayConnection",u"addEventHandler"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XDisplayConnection",u"removeEventHandler"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XDisplayConnection",u"addErrorHandler"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XDisplayConnection",u"removeErrorHandler"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XExtendedToolkit",u"addTopWindowListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XExtendedToolkit",u"removeTopWindowListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XExtendedToolkit",u"addKeyHandler"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XExtendedToolkit",u"removeKeyHandler"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XExtendedToolkit",u"addFocusListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XExtendedToolkit",u"removeFocusListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XExtendedToolkit",u"fireFocusGained"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XExtendedToolkit",u"fireFocusLost"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XFileDialog",u"setPath"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XFileDialog",u"setFilters"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XFileDialog",u"setCurrentFilter"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XFixedHyperlink",u"setText"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XFixedHyperlink",u"setURL"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XFixedHyperlink",u"setAlignment"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XFixedHyperlink",u"addActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XFixedHyperlink",u"removeActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XFixedText",u"setText"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XFixedText",u"setAlignment"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XFocusListener",u"focusGained"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XFocusListener",u"focusLost"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"setFont"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"selectFont"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"setTextColor"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"setTextFillColor"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"setLineColor"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"setFillColor"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"setRasterOp"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"setClipRegion"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"intersectClipRegion"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"push"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"pop"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"copy"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"draw"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawPixel"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawLine"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawRect"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawRoundedRect"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawPolyLine"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawPolygon"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawPolyPolygon"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawEllipse"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawArc"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawPie"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawChord"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawGradient"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawText"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XGraphics",u"drawTextArray"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XImageButton",u"addActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XImageButton",u"removeActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XImageButton",u"setActionCommand"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XImageConsumer",u"init"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XImageConsumer",u"setColorModel"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XImageConsumer",u"setPixelsByBytes"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XImageConsumer",u"setPixelsByLongs"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XImageConsumer",u"complete"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XImageProducer",u"addConsumer"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XImageProducer",u"removeConsumer"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XImageProducer",u"startProduction"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XItemEventBroadcaster",u"addItemListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XItemEventBroadcaster",u"removeItemListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XItemListener",u"itemStateChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XKeyListener",u"keyPressed"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XKeyListener",u"keyReleased"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XListBox",u"addItemListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XListBox",u"removeItemListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XListBox",u"addActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XListBox",u"removeActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XListBox",u"addItem"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XListBox",u"addItems"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XListBox",u"removeItems"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XListBox",u"selectItemPos"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XListBox",u"selectItemsPos"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XListBox",u"selectItem"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XListBox",u"setMultipleMode"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XListBox",u"setDropDownLineCount"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XListBox",u"makeVisible"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMenu",u"addMenuListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMenu",u"removeMenuListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMenu",u"insertItem"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMenu",u"removeItem"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMenu",u"enableItem"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMenu",u"setItemText"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMenu",u"setPopupMenu"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMenuListener",u"highlight"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMenuListener",u"select"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMenuListener",u"activate"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMenuListener",u"deactivate"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMessageBox",u"setCaptionText"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMessageBox",u"setMessageText"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMouseListener",u"mousePressed"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMouseListener",u"mouseReleased"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMouseListener",u"mouseEntered"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XMouseListener",u"mouseExited"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XNumericField",u"setValue"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XNumericField",u"setMin"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XNumericField",u"setMax"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XNumericField",u"setFirst"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XNumericField",u"setLast"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XNumericField",u"setSpinSize"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XNumericField",u"setDecimalDigits"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XNumericField",u"setStrictFormat"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XPaintListener",u"windowPaint"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XPatternField",u"setMasks"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XPatternField",u"setString"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XPatternField",u"setStrictFormat"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XPointer",u"setType"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XPopupMenu",u"insertSeparator"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XPopupMenu",u"setDefaultItem"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XPopupMenu",u"checkItem"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XProgressBar",u"setForegroundColor"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XProgressBar",u"setBackgroundColor"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XProgressBar",u"setRange"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XProgressBar",u"setValue"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XProgressMonitor",u"addText"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XProgressMonitor",u"removeText"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XProgressMonitor",u"updateText"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRadioButton",u"addItemListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRadioButton",u"removeItemListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRadioButton",u"setState"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRadioButton",u"setLabel"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRegion",u"clear"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRegion",u"move"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRegion",u"unionRectangle"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRegion",u"intersectRectangle"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRegion",u"excludeRectangle"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRegion",u"xOrRectangle"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRegion",u"unionRegion"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRegion",u"intersectRegion"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRegion",u"excludeRegion"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XRegion",u"xOrRegion"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XScrollBar",u"addAdjustmentListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XScrollBar",u"removeAdjustmentListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XScrollBar",u"setValue"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XScrollBar",u"setValues"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XScrollBar",u"setMaximum"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XScrollBar",u"setLineIncrement"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XScrollBar",u"setBlockIncrement"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XScrollBar",u"setVisibleSize"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XScrollBar",u"setOrientation"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinField",u"addSpinListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinField",u"removeSpinListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinField",u"up"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinField",u"down"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinField",u"first"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinField",u"last"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinField",u"enableRepeat"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinListener",u"up"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinListener",u"down"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinListener",u"first"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinListener",u"last"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinValue",u"addAdjustmentListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinValue",u"removeAdjustmentListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinValue",u"setValue"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinValue",u"setValues"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinValue",u"setMinimum"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinValue",u"setMaximum"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XSpinValue",u"setSpinIncrement"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTabController",u"setModel"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTabController",u"setContainer"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTabController",u"autoTabOrder"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTabController",u"activateTabOrder"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTabController",u"activateFirst"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTabController",u"activateLast"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTabControllerModel",u"setGroupControl"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTabControllerModel",u"setControlModels"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTabControllerModel",u"setGroup"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTextComponent",u"addTextListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTextComponent",u"removeTextListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTextComponent",u"setText"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTextComponent",u"insertText"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTextComponent",u"setSelection"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTextComponent",u"setEditable"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTextComponent",u"setMaxTextLen"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTextEditField",u"setEchoChar"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTextListener",u"textChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTimeField",u"setTime"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTimeField",u"setMin"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTimeField",u"setMax"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTimeField",u"setFirst"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTimeField",u"setLast"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTimeField",u"setStrictFormat"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTopWindow",u"addTopWindowListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTopWindow",u"removeTopWindowListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTopWindow",u"toFront"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTopWindow",u"toBack"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTopWindow",u"setMenuBar"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTopWindowListener",u"windowOpened"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTopWindowListener",u"windowClosing"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTopWindowListener",u"windowClosed"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTopWindowListener",u"windowMinimized"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTopWindowListener",u"windowNormalized"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTopWindowListener",u"windowActivated"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XTopWindowListener",u"windowDeactivated"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XUnoControlContainer",u"setTabControllers"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XUnoControlContainer",u"addTabController"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XUnoControlContainer",u"removeTabController"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XUserInputInterception",u"addKeyHandler"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XUserInputInterception",u"removeKeyHandler"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XUserInputInterception",u"addMouseClickHandler"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XUserInputInterception",u"removeMouseClickHandler"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XVclContainer",u"addVclContainerListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XVclContainer",u"removeVclContainerListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XVclContainerListener",u"windowAdded"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XVclContainerListener",u"windowRemoved"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XVclContainerPeer",u"enableDialogControl"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XVclContainerPeer",u"setTabOrder"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XVclContainerPeer",u"setGroup"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XVclWindowPeer",u"setDesignMode"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XVclWindowPeer",u"enableClipSiblings"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XVclWindowPeer",u"setForeground"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XVclWindowPeer",u"setControlFont"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XView",u"draw"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XView",u"setZoom"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"setPosSize"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"setVisible"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"setEnable"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"setFocus"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"addWindowListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"removeWindowListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"addFocusListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"removeFocusListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"addKeyListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"removeKeyListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"addMouseListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"removeMouseListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"addMouseMotionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"removeMouseMotionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"addPaintListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindow",u"removePaintListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindowListener",u"windowResized"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindowListener",u"windowMoved"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindowListener",u"windowShown"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindowListener",u"windowHidden"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindowListener2",u"windowEnabled"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindowListener2",u"windowDisabled"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindowPeer",u"setPointer"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindowPeer",u"setBackground"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindowPeer",u"invalidate"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.XWindowPeer",u"invalidateRect"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.grid.XGridSelectionListener",u"selectionChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.tab.XTabPageContainer",u"addTabPageContainerListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.tab.XTabPageContainer",u"removeTabPageContainerListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.awt.tab.XTabPageContainerListener",u"tabPageActivated"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.configuration.backend.XBackendChangesNotifier",u"addChangesListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.configuration.backend.XBackendChangesNotifier",u"removeChangesListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.clipboard.XClipboard",u"setContents"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.clipboard.XClipboardListener",u"changedContents"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.clipboard.XClipboardNotifier",u"addClipboardListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.clipboard.XClipboardNotifier",u"removeClipboardListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.clipboard.XClipboardOwner",u"lostOwnership"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XAutoscroll",u"autoscroll"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDragGestureListener",u"dragGestureRecognized"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDragGestureRecognizer",u"addDragGestureListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDragGestureRecognizer",u"removeDragGestureListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDragSource",u"startDrag"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDragSourceContext",u"setCursor"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDragSourceContext",u"setImage"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDragSourceContext",u"transferablesFlavorsChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDragSourceListener",u"dragDropEnd"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDragSourceListener",u"dragEnter"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDragSourceListener",u"dragExit"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDragSourceListener",u"dragOver"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDragSourceListener",u"dropActionChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDropTarget",u"addDropTargetListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDropTarget",u"removeDropTargetListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDropTarget",u"setDefaultActions"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDropTargetDragContext",u"acceptDrag"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDropTargetDragContext",u"rejectDrag"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDropTargetDropContext",u"acceptDrop"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDropTargetDropContext",u"rejectDrop"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDropTargetDropContext",u"dropComplete"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDropTargetListener",u"dragEnter"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDropTargetListener",u"dragExit"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDropTargetListener",u"dragOver"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.datatransfer.dnd.XDropTargetListener",u"dropActionChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.document.XEventBroadcaster",u"addEventListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.document.XEventBroadcaster",u"removeEventListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.document.XEventListener",u"notifyEvent"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.document.XStorageChangeListener",u"notifyStorageChange"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.drawing.XControlShape",u"setControl"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XApproveActionBroadcaster",u"addApproveActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XApproveActionBroadcaster",u"removeApproveActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XBoundControl",u"setLock"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XChangeBroadcaster",u"addChangeListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XChangeBroadcaster",u"removeChangeListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XChangeListener",u"changed"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XConfirmDeleteBroadcaster",u"addConfirmDeleteListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XConfirmDeleteBroadcaster",u"removeConfirmDeleteListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XDatabaseParameterBroadcaster",u"addParameterListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XDatabaseParameterBroadcaster",u"removeParameterListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XDatabaseParameterBroadcaster2",u"addDatabaseParameterListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XDatabaseParameterBroadcaster2",u"removeDatabaseParameterListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XErrorBroadcaster",u"addErrorListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XErrorBroadcaster",u"removeErrorListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XFormController",u"addActivateListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XFormController",u"removeActivateListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XFormControllerListener",u"formActivated"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XFormControllerListener",u"formDeactivated"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XGrid",u"setCurrentColumnPosition"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XGridPeer",u"setColumns"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XLoadListener",u"loaded"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XLoadListener",u"unloading"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XLoadListener",u"unloaded"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XLoadListener",u"reloading"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XLoadListener",u"reloaded"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XLoadable",u"load"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XLoadable",u"unload"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XLoadable",u"reload"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XLoadable",u"addLoadListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XLoadable",u"removeLoadListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XPositioningListener",u"positioned"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XReset",u"reset"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XReset",u"addResetListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XReset",u"removeResetListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XResetListener",u"resetted"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XSubmit",u"submit"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XSubmit",u"addSubmitListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XSubmit",u"removeSubmitListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XUpdateBroadcaster",u"addUpdateListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.form.XUpdateBroadcaster",u"removeUpdateListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XBrowseHistoryRegistry",u"updateViewData"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XBrowseHistoryRegistry",u"createNewEntry"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XConfigManager",u"addPropertyChangeListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XConfigManager",u"removePropertyChangeListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XConfigManager",u"flush"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XDesktop",u"addTerminateListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XDesktop",u"removeTerminateListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XDispatch",u"dispatch"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XDispatch",u"addStatusListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XDispatch",u"removeStatusListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XDocumentTemplates",u"update"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XFrame",u"setCreator"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XFrame",u"setName"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XFrame",u"activate"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XFrame",u"deactivate"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XFrame",u"addFrameActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XFrame",u"removeFrameActionListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XFrameActionListener",u"frameAction"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XFrameLoader",u"load"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XFrameLoader",u"cancel"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XLoadEventListener",u"loadFinished"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XLoadEventListener",u"loadCancelled"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XModel",u"connectController"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XModel",u"disconnectController"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XModel",u"lockControllers"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XModel",u"unlockControllers"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XNotifyingDispatch",u"dispatchWithNotification"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XRecordableDispatch",u"dispatchAndRecord"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XSessionManagerClient",u"addSessionManagerListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XSessionManagerClient",u"removeSessionManagerListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XSessionManagerClient",u"queryInteraction"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XSessionManagerClient",u"interactionDone"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XSessionManagerClient",u"saveDone"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XSessionManagerListener",u"doSave"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XSessionManagerListener",u"approveInteraction"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XSessionManagerListener",u"shutdownCanceled"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XSessionManagerListener2",u"doQuit"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XStatusListener",u"statusChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XTask",u"tileWindows"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XTask",u"arrangeWindowsVertical"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XTask",u"arrangeWindowsHorizontal"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.frame.XWindowArranger",u"arrange"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.inspection.XPropertyControlContext",u"activateNextControl"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.inspection.XPropertyControlObserver",u"focusGained"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.inspection.XPropertyControlObserver",u"valueChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XCloseSessionListener",u"sessionClosed"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XMenuProxy",u"addMenuProxyListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XMenuProxy",u"removeMenuProxyListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XPluginInstance",u"start"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XPluginInstance",u"stop"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XPluginInstance",u"destroy"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XPluginInstance",u"createWindow"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XPluginInstance",u"newStream"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XPluginInstance",u"newURL"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XPluginInstanceNotifySink",u"notifyURL"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XPluginInstancePeer",u"showStatusMessage"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XPluginInstancePeer",u"enableScripting"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XPluginInstancePeer",u"newStream"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XPluginInstancePeer",u"getURL"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XPluginInstancePeer",u"postURL"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.mozilla.XPluginWindowPeer",u"setChildWindow"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.script.vba.XVBACompatibility",u"addVBAScriptListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.script.vba.XVBACompatibility",u"removeVBAScriptListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdb.XDatabaseAccess",u"addDatabaseAccessListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdb.XDatabaseAccess",u"removeDatabaseAccessListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdb.XDatabaseAccessListener",u"connectionChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdb.XDatabaseAccessListener",u"connectionClosing"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdb.XRowSetApproveBroadcaster",u"addRowSetApproveListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdb.XRowSetApproveBroadcaster",u"removeRowSetApproveListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdb.XRowSetChangeListener",u"onRowSetChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdb.XRowSetSupplier",u"setRowSet"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdb.XRowsChangeListener",u"rowsChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdb.XSQLErrorBroadcaster",u"addSQLErrorListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdb.XSQLErrorBroadcaster",u"removeSQLErrorListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdbc.XRowSet",u"addRowSetListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdbc.XRowSet",u"removeRowSetListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdbc.XRowSetListener",u"cursorMoved"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdbc.XRowSetListener",u"rowChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sdbc.XRowSetListener",u"rowSetChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sheet.XCalculatable",u"enableAutomaticCalculation"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sheet.XVolatileResult",u"addResultListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.sheet.XVolatileResult",u"removeResultListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.task.XJobExecutor",u"trigger"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.task.XStatusIndicator",u"start"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.task.XStatusIndicator",u"end"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.task.XStatusIndicator",u"setText"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.task.XStatusIndicator",u"setValue"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.task.XStatusIndicator",u"reset"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.text.XSimpleText",u"insertString"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.text.XTextCursor",u"collapseToStart"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.text.XTextCursor",u"collapseToEnd"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.text.XTextRange",u"setString"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.text.XTextViewCursor",u"setVisible"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.ucb.XCommandProcessor",u"abort"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.ucb.XCommandProcessor2",u"releaseCommandIdentifier"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.ucb.XContent",u"addContentEventListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.ucb.XContent",u"removeContentEventListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.ucb.XContentProviderManager",u"deregisterContentProvider"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.ucb.XContentTransmitter",u"transmit"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.ucb.XPropertySetRegistry",u"removePropertySet"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.ui.XUIConfigurationListener",u"elementInserted"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.ui.XUIConfigurationListener",u"elementRemoved"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.ui.XUIConfigurationListener",u"elementReplaced"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.ui.dialogs.XFilePickerNotifier",u"addFilePickerListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.ui.dialogs.XFilePickerNotifier",u"removeFilePickerListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XBroadcaster",u"lockBroadcasts"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XBroadcaster",u"unlockBroadcasts"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XChangesListener",u"changesOccurred"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XChangesNotifier",u"addChangesListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XChangesNotifier",u"removeChangesListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XCloseBroadcaster",u"addCloseListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XCloseBroadcaster",u"removeCloseListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XFlushable",u"addFlushListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XFlushable",u"removeFlushListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XModeChangeListener",u"modeChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XModifyBroadcaster",u"addModifyListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XModifyBroadcaster",u"removeModifyListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XRefreshable",u"addRefreshListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XRefreshable",u"removeRefreshListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.util.XSearchDescriptor",u"setSearchString"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.view.XPrintJobBroadcaster",u"addPrintJobListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.view.XPrintJobBroadcaster",u"removePrintJobListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.view.XPrintJobListener",u"printJobEvent"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.view.XPrintableBroadcaster",u"addPrintableListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.view.XPrintableBroadcaster",u"removePrintableListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.view.XPrintableListener",u"stateChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.view.XSelectionChangeListener",u"selectionChanged"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.beans.XMultiPropertySet",u"addPropertiesChangeListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.beans.XMultiPropertySet",u"removePropertiesChangeListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.beans.XMultiPropertySet",u"firePropertiesChangeEvent"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.beans.XPropertiesChangeNotifier",u"addPropertiesChangeListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.beans.XPropertiesChangeNotifier",u"removePropertiesChangeListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.container.XContainer",u"addContainerListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.container.XContainer",u"removeContainerListener"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.container.XContainerListener",u"elementInserted"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.container.XContainerListener",u"elementRemoved"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.container.XContainerListener",u"elementReplaced"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.container.XNamed",u"setName"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.io.XDataExporter",u"exportData"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.io.XDataExporter",u"cancel"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.io.XDataImporter",u"importData"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.io.XDataImporter",u"cancel"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.io.XDataTransferEventListener",u"finished"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.io.XDataTransferEventListener",u"cancelled"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.lang.XConnectionPointContainer",u"advise"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.lang.XConnectionPointContainer",u"unadvise"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.script.XAllListener",u"firing"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.uno.XInterface",u"acquire"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.uno.XInterface",u"release"),
            pair<std::u16string_view,std::u16string_view>(u"com.sun.star.uno.XReference",u"dispose")};

        pair<std::u16string_view,std::u16string_view> k(_rListenerType, _rMethodName);
        return delayed_event_listeners.find(k) != delayed_event_listeners.end();
    }


    void FormScriptListener::impl_doFireScriptEvent_nothrow( std::unique_lock<std::mutex>& _rGuard, const ScriptEvent& _rEvent, Any* _pSynchronousResult )
    {
        OSL_PRECOND( m_pScriptExecutor, "FormScriptListener::impl_doFireScriptEvent_nothrow: this will crash!" );

        _rGuard.unlock();
        m_pScriptExecutor->doFireScriptEvent( _rEvent, _pSynchronousResult );
    }


    void SAL_CALL FormScriptListener::firing( const ScriptEvent& _rEvent )
    {
        if ( _rEvent.ScriptType == "VBAInterop" )
           return; // not handled here

        std::unique_lock aGuard( m_aMutex );

        if ( impl_isDisposed_nothrow() )
            return;

        if ( !impl_allowAsynchronousCall_nothrow( _rEvent.ListenerType.getTypeName(), _rEvent.MethodName ) )
        {
            impl_doFireScriptEvent_nothrow( aGuard, _rEvent, nullptr );
            return;
        }

        acquire();
        Application::PostUserEvent( LINK( this, FormScriptListener, OnAsyncScriptEvent ), new ScriptEvent( _rEvent ) );
    }


    Any SAL_CALL FormScriptListener::approveFiring( const ScriptEvent& _rEvent )
    {
        Any aResult;

        std::unique_lock aGuard( m_aMutex );
        if ( !impl_isDisposed_nothrow() )
            impl_doFireScriptEvent_nothrow( aGuard, _rEvent, &aResult );

        return aResult;
    }


    void SAL_CALL FormScriptListener::disposing( const EventObject& /*Source*/ )
    {
        // not interested in
    }


    void FormScriptListener::dispose()
    {
        std::unique_lock aGuard( m_aMutex );
        m_pScriptExecutor = nullptr;
    }

    IMPL_LINK( FormScriptListener, OnAsyncScriptEvent, void*, p, void )
    {
        ScriptEvent* _pEvent = static_cast<ScriptEvent*>(p);
        OSL_PRECOND( _pEvent != nullptr, "FormScriptListener::OnAsyncScriptEvent: invalid event!" );
        if ( !_pEvent )
            return;

        {
            std::unique_lock aGuard( m_aMutex );

            if ( !impl_isDisposed_nothrow() )
                impl_doFireScriptEvent_nothrow( aGuard, *_pEvent, nullptr );
        }

        delete _pEvent;
        // we acquired ourself immediately before posting the event
        release();
    }

    FormScriptingEnvironment::FormScriptingEnvironment( FmFormModel& _rModel )
        :m_rFormModel( _rModel )
        ,m_bDisposed( false )
    {
        m_pScriptListener = new FormScriptListener( this );
        // note that this is a cyclic reference between the FormScriptListener and the FormScriptingEnvironment
        // This cycle is broken up when our instance is disposed.
    }

    void FormScriptingEnvironment::impl_registerOrRevoke_throw( const Reference< XEventAttacherManager >& _rxManager, bool _bRegister )
    {
        std::unique_lock aGuard( m_aMutex );

        if ( !_rxManager.is() )
            throw IllegalArgumentException();
        if ( m_bDisposed )
            throw DisposedException();

        try
        {
            if ( _bRegister )
                _rxManager->addScriptListener( m_pScriptListener );
            else
                _rxManager->removeScriptListener( m_pScriptListener );
        }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }


    void FormScriptingEnvironment::registerEventAttacherManager( const Reference< XEventAttacherManager >& _rxManager )
    {
        impl_registerOrRevoke_throw( _rxManager, true );
    }


    void FormScriptingEnvironment::revokeEventAttacherManager( const Reference< XEventAttacherManager >& _rxManager )
    {
        impl_registerOrRevoke_throw( _rxManager, false );
    }

#if HAVE_FEATURE_SCRIPTING
    namespace
    {
        class NewStyleUNOScript
        {
            SfxObjectShell&         m_rObjectShell;
            const OUString   m_sScriptCode;

        public:
            NewStyleUNOScript( SfxObjectShell& _rObjectShell, OUString _aScriptCode )
                :m_rObjectShell( _rObjectShell )
                ,m_sScriptCode(std::move( _aScriptCode ))
            {
            }

            void invoke( const Sequence< Any >& _rArguments, Any& _rSynchronousResult );
        };


        void NewStyleUNOScript::invoke( const Sequence< Any >& _rArguments, Any& _rSynchronousResult )
        {
            Sequence< sal_Int16 > aOutArgsIndex;
            Sequence< Any > aOutArgs;
            EventObject aEvent;
            Any aCaller;
            if ( _rArguments.hasElements() && ( _rArguments[ 0 ] >>= aEvent ) )
            {
                try
                {
                    Reference< XControl > xControl( aEvent.Source, UNO_QUERY_THROW );
                    Reference< XPropertySet > xProps( xControl->getModel(), UNO_QUERY_THROW );
                    aCaller = xProps->getPropertyValue(u"Name"_ustr);
                }
                catch( Exception& ) {}
            }
            m_rObjectShell.CallXScript( m_sScriptCode, _rArguments, _rSynchronousResult, aOutArgsIndex, aOutArgs, true, aCaller.hasValue() ? &aCaller : nullptr );
        }
    }
#endif

    void FormScriptingEnvironment::doFireScriptEvent( const ScriptEvent& _rEvent, Any* _pSynchronousResult )
    {
#if !HAVE_FEATURE_SCRIPTING
        (void) _rEvent;
        (void) _pSynchronousResult;
        (void) m_rFormModel;
#else
        SolarMutexClearableGuard aSolarGuard;
        std::unique_lock aGuard( m_aMutex );

        if ( m_bDisposed )
            return;

        // SfxObjectShellRef is good here since the model controls the lifetime of the object
        SfxObjectShellRef xObjectShell = m_rFormModel.GetObjectShell();
        if( !xObjectShell.is() )
            return;

        // the script to execute
        std::shared_ptr< NewStyleUNOScript > pScript;

        if ( _rEvent.ScriptType != "StarBasic" )
        {
            pScript = std::make_shared<NewStyleUNOScript>( *xObjectShell, _rEvent.ScriptCode );
        }
        else
        {
            OUString sScriptCode = _rEvent.ScriptCode;
            OUString sMacroLocation;

            // is there a location in the script name ("application" or "document")?
            sal_Int32 nPrefixLen = sScriptCode.indexOf( ':' );
            DBG_ASSERT( 0 <= nPrefixLen, "FormScriptingEnvironment::doFireScriptEvent: Basic script name in old format encountered!" );

            if ( 0 <= nPrefixLen )
            {
                // and it has such a prefix
                sMacroLocation = sScriptCode.copy( 0, nPrefixLen );
                DBG_ASSERT( sMacroLocation == "document"
                        ||  sMacroLocation == "application",
                        "FormScriptingEnvironment::doFireScriptEvent: invalid (unknown) prefix!" );

                // strip the prefix: the SfxObjectShell::CallScript knows nothing about such prefixes
                sScriptCode = sScriptCode.copy( nPrefixLen + 1 );
            }

            if ( sMacroLocation.isEmpty() )
            {
                // legacy format: use the app-wide Basic, if it has a respective method, otherwise fall back to the doc's Basic
                if ( SfxApplication::GetBasicManager()->HasMacro( sScriptCode ) )
                    sMacroLocation = "application";
                else
                    sMacroLocation = "document";
            }

            OUString sScriptURI = "vnd.sun.star.script:" +
                sScriptCode +
                "?language=Basic&location=" +
                sMacroLocation;

            pScript = std::make_shared<NewStyleUNOScript>( *xObjectShell, sScriptURI );
        }

        assert(pScript && "FormScriptingEnvironment::doFireScriptEvent: no script to execute!");

        aGuard.unlock();
        aSolarGuard.clear();

        Any aIgnoreResult;
        pScript->invoke( _rEvent.Arguments, _pSynchronousResult ? *_pSynchronousResult : aIgnoreResult );
        pScript.reset();

        {
            // object shells are not thread safe, so guard the destruction
            SolarMutexGuard aSolarGuarsReset;
            xObjectShell = nullptr;
        }
#endif
    }


    void FormScriptingEnvironment::dispose()
    {
        std::unique_lock aGuard( m_aMutex );
        m_bDisposed = true;
        m_pScriptListener->dispose();
        m_pScriptListener.clear();
    }

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
