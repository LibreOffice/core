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
#pragma once

#include <com/sun/star/document/XUndoAction.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <memory>

namespace com::sun::star::frame { class XModel; }

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
    virtual OUString SAL_CALL getTitle() override;
    virtual void SAL_CALL undo(  ) override;
    virtual void SAL_CALL redo(  ) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

protected:
    virtual ~UndoElement() override;

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
    explicit ShapeUndoElement( std::unique_ptr<SdrUndoAction> xSdrUndoAction );

    // XUndoAction
    virtual OUString SAL_CALL getTitle() override;
    virtual void SAL_CALL undo(  ) override;
    virtual void SAL_CALL redo(  ) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

protected:
    virtual ~ShapeUndoElement() override;

private:
    std::unique_ptr<SdrUndoAction> m_xAction;
};

} // namespace impl
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
