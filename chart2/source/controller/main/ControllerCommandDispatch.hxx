/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ControllerCommandDispatch.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:58:37 $
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
#ifndef CHART2_CONTROLLERCOMMANDDISPATCH_HXX
#define CHART2_CONTROLLERCOMMANDDISPATCH_HXX

#include "CommandDispatch.hxx"

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include <memory>

namespace chart
{

namespace impl
{
struct ModelState;
struct ControllerState;

// #i63017# : need to implement the XSelectionChangeListener in order
// to update the ControllerState when the selection changes.
typedef ::cppu::ImplInheritanceHelper1<
        CommandDispatch,
        ::com::sun::star::view::XSelectionChangeListener >
    ControllerCommandDispatch_Base;
}

/** This class is a CommandDispatch that is responsible for all commands that
    the ChartController supports.

    This class determines which commands are currently available (via the model
    state) and if an available command is called forwards it to the
    ChartController.
 */
class ControllerCommandDispatch : public impl::ControllerCommandDispatch_Base
{
public:
    explicit ControllerCommandDispatch(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XController > & xController );
    virtual ~ControllerCommandDispatch();

    // late initialisation, especially for adding as listener
    virtual void initialize();

protected:
    // ____ XDispatch ____
    virtual void SAL_CALL dispatch(
        const ::com::sun::star::util::URL& URL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ WeakComponentImplHelperBase ____
    /// is called when this is disposed
    virtual void SAL_CALL disposing();

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void fireStatusEvent(
        const ::rtl::OUString & rURL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xSingleListener );

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

private:
    void fireStatusEventForURLImpl(
        const ::rtl::OUString & rURL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xSingleListener );

    bool commandAvailable( const ::rtl::OUString & rCommand );
    void updateCommandAvailability();

    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XController > m_xController;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::view::XSelectionSupplier > m_xSelectionSupplier;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XDispatch > m_xDispatch;

    ::std::auto_ptr< impl::ModelState > m_apModelState;
    ::std::auto_ptr< impl::ControllerState > m_apControllerState;

    mutable ::std::map< ::rtl::OUString, bool > m_aCommandAvailability;
    mutable ::std::map< ::rtl::OUString, ::com::sun::star::uno::Any > m_aCommandArguments;
};

} //  namespace chart

// CHART2_CONTROLLERCOMMANDDISPATCH_HXX
#endif
