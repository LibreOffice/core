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
#ifndef CHART2_IMPLUNDOMANAGER_HXX
#define CHART2_IMPLUNDOMANAGER_HXX

#include "ConfigItemListener.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XUndoAction.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/ustring.hxx>
#include <unotools/configitem.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <utility>
#include <deque>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

class SdrUndoAction;

namespace com { namespace sun { namespace star {
namespace chart2 {
    class XInternalDataProvider;
}
}}}


namespace chart
{
class ChartModelClone;

namespace impl
{

typedef ::cppu::BaseMutex                                                           UndoElement_MBase;
typedef ::cppu::WeakComponentImplHelper1< ::com::sun::star::document::XUndoAction > UndoElement_TBase;

class UndoElement   :public UndoElement_MBase
                    ,public UndoElement_TBase
                    ,public ::boost::noncopyable
{
public:
    /** creates a new undo action

        @param i_actionString
            is the title of the Undo action
        @param i_documentModel
            is the actual document model which the undo actions operates on
        @param i_modelClone
            is the cloned model from before the changes, which the Undo action represents, have been applied.
            Upon <member>invoking</member>, the clone model is applied to the document model.
    */
    UndoElement( const ::rtl::OUString & i_actionString,
                 const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& i_documentModel,
                 const ::boost::shared_ptr< ChartModelClone >& i_modelClone
               );

    // XUndoAction
    virtual ::rtl::OUString SAL_CALL getTitle() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL undo(  ) throw (::com::sun::star::document::UndoFailedException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL redo(  ) throw (::com::sun::star::document::UndoFailedException, ::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

protected:
    virtual ~UndoElement();

private:
    void    impl_toggleModelState();

private:
    ::rtl::OUString                                                     m_sActionString;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > m_xDocumentModel;
    ::boost::shared_ptr< ChartModelClone >                              m_pModelClone;
};


typedef ::cppu::BaseMutex                                                           ShapeUndoElement_MBase;
typedef ::cppu::WeakComponentImplHelper1< ::com::sun::star::document::XUndoAction > ShapeUndoElement_TBase;
class ShapeUndoElement  :public ShapeUndoElement_MBase
                        ,public ShapeUndoElement_TBase
{
public:
    ShapeUndoElement( SdrUndoAction& i_sdrUndoAction );

    // XUndoAction
    virtual ::rtl::OUString SAL_CALL getTitle() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL undo(  ) throw (::com::sun::star::document::UndoFailedException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL redo(  ) throw (::com::sun::star::document::UndoFailedException, ::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

protected:
    virtual ~ShapeUndoElement();

private:
    SdrUndoAction*  m_pAction;
};

} // namespace impl
} //  namespace chart

// CHART2_IMPLUNDOMANAGER_HXX
#endif
