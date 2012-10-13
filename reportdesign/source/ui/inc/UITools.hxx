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
#ifndef RPTUI_UITOOLS_HXX
#define RPTUI_UITOOLS_HXX

#include <com/sun/star/report/XGroup.hpp>
#include <com/sun/star/report/XReportControlFormat.hpp>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include "ReportSection.hxx"
#include <rtl/ref.hxx>
#include <vcl/taskpanelist.hxx>
#include <comphelper/stl_types.hxx>
#include <functional>

class SdrPage;
class SdrObject;
class SdrUnoObj;
class SdrView;
class Rectangle;
namespace comphelper
{
    class OPropertyChangeMultiplexer;
    class OPropertyChangeListener;
}
namespace rptui
{
    /** returns the position of the object inside the index container
        @param  _xReportDefinition  the report definition to get the groups
        @param  _xGroup the group to search
        @return returns the position of the group in the list, otherwise -1
    */
    template<typename T> sal_Int32 getPositionInIndexAccess(
                                const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& _xCollection
                                ,const ::com::sun::star::uno::Reference< T >& _xSearch)
    {
        sal_Int32 nCount = _xCollection->getCount();
        sal_Int32 i = (nCount == 0) ? -1 : 0;
        for (;i<nCount ; ++i)
        {
            ::com::sun::star::uno::Reference< T > xObject(_xCollection->getByIndex(i),::com::sun::star::uno::UNO_QUERY);
            if ( xObject == _xSearch )
                break;
        }
        return i;
    }

    /** set the name of the header and footer of the group by the expression appended by the localized name of the section
        @param  _xGroup the group where the header/footer name is set by the expression of the group
    */
    void adjustSectionName(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup >& _xGroup,sal_Int32 _nPos);

    /** add a listener for the properties size, left margin, right margin to the page style
    *
    * \param _xReportDefinition
    * \param _pListener
    * \return
    */
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer> addStyleListener( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _xReportDefinition
                                                                ,::comphelper::OPropertyChangeListener* _pListener);

    /** opens the common character font dialog
    */
    bool    openCharDialog(
                const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportControlFormat>& _xReportControlFormat,
                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>& _xWindow,
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _out_rNewValues
            );

    /** opens the area dialog for shapes
    */
    bool openAreaDialog(
             const ::com::sun::star::uno::Reference< ::com::sun::star::report::XShape >& _xShape
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>& _xWindow
            );

    /** opens the formula dialog
        @param _out_rFormula
                the formula chosen by the user
        @precond
            we're really inspecting a database report (well, a RowSet at least)
        @return
            <TRUE/> if and only if the user successfully chose a clause
    */
    bool openDialogFormula_nothrow( ::rtl::OUString& _in_out_rFormula
                               , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xContext
                               , const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>& _xWindow
                               , const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet >& _xRowSet
                               );

    /** applies the character settings previously obtained via openCharDialog
    */
    void    applyCharacterSettings(
                const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportControlFormat >& _rxReportControlFormat,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _rSettings
            );

    /** notifySystemWindow adds or remove the given window _pToRegister at the Systemwindow found when search _pWindow.
        @param  _pWindow
            The window which is used to search for the SystemWindow.
        @param  _pToRegister
            The window which should be added or removed on the TaskPaneList.
        @param  _rMemFunc
            The member function which should be called at the SystemWindow when found.
            Possible values are:
            ::comphelper::mem_fun(&TaskPaneList::AddWindow)
            ::comphelper::mem_fun(&TaskPaneList::RemoveWindow)
    */
    void notifySystemWindow(Window* _pWindow,Window* _pToRegister, ::comphelper::mem_fun1_t<TaskPaneList,Window*> _rMemFunc);


    const sal_Int16 ISOVER_IGNORE_CUSTOMSHAPES = 1;

    /** checks whether the given rectangle overlapps another OUnoObject object in that view.
    *
    * \param _rRect
    * \param _rPage
    * \param _bAllObjects  if <TRUE/> all objects are taken into account, otherwise only not marked ones
    * \return the object which is overlapped, otherwise <NULL/>
    */
    SdrObject* isOver(const Rectangle& _rRect,SdrPage& _rPage,SdrView& _rView,bool _bAllObjects = false,SdrObject* _pIgnore = NULL, sal_Int16 _nIgnoreType=0);

    SdrObject* isOver(const Rectangle& _rRect,SdrPage& _rPage,SdrView& _rView,bool _bAllObjects, SdrUnoObj* _pIgnoreList[], int _nIgnoreListLength);

    /** checks whether the given OUnoObject object rectangle overlapps another object in that view.
    *
    * \param _pObj
    * \param _rPage
    * \param _rView
    * \param _bAllObjects  if <TRUE/> all objects are taken into account, otherwise only not marked ones
    * \return the object which is overlapped, otherwise <NULL/>. If the given object is not of type OUnoObject <NULL/> will be returned.
    */
    SdrObject* isOver(SdrObject* _pObj,SdrPage& _rPage,SdrView& _rView,bool _bAllObjects = false);

    /** retrieves the names of the parameters of the command which the given RowSet is bound to
    */
    ::com::sun::star::uno::Sequence< ::rtl::OUString >
        getParameterNames( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet );

    /** ensures that no control overlaps the given one.
    *
    * \param pControl           the control which should place in the section without overlapping
    * \param _pReportSection    the section
    * \param _bInsert           sal_True whe the control should be inserted, otherwise not.
    */
    void correctOverlapping(SdrObject* pControl,OReportSection& _aReportSection,bool _bInsert = true);

    /** returns a Rectangle of a given SdrObject
     *
     * \param pControl          the SdrObject
     */

    Rectangle getRectangleFromControl(SdrObject* pControl);

    /** sets the map mode at the window
        @param  _aZoom      the zoom scale
        @param  _rWindow    where to set the map mode
    */
    void setZoomFactor(const Fraction& _aZoom,Window& _rWindow);
}
#endif //RPTUI_UITOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
