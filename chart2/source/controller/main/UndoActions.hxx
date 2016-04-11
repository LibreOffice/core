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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_UNDOACTIONS_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_UNDOACTIONS_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XUndoAction.hpp>

#include <rtl/ustring.hxx>
#include <unotools/configitem.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <memory>
#include <deque>
#include <utility>

class SdrUndoAction;

namespace chart
{
class ChartModelClone;

namespace impl
{

typedef ::cppu::BaseMutex                                                           UndoElement_MBase;
typedef ::cppu::WeakComponentImplHelper< css::document::XUndoAction > UndoElement_TBase;

class UndoElement   :public UndoElement_MBase
                    ,public UndoElement_TBase
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
    UndoElement( const OUString & i_actionString,
                 const css::uno::Reference< css::frame::XModel >& i_documentModel,
                 const std::shared_ptr< ChartModelClone >& i_modelClone
               );

    UndoElement(const UndoElement&) = delete;
    const UndoElement& operator=(const UndoElement&) = delete;

    // XUndoAction
    virtual OUString SAL_CALL getTitle() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL undo(  ) throw (css::document::UndoFailedException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL redo(  ) throw (css::document::UndoFailedException, css::uno::RuntimeException, std::exception) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

protected:
    virtual ~UndoElement();

private:
    void    impl_toggleModelState();

private:
    OUString                                      m_sActionString;
    css::uno::Reference< css::frame::XModel >     m_xDocumentModel;
    std::shared_ptr< ChartModelClone >            m_pModelClone;
};

typedef ::cppu::BaseMutex                                                           ShapeUndoElement_MBase;
typedef ::cppu::WeakComponentImplHelper< css::document::XUndoAction > ShapeUndoElement_TBase;
class ShapeUndoElement  :public ShapeUndoElement_MBase
                        ,public ShapeUndoElement_TBase
{
public:
    explicit ShapeUndoElement( SdrUndoAction& i_sdrUndoAction );

    // XUndoAction
    virtual OUString SAL_CALL getTitle() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL undo(  ) throw (css::document::UndoFailedException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL redo(  ) throw (css::document::UndoFailedException, css::uno::RuntimeException, std::exception) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

protected:
    virtual ~ShapeUndoElement();

private:
    SdrUndoAction*  m_pAction;
};

} // namespace impl
} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_UNDOACTIONS_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
