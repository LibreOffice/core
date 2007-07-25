/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UndoManager.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-25 08:47:48 $
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
#ifndef CHART2_UNDOMANAGER_HXX
#define CHART2_UNDOMANAGER_HXX

#include "ConfigItemListener.hxx"
#include "MutexContainer.hxx"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/chart2/XUndoManager.hpp>
#include <com/sun/star/chart2/XUndoHelper.hpp>

#include <cppuhelper/compbase3.hxx>
#include <rtl/ustring.hxx>

// for pair
#include <utility>
// for auto_ptr
#include <memory>

namespace com { namespace sun { namespace star {
namespace frame {
    class XModel;
}
}}}

namespace chart
{
// ----------------------------------------
namespace impl
{

class  UndoStepsConfigItem;
class UndoElement;
class  UndoStack;
class  ModifyBroadcaster;

typedef ::cppu::WeakComponentImplHelper3<
            ::com::sun::star::util::XModifyBroadcaster,
            ::com::sun::star::chart2::XUndoManager,
            ::com::sun::star::chart2::XUndoHelper >
    UndoManager_Base;

} // namespace impl
// ----------------------------------------

/** Manages undo by storing the entire XModel in the undo- and redo-buffers.
    Note, that therefore this should not be used for "big" XModels.

    A prerequisite for this to work is that the XModels added to the undo-
    redo-stacks support the css::util::XCloneable interface, which is
    implemented such that the entire model is cloned.
 */
class UndoManager :
        public MutexContainer,
        public ConfigItemListener,
        public impl::UndoManager_Base
{
public:
    explicit UndoManager();
    virtual ~UndoManager();

protected:
    // ____ ConfigItemListener ____
    virtual void notify( const ::rtl::OUString & rPropertyName );

    // ____ util::XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ chart2::XUndoManager ____
    virtual void SAL_CALL preAction( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModelBeforeChange )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL preActionWithArguments(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModelBeforeChange,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL postAction( const ::rtl::OUString& aUndoText )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancelAction()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancelActionWithUndo( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModelToRestore )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL undo( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xCurrentModel )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL redo( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xCurrentModel )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL undoPossible()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL redoPossible()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getCurrentUndoString()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getCurrentRedoString()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAllUndoStrings()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAllRedoStrings()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XUndoHelper ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL getModelCloneForUndo(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModelBeforeChange )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL applyModelContent(
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModelToChange,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModelToCopyFrom )
        throw (::com::sun::star::uno::RuntimeException);

private:
    void retrieveConfigUndoSteps();
    void fireModifyEvent();
    void impl_undoRedo(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xCurrentModel,
        impl::UndoStack * pStackToRemoveFrom,
        impl::UndoStack * pStackToAddTo );

    ::std::auto_ptr< impl::UndoStack > m_apUndoStack;
    ::std::auto_ptr< impl::UndoStack > m_apRedoStack;

    impl::UndoElement *  m_pLastRemeberedUndoElement;

    ::std::auto_ptr< impl::UndoStepsConfigItem > m_apUndoStepsConfigItem;
    sal_Int32   m_nMaxNumberOfUndos;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::util::XModifyBroadcaster > m_xModifyBroadcaster;
    // pointer is valid as long as m_xModifyBroadcaster.is()
    impl::ModifyBroadcaster * m_pModifyBroadcaster;
};

} //  namespace chart

// CHART2_UNDOMANAGER_HXX
#endif
