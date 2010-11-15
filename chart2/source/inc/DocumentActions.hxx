/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef CHART2_UNDOMANAGER_HXX
#define CHART2_UNDOMANAGER_HXX

#include "MutexContainer.hxx"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/chart2/XDocumentActions.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XUndoManager.hpp>

#include <cppuhelper/compbase3.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>

// for pair
#include <utility>
// for auto_ptr
#include <memory>

#include <boost/shared_ptr.hpp>

class SdrUndoAction;

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

class UndoStepsConfigItem;
class UndoElement;
class UndoStack;
class ChartModelClone;
class ModifyBroadcaster;

typedef ::cppu::WeakComponentImplHelper3<
            ::com::sun::star::util::XModifyBroadcaster,
            ::com::sun::star::chart2::XDocumentActions,
            ::com::sun::star::lang::XUnoTunnel >
    DocumentActions_Base;

} // namespace impl
// ----------------------------------------

/** Manages undo by storing the entire XModel in the undo- and redo-buffers.
    Note, that therefore this should not be used for "big" XModels.

    A prerequisite for this to work is that the XModels added to the undo-
    redo-stacks support the css::util::XCloneable interface, which is
    implemented such that the entire model is cloned.
 */
class DocumentActions :
        public MutexContainer,
        public impl::DocumentActions_Base
{
public:
    explicit DocumentActions( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rModel );
    virtual ~DocumentActions();

    void addShapeUndoAction( SdrUndoAction* pAction );

    // ____ XUnoTunnel ____
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId )
        throw (::com::sun::star::uno::RuntimeException);

    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();
    static DocumentActions* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xObj );

protected:
    // ____ util::XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ chart2::XDocumentActions ____
    virtual void SAL_CALL preAction(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL preActionWithArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL postAction( const ::rtl::OUString& aUndoText ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancelAction(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancelActionWithUndo(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL undo(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL redo(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL undoPossible(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL redoPossible(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getCurrentUndoString(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getCurrentRedoString(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAllUndoStrings(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAllRedoStrings(  ) throw (::com::sun::star::uno::RuntimeException);

private:
    void                                                                impl_fireModifyEvent();
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > impl_getModel() const;

    ::boost::shared_ptr< impl::ChartModelClone >                                    m_pDocumentSnapshot;
    ::rtl::Reference< impl::ModifyBroadcaster >                                     m_pModifyBroadcaster;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel >         m_aModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager >    m_xUndoManager;
};

} //  namespace chart

// CHART2_UNDOMANAGER_HXX
#endif
