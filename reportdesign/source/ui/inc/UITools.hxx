#ifndef RPTUI_UITOOLS_HXX
#define RPTUI_UITOOLS_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UITools.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:48:54 $
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

#ifndef _COM_SUN_STAR_REPORT_XGROUP_HPP_
#include <com/sun/star/report/XGroup.hpp>
#endif
#include <com/sun/star/report/XReportControlFormat.hpp>
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
        } // for (;i<nCount ; ++i)
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
                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>& _xWindow
            );

    /** opens the common character font dialog
    */
    bool    openCharDialog(
                const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportControlFormat>& _xReportControlFormat,
                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>& _xWindow,
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _out_rNewValues
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

    /** checks whether the given rectangle overlapps another OUnoObject object in that view.
    *
    * \param _rRect
    * \param _rPage
    * \param _bAllObjects  if <TRUE/> all objects are taken into account, otherwise only not marked ones
    * \return the object which is overlapped, otherwise <NULL/>
    */
    SdrObject* isOver(const Rectangle& _rRect,SdrPage& _rPage,SdrView& _rView,bool _bAllObjects = false,SdrObject* _pIgnore = NULL);

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
    * \param _bInsert           TRUE whe the control should be inserted, otherwise not.
    */
    void correctOverlapping(SdrObject* pControl,::boost::shared_ptr<OReportSection> _pReportSection,bool _bInsert = true);
}
#endif //RPTUI_UITOOLS_HXX

